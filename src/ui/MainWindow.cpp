/**
 * @file MainWindow.cpp
 * @brief Main application window implementation
 */

#include "MainWindow.h"
#include "FileListWidget.h"
#include "DropZone.h"
#include "SettingsDialog.h"
#include "ProgressWidget.h"
#include "PreviewWidget.h"
#include "ThemeManager.h"
#include "Settings.h"
#include "Logger.h"
#include "FileUtils.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QShortcut>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_jobQueue(std::make_unique<JobQueue>())
{
    setWindowTitle("DFCompressor - Advanced Media Converter");
    setMinimumSize(1200, 800);
    setAcceptDrops(true);
    
    setupUI();
    setupConnections();
    setupShortcuts();
    loadSettings();
    
    Logger::info("MainWindow initialized");
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setGPUInfo(const GPUInfo& info)
{
    m_gpuInfo = info;
    updateStatusBar();
}

void MainWindow::setupUI()
{
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
    
    // Set window icon
    setWindowIcon(QIcon(":/icons/app_icon.svg"));
}

void MainWindow::setupMenuBar()
{
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    
    auto* addFilesAction = fileMenu->addAction(QIcon(":/icons/add_file.svg"), 
        tr("Add Files..."), this, &MainWindow::onAddFiles);
    addFilesAction->setShortcut(QKeySequence::Open);
    
    auto* addFolderAction = fileMenu->addAction(QIcon(":/icons/add_folder.svg"),
        tr("Add Folder..."), this, &MainWindow::onAddFolder);
    addFolderAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    
    fileMenu->addSeparator();
    
    fileMenu->addAction(QIcon(":/icons/output.svg"),
        tr("Select Output Folder..."), this, &MainWindow::onSelectOutputFolder);
    
    fileMenu->addSeparator();
    
    auto* exitAction = fileMenu->addAction(QIcon(":/icons/exit.svg"),
        tr("Exit"), this, &QMainWindow::close);
    exitAction->setShortcut(QKeySequence::Quit);
    
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    
    editMenu->addAction(QIcon(":/icons/remove.svg"),
        tr("Remove Selected"), this, &MainWindow::onRemoveSelected);
    editMenu->addAction(QIcon(":/icons/clear.svg"),
        tr("Clear All"), this, &MainWindow::onClearAll);
    
    editMenu->addSeparator();
    
    auto* settingsAction = editMenu->addAction(QIcon(":/icons/settings.svg"),
        tr("Settings..."), this, &MainWindow::onOpenSettings);
    settingsAction->setShortcut(QKeySequence::Preferences);
    
    auto* processMenu = menuBar()->addMenu(tr("&Process"));
    
    processMenu->addAction(QIcon(":/icons/start.svg"),
        tr("Start Conversion"), this, &MainWindow::onStartConversion);
    processMenu->addAction(QIcon(":/icons/pause.svg"),
        tr("Pause"), this, &MainWindow::onPauseConversion);
    processMenu->addAction(QIcon(":/icons/stop.svg"),
        tr("Stop"), this, &MainWindow::onStopConversion);
    
    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    
    viewMenu->addAction(QIcon(":/icons/theme.svg"),
        tr("Toggle Theme"), this, &MainWindow::onToggleTheme);
    
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    
    helpMenu->addAction(QIcon(":/icons/about.svg"),
        tr("About DFCompressor"), this, &MainWindow::onShowAbout);
}

void MainWindow::setupToolBar()
{
    auto* toolbar = addToolBar(tr("Main Toolbar"));
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setIconSize(QSize(32, 32));
    toolbar->setStyleSheet("QToolBar { spacing: 8px; padding: 8px; border: none; }");
    
    // Add files button
    m_btnAddFiles = new QToolButton;
    m_btnAddFiles->setIcon(QIcon(":/icons/add_file.svg"));
    m_btnAddFiles->setText(tr("Add Files"));
    m_btnAddFiles->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(m_btnAddFiles);
    
    // Add folder button
    m_btnAddFolder = new QToolButton;
    m_btnAddFolder->setIcon(QIcon(":/icons/add_folder.svg"));
    m_btnAddFolder->setText(tr("Add Folder"));
    m_btnAddFolder->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(m_btnAddFolder);
    
    toolbar->addSeparator();
    
    // Remove button
    m_btnRemove = new QToolButton;
    m_btnRemove->setIcon(QIcon(":/icons/remove.svg"));
    m_btnRemove->setText(tr("Remove"));
    m_btnRemove->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(m_btnRemove);
    
    // Clear button
    m_btnClear = new QToolButton;
    m_btnClear->setIcon(QIcon(":/icons/clear.svg"));
    m_btnClear->setText(tr("Clear All"));
    m_btnClear->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(m_btnClear);
    
    toolbar->addSeparator();
    
    // Start button
    m_btnStart = new QToolButton;
    m_btnStart->setIcon(QIcon(":/icons/start.svg"));
    m_btnStart->setText(tr("Start"));
    m_btnStart->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_btnStart->setStyleSheet("QToolButton { color: #4CAF50; font-weight: bold; }");
    toolbar->addWidget(m_btnStart);
    
    // Pause button
    m_btnPause = new QToolButton;
    m_btnPause->setIcon(QIcon(":/icons/pause.svg"));
    m_btnPause->setText(tr("Pause"));
    m_btnPause->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_btnPause->setEnabled(false);
    toolbar->addWidget(m_btnPause);
    
    // Stop button
    m_btnStop = new QToolButton;
    m_btnStop->setIcon(QIcon(":/icons/stop.svg"));
    m_btnStop->setText(tr("Stop"));
    m_btnStop->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_btnStop->setEnabled(false);
    toolbar->addWidget(m_btnStop);
    
    // Spacer
    auto* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);
    
    // Settings button
    m_btnSettings = new QToolButton;
    m_btnSettings->setIcon(QIcon(":/icons/settings.svg"));
    m_btnSettings->setText(tr("Settings"));
    m_btnSettings->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(m_btnSettings);
    
    // Theme button
    m_btnTheme = new QToolButton;
    m_btnTheme->setIcon(QIcon(":/icons/theme.svg"));
    m_btnTheme->setText(tr("Theme"));
    m_btnTheme->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->addWidget(m_btnTheme);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(tr("Ready"));
    m_gpuStatusLabel = new QLabel;
    m_globalProgress = new QProgressBar;
    
    m_globalProgress->setMinimum(0);
    m_globalProgress->setMaximum(100);
    m_globalProgress->setValue(0);
    m_globalProgress->setFixedWidth(200);
    m_globalProgress->setTextVisible(true);
    m_globalProgress->setVisible(false);
    
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_gpuStatusLabel);
    statusBar()->addPermanentWidget(m_globalProgress);
}

void MainWindow::setupCentralWidget()
{
    auto* centralWidget = new QWidget;
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    // Create splitter for main content
    auto* splitter = new QSplitter(Qt::Horizontal);
    
    // Left side: Drop zone and file list
    auto* leftWidget = new QWidget;
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);
    
    // Stacked widget for drop zone / file list
    m_stackedWidget = new QStackedWidget;
    
    // Drop zone (initial view when no files)
    m_dropZone = new DropZone;
    m_stackedWidget->addWidget(m_dropZone);
    
    // File list (shown when files are added)
    m_fileListWidget = new FileListWidget;
    m_stackedWidget->addWidget(m_fileListWidget);
    
    leftLayout->addWidget(m_stackedWidget);
    
    // Progress widget (bottom of left side)
    m_progressWidget = new ProgressWidget;
    m_progressWidget->setVisible(false);
    leftLayout->addWidget(m_progressWidget);
    
    splitter->addWidget(leftWidget);
    
    // Right side: Preview
    m_previewWidget = new PreviewWidget;
    m_previewWidget->setMinimumWidth(300);
    splitter->addWidget(m_previewWidget);
    
    // Set splitter proportions
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);
    
    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections()
{
    // Toolbar buttons
    connect(m_btnAddFiles, &QToolButton::clicked, this, &MainWindow::onAddFiles);
    connect(m_btnAddFolder, &QToolButton::clicked, this, &MainWindow::onAddFolder);
    connect(m_btnRemove, &QToolButton::clicked, this, &MainWindow::onRemoveSelected);
    connect(m_btnClear, &QToolButton::clicked, this, &MainWindow::onClearAll);
    connect(m_btnStart, &QToolButton::clicked, this, &MainWindow::onStartConversion);
    connect(m_btnPause, &QToolButton::clicked, this, &MainWindow::onPauseConversion);
    connect(m_btnStop, &QToolButton::clicked, this, &MainWindow::onStopConversion);
    connect(m_btnSettings, &QToolButton::clicked, this, &MainWindow::onOpenSettings);
    connect(m_btnTheme, &QToolButton::clicked, this, &MainWindow::onToggleTheme);
    
    // Drop zone
    connect(m_dropZone, &DropZone::filesDropped, this, &MainWindow::addFilesToQueue);
    connect(m_dropZone, &DropZone::browseClicked, this, &MainWindow::onAddFiles);
    
    // File list
    connect(m_fileListWidget, &FileListWidget::fileDoubleClicked, 
            this, &MainWindow::onFileDoubleClicked);
    connect(m_fileListWidget, &FileListWidget::selectionChanged, [this]() {
        QString selectedFile = m_fileListWidget->selectedFilePath();
        if (!selectedFile.isEmpty()) {
            m_previewWidget->loadPreview(selectedFile);
        }
    });
    
    // Job queue
    connect(m_jobQueue.get(), &JobQueue::jobProgress, 
            this, &MainWindow::onJobProgress);
    connect(m_jobQueue.get(), &JobQueue::jobCompleted, 
            this, &MainWindow::onJobCompleted);
    connect(m_jobQueue.get(), &JobQueue::jobFailed, 
            this, &MainWindow::onJobFailed);
    connect(m_jobQueue.get(), &JobQueue::allJobsCompleted, 
            this, &MainWindow::onAllJobsCompleted);
}

void MainWindow::setupShortcuts()
{
    new QShortcut(QKeySequence::Delete, this, [this]() {
        if (!m_isProcessing) onRemoveSelected();
    });
    
    new QShortcut(QKeySequence(Qt::Key_Escape), this, [this]() {
        if (m_isProcessing) onStopConversion();
    });
    
    new QShortcut(QKeySequence(Qt::Key_Space), this, [this]() {
        if (m_isProcessing) {
            onPauseConversion();
        } else if (m_fileListWidget->fileCount() > 0) {
            onStartConversion();
        }
    });
}

void MainWindow::loadSettings()
{
    auto& settings = Settings::instance();
    
    // Restore window geometry
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    restoreState(settings.value("MainWindow/state").toByteArray());
    
    m_lastOutputFolder = settings.outputFolder();
}

void MainWindow::saveSettings()
{
    auto& settings = Settings::instance();
    
    // Save window geometry
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/state", saveState());
    
    settings.save();
}

void MainWindow::updateStatusBar()
{
    if (m_gpuInfo.hasNvidia) {
        QString gpuText = QString("🖥️ %1 | VRAM: %2 GB")
            .arg(m_gpuInfo.deviceName)
            .arg(m_gpuInfo.vramMB / 1024.0, 0, 'f', 1);
        
        if (m_gpuInfo.hasNvenc) {
            gpuText += " | NVENC ✓";
        }
        
        m_gpuStatusLabel->setText(gpuText);
        m_gpuStatusLabel->setStyleSheet("color: #4CAF50; font-weight: 500;");
    } else {
        m_gpuStatusLabel->setText("💻 CPU Mode");
        m_gpuStatusLabel->setStyleSheet("color: #FF9800; font-weight: 500;");
    }
}

void MainWindow::addFilesToQueue(const QStringList& files)
{
    Logger::info(QString("Adding %1 files to queue").arg(files.count()));
    
    for (const QString& file : files) {
        m_fileListWidget->addFile(file);
    }
    
    // Switch to file list view if we have files
    if (m_fileListWidget->fileCount() > 0) {
        m_stackedWidget->setCurrentWidget(m_fileListWidget);
    }
    
    m_statusLabel->setText(tr("%1 file(s) ready").arg(m_fileListWidget->fileCount()));
}

void MainWindow::processDroppedItems(const QList<QUrl>& urls)
{
    QStringList files;
    
    for (const QUrl& url : urls) {
        if (url.isLocalFile()) {
            QString path = url.toLocalFile();
            QFileInfo info(path);
            
            if (info.isDir()) {
                files.append(FileUtils::scanDirectory(path, 
                    Settings::instance().recursiveScan()));
            } else if (FileUtils::isSupportedFile(path)) {
                files.append(path);
            }
        }
    }
    
    if (!files.isEmpty()) {
        addFilesToQueue(files);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isProcessing) {
        auto result = QMessageBox::question(this, tr("Confirm Exit"),
            tr("Processing is in progress. Are you sure you want to exit?"),
            QMessageBox::Yes | QMessageBox::No);
        
        if (result == QMessageBox::No) {
            event->ignore();
            return;
        }
        
        m_jobQueue->stopAll();
    }
    
    saveSettings();
    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    processDroppedItems(event->mimeData()->urls());
}

// Slots implementation
void MainWindow::onAddFiles()
{
    QStringList filters;
    filters << tr("All Supported Files (*.png *.jpg *.jpeg *.webp *.avif *.heic *.heif *.tiff *.tif *.bmp *.mp4 *.mkv *.avi *.mov *.webm)")
            << tr("Images (*.png *.jpg *.jpeg *.webp *.avif *.heic *.heif *.tiff *.tif *.bmp)")
            << tr("Videos (*.mp4 *.mkv *.avi *.mov *.webm)")
            << tr("All Files (*)");
    
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Add Files"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        filters.join(";;")
    );
    
    if (!files.isEmpty()) {
        addFilesToQueue(files);
    }
}

void MainWindow::onAddFolder()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        tr("Add Folder"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        QFileDialog::ShowDirsOnly
    );
    
    if (!folder.isEmpty()) {
        QStringList files = FileUtils::scanDirectory(folder, 
            Settings::instance().recursiveScan());
        
        if (!files.isEmpty()) {
            addFilesToQueue(files);
        } else {
            QMessageBox::information(this, tr("No Files Found"),
                tr("No supported media files were found in the selected folder."));
        }
    }
}

void MainWindow::onRemoveSelected()
{
    m_fileListWidget->removeSelected();
    
    if (m_fileListWidget->fileCount() == 0) {
        m_stackedWidget->setCurrentWidget(m_dropZone);
        m_statusLabel->setText(tr("Ready"));
    } else {
        m_statusLabel->setText(tr("%1 file(s) ready").arg(m_fileListWidget->fileCount()));
    }
}

void MainWindow::onClearAll()
{
    if (m_fileListWidget->fileCount() > 0) {
        auto result = QMessageBox::question(this, tr("Clear All"),
            tr("Are you sure you want to remove all files from the list?"),
            QMessageBox::Yes | QMessageBox::No);
        
        if (result == QMessageBox::Yes) {
            m_fileListWidget->clear();
            m_stackedWidget->setCurrentWidget(m_dropZone);
            m_previewWidget->clear();
            m_statusLabel->setText(tr("Ready"));
        }
    }
}

void MainWindow::onStartConversion()
{
    if (m_fileListWidget->fileCount() == 0) {
        QMessageBox::warning(this, tr("No Files"),
            tr("Please add files to convert."));
        return;
    }
    
    // Check output folder
    QString outputFolder = Settings::instance().outputFolder();
    if (outputFolder.isEmpty() && !Settings::instance().overwriteOriginal()) {
        onSelectOutputFolder();
        if (Settings::instance().outputFolder().isEmpty()) {
            return;
        }
    }
    
    m_isProcessing = true;
    
    // Update UI
    m_btnStart->setEnabled(false);
    m_btnPause->setEnabled(true);
    m_btnStop->setEnabled(true);
    m_btnAddFiles->setEnabled(false);
    m_btnAddFolder->setEnabled(false);
    m_btnRemove->setEnabled(false);
    m_btnClear->setEnabled(false);
    
    m_globalProgress->setValue(0);
    m_globalProgress->setVisible(true);
    m_progressWidget->setVisible(true);
    
    // Create jobs and start processing
    auto files = m_fileListWidget->allFiles();
    for (const auto& file : files) {
        m_jobQueue->addJob(file, Settings::instance());
    }
    
    m_jobQueue->start();
    
    m_statusLabel->setText(tr("Processing %1 file(s)...").arg(files.count()));
    Logger::info(QString("Started processing %1 files").arg(files.count()));
}

void MainWindow::onPauseConversion()
{
    if (m_jobQueue->isPaused()) {
        m_jobQueue->resume();
        m_btnPause->setText(tr("Pause"));
        m_statusLabel->setText(tr("Processing..."));
    } else {
        m_jobQueue->pause();
        m_btnPause->setText(tr("Resume"));
        m_statusLabel->setText(tr("Paused"));
    }
}

void MainWindow::onStopConversion()
{
    auto result = QMessageBox::question(this, tr("Stop Conversion"),
        tr("Are you sure you want to stop the conversion?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        m_jobQueue->stopAll();
        
        m_isProcessing = false;
        
        m_btnStart->setEnabled(true);
        m_btnPause->setEnabled(false);
        m_btnPause->setText(tr("Pause"));
        m_btnStop->setEnabled(false);
        m_btnAddFiles->setEnabled(true);
        m_btnAddFolder->setEnabled(true);
        m_btnRemove->setEnabled(true);
        m_btnClear->setEnabled(true);
        
        m_globalProgress->setVisible(false);
        m_progressWidget->setVisible(false);
        
        m_statusLabel->setText(tr("Conversion stopped"));
        Logger::info("Conversion stopped by user");
    }
}

void MainWindow::onOpenSettings()
{
    if (!m_settingsDialog) {
        m_settingsDialog = new SettingsDialog(this);
        connect(m_settingsDialog, &SettingsDialog::settingsChanged, [this]() {
            updateStatusBar();
        });
    }
    
    m_settingsDialog->exec();
}

void MainWindow::onToggleTheme()
{
    auto& themeManager = ThemeManager::instance();
    QString currentTheme = Settings::instance().theme();
    
    QString newTheme = (currentTheme == "dark") ? "light" : "dark";
    themeManager.applyTheme(newTheme);
    
    Settings::instance().setTheme(newTheme);
    Settings::instance().save();
}

void MainWindow::onFileDoubleClicked(const QString& filePath)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

void MainWindow::onJobProgress(const QString& jobId, int progress)
{
    m_fileListWidget->updateProgress(jobId, progress);
    m_progressWidget->updateJob(jobId, progress);
    
    // Update global progress
    int totalProgress = m_jobQueue->totalProgress();
    m_globalProgress->setValue(totalProgress);
}

void MainWindow::onJobCompleted(const QString& jobId)
{
    m_fileListWidget->setJobStatus(jobId, FileListWidget::Status::Completed);
    m_progressWidget->setJobCompleted(jobId);
    
    Logger::info(QString("Job completed: %1").arg(jobId));
}

void MainWindow::onJobFailed(const QString& jobId, const QString& error)
{
    m_fileListWidget->setJobStatus(jobId, FileListWidget::Status::Failed);
    m_progressWidget->setJobFailed(jobId, error);
    
    Logger::error(QString("Job failed: %1 - %2").arg(jobId, error));
}

void MainWindow::onAllJobsCompleted()
{
    m_isProcessing = false;
    
    m_btnStart->setEnabled(true);
    m_btnPause->setEnabled(false);
    m_btnPause->setText(tr("Pause"));
    m_btnStop->setEnabled(false);
    m_btnAddFiles->setEnabled(true);
    m_btnAddFolder->setEnabled(true);
    m_btnRemove->setEnabled(true);
    m_btnClear->setEnabled(true);
    
    m_globalProgress->setValue(100);
    
    auto stats = m_jobQueue->statistics();
    m_statusLabel->setText(tr("Completed: %1 successful, %2 failed")
        .arg(stats.completed)
        .arg(stats.failed));
    
    Logger::info(QString("All jobs completed: %1 successful, %2 failed")
        .arg(stats.completed)
        .arg(stats.failed));
    
    // Show notification
    if (m_trayIcon) {
        m_trayIcon->showMessage(
            tr("Conversion Complete"),
            tr("%1 file(s) processed successfully").arg(stats.completed),
            QSystemTrayIcon::Information,
            3000
        );
    }
    
    // Play completion sound
    // TODO: Add completion sound
}

void MainWindow::onShowAbout()
{
    QString aboutText = tr(
        "<h2>DFCompressor</h2>"
        "<p>Version %1</p>"
        "<p>Advanced Media Converter & Compressor</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>Lossless image compression (PNG, WebP, AVIF, JPEG XL)</li>"
        "<li>Lossless video compression (AV1, H.265)</li>"
        "<li>GPU acceleration (NVIDIA NVENC)</li>"
        "<li>Batch processing</li>"
        "</ul>"
        "<p>© 2024-2025 DuckForge</p>"
    ).arg(MEDIAFORGE_VERSION);
    
    QMessageBox::about(this, tr("About DFCompressor"), aboutText);
}

void MainWindow::onSelectOutputFolder()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        tr("Select Output Folder"),
        m_lastOutputFolder.isEmpty() 
            ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            : m_lastOutputFolder,
        QFileDialog::ShowDirsOnly
    );
    
    if (!folder.isEmpty()) {
        m_lastOutputFolder = folder;
        Settings::instance().setOutputFolder(folder);
        Settings::instance().save();
        
        m_statusLabel->setText(tr("Output folder: %1").arg(folder));
    }
}
