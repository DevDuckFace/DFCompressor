/**
 * @file Job.cpp
 * @brief Job class implementation
 */

#include "Job.h"
#include "Settings.h"

#include <QFileInfo>
#include <QDir>
#include <QUuid>

Job::Job(const QString& inputPath, const Settings& settings)
    : m_inputPath(inputPath)
{
    m_id = generateJobId();
    
    QFileInfo info(inputPath);
    m_inputSize = info.size();
    m_inputFormat = info.suffix().toUpper();
    
    determineJobType();
    generateOutputPath(settings);
}

void Job::setStatus(JobStatus status)
{
    m_status = status;
    
    if (status == JobStatus::Processing && !m_startTime.isValid()) {
        m_startTime = QDateTime::currentDateTime();
    } else if (status == JobStatus::Completed || 
               status == JobStatus::Failed ||
               status == JobStatus::Cancelled) {
        m_endTime = QDateTime::currentDateTime();
    }
}

void Job::setProgress(int progress)
{
    m_progress = qBound(0, progress, 100);
}

void Job::setError(const QString& error)
{
    m_errorMessage = error;
    m_status = JobStatus::Failed;
    m_endTime = QDateTime::currentDateTime();
}

qint64 Job::processingTimeMs() const
{
    if (!m_startTime.isValid()) return 0;
    
    if (m_endTime.isValid()) {
        return m_startTime.msecsTo(m_endTime);
    }
    
    return m_startTime.msecsTo(QDateTime::currentDateTime());
}

void Job::determineJobType()
{
    static const QStringList imageExts = {
        "png", "jpg", "jpeg", "webp", "avif", "heic", "heif", 
        "tiff", "tif", "bmp", "jxl", "gif"
    };
    static const QStringList videoExts = {
        "mp4", "mkv", "avi", "mov", "webm", "wmv", "flv", "m4v"
    };
    
    QString ext = QFileInfo(m_inputPath).suffix().toLower();
    
    if (imageExts.contains(ext)) {
        m_type = JobType::Image;
    } else if (videoExts.contains(ext)) {
        m_type = JobType::Video;
    } else {
        m_type = JobType::Unknown;
    }
}

void Job::generateOutputPath(const Settings& settings)
{
    QFileInfo inputInfo(m_inputPath);
    QString baseName = inputInfo.completeBaseName();
    QString outputDir;
    
    if (settings.overwriteOriginal()) {
        outputDir = inputInfo.absolutePath();
    } else if (!settings.outputFolder().isEmpty()) {
        outputDir = settings.outputFolder();
    } else {
        outputDir = inputInfo.absolutePath();
    }
    
    // Determine output format
    if (m_type == JobType::Image) {
        QString format = settings.imageOutputFormat();
        if (format == "keep") {
            m_outputFormat = m_inputFormat;
        } else {
            m_outputFormat = format.toUpper();
        }
    } else if (m_type == JobType::Video) {
        QString container = settings.videoOutputFormat();
        if (container == "keep") {
            m_outputFormat = m_inputFormat;
        } else {
            m_outputFormat = container.toUpper();
        }
    }
    
    // Build output path
    QString extension = m_outputFormat.toLower();
    
    // Handle special cases
    if (extension == "jpeg") extension = "jpg";
    
    m_outputPath = QDir(outputDir).filePath(
        baseName + "_converted." + extension
    );
    
    // If overwriting and same format, use original path
    if (settings.overwriteOriginal() && 
        m_outputFormat == m_inputFormat) {
        m_outputPath = m_inputPath;
    }
}

QString Job::generateJobId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
