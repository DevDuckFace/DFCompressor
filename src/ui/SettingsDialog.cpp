/**
 * @file SettingsDialog.cpp
 * @brief Settings dialog implementation
 */

#include "SettingsDialog.h"
#include "Settings.h"
#include "GPUDetector.h"
#include "Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QScrollArea>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setMinimumSize(600, 500);
    setModal(true);
    
    setupUI();
    loadSettings();
}

void SettingsDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    
    m_tabWidget = new QTabWidget;
    
    // Create tabs
    auto* generalTab = new QWidget;
    auto* imageTab = new QWidget;
    auto* videoTab = new QWidget;
    auto* gpuTab = new QWidget;
    
    setupGeneralTab(generalTab);
    setupImageTab(imageTab);
    setupVideoTab(videoTab);
    setupGPUTab(gpuTab);
    
    m_tabWidget->addTab(generalTab, QIcon(":/icons/settings.svg"), tr("General"));
    m_tabWidget->addTab(imageTab, QIcon(":/icons/file_image.svg"), tr("Images"));
    m_tabWidget->addTab(videoTab, QIcon(":/icons/file_video.svg"), tr("Videos"));
    m_tabWidget->addTab(gpuTab, QIcon(":/icons/gpu.svg"), tr("GPU"));
    
    mainLayout->addWidget(m_tabWidget);
    
    // Button box
    auto* buttonBox = new QDialogButtonBox;
    
    m_applyButton = buttonBox->addButton(tr("Apply"), QDialogButtonBox::ApplyRole);
    m_resetButton = buttonBox->addButton(tr("Reset to Defaults"), QDialogButtonBox::ResetRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onReset);
    connect(buttonBox, &QDialogButtonBox::accepted, [this]() {
        onApply();
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    mainLayout->addWidget(buttonBox);
}

void SettingsDialog::setupGeneralTab(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);
    layout->setSpacing(16);
    
    // Output folder group
    auto* outputGroup = new QGroupBox(tr("Output"));
    auto* outputLayout = new QFormLayout(outputGroup);
    
    auto* folderLayout = new QHBoxLayout;
    m_outputFolderEdit = new QLineEdit;
    m_outputFolderEdit->setPlaceholderText(tr("Leave empty to use same folder as input"));
    auto* browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseOutputFolder);
    folderLayout->addWidget(m_outputFolderEdit);
    folderLayout->addWidget(browseButton);
    outputLayout->addRow(tr("Output Folder:"), folderLayout);
    
    m_overwriteOriginalCheck = new QCheckBox(tr("Overwrite original files (dangerous!)"));
    outputLayout->addRow("", m_overwriteOriginalCheck);
    
    layout->addWidget(outputGroup);
    
    // Processing group
    auto* processingGroup = new QGroupBox(tr("Processing"));
    auto* processingLayout = new QFormLayout(processingGroup);
    
    m_recursiveScanCheck = new QCheckBox(tr("Scan subfolders when adding folders"));
    processingLayout->addRow("", m_recursiveScanCheck);
    
    m_threadCountSpin = new QSpinBox;
    m_threadCountSpin->setRange(1, 32);
    m_threadCountSpin->setSuffix(tr(" threads"));
    processingLayout->addRow(tr("Thread Count:"), m_threadCountSpin);
    
    layout->addWidget(processingGroup);
    
    // Appearance group
    auto* appearanceGroup = new QGroupBox(tr("Appearance"));
    auto* appearanceLayout = new QFormLayout(appearanceGroup);
    
    m_themeCombo = new QComboBox;
    m_themeCombo->addItem(tr("Dark"), "dark");
    m_themeCombo->addItem(tr("Light"), "light");
    m_themeCombo->addItem(tr("System"), "system");
    appearanceLayout->addRow(tr("Theme:"), m_themeCombo);
    
    m_showNotificationsCheck = new QCheckBox(tr("Show desktop notifications"));
    appearanceLayout->addRow("", m_showNotificationsCheck);
    
    m_playSoundsCheck = new QCheckBox(tr("Play sounds on completion"));
    appearanceLayout->addRow("", m_playSoundsCheck);
    
    layout->addWidget(appearanceGroup);
    
    // Paths group
    auto* pathsGroup = new QGroupBox(tr("External Tools"));
    auto* pathsLayout = new QFormLayout(pathsGroup);
    
    auto* ffmpegLayout = new QHBoxLayout;
    m_ffmpegPathEdit = new QLineEdit;
    m_ffmpegPathEdit->setPlaceholderText(tr("Auto-detect"));
    auto* ffmpegBrowse = new QPushButton(tr("Browse..."));
    connect(ffmpegBrowse, &QPushButton::clicked, this, &SettingsDialog::onBrowseFFmpegPath);
    ffmpegLayout->addWidget(m_ffmpegPathEdit);
    ffmpegLayout->addWidget(ffmpegBrowse);
    pathsLayout->addRow(tr("FFmpeg:"), ffmpegLayout);
    
    auto* vipsLayout = new QHBoxLayout;
    m_vipsPathEdit = new QLineEdit;
    m_vipsPathEdit->setPlaceholderText(tr("Auto-detect"));
    auto* vipsBrowse = new QPushButton(tr("Browse..."));
    connect(vipsBrowse, &QPushButton::clicked, this, &SettingsDialog::onBrowseVipsPath);
    vipsLayout->addWidget(m_vipsPathEdit);
    vipsLayout->addWidget(vipsBrowse);
    pathsLayout->addRow(tr("libvips:"), vipsLayout);
    
    layout->addWidget(pathsGroup);
    
    layout->addStretch();
}

void SettingsDialog::setupImageTab(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);
    layout->setSpacing(16);
    
    // Format group
    auto* formatGroup = new QGroupBox(tr("Output Format"));
    auto* formatLayout = new QFormLayout(formatGroup);
    
    m_imageOutputFormatCombo = new QComboBox;
    m_imageOutputFormatCombo->addItem("AVIF (.avif)", "avif");
    m_imageOutputFormatCombo->addItem("WebP (.webp)", "webp");
    m_imageOutputFormatCombo->addItem("PNG (.png)", "png");
    m_imageOutputFormatCombo->addItem(tr("Keep Original Format"), "keep");
    formatLayout->addRow(tr("Format:"), m_imageOutputFormatCombo);
    
    layout->addWidget(formatGroup);
    
    // Compression group
    auto* compressionGroup = new QGroupBox(tr("Compression"));
    auto* compressionLayout = new QFormLayout(compressionGroup);
    
    m_imageCompressionModeCombo = new QComboBox;
    m_imageCompressionModeCombo->addItem(tr("Lossless (Recommended)"), "lossless");
    m_imageCompressionModeCombo->addItem(tr("Lossy - High Quality"), "lossy_high");
    m_imageCompressionModeCombo->addItem(tr("Lossy - Medium Quality"), "lossy_medium");
    m_imageCompressionModeCombo->addItem(tr("Lossy - Web Optimized"), "lossy_web");
    compressionLayout->addRow(tr("Mode:"), m_imageCompressionModeCombo);
    
    m_imageQualitySpin = new QSpinBox;
    m_imageQualitySpin->setRange(1, 100);
    m_imageQualitySpin->setValue(95);
    m_imageQualitySpin->setSuffix("%");
    compressionLayout->addRow(tr("Quality (Lossy):"), m_imageQualitySpin);
    
    layout->addWidget(compressionGroup);
    
    // Advanced group
    auto* advancedGroup = new QGroupBox(tr("Advanced"));
    auto* advancedLayout = new QFormLayout(advancedGroup);
    
    m_preserveMetadataCheck = new QCheckBox(tr("Preserve EXIF/metadata"));
    advancedLayout->addRow("", m_preserveMetadataCheck);
    
    m_preserveColorProfileCheck = new QCheckBox(tr("Preserve color profile (ICC)"));
    m_preserveColorProfileCheck->setChecked(true);
    advancedLayout->addRow("", m_preserveColorProfileCheck);
    
    // JPEG XL removed - not supported by bundled libvips
    
    m_avifSpeedSpin = new QSpinBox;
    m_avifSpeedSpin->setRange(0, 10);
    m_avifSpeedSpin->setValue(6);
    m_avifSpeedSpin->setToolTip(tr("0 = slowest/best, 10 = fastest"));
    advancedLayout->addRow(tr("AVIF Speed:"), m_avifSpeedSpin);
    
    m_webpMethodSpin = new QSpinBox;
    m_webpMethodSpin->setRange(0, 6);
    m_webpMethodSpin->setValue(4);
    m_webpMethodSpin->setToolTip(tr("0 = fastest, 6 = slowest/best"));
    advancedLayout->addRow(tr("WebP Method:"), m_webpMethodSpin);
    
    layout->addWidget(advancedGroup);
    
    layout->addStretch();
}

void SettingsDialog::setupVideoTab(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);
    layout->setSpacing(16);
    
    // Format group
    auto* formatGroup = new QGroupBox(tr("Output Format"));
    auto* formatLayout = new QFormLayout(formatGroup);
    
    m_videoOutputFormatCombo = new QComboBox;
    m_videoOutputFormatCombo->addItem("MP4 (.mp4)", "mp4");
    m_videoOutputFormatCombo->addItem("MKV (.mkv)", "mkv");
    m_videoOutputFormatCombo->addItem("WebM (.webm)", "webm");
    m_videoOutputFormatCombo->addItem(tr("Keep Original Container"), "keep");
    formatLayout->addRow(tr("Container:"), m_videoOutputFormatCombo);
    
    m_videoCodecCombo = new QComboBox;
    m_videoCodecCombo->addItem("H.265/HEVC (Best compression)", "hevc");
    m_videoCodecCombo->addItem("H.264/AVC (Most compatible)", "h264");
    m_videoCodecCombo->addItem("VP9 (WebM)", "vp9");
    m_videoCodecCombo->addItem(tr("Copy (No re-encoding)"), "copy");
    formatLayout->addRow(tr("Video Codec:"), m_videoCodecCombo);
    
    layout->addWidget(formatGroup);
    
    // Quality group
    auto* qualityGroup = new QGroupBox(tr("Quality"));
    auto* qualityLayout = new QFormLayout(qualityGroup);
    
    m_videoCompressionModeCombo = new QComboBox;
    m_videoCompressionModeCombo->addItem(tr("Lossless (CRF 0)"), "lossless");
    m_videoCompressionModeCombo->addItem(tr("Visually Lossless (CRF 18)"), "visually_lossless");
    m_videoCompressionModeCombo->addItem(tr("High Quality (CRF 23)"), "high");
    m_videoCompressionModeCombo->addItem(tr("Medium Quality (CRF 28)"), "medium");
    m_videoCompressionModeCombo->addItem(tr("Web Optimized (CRF 32)"), "web");
    m_videoCompressionModeCombo->addItem(tr("Custom CRF"), "custom");
    qualityLayout->addRow(tr("Mode:"), m_videoCompressionModeCombo);
    
    m_videoCrfSpin = new QSpinBox;
    m_videoCrfSpin->setRange(0, 63);
    m_videoCrfSpin->setValue(23);
    m_videoCrfSpin->setToolTip(tr("0 = lossless, 51+ = very lossy"));
    qualityLayout->addRow(tr("CRF Value:"), m_videoCrfSpin);
    
    m_videoPresetCombo = new QComboBox;
    m_videoPresetCombo->addItem(tr("Ultrafast"), "ultrafast");
    m_videoPresetCombo->addItem(tr("Superfast"), "superfast");
    m_videoPresetCombo->addItem(tr("Veryfast"), "veryfast");
    m_videoPresetCombo->addItem(tr("Faster"), "faster");
    m_videoPresetCombo->addItem(tr("Fast"), "fast");
    m_videoPresetCombo->addItem(tr("Medium"), "medium");
    m_videoPresetCombo->addItem(tr("Slow"), "slow");
    m_videoPresetCombo->addItem(tr("Slower"), "slower");
    m_videoPresetCombo->addItem(tr("Veryslow"), "veryslow");
    m_videoPresetCombo->setCurrentIndex(5); // Medium
    qualityLayout->addRow(tr("Preset:"), m_videoPresetCombo);
    
    layout->addWidget(qualityGroup);
    
    // Audio group
    auto* audioGroup = new QGroupBox(tr("Audio"));
    auto* audioLayout = new QFormLayout(audioGroup);
    
    m_preserveAudioCheck = new QCheckBox(tr("Preserve audio track"));
    m_preserveAudioCheck->setChecked(true);
    audioLayout->addRow("", m_preserveAudioCheck);
    
    m_audioCodecCombo = new QComboBox;
    m_audioCodecCombo->addItem("AAC", "aac");
    m_audioCodecCombo->addItem("Opus (Recommended)", "opus");
    m_audioCodecCombo->addItem("FLAC (Lossless)", "flac");
    m_audioCodecCombo->addItem(tr("Copy (No re-encoding)"), "copy");
    audioLayout->addRow(tr("Audio Codec:"), m_audioCodecCombo);
    
    m_audioBitrateSpin = new QSpinBox;
    m_audioBitrateSpin->setRange(64, 512);
    m_audioBitrateSpin->setValue(192);
    m_audioBitrateSpin->setSuffix(" kbps");
    audioLayout->addRow(tr("Audio Bitrate:"), m_audioBitrateSpin);
    
    layout->addWidget(audioGroup);
    
    layout->addStretch();
}

void SettingsDialog::setupGPUTab(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);
    layout->setSpacing(16);
    
    // GPU Info
    auto* infoGroup = new QGroupBox(tr("GPU Information"));
    auto* infoLayout = new QVBoxLayout(infoGroup);
    
    m_gpuInfoLabel = new QLabel;
    m_gpuInfoLabel->setWordWrap(true);
    m_gpuInfoLabel->setStyleSheet("QLabel { padding: 12px; background: #2d2d2d; border-radius: 8px; }");
    
    // Detect GPU
    GPUDetector detector;
    auto gpuInfo = detector.detect();
    
    if (gpuInfo.hasNvidia) {
        m_gpuInfoLabel->setText(QString(
            "<b>GPU:</b> %1<br>"
            "<b>CUDA Cores:</b> %2<br>"
            "<b>VRAM:</b> %3 GB<br>"
            "<b>Compute Capability:</b> %4.%5<br>"
            "<b>NVENC:</b> %6<br>"
            "<b>NVDEC:</b> %7"
        )
            .arg(gpuInfo.deviceName)
            .arg(gpuInfo.cudaCores)
            .arg(gpuInfo.vramMB / 1024.0, 0, 'f', 1)
            .arg(gpuInfo.computeCapabilityMajor)
            .arg(gpuInfo.computeCapabilityMinor)
            .arg(gpuInfo.hasNvenc ? tr("Available ✓") : tr("Not available"))
            .arg(gpuInfo.hasNvdec ? tr("Available ✓") : tr("Not available"))
        );
    } else {
        m_gpuInfoLabel->setText(tr(
            "<b>No NVIDIA GPU detected</b><br><br>"
            "GPU acceleration is only available with NVIDIA graphics cards "
            "that support CUDA. Processing will use CPU instead."
        ));
    }
    
    infoLayout->addWidget(m_gpuInfoLabel);
    layout->addWidget(infoGroup);
    
    // GPU Settings
    auto* settingsGroup = new QGroupBox(tr("GPU Acceleration"));
    auto* settingsLayout = new QFormLayout(settingsGroup);
    
    m_useGpuCheck = new QCheckBox(tr("Enable GPU acceleration"));
    m_useGpuCheck->setEnabled(gpuInfo.hasNvidia);
    m_useGpuCheck->setChecked(gpuInfo.hasNvidia);
    settingsLayout->addRow("", m_useGpuCheck);
    
    m_gpuModeCombo = new QComboBox;
    m_gpuModeCombo->addItem(tr("Auto"), "auto");
    m_gpuModeCombo->addItem(tr("Always Use GPU"), "always");
    m_gpuModeCombo->addItem(tr("GPU for Videos Only"), "video_only");
    m_gpuModeCombo->addItem(tr("CPU Only"), "cpu");
    m_gpuModeCombo->setEnabled(gpuInfo.hasNvidia);
    settingsLayout->addRow(tr("Mode:"), m_gpuModeCombo);
    
    m_useNvencCheck = new QCheckBox(tr("Use NVENC for video encoding"));
    m_useNvencCheck->setEnabled(gpuInfo.hasNvenc);
    m_useNvencCheck->setChecked(gpuInfo.hasNvenc);
    settingsLayout->addRow("", m_useNvencCheck);
    
    m_useNvdecCheck = new QCheckBox(tr("Use NVDEC for video decoding"));
    m_useNvdecCheck->setEnabled(gpuInfo.hasNvdec);
    m_useNvdecCheck->setChecked(gpuInfo.hasNvdec);
    settingsLayout->addRow("", m_useNvdecCheck);
    
    m_gpuMemoryLimitSpin = new QSpinBox;
    m_gpuMemoryLimitSpin->setRange(512, 65536);
    m_gpuMemoryLimitSpin->setValue(gpuInfo.vramMB > 0 ? gpuInfo.vramMB * 0.8 : 4096);
    m_gpuMemoryLimitSpin->setSuffix(" MB");
    m_gpuMemoryLimitSpin->setEnabled(gpuInfo.hasNvidia);
    settingsLayout->addRow(tr("VRAM Limit:"), m_gpuMemoryLimitSpin);
    
    layout->addWidget(settingsGroup);
    
    layout->addStretch();
}

void SettingsDialog::loadSettings()
{
    auto& settings = Settings::instance();
    
    // General
    m_outputFolderEdit->setText(settings.outputFolder());
    m_overwriteOriginalCheck->setChecked(settings.overwriteOriginal());
    m_recursiveScanCheck->setChecked(settings.recursiveScan());
    m_threadCountSpin->setValue(settings.threadCount());
    
    int themeIndex = m_themeCombo->findData(settings.theme());
    if (themeIndex >= 0) m_themeCombo->setCurrentIndex(themeIndex);
    
    m_showNotificationsCheck->setChecked(settings.showNotifications());
    m_playSoundsCheck->setChecked(settings.playSounds());
    
    // Image
    int imageFormatIndex = m_imageOutputFormatCombo->findData(settings.imageOutputFormat());
    if (imageFormatIndex >= 0) m_imageOutputFormatCombo->setCurrentIndex(imageFormatIndex);
    
    int imageCompIndex = m_imageCompressionModeCombo->findData(settings.imageCompressionMode());
    if (imageCompIndex >= 0) m_imageCompressionModeCombo->setCurrentIndex(imageCompIndex);
    
    m_imageQualitySpin->setValue(settings.imageQuality());
    m_preserveMetadataCheck->setChecked(settings.preserveMetadata());
    m_preserveColorProfileCheck->setChecked(settings.preserveColorProfile());
    // m_jpegXLEffortSpin removed - JXL not supported
    m_avifSpeedSpin->setValue(settings.avifSpeed());
    m_webpMethodSpin->setValue(settings.webpMethod());
    
    // Video
    int videoFormatIndex = m_videoOutputFormatCombo->findData(settings.videoOutputFormat());
    if (videoFormatIndex >= 0) m_videoOutputFormatCombo->setCurrentIndex(videoFormatIndex);
    
    int videoCodecIndex = m_videoCodecCombo->findData(settings.videoCodec());
    if (videoCodecIndex >= 0) m_videoCodecCombo->setCurrentIndex(videoCodecIndex);
    
    int videoCompIndex = m_videoCompressionModeCombo->findData(settings.videoCompressionMode());
    if (videoCompIndex >= 0) m_videoCompressionModeCombo->setCurrentIndex(videoCompIndex);
    
    m_videoCrfSpin->setValue(settings.videoCrf());
    
    int presetIndex = m_videoPresetCombo->findData(settings.videoPreset());
    if (presetIndex >= 0) m_videoPresetCombo->setCurrentIndex(presetIndex);
    
    m_preserveAudioCheck->setChecked(settings.preserveAudio());
    
    int audioCodecIndex = m_audioCodecCombo->findData(settings.audioCodec());
    if (audioCodecIndex >= 0) m_audioCodecCombo->setCurrentIndex(audioCodecIndex);
    
    m_audioBitrateSpin->setValue(settings.audioBitrate());
    
    // GPU
    m_useGpuCheck->setChecked(settings.useGpu());
    
    int gpuModeIndex = m_gpuModeCombo->findData(settings.gpuMode());
    if (gpuModeIndex >= 0) m_gpuModeCombo->setCurrentIndex(gpuModeIndex);
    
    m_useNvencCheck->setChecked(settings.useNvenc());
    m_useNvdecCheck->setChecked(settings.useNvdec());
    m_gpuMemoryLimitSpin->setValue(settings.gpuMemoryLimit());
    
    // Paths
    m_ffmpegPathEdit->setText(settings.ffmpegPath());
    m_vipsPathEdit->setText(settings.vipsPath());
}

void SettingsDialog::saveSettings()
{
    auto& settings = Settings::instance();
    
    // General
    settings.setOutputFolder(m_outputFolderEdit->text());
    settings.setOverwriteOriginal(m_overwriteOriginalCheck->isChecked());
    settings.setRecursiveScan(m_recursiveScanCheck->isChecked());
    settings.setThreadCount(m_threadCountSpin->value());
    settings.setTheme(m_themeCombo->currentData().toString());
    settings.setShowNotifications(m_showNotificationsCheck->isChecked());
    settings.setPlaySounds(m_playSoundsCheck->isChecked());
    
    // Image
    settings.setImageOutputFormat(m_imageOutputFormatCombo->currentData().toString());
    settings.setImageCompressionMode(m_imageCompressionModeCombo->currentData().toString());
    settings.setImageQuality(m_imageQualitySpin->value());
    settings.setPreserveMetadata(m_preserveMetadataCheck->isChecked());
    settings.setPreserveColorProfile(m_preserveColorProfileCheck->isChecked());
    // m_jpegXLEffortSpin removed - JXL not supported
    settings.setAvifSpeed(m_avifSpeedSpin->value());
    settings.setWebpMethod(m_webpMethodSpin->value());
    
    // Video
    settings.setVideoOutputFormat(m_videoOutputFormatCombo->currentData().toString());
    settings.setVideoCodec(m_videoCodecCombo->currentData().toString());
    settings.setVideoCompressionMode(m_videoCompressionModeCombo->currentData().toString());
    settings.setVideoCrf(m_videoCrfSpin->value());
    settings.setVideoPreset(m_videoPresetCombo->currentData().toString());
    settings.setPreserveAudio(m_preserveAudioCheck->isChecked());
    settings.setAudioCodec(m_audioCodecCombo->currentData().toString());
    settings.setAudioBitrate(m_audioBitrateSpin->value());
    
    // GPU
    settings.setUseGpu(m_useGpuCheck->isChecked());
    settings.setGpuMode(m_gpuModeCombo->currentData().toString());
    settings.setUseNvenc(m_useNvencCheck->isChecked());
    settings.setUseNvdec(m_useNvdecCheck->isChecked());
    settings.setGpuMemoryLimit(m_gpuMemoryLimitSpin->value());
    
    // Paths
    settings.setFfmpegPath(m_ffmpegPathEdit->text());
    settings.setVipsPath(m_vipsPathEdit->text());
    
    settings.save();
}

void SettingsDialog::onApply()
{
    saveSettings();
    emit settingsChanged();
    Logger::info("Settings applied");
}

void SettingsDialog::onReset()
{
    Settings::instance().resetToDefaults();
    loadSettings();
}

void SettingsDialog::onBrowseOutputFolder()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        tr("Select Output Folder"),
        m_outputFolderEdit->text().isEmpty() 
            ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            : m_outputFolderEdit->text()
    );
    
    if (!folder.isEmpty()) {
        m_outputFolderEdit->setText(folder);
    }
}

void SettingsDialog::onBrowseFFmpegPath()
{
    QString file = QFileDialog::getOpenFileName(
        this,
        tr("Select FFmpeg Executable"),
        QString(),
        tr("FFmpeg (ffmpeg.exe)")
    );
    
    if (!file.isEmpty()) {
        m_ffmpegPathEdit->setText(file);
    }
}

void SettingsDialog::onBrowseVipsPath()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        tr("Select libvips Directory"),
        QString()
    );
    
    if (!folder.isEmpty()) {
        m_vipsPathEdit->setText(folder);
    }
}
