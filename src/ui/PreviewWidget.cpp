/**
 * @file PreviewWidget.cpp
 * @brief Media preview widget implementation
 */

#include "PreviewWidget.h"

#include <QVBoxLayout>
#include <QFileInfo>
#include <QImageReader>
#include <QDateTime>
#include <QAudioOutput>

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

PreviewWidget::~PreviewWidget()
{
    if (m_mediaPlayer) {
        m_mediaPlayer->stop();
    }
}

void PreviewWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Header
    auto* header = new QLabel(tr("Preview"));
    header->setStyleSheet("font-weight: 600; font-size: 14px; color: #ffffff; padding: 8px;");
    layout->addWidget(header);
    
    // Stacked widget for different preview types
    m_stackedWidget = new QStackedWidget;
    m_stackedWidget->setStyleSheet(R"(
        QStackedWidget {
            border: 1px solid #3d3d3d;
            border-radius: 8px;
            background: #1e1e1e;
        }
    )");
    
    // No preview placeholder
    m_noPreviewLabel = new QLabel(tr("Select a file to preview"));
    m_noPreviewLabel->setAlignment(Qt::AlignCenter);
    m_noPreviewLabel->setStyleSheet("color: #666666; font-size: 14px;");
    m_stackedWidget->addWidget(m_noPreviewLabel);
    
    // Image preview
    m_imageLabel = new QLabel;
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setScaledContents(false);
    m_imageLabel->setMinimumSize(200, 200);
    m_stackedWidget->addWidget(m_imageLabel);
    
    // Video preview
    m_videoWidget = new QVideoWidget;
    m_mediaPlayer = new QMediaPlayer;
    auto* audioOutput = new QAudioOutput;
    m_mediaPlayer->setAudioOutput(audioOutput);
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    m_stackedWidget->addWidget(m_videoWidget);
    
    layout->addWidget(m_stackedWidget, 1);
    
    // Info label
    m_infoLabel = new QLabel;
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet(R"(
        QLabel {
            background: #252525;
            border: 1px solid #3d3d3d;
            border-radius: 8px;
            padding: 12px;
            font-size: 12px;
            color: #cccccc;
        }
    )");
    m_infoLabel->setVisible(false);
    layout->addWidget(m_infoLabel);
}

void PreviewWidget::loadPreview(const QString& filePath)
{
    m_currentPath = filePath;
    
    // Stop any playing video
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->stop();
    }
    
    if (isImageFile(filePath)) {
        showImage(filePath);
    } else if (isVideoFile(filePath)) {
        showVideo(filePath);
    } else {
        m_stackedWidget->setCurrentWidget(m_noPreviewLabel);
    }
    
    showInfo(filePath);
}

void PreviewWidget::clear()
{
    m_currentPath.clear();
    m_stackedWidget->setCurrentWidget(m_noPreviewLabel);
    m_infoLabel->setVisible(false);
    
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->stop();
    }
}

bool PreviewWidget::isImageFile(const QString& path) const
{
    static const QStringList imageExts = {
        "png", "jpg", "jpeg", "webp", "avif", "heic", "heif", 
        "tiff", "tif", "bmp", "gif", "jxl", "ico"
    };
    
    QFileInfo info(path);
    return imageExts.contains(info.suffix().toLower());
}

bool PreviewWidget::isVideoFile(const QString& path) const
{
    static const QStringList videoExts = {
        "mp4", "mkv", "avi", "mov", "webm", "wmv", "flv", "m4v"
    };
    
    QFileInfo info(path);
    return videoExts.contains(info.suffix().toLower());
}

void PreviewWidget::showImage(const QString& path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    
    QImage image = reader.read();
    
    if (!image.isNull()) {
        // Scale to fit widget while maintaining aspect ratio
        QPixmap pixmap = QPixmap::fromImage(image);
        QSize targetSize = m_imageLabel->size() - QSize(20, 20);
        
        if (pixmap.width() > targetSize.width() || 
            pixmap.height() > targetSize.height()) {
            pixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatio, 
                                   Qt::SmoothTransformation);
        }
        
        m_imageLabel->setPixmap(pixmap);
        m_stackedWidget->setCurrentWidget(m_imageLabel);
    } else {
        m_noPreviewLabel->setText(tr("Cannot load image"));
        m_stackedWidget->setCurrentWidget(m_noPreviewLabel);
    }
}

void PreviewWidget::showVideo(const QString& path)
{
    m_mediaPlayer->setSource(QUrl::fromLocalFile(path));
    m_stackedWidget->setCurrentWidget(m_videoWidget);
    
    // Auto-play video preview (muted)
    m_mediaPlayer->audioOutput()->setVolume(0);
    m_mediaPlayer->play();
}

void PreviewWidget::showInfo(const QString& path)
{
    QFileInfo info(path);
    
    QString infoText;
    infoText += QString("<b>%1</b><br>").arg(tr("File Information"));
    infoText += QString("%1: %2<br>").arg(tr("Name"), info.fileName());
    infoText += QString("%1: %2<br>").arg(tr("Size"), formatFileSize(info.size()));
    infoText += QString("%1: %2<br>").arg(tr("Type"), info.suffix().toUpper());
    infoText += QString("%1: %2").arg(tr("Modified"), 
        info.lastModified().toString("yyyy-MM-dd hh:mm"));
    
    // Add image dimensions if it's an image
    if (isImageFile(path)) {
        QImageReader reader(path);
        QSize size = reader.size();
        if (size.isValid()) {
            infoText += QString("<br>%1: %2 × %3")
                .arg(tr("Dimensions"))
                .arg(size.width())
                .arg(size.height());
        }
    }
    
    m_infoLabel->setText(infoText);
    m_infoLabel->setVisible(true);
}

QString PreviewWidget::formatFileSize(qint64 bytes) const
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
