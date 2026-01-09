/**
 * @file DropZone.h
 * @brief Drag and drop zone widget header
 */

#ifndef DROPZONE_H
#define DROPZONE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class DropZone : public QWidget
{
    Q_OBJECT

public:
    explicit DropZone(QWidget *parent = nullptr);
    ~DropZone() = default;

signals:
    void filesDropped(const QStringList& files);
    void browseClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUI();
    QStringList extractFiles(const QList<QUrl>& urls);

private:
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    QPushButton* m_browseButton = nullptr;
    bool m_isDragOver = false;
};

#endif // DROPZONE_H
