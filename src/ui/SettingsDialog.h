/**
 * @file SettingsDialog.h
 * @brief Settings dialog header
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() = default;

signals:
    void settingsChanged();

private slots:
    void onApply();
    void onReset();
    void onBrowseOutputFolder();
    void onBrowseFFmpegPath();
    void onBrowseVipsPath();

private:
    void setupUI();
    void setupGeneralTab(QWidget* tab);
    void setupImageTab(QWidget* tab);
    void setupVideoTab(QWidget* tab);
    void setupGPUTab(QWidget* tab);
    void loadSettings();
    void saveSettings();

private:
    QTabWidget* m_tabWidget = nullptr;

    // General settings
    QLineEdit* m_outputFolderEdit = nullptr;
    QCheckBox* m_overwriteOriginalCheck = nullptr;
    QCheckBox* m_recursiveScanCheck = nullptr;
    QSpinBox* m_threadCountSpin = nullptr;
    QComboBox* m_themeCombo = nullptr;
    QCheckBox* m_showNotificationsCheck = nullptr;
    QCheckBox* m_playSoundsCheck = nullptr;

    // Image settings
    QComboBox* m_imageOutputFormatCombo = nullptr;
    QComboBox* m_imageCompressionModeCombo = nullptr;
    QSpinBox* m_imageQualitySpin = nullptr;
    QCheckBox* m_preserveMetadataCheck = nullptr;
    QCheckBox* m_preserveColorProfileCheck = nullptr;
    QSpinBox* m_jpegXLEffortSpin = nullptr;
    QSpinBox* m_avifSpeedSpin = nullptr;
    QSpinBox* m_webpMethodSpin = nullptr;

    // Video settings
    QComboBox* m_videoOutputFormatCombo = nullptr;
    QComboBox* m_videoCodecCombo = nullptr;
    QComboBox* m_videoCompressionModeCombo = nullptr;
    QSpinBox* m_videoCrfSpin = nullptr;
    QComboBox* m_videoPresetCombo = nullptr;
    QCheckBox* m_preserveAudioCheck = nullptr;
    QComboBox* m_audioCodecCombo = nullptr;
    QSpinBox* m_audioBitrateSpin = nullptr;

    // GPU settings
    QCheckBox* m_useGpuCheck = nullptr;
    QComboBox* m_gpuModeCombo = nullptr;
    QLabel* m_gpuInfoLabel = nullptr;
    QCheckBox* m_useNvencCheck = nullptr;
    QCheckBox* m_useNvdecCheck = nullptr;
    QSpinBox* m_gpuMemoryLimitSpin = nullptr;

    // Paths
    QLineEdit* m_ffmpegPathEdit = nullptr;
    QLineEdit* m_vipsPathEdit = nullptr;

    QPushButton* m_applyButton = nullptr;
    QPushButton* m_resetButton = nullptr;
};

#endif // SETTINGSDIALOG_H
