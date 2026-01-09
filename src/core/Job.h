/**
 * @file Job.h
 * @brief Job class for processing tasks
 */

#ifndef JOB_H
#define JOB_H

#include <QString>
#include <QDateTime>
#include <memory>

class Settings;

enum class JobStatus {
    Pending,
    Processing,
    Paused,
    Completed,
    Failed,
    Cancelled
};

enum class JobType {
    Image,
    Video,
    Unknown
};

class Job
{
public:
    explicit Job(const QString& inputPath, const Settings& settings);
    ~Job() = default;

    // Getters
    QString id() const { return m_id; }
    QString inputPath() const { return m_inputPath; }
    QString outputPath() const { return m_outputPath; }
    JobType type() const { return m_type; }
    JobStatus status() const { return m_status; }
    int progress() const { return m_progress; }
    QString errorMessage() const { return m_errorMessage; }
    
    qint64 inputSize() const { return m_inputSize; }
    qint64 outputSize() const { return m_outputSize; }
    
    QDateTime startTime() const { return m_startTime; }
    QDateTime endTime() const { return m_endTime; }
    qint64 processingTimeMs() const;
    
    QString inputFormat() const { return m_inputFormat; }
    QString outputFormat() const { return m_outputFormat; }

    // Setters
    void setOutputPath(const QString& path) { m_outputPath = path; }
    void setStatus(JobStatus status);
    void setProgress(int progress);
    void setError(const QString& error);
    void setOutputSize(qint64 size) { m_outputSize = size; }
    void setOutputFormat(const QString& format) { m_outputFormat = format; }

private:
    void determineJobType();
    void generateOutputPath(const Settings& settings);
    QString generateJobId() const;

private:
    QString m_id;
    QString m_inputPath;
    QString m_outputPath;
    QString m_inputFormat;
    QString m_outputFormat;
    
    JobType m_type = JobType::Unknown;
    JobStatus m_status = JobStatus::Pending;
    int m_progress = 0;
    QString m_errorMessage;
    
    qint64 m_inputSize = 0;
    qint64 m_outputSize = 0;
    
    QDateTime m_startTime;
    QDateTime m_endTime;
};

#endif // JOB_H
