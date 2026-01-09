/**
 * @file FormatUtils.h
 * @brief Format utility functions header
 */

#ifndef FORMATUTILS_H
#define FORMATUTILS_H

#include <QString>

class Settings;

class FormatUtils
{
public:
    static QString getOutputFormat(const QString& type, 
                                   const QString& inputFormat,
                                   const Settings& settings);
    
    static QString getFileExtension(const QString& format);
    static QString getMimeType(const QString& format);
    static bool isLosslessFormat(const QString& format);
    static QString getFormatDescription(const QString& format);
};

#endif // FORMATUTILS_H
