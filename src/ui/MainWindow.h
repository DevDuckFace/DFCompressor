/**
 * @file MainWindow.h
 * @brief Main application window header
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QProgressBar>
#include <QLabel>
#include <QToolButton>
#include <QSystemTrayIcon>
#include <memory>

#include "GPUDetector.h"
#include "JobQueue.h"

class FileListWidget;
class DropZone;
class SettingsDialog;
class ProgressWidget;
class PreviewWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setGPUInfo(const GPUInfo& info);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onAddFiles();
    void onAddFolder();
    void onRemoveSelected();
    void onClearAll();
    void onStartConversion();
    void onPauseConversion();
    void onStopConversion();
    void onOpenSettings();
    void onToggleTheme();
    void onFileDoubleClicked(const QString& filePath);
    void onJobProgress(const QString& jobId, int progress);
    void onJobCompleted(const QString& jobId);
    void onJobFailed(const QString& jobId, const QString& error);
    void onAllJobsCompleted();
    void onShowAbout();
    void onSelectOutputFolder();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupConnections();
    void setupShortcuts();
    void loadSettings();
    void saveSettings();
    void updateStatusBar();
    void addFilesToQueue(const QStringList& files);
    void processDroppedItems(const QList<QUrl>& urls);

private:
    // Central widgets
    QStackedWidget* m_stackedWidget = nullptr;
    DropZone* m_dropZone = nullptr;
    FileListWidget* m_fileListWidget = nullptr;
    ProgressWidget* m_progressWidget = nullptr;
    PreviewWidget* m_previewWidget = nullptr;

    // Toolbar buttons
    QToolButton* m_btnAddFiles = nullptr;
    QToolButton* m_btnAddFolder = nullptr;
    QToolButton* m_btnRemove = nullptr;
    QToolButton* m_btnClear = nullptr;
    QToolButton* m_btnStart = nullptr;
    QToolButton* m_btnPause = nullptr;
    QToolButton* m_btnStop = nullptr;
    QToolButton* m_btnSettings = nullptr;
    QToolButton* m_btnTheme = nullptr;

    // Status bar widgets
    QLabel* m_statusLabel = nullptr;
    QLabel* m_gpuStatusLabel = nullptr;
    QProgressBar* m_globalProgress = nullptr;

    // Dialogs
    SettingsDialog* m_settingsDialog = nullptr;

    // System tray
    QSystemTrayIcon* m_trayIcon = nullptr;

    // Core components
    std::unique_ptr<JobQueue> m_jobQueue;
    GPUInfo m_gpuInfo;

    // State
    bool m_isProcessing = false;
    QString m_lastOutputFolder;
};

#endif // MAINWINDOW_H
