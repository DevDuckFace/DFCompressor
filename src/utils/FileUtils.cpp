/**
 * @file FileUtils.cpp
 * @brief File utility functions implementation
 */

#include "FileUtils.h"

#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

QStringList FileUtils::supportedImageExtensions()
{
    return {"png", "jpg", "jpeg", "webp", "avif", "heic", "heif", 
            "tiff", "tif", "bmp", "jxl", "gif"};
}

QStringList FileUtils::supportedVideoExtensions()
{
    return {"mp4", "mkv", "avi", "mov", "webm", "wmv", "flv", "m4v"};
}

QStringList FileUtils::allSupportedExtensions()
{
    QStringList all = supportedImageExtensions();
    all.append(supportedVideoExtensions());
    return all;
}

QStringList FileUtils::scanDirectory(const QString& path, bool recursive)
{
    QStringList files;
    
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
    if (recursive) {
        flags |= QDirIterator::Subdirectories;
    }
    
    QDirIterator it(path, 
                    QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                    flags);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        if (isSupportedFile(filePath)) {
            files.append(filePath);
        }
    }
    
    return files;
}

bool FileUtils::isSupportedFile(const QString& path)
{
    QFileInfo info(path);
    QString ext = info.suffix().toLower();
    return allSupportedExtensions().contains(ext);
}

bool FileUtils::isImageFile(const QString& path)
{
    QFileInfo info(path);
    QString ext = info.suffix().toLower();
    return supportedImageExtensions().contains(ext);
}

bool FileUtils::isVideoFile(const QString& path)
{
    QFileInfo info(path);
    QString ext = info.suffix().toLower();
    return supportedVideoExtensions().contains(ext);
}

QString FileUtils::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    const qint64 TB = GB * 1024;
    
    if (bytes >= TB) {
        return QString("%1 TB").arg(bytes / (double)TB, 0, 'f', 2);
    } else if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / (double)GB, 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / (double)MB, 0, 'f', 2);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / (double)KB, 0, 'f', 1);
    } else {
        return QString("%1 B").arg(bytes);
    }
}

QString FileUtils::getUniqueFileName(const QString& path)
{
    QFileInfo info(path);
    
    if (!info.exists()) {
        return path;
    }
    
    QString baseName = info.completeBaseName();
    QString suffix = info.suffix();
    QString dir = info.absolutePath();
    
    int counter = 1;
    QString newPath;
    
    do {
        newPath = QString("%1/%2_%3.%4")
            .arg(dir)
            .arg(baseName)
            .arg(counter++)
            .arg(suffix);
    } while (QFileInfo::exists(newPath));
    
    return newPath;
}

bool FileUtils::ensureDirectoryExists(const QString& path)
{
    QDir dir(path);
    
    if (dir.exists()) {
        return true;
    }
    
    return dir.mkpath(".");
}
