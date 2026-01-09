/**
 * @file JobQueue.h
 * @brief Job queue manager header
 */

#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QThreadPool>
#include <memory>

#include "Job.h"

class Settings;

struct JobStatistics {
    int total = 0;
    int completed = 0;
    int failed = 0;
    int pending = 0;
    int processing = 0;
    qint64 totalInputSize = 0;
    qint64 totalOutputSize = 0;
    qint64 totalTimeMs = 0;
};

class JobQueue : public QObject
{
    Q_OBJECT

public:
    explicit JobQueue(QObject *parent = nullptr);
    ~JobQueue();

    void addJob(const QString& filePath, const Settings& settings);
    void addJobs(const QStringList& filePaths, const Settings& settings);
    
    void start();
    void pause();
    void resume();
    void stopAll();
    void cancel(const QString& jobId);
    
    bool isPaused() const { return m_isPaused; }
    bool isProcessing() const { return m_isProcessing; }
    
    int totalProgress() const;
    JobStatistics statistics() const;
    
    Job* getJob(const QString& jobId) const;
    QList<Job*> allJobs() const;
    int jobCount() const;
    
    void clear();

signals:
    void jobAdded(const QString& jobId);
    void jobStarted(const QString& jobId);
    void jobProgress(const QString& jobId, int progress);
    void jobCompleted(const QString& jobId);
    void jobFailed(const QString& jobId, const QString& error);
    void allJobsCompleted();
    void progressChanged(int totalProgress);

private:
    void processNextJob();
    void onJobFinished(const QString& jobId, bool success, const QString& error);

private:
    QList<std::shared_ptr<Job>> m_jobs;
    QThreadPool* m_threadPool = nullptr;
    mutable QMutex m_mutex;
    
    bool m_isProcessing = false;
    bool m_isPaused = false;
    int m_currentJobIndex = 0;
    int m_maxConcurrentJobs = 4;
};

#endif // JOBQUEUE_H
