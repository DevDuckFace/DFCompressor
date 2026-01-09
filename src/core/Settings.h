/**
 * @file Settings.h
 * @brief Application settings header
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QSettings>
#include <QVariant>

class Settings
{
public:
    static Settings& instance();

    void load();
    void save();
    void resetToDefaults();

    // Generic value access
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);

    // General
    QString outputFolder() const;
    void setOutputFolder(const QString& folder);
    
    bool overwriteOriginal() const;
    void setOverwriteOriginal(bool overwrite);
    
    bool recursiveScan() const;
    void setRecursiveScan(bool recursive);
    
    int threadCount() const;
    void setThreadCount(int count);
    
    QString theme() const;
    void setTheme(const QString& theme);
    
    bool showNotifications() const;
    void setShowNotifications(bool show);
    
    bool playSounds() const;
    void setPlaySounds(bool play);

    // Image settings
    QString imageOutputFormat() const;
    void setImageOutputFormat(const QString& format);
    
    QString imageCompressionMode() const;
    void setImageCompressionMode(const QString& mode);
    
    int imageQuality() const;
    void setImageQuality(int quality);
    
    bool preserveMetadata() const;
    void setPreserveMetadata(bool preserve);
    
    bool preserveColorProfile() const;
    void setPreserveColorProfile(bool preserve);
    
    int jpegXlEffort() const;
    void setJpegXlEffort(int effort);
    
    int avifSpeed() const;
    void setAvifSpeed(int speed);
    
    int webpMethod() const;
    void setWebpMethod(int method);

    // Video settings
    QString videoOutputFormat() const;
    void setVideoOutputFormat(const QString& format);
    
    QString videoCodec() const;
    void setVideoCodec(const QString& codec);
    
    QString videoCompressionMode() const;
    void setVideoCompressionMode(const QString& mode);
    
    int videoCrf() const;
    void setVideoCrf(int crf);
    
    QString videoPreset() const;
    void setVideoPreset(const QString& preset);
    
    bool preserveAudio() const;
    void setPreserveAudio(bool preserve);
    
    QString audioCodec() const;
    void setAudioCodec(const QString& codec);
    
    int audioBitrate() const;
    void setAudioBitrate(int bitrate);

    // GPU settings
    bool useGpu() const;
    void setUseGpu(bool use);
    
    QString gpuMode() const;
    void setGpuMode(const QString& mode);
    
    bool useNvenc() const;
    void setUseNvenc(bool use);
    
    bool useNvdec() const;
    void setUseNvdec(bool use);
    
    int gpuMemoryLimit() const;
    void setGpuMemoryLimit(int limitMB);

    // Paths
    QString ffmpegPath() const;
    void setFfmpegPath(const QString& path);
    
    QString vipsPath() const;
    void setVipsPath(const QString& path);

private:
    Settings();
    ~Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

private:
    QSettings m_settings;
};

#endif // SETTINGS_H
