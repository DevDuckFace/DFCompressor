/**
 * @file Settings.cpp
 * @brief Application settings implementation
 */

#include "Settings.h"
#include <QThread>
#include <QStandardPaths>

Settings::Settings()
    : m_settings("DuckForge", "DFCompressor")
{
}

Settings& Settings::instance()
{
    static Settings instance;
    return instance;
}

void Settings::load()
{
    m_settings.sync();
}

void Settings::save()
{
    m_settings.sync();
}

void Settings::resetToDefaults()
{
    m_settings.clear();
    
    // Set defaults
    setOutputFolder("");
    setOverwriteOriginal(false);
    setRecursiveScan(true);
    setThreadCount(QThread::idealThreadCount());
    setTheme("dark");
    setShowNotifications(true);
    setPlaySounds(true);
    
    setImageOutputFormat("png");
    setImageCompressionMode("lossless");
    setImageQuality(95);
    setPreserveMetadata(false);
    setPreserveColorProfile(true);
    setJpegXlEffort(7);
    setAvifSpeed(6);
    setWebpMethod(4);
    
    setVideoOutputFormat("mp4");
    setVideoCodec("av1");
    setVideoCompressionMode("visually_lossless");
    setVideoCrf(18);
    setVideoPreset("medium");
    setPreserveAudio(true);
    setAudioCodec("opus");
    setAudioBitrate(192);
    
    setUseGpu(true);
    setGpuMode("auto");
    setUseNvenc(true);
    setUseNvdec(true);
    setGpuMemoryLimit(4096);
    
    setFfmpegPath("");
    setVipsPath("");
    
    save();
}

QVariant Settings::value(const QString& key, const QVariant& defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void Settings::setValue(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
}

// General settings
QString Settings::outputFolder() const
{
    return m_settings.value("general/outputFolder", "").toString();
}

void Settings::setOutputFolder(const QString& folder)
{
    m_settings.setValue("general/outputFolder", folder);
}

bool Settings::overwriteOriginal() const
{
    return m_settings.value("general/overwriteOriginal", false).toBool();
}

void Settings::setOverwriteOriginal(bool overwrite)
{
    m_settings.setValue("general/overwriteOriginal", overwrite);
}

bool Settings::recursiveScan() const
{
    return m_settings.value("general/recursiveScan", true).toBool();
}

void Settings::setRecursiveScan(bool recursive)
{
    m_settings.setValue("general/recursiveScan", recursive);
}

int Settings::threadCount() const
{
    return m_settings.value("general/threadCount", QThread::idealThreadCount()).toInt();
}

void Settings::setThreadCount(int count)
{
    m_settings.setValue("general/threadCount", count);
}

QString Settings::theme() const
{
    return m_settings.value("general/theme", "dark").toString();
}

void Settings::setTheme(const QString& theme)
{
    m_settings.setValue("general/theme", theme);
}

bool Settings::showNotifications() const
{
    return m_settings.value("general/showNotifications", true).toBool();
}

void Settings::setShowNotifications(bool show)
{
    m_settings.setValue("general/showNotifications", show);
}

bool Settings::playSounds() const
{
    return m_settings.value("general/playSounds", true).toBool();
}

void Settings::setPlaySounds(bool play)
{
    m_settings.setValue("general/playSounds", play);
}

// Image settings
QString Settings::imageOutputFormat() const
{
    return m_settings.value("image/outputFormat", "png").toString();
}

void Settings::setImageOutputFormat(const QString& format)
{
    m_settings.setValue("image/outputFormat", format);
}

QString Settings::imageCompressionMode() const
{
    return m_settings.value("image/compressionMode", "lossless").toString();
}

void Settings::setImageCompressionMode(const QString& mode)
{
    m_settings.setValue("image/compressionMode", mode);
}

int Settings::imageQuality() const
{
    return m_settings.value("image/quality", 95).toInt();
}

void Settings::setImageQuality(int quality)
{
    m_settings.setValue("image/quality", quality);
}

bool Settings::preserveMetadata() const
{
    return m_settings.value("image/preserveMetadata", false).toBool();
}

void Settings::setPreserveMetadata(bool preserve)
{
    m_settings.setValue("image/preserveMetadata", preserve);
}

bool Settings::preserveColorProfile() const
{
    return m_settings.value("image/preserveColorProfile", true).toBool();
}

void Settings::setPreserveColorProfile(bool preserve)
{
    m_settings.setValue("image/preserveColorProfile", preserve);
}

int Settings::jpegXlEffort() const
{
    return m_settings.value("image/jpegXlEffort", 7).toInt();
}

void Settings::setJpegXlEffort(int effort)
{
    m_settings.setValue("image/jpegXlEffort", effort);
}

int Settings::avifSpeed() const
{
    return m_settings.value("image/avifSpeed", 6).toInt();
}

void Settings::setAvifSpeed(int speed)
{
    m_settings.setValue("image/avifSpeed", speed);
}

int Settings::webpMethod() const
{
    return m_settings.value("image/webpMethod", 4).toInt();
}

void Settings::setWebpMethod(int method)
{
    m_settings.setValue("image/webpMethod", method);
}

// Video settings
QString Settings::videoOutputFormat() const
{
    return m_settings.value("video/outputFormat", "mp4").toString();
}

void Settings::setVideoOutputFormat(const QString& format)
{
    m_settings.setValue("video/outputFormat", format);
}

QString Settings::videoCodec() const
{
    return m_settings.value("video/codec", "av1").toString();
}

void Settings::setVideoCodec(const QString& codec)
{
    m_settings.setValue("video/codec", codec);
}

QString Settings::videoCompressionMode() const
{
    return m_settings.value("video/compressionMode", "visually_lossless").toString();
}

void Settings::setVideoCompressionMode(const QString& mode)
{
    m_settings.setValue("video/compressionMode", mode);
}

int Settings::videoCrf() const
{
    return m_settings.value("video/crf", 18).toInt();
}

void Settings::setVideoCrf(int crf)
{
    m_settings.setValue("video/crf", crf);
}

QString Settings::videoPreset() const
{
    return m_settings.value("video/preset", "medium").toString();
}

void Settings::setVideoPreset(const QString& preset)
{
    m_settings.setValue("video/preset", preset);
}

bool Settings::preserveAudio() const
{
    return m_settings.value("video/preserveAudio", true).toBool();
}

void Settings::setPreserveAudio(bool preserve)
{
    m_settings.setValue("video/preserveAudio", preserve);
}

QString Settings::audioCodec() const
{
    return m_settings.value("video/audioCodec", "opus").toString();
}

void Settings::setAudioCodec(const QString& codec)
{
    m_settings.setValue("video/audioCodec", codec);
}

int Settings::audioBitrate() const
{
    return m_settings.value("video/audioBitrate", 192).toInt();
}

void Settings::setAudioBitrate(int bitrate)
{
    m_settings.setValue("video/audioBitrate", bitrate);
}

// GPU settings
bool Settings::useGpu() const
{
    return m_settings.value("gpu/useGpu", true).toBool();
}

void Settings::setUseGpu(bool use)
{
    m_settings.setValue("gpu/useGpu", use);
}

QString Settings::gpuMode() const
{
    return m_settings.value("gpu/mode", "auto").toString();
}

void Settings::setGpuMode(const QString& mode)
{
    m_settings.setValue("gpu/mode", mode);
}

bool Settings::useNvenc() const
{
    return m_settings.value("gpu/useNvenc", true).toBool();
}

void Settings::setUseNvenc(bool use)
{
    m_settings.setValue("gpu/useNvenc", use);
}

bool Settings::useNvdec() const
{
    return m_settings.value("gpu/useNvdec", true).toBool();
}

void Settings::setUseNvdec(bool use)
{
    m_settings.setValue("gpu/useNvdec", use);
}

int Settings::gpuMemoryLimit() const
{
    return m_settings.value("gpu/memoryLimit", 4096).toInt();
}

void Settings::setGpuMemoryLimit(int limitMB)
{
    m_settings.setValue("gpu/memoryLimit", limitMB);
}

// Paths
QString Settings::ffmpegPath() const
{
    return m_settings.value("paths/ffmpeg", "").toString();
}

void Settings::setFfmpegPath(const QString& path)
{
    m_settings.setValue("paths/ffmpeg", path);
}

QString Settings::vipsPath() const
{
    return m_settings.value("paths/vips", "").toString();
}

void Settings::setVipsPath(const QString& path)
{
    m_settings.setValue("paths/vips", path);
}
