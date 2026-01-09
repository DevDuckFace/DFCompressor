/**
 * @file JobQueue.cpp
 * @brief Job queue manager implementation
 */

#include "JobQueue.h"
#include "Settings.h"
#include "ImageProcessor.h"
#include "VideoProcessor.h"
#include "Logger.h"

#include <QRunnable>
#include <QThread>

class JobRunner : public QRunnable
{
public:
    JobRunner(std::shared_ptr<Job> job, 
              std::function<void(const QString&, int)> progressCallback,
              std::function<void(const QString&, bool, const QString&)> finishedCallback)
        : m_job(job)
        , m_progressCallback(progressCallback)
        , m_finishedCallback(finishedCallback)
    {}

    void run() override
    {
        if (!m_job) return;
        
        m_job->setStatus(JobStatus::Processing);
        
        bool success = false;
        QString error;
        
        try {
            if (m_job->type() == JobType::Image) {
                ImageProcessor processor;
                processor.setProgressCallback([this](int progress) {
                    m_progressCallback(m_job->id(), progress);
                });
                success = processor.process(m_job.get());
                if (!success) {
                    error = processor.lastError();
                }
            } else if (m_job->type() == JobType::Video) {
                VideoProcessor processor;
                processor.setProgressCallback([this](int progress) {
                    m_progressCallback(m_job->id(), progress);
                });
                success = processor.process(m_job.get());
                if (!success) {
                    error = processor.lastError();
                }
            } else {
                error = "Unsupported file type";
            }
        } catch (const std::exception& e) {
            error = QString::fromStdString(e.what());
        }
        
        m_finishedCallback(m_job->id(), success, error);
    }

private:
    std::shared_ptr<Job> m_job;
    std::function<void(const QString&, int)> m_progressCallback;
    std::function<void(const QString&, bool, const QString&)> m_finishedCallback;
};

JobQueue::JobQueue(QObject *parent)
    : QObject(parent)
{
    m_threadPool = new QThreadPool(this);
    m_threadPool->setMaxThreadCount(m_maxConcurrentJobs);
}

JobQueue::~JobQueue()
{
    stopAll();
}

void JobQueue::addJob(const QString& filePath, const Settings& settings)
{
    QMutexLocker locker(&m_mutex);
    
    auto job = std::make_shared<Job>(filePath, settings);
    m_jobs.append(job);
    
    emit jobAdded(job->id());
}

void JobQueue::addJobs(const QStringList& filePaths, const Settings& settings)
{
    for (const QString& path : filePaths) {
        addJob(path, settings);
    }
}

void JobQueue::start()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isProcessing) return;
    if (m_jobs.isEmpty()) return;
    
    m_isProcessing = true;
    m_isPaused = false;
    m_currentJobIndex = 0;
    
    // Set thread count from settings
    m_threadPool->setMaxThreadCount(Settings::instance().threadCount());
    
    locker.unlock();
    
    // Start processing
    processNextJob();
    
    Logger::info("Job queue started");
}

void JobQueue::pause()
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = true;
    Logger::info("Job queue paused");
}

void JobQueue::resume()
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = false;
    Logger::info("Job queue resumed");
    
    locker.unlock();
    processNextJob();
}

void JobQueue::stopAll()
{
    QMutexLocker locker(&m_mutex);
    
    m_isProcessing = false;
    m_isPaused = false;
    
    m_threadPool->clear();
    m_threadPool->waitForDone();
    
    // Mark processing jobs as cancelled
    for (auto& job : m_jobs) {
        if (job->status() == JobStatus::Processing) {
            job->setStatus(JobStatus::Cancelled);
        }
    }
    
    Logger::info("Job queue stopped");
}

void JobQueue::cancel(const QString& jobId)
{
    QMutexLocker locker(&m_mutex);
    
    for (auto& job : m_jobs) {
        if (job->id() == jobId) {
            if (job->status() == JobStatus::Pending) {
                job->setStatus(JobStatus::Cancelled);
            }
            break;
        }
    }
}

int JobQueue::totalProgress() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_jobs.isEmpty()) return 0;
    
    int totalProgress = 0;
    int completedCount = 0;
    
    for (const auto& job : m_jobs) {
        if (job->status() == JobStatus::Completed) {
            totalProgress += 100;
            completedCount++;
        } else if (job->status() == JobStatus::Processing) {
            totalProgress += job->progress();
        }
    }
    
    return totalProgress / m_jobs.size();
}

JobStatistics JobQueue::statistics() const
{
    QMutexLocker locker(&m_mutex);
    
    JobStatistics stats;
    stats.total = m_jobs.size();
    
    for (const auto& job : m_jobs) {
        stats.totalInputSize += job->inputSize();
        
        switch (job->status()) {
            case JobStatus::Completed:
                stats.completed++;
                stats.totalOutputSize += job->outputSize();
                stats.totalTimeMs += job->processingTimeMs();
                break;
            case JobStatus::Failed:
                stats.failed++;
                break;
            case JobStatus::Pending:
                stats.pending++;
                break;
            case JobStatus::Processing:
                stats.processing++;
                break;
            default:
                break;
        }
    }
    
    return stats;
}

Job* JobQueue::getJob(const QString& jobId) const
{
    QMutexLocker locker(&m_mutex);
    
    for (const auto& job : m_jobs) {
        if (job->id() == jobId) {
            return job.get();
        }
    }
    return nullptr;
}

QList<Job*> JobQueue::allJobs() const
{
    QMutexLocker locker(&m_mutex);
    
    QList<Job*> result;
    for (const auto& job : m_jobs) {
        result.append(job.get());
    }
    return result;
}

int JobQueue::jobCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_jobs.size();
}

void JobQueue::clear()
{
    QMutexLocker locker(&m_mutex);
    
    stopAll();
    m_jobs.clear();
    m_currentJobIndex = 0;
}

void JobQueue::processNextJob()
{
    if (m_isPaused || !m_isProcessing) return;
    
    QMutexLocker locker(&m_mutex);
    
    // Find next pending job
    std::shared_ptr<Job> nextJob;
    for (auto& job : m_jobs) {
        if (job->status() == JobStatus::Pending) {
            nextJob = job;
            break;
        }
    }
    
    if (!nextJob) {
        // Check if all jobs are done
        bool allDone = true;
        for (const auto& job : m_jobs) {
            if (job->status() == JobStatus::Processing ||
                job->status() == JobStatus::Pending) {
                allDone = false;
                break;
            }
        }
        
        if (allDone) {
            m_isProcessing = false;
            locker.unlock();
            emit allJobsCompleted();
        }
        return;
    }
    
    locker.unlock();
    
    emit jobStarted(nextJob->id());
    
    // Create and start job runner
    auto progressCallback = [this](const QString& id, int progress) {
        QMetaObject::invokeMethod(this, [this, id, progress]() {
            emit jobProgress(id, progress);
            emit progressChanged(totalProgress());
        }, Qt::QueuedConnection);
    };
    
    auto finishedCallback = [this](const QString& id, bool success, const QString& error) {
        QMetaObject::invokeMethod(this, [this, id, success, error]() {
            onJobFinished(id, success, error);
        }, Qt::QueuedConnection);
    };
    
    auto* runner = new JobRunner(nextJob, progressCallback, finishedCallback);
    m_threadPool->start(runner);
}

void JobQueue::onJobFinished(const QString& jobId, bool success, const QString& error)
{
    QMutexLocker locker(&m_mutex);
    
    for (auto& job : m_jobs) {
        if (job->id() == jobId) {
            if (success) {
                job->setStatus(JobStatus::Completed);
                job->setProgress(100);
            } else {
                job->setError(error);
            }
            break;
        }
    }
    
    locker.unlock();
    
    if (success) {
        emit jobCompleted(jobId);
    } else {
        emit jobFailed(jobId, error);
    }
    
    emit progressChanged(totalProgress());
    
    // Process next job
    processNextJob();
}
