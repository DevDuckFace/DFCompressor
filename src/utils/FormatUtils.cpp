/**
 * @file FormatUtils.cpp
 * @brief Format utility functions implementation
 */

#include "FormatUtils.h"
#include "Settings.h"

QString FormatUtils::getOutputFormat(const QString& type,
                                     const QString& inputFormat,
                                     const Settings& settings)
{
    if (type == "image") {
        QString format = settings.imageOutputFormat();
        if (format == "keep") {
            return inputFormat;
        }
        return format.toUpper();
    } else if (type == "video") {
        QString format = settings.videoOutputFormat();
        if (format == "keep") {
            return inputFormat;
        }
        return format.toUpper();
    }
    
    return inputFormat;
}

QString FormatUtils::getFileExtension(const QString& format)
{
    QString lower = format.toLower();
    
    // Handle special cases
    if (lower == "jpeg") return "jpg";
    if (lower == "jxl" || lower == "jpeg xl") return "jxl";
    if (lower == "hevc" || lower == "h.265" || lower == "h265") return "mp4";
    if (lower == "h264" || lower == "h.264" || lower == "avc") return "mp4";
    if (lower == "av1") return "mp4";
    
    return lower;
}

QString FormatUtils::getMimeType(const QString& format)
{
    QString lower = format.toLower();
    
    // Images
    if (lower == "png") return "image/png";
    if (lower == "jpg" || lower == "jpeg") return "image/jpeg";
    if (lower == "webp") return "image/webp";
    if (lower == "avif") return "image/avif";
    if (lower == "heic" || lower == "heif") return "image/heif";
    if (lower == "jxl") return "image/jxl";
    if (lower == "gif") return "image/gif";
    if (lower == "tiff" || lower == "tif") return "image/tiff";
    if (lower == "bmp") return "image/bmp";
    
    // Videos
    if (lower == "mp4") return "video/mp4";
    if (lower == "mkv") return "video/x-matroska";
    if (lower == "webm") return "video/webm";
    if (lower == "avi") return "video/x-msvideo";
    if (lower == "mov") return "video/quicktime";
    
    return "application/octet-stream";
}

bool FormatUtils::isLosslessFormat(const QString& format)
{
    QString lower = format.toLower();
    
    // Lossless image formats
    if (lower == "png") return true;
    if (lower == "tiff" || lower == "tif") return true;
    if (lower == "bmp") return true;
    
    // Note: These CAN be lossless but aren't always
    // The actual lossless-ness depends on encoding settings
    // We return false to indicate they need special handling
    if (lower == "webp") return false;  // Can be lossless
    if (lower == "avif") return false;  // Can be lossless
    if (lower == "jxl") return false;   // Can be lossless
    if (lower == "heic" || lower == "heif") return false;
    
    // Lossy formats
    if (lower == "jpg" || lower == "jpeg") return false;
    
    return false;
}

QString FormatUtils::getFormatDescription(const QString& format)
{
    QString lower = format.toLower();
    
    // Images
    if (lower == "png") return "PNG - Portable Network Graphics";
    if (lower == "jpg" || lower == "jpeg") return "JPEG - Joint Photographic Experts Group";
    if (lower == "webp") return "WebP - Google's modern image format";
    if (lower == "avif") return "AVIF - AV1 Image File Format";
    if (lower == "jxl") return "JPEG XL - Next-gen JPEG replacement";
    if (lower == "heic" || lower == "heif") return "HEIF/HEIC - High Efficiency Image Format";
    if (lower == "gif") return "GIF - Graphics Interchange Format";
    if (lower == "tiff" || lower == "tif") return "TIFF - Tagged Image File Format";
    if (lower == "bmp") return "BMP - Bitmap Image";
    
    // Videos
    if (lower == "mp4") return "MP4 - MPEG-4 Part 14";
    if (lower == "mkv") return "MKV - Matroska Video";
    if (lower == "webm") return "WebM - Open web media format";
    if (lower == "avi") return "AVI - Audio Video Interleave";
    if (lower == "mov") return "MOV - Apple QuickTime Movie";
    
    // Codecs
    if (lower == "av1") return "AV1 - AOMedia Video 1 (open, royalty-free)";
    if (lower == "hevc" || lower == "h265" || lower == "h.265") return "H.265/HEVC - High Efficiency Video Coding";
    if (lower == "h264" || lower == "h.264" || lower == "avc") return "H.264/AVC - Advanced Video Coding";
    if (lower == "vp9") return "VP9 - Google's open video codec";
    
    return format.toUpper();
}
