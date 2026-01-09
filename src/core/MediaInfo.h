/**
 * @file MediaInfo.h
 * @brief Media file information header
 */

#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <QString>
#include <QSize>

struct ImageInfo {
    int width = 0;
    int height = 0;
    int channels = 0;
    int bitDepth = 0;
    QString format;
    QString colorSpace;
    bool hasAlpha = false;
    bool hasIccProfile = false;
    qint64 fileSize = 0;
};

struct VideoInfo {
    int width = 0;
    int height = 0;
    double fps = 0;
    double duration = 0;  // seconds
    int64_t bitrate = 0;
    QString videoCodec;
    QString audioCodec;
    QString container;
    int audioChannels = 0;
    int audioSampleRate = 0;
    qint64 fileSize = 0;
};

class MediaInfo
{
public:
    static ImageInfo getImageInfo(const QString& filePath);
    static VideoInfo getVideoInfo(const QString& filePath);
    static bool isImage(const QString& filePath);
    static bool isVideo(const QString& filePath);
};

#endif // MEDIAINFO_H
