/**
 * @file DropZone.cpp
 * @brief Drag and drop zone widget implementation
 */

#include "DropZone.h"
#include "FileUtils.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>

DropZone::DropZone(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
    setupUI();
}

void DropZone::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(16);
    
    // Icon
    m_iconLabel = new QLabel;
    m_iconLabel->setPixmap(QIcon(":/icons/drop_zone.svg").pixmap(96, 96));
    m_iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_iconLabel);
    
    // Title
    m_titleLabel = new QLabel(tr("Drop files here"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 24px;
            font-weight: 600;
            color: #ffffff;
        }
    )");
    layout->addWidget(m_titleLabel);
    
    // Subtitle
    m_subtitleLabel = new QLabel(tr("or drag a folder to add all files"));
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 14px;
            color: #888888;
        }
    )");
    layout->addWidget(m_subtitleLabel);
    
    // Supported formats info
    auto* formatsLabel = new QLabel(
        tr("Supports: PNG, JPG, WebP, AVIF, HEIC, TIFF, MP4, MKV, AVI, MOV, WebM")
    );
    formatsLabel->setAlignment(Qt::AlignCenter);
    formatsLabel->setStyleSheet(R"(
        QLabel {
            font-size: 12px;
            color: #666666;
            padding: 8px;
        }
    )");
    layout->addWidget(formatsLabel);
    
    // Browse button
    m_browseButton = new QPushButton(tr("Browse Files"));
    m_browseButton->setFixedSize(160, 44);
    m_browseButton->setCursor(Qt::PointingHandCursor);
    m_browseButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
            border: none;
            border-radius: 22px;
            font-size: 14px;
            font-weight: 600;
            padding: 12px 24px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7b8fed, stop:1 #8b5cb8);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #5269d6, stop:1 #623d8e);
        }
    )");
    
    auto* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_browseButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
    
    connect(m_browseButton, &QPushButton::clicked, 
            this, &DropZone::browseClicked);
    
    setMinimumSize(400, 300);
}

void DropZone::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    QColor bgColor = m_isDragOver ? QColor(102, 126, 234, 30) : QColor(30, 30, 30);
    
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 16, 16);
    
    painter.fillPath(path, bgColor);
    
    // Border
    QPen borderPen;
    if (m_isDragOver) {
        borderPen = QPen(QColor(102, 126, 234), 3, Qt::DashLine);
    } else {
        borderPen = QPen(QColor(60, 60, 60), 2, Qt::DashLine);
    }
    
    painter.setPen(borderPen);
    painter.drawPath(path);
}

void DropZone::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        m_isDragOver = true;
        event->acceptProposedAction();
        update();
    }
}

void DropZone::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    m_isDragOver = false;
    update();
}

void DropZone::dropEvent(QDropEvent *event)
{
    m_isDragOver = false;
    update();
    
    QStringList files = extractFiles(event->mimeData()->urls());
    
    if (!files.isEmpty()) {
        emit filesDropped(files);
    }
}

QStringList DropZone::extractFiles(const QList<QUrl>& urls)
{
    QStringList files;
    
    for (const QUrl& url : urls) {
        if (!url.isLocalFile()) continue;
        
        QString path = url.toLocalFile();
        QFileInfo info(path);
        
        if (info.isDir()) {
            // Recursively add files from directory
            files.append(FileUtils::scanDirectory(path, true));
        } else if (FileUtils::isSupportedFile(path)) {
            files.append(path);
        }
    }
    
    return files;
}
