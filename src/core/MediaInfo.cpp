/**
 * @file MediaInfo.cpp
 * @brief Media file information implementation
 */

#include "MediaInfo.h"
#include <QFileInfo>
#include <QImageReader>
#include <QProcess>

ImageInfo MediaInfo::getImageInfo(const QString& filePath)
{
    ImageInfo info;
    QFileInfo fileInfo(filePath);
    info.fileSize = fileInfo.size();
    info.format = fileInfo.suffix().toUpper();
    
    QImageReader reader(filePath);
    if (reader.canRead()) {
        QSize size = reader.size();
        info.width = size.width();
        info.height = size.height();
        
        QImage::Format imgFormat = reader.imageFormat();
        info.hasAlpha = (imgFormat == QImage::Format_ARGB32 ||
                         imgFormat == QImage::Format_ARGB32_Premultiplied ||
                         imgFormat == QImage::Format_RGBA8888);
    }
    
    return info;
}

VideoInfo MediaInfo::getVideoInfo(const QString& filePath)
{
    VideoInfo info;
    QFileInfo fileInfo(filePath);
    info.fileSize = fileInfo.size();
    info.container = fileInfo.suffix().toUpper();
    
    // Use ffprobe if available
    QProcess process;
    process.start("ffprobe", {
        "-v", "quiet",
        "-print_format", "json",
        "-show_format",
        "-show_streams",
        filePath
    });
    
    if (process.waitForFinished(5000)) {
        // Parse JSON output (simplified)
        QString output = process.readAllStandardOutput();
        // TODO: Parse JSON to extract video info
    }
    
    return info;
}

bool MediaInfo::isImage(const QString& filePath)
{
    static const QStringList exts = {
        "png", "jpg", "jpeg", "webp", "avif", "heic", 
        "heif", "tiff", "tif", "bmp", "jxl", "gif"
    };
    
    QFileInfo info(filePath);
    return exts.contains(info.suffix().toLower());
}

bool MediaInfo::isVideo(const QString& filePath)
{
    static const QStringList exts = {
        "mp4", "mkv", "avi", "mov", "webm", "wmv", "flv", "m4v"
    };
    
    QFileInfo info(filePath);
    return exts.contains(info.suffix().toLower());
}
