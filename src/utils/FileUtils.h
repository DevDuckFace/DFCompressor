/**
 * @file FileUtils.h
 * @brief File utility functions header
 */

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>
#include <QStringList>

class FileUtils
{
public:
    static QStringList scanDirectory(const QString& path, bool recursive = true);
    static bool isSupportedFile(const QString& path);
    static bool isImageFile(const QString& path);
    static bool isVideoFile(const QString& path);
    static QString formatFileSize(qint64 bytes);
    static QString getUniqueFileName(const QString& path);
    static bool ensureDirectoryExists(const QString& path);
    
    static QStringList supportedImageExtensions();
    static QStringList supportedVideoExtensions();
    static QStringList allSupportedExtensions();
};

#endif // FILEUTILS_H
