/**
 * @file FileListWidget.cpp
 * @brief File list widget implementation
 */

#include "FileListWidget.h"
#include "Settings.h"
#include "FormatUtils.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QFileInfo>
#include <QUuid>
#include <QDesktopServices>
#include <QUrl>
#include <QContextMenuEvent>

FileListWidget::FileListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void FileListWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_treeWidget = new QTreeWidget;
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeWidget->setSortingEnabled(true);
    m_treeWidget->setContextMenuPolicy(Qt::DefaultContextMenu);
    
    // Set up columns
    QStringList headers;
    headers << tr("Status") << tr("Name") << tr("Type") << tr("Input Format")
            << tr("Output Format") << tr("Size") << tr("Output Size") 
            << tr("Compression") << tr("Progress");
    m_treeWidget->setHeaderLabels(headers);
    
    // Adjust column widths
    auto* header = m_treeWidget->header();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->resizeSection(0, 50);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Fixed);
    header->resizeSection(2, 70);
    header->setSectionResizeMode(3, QHeaderView::Fixed);
    header->resizeSection(3, 100);
    header->setSectionResizeMode(4, QHeaderView::Fixed);
    header->resizeSection(4, 100);
    header->setSectionResizeMode(5, QHeaderView::Fixed);
    header->resizeSection(5, 90);
    header->setSectionResizeMode(6, QHeaderView::Fixed);
    header->resizeSection(6, 90);
    header->setSectionResizeMode(7, QHeaderView::Fixed);
    header->resizeSection(7, 90);
    header->setSectionResizeMode(8, QHeaderView::Fixed);
    header->resizeSection(8, 120);
    
    // Styling
    m_treeWidget->setStyleSheet(R"(
        QTreeWidget {
            border: 1px solid #3d3d3d;
            border-radius: 8px;
            background-color: #1e1e1e;
            font-size: 13px;
        }
        QTreeWidget::item {
            height: 36px;
            padding: 4px;
        }
        QTreeWidget::item:selected {
            background-color: #0078d4;
        }
        QTreeWidget::item:hover:!selected {
            background-color: #2d2d2d;
        }
        QHeaderView::section {
            background-color: #2d2d2d;
            color: #ffffff;
            padding: 8px;
            border: none;
            border-bottom: 1px solid #3d3d3d;
            font-weight: 600;
        }
    )");
    
    layout->addWidget(m_treeWidget);
    
    // Connections
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked,
            [this](QTreeWidgetItem* item, int) {
        QString jobId = item->data(0, Qt::UserRole).toString();
        if (m_items.contains(jobId)) {
            emit fileDoubleClicked(m_items[jobId].path);
        }
    });
    
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged,
            this, &FileListWidget::selectionChanged);
}

void FileListWidget::addFile(const QString& filePath)
{
    QFileInfo info(filePath);
    if (!info.exists()) return;
    
    // Check if file is already added
    for (const auto& item : m_items) {
        if (item.path == filePath) return;
    }
    
    FileItem fileItem;
    fileItem.id = generateJobId();
    fileItem.path = filePath;
    fileItem.name = info.fileName();
    fileItem.inputFormat = info.suffix().toUpper();
    fileItem.originalSize = info.size();
    fileItem.outputSize = 0;
    fileItem.progress = 0;
    fileItem.status = static_cast<int>(Status::Pending);
    
    // Determine file type
    fileItem.type = getFileType(info.suffix().toLower());
    
    // Determine output format based on settings
    fileItem.outputFormat = FormatUtils::getOutputFormat(
        fileItem.type, 
        fileItem.inputFormat,
        Settings::instance()
    );
    
    m_items[fileItem.id] = fileItem;
    
    // Create tree item
    auto* treeItem = new QTreeWidgetItem;
    treeItem->setData(0, Qt::UserRole, fileItem.id);
    updateItemDisplay(treeItem, fileItem);
    
    m_treeWidget->addTopLevelItem(treeItem);
    
    emit filesAdded(1);
}

void FileListWidget::addFiles(const QStringList& filePaths)
{
    for (const QString& path : filePaths) {
        addFile(path);
    }
}

void FileListWidget::removeSelected()
{
    auto selected = m_treeWidget->selectedItems();
    int count = selected.count();
    
    for (auto* item : selected) {
        QString jobId = item->data(0, Qt::UserRole).toString();
        m_items.remove(jobId);
        delete item;
    }
    
    if (count > 0) {
        emit filesRemoved(count);
    }
}

void FileListWidget::clear()
{
    m_treeWidget->clear();
    m_items.clear();
    m_jobCounter = 0;
}

int FileListWidget::fileCount() const
{
    return m_items.count();
}

QString FileListWidget::selectedFilePath() const
{
    auto selected = m_treeWidget->selectedItems();
    if (selected.isEmpty()) return QString();
    
    QString jobId = selected.first()->data(0, Qt::UserRole).toString();
    if (m_items.contains(jobId)) {
        return m_items[jobId].path;
    }
    return QString();
}

QStringList FileListWidget::allFiles() const
{
    QStringList files;
    for (const auto& item : m_items) {
        files.append(item.path);
    }
    return files;
}

QList<FileItem> FileListWidget::allItems() const
{
    return m_items.values();
}

void FileListWidget::updateProgress(const QString& jobId, int progress)
{
    if (!m_items.contains(jobId)) return;
    
    m_items[jobId].progress = progress;
    m_items[jobId].status = static_cast<int>(Status::Processing);
    
    // Find and update tree item
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        auto* item = m_treeWidget->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == jobId) {
            updateItemDisplay(item, m_items[jobId]);
            break;
        }
    }
}

void FileListWidget::setJobStatus(const QString& jobId, Status status)
{
    if (!m_items.contains(jobId)) return;
    
    m_items[jobId].status = static_cast<int>(status);
    if (status == Status::Completed) {
        m_items[jobId].progress = 100;
    }
    
    // Find and update tree item
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        auto* item = m_treeWidget->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == jobId) {
            updateItemDisplay(item, m_items[jobId]);
            break;
        }
    }
}

void FileListWidget::setOutputSize(const QString& jobId, qint64 size)
{
    if (!m_items.contains(jobId)) return;
    
    m_items[jobId].outputSize = size;
    
    // Find and update tree item
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        auto* item = m_treeWidget->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == jobId) {
            updateItemDisplay(item, m_items[jobId]);
            break;
        }
    }
}

void FileListWidget::contextMenuEvent(QContextMenuEvent *event)
{
    auto* item = m_treeWidget->itemAt(m_treeWidget->mapFromParent(event->pos()));
    if (!item) return;
    
    QMenu menu(this);
    
    menu.addAction(QIcon(":/icons/open.svg"), tr("Open File"), [this, item]() {
        QString jobId = item->data(0, Qt::UserRole).toString();
        if (m_items.contains(jobId)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_items[jobId].path));
        }
    });
    
    menu.addAction(QIcon(":/icons/folder.svg"), tr("Open Folder"), [this, item]() {
        QString jobId = item->data(0, Qt::UserRole).toString();
        if (m_items.contains(jobId)) {
            QFileInfo info(m_items[jobId].path);
            QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
        }
    });
    
    menu.addSeparator();
    
    menu.addAction(QIcon(":/icons/remove.svg"), tr("Remove"), [this]() {
        removeSelected();
    });
    
    menu.exec(event->globalPos());
}

void FileListWidget::updateItemDisplay(QTreeWidgetItem* item, const FileItem& fileItem)
{
    Status status = static_cast<Status>(fileItem.status);
    
    // Status icon
    item->setIcon(0, getStatusIcon(status));
    
    // File type icon
    item->setIcon(1, getFileTypeIcon(fileItem.type));
    item->setText(1, fileItem.name);
    
    // Type
    item->setText(2, fileItem.type == "image" ? tr("Image") : tr("Video"));
    
    // Formats
    item->setText(3, fileItem.inputFormat);
    item->setText(4, fileItem.outputFormat);
    
    // Original size
    item->setText(5, formatFileSize(fileItem.originalSize));
    
    // Output size
    if (fileItem.outputSize > 0) {
        item->setText(6, formatFileSize(fileItem.outputSize));
        
        // Compression ratio
        double ratio = 100.0 * (1.0 - (double)fileItem.outputSize / fileItem.originalSize);
        QString compressionText = QString("%1%").arg(ratio, 0, 'f', 1);
        if (ratio > 0) {
            compressionText = QString("-%1%").arg(ratio, 0, 'f', 1);
            item->setForeground(7, QBrush(QColor("#4CAF50")));
        } else if (ratio < 0) {
            compressionText = QString("+%1%").arg(-ratio, 0, 'f', 1);
            item->setForeground(7, QBrush(QColor("#f44336")));
        }
        item->setText(7, compressionText);
    } else {
        item->setText(6, "-");
        item->setText(7, "-");
    }
    
    // Progress
    QString progressText;
    switch (status) {
        case Status::Pending:
            progressText = tr("Pending");
            break;
        case Status::Processing:
            progressText = QString("%1%").arg(fileItem.progress);
            break;
        case Status::Completed:
            progressText = tr("✓ Completed");
            item->setForeground(8, QBrush(QColor("#4CAF50")));
            break;
        case Status::Failed:
            progressText = tr("✗ Failed");
            item->setForeground(8, QBrush(QColor("#f44336")));
            break;
    }
    item->setText(8, progressText);
}

QString FileListWidget::formatFileSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / (double)GB, 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / (double)MB, 0, 'f', 2);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / (double)KB, 0, 'f', 1);
    } else {
        return QString("%1 B").arg(bytes);
    }
}

QString FileListWidget::getFileType(const QString& extension) const
{
    static const QStringList imageExts = {
        "png", "jpg", "jpeg", "webp", "avif", "heic", "heif", 
        "tiff", "tif", "bmp", "jxl", "gif"
    };
    static const QStringList videoExts = {
        "mp4", "mkv", "avi", "mov", "webm", "wmv", "flv", "m4v"
    };
    
    if (imageExts.contains(extension)) {
        return "image";
    } else if (videoExts.contains(extension)) {
        return "video";
    }
    return "unknown";
}

QString FileListWidget::generateJobId() const
{
    return QString("job_%1_%2")
        .arg(++const_cast<FileListWidget*>(this)->m_jobCounter)
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces).left(8));
}

QIcon FileListWidget::getStatusIcon(Status status) const
{
    switch (status) {
        case Status::Pending:
            return QIcon(":/icons/status_pending.svg");
        case Status::Processing:
            return QIcon(":/icons/status_processing.svg");
        case Status::Completed:
            return QIcon(":/icons/status_completed.svg");
        case Status::Failed:
            return QIcon(":/icons/status_failed.svg");
    }
    return QIcon();
}

QIcon FileListWidget::getFileTypeIcon(const QString& type) const
{
    if (type == "image") {
        return QIcon(":/icons/file_image.svg");
    } else if (type == "video") {
        return QIcon(":/icons/file_video.svg");
    }
    return QIcon(":/icons/file_unknown.svg");
}
