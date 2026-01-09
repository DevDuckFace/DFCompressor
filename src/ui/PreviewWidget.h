/**
 * @file PreviewWidget.h
 * @brief Media preview widget header
 */

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QStackedWidget>

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();

    void loadPreview(const QString& filePath);
    void clear();

private:
    void setupUI();
    bool isImageFile(const QString& path) const;
    bool isVideoFile(const QString& path) const;
    void showImage(const QString& path);
    void showVideo(const QString& path);
    void showInfo(const QString& path);
    QString formatFileSize(qint64 bytes) const;

private:
    QStackedWidget* m_stackedWidget = nullptr;
    QLabel* m_imageLabel = nullptr;
    QVideoWidget* m_videoWidget = nullptr;
    QMediaPlayer* m_mediaPlayer = nullptr;
    QLabel* m_infoLabel = nullptr;
    QLabel* m_noPreviewLabel = nullptr;
    
    QString m_currentPath;
};

#endif // PREVIEWWIDGET_H
