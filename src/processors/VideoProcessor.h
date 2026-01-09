/**
 * @file VideoProcessor.h
 * @brief Video processor header
 */

#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QString>
#include <QProcess>
#include <functional>

class Job;

class VideoProcessor
{
public:
    VideoProcessor();
    ~VideoProcessor();

    bool process(Job* job);
    QString lastError() const { return m_lastError; }

    void setProgressCallback(std::function<void(int)> callback);

private:
    bool checkFFmpeg();
    QStringList buildFFmpegArgs(Job* job);
    QString getVideoEncoder() const;
    QString getAudioEncoder() const;
    void parseFFmpegProgress(const QString& line, double totalDuration);
    void reportProgress(int progress);

private:
    QString m_lastError;
    std::function<void(int)> m_progressCallback;
    QString m_ffmpegPath;
    bool m_hasNvenc = false;
    bool m_hasNvdec = false;
};

#endif // VIDEOPROCESSOR_H
