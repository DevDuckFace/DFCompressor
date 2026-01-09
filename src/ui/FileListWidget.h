/**
 * @file FileListWidget.h
 * @brief File list widget for displaying queued files
 */

#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QList>
#include <memory>

struct FileItem {
    QString id;
    QString path;
    QString name;
    QString type;          // "image" or "video"
    QString inputFormat;
    QString outputFormat;
    qint64 originalSize;
    qint64 outputSize;
    int progress;
    int status;
};

class FileListWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Status {
        Pending = 0,
        Processing,
        Completed,
        Failed
    };
    Q_ENUM(Status)

    explicit FileListWidget(QWidget *parent = nullptr);
    ~FileListWidget() = default;

    void addFile(const QString& filePath);
    void addFiles(const QStringList& filePaths);
    void removeSelected();
    void clear();
    
    int fileCount() const;
    QString selectedFilePath() const;
    QStringList allFiles() const;
    QList<FileItem> allItems() const;
    
    void updateProgress(const QString& jobId, int progress);
    void setJobStatus(const QString& jobId, Status status);
    void setOutputSize(const QString& jobId, qint64 size);

signals:
    void fileDoubleClicked(const QString& filePath);
    void selectionChanged();
    void filesAdded(int count);
    void filesRemoved(int count);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void setupUI();
    void updateItemDisplay(QTreeWidgetItem* item, const FileItem& fileItem);
    QString formatFileSize(qint64 bytes) const;
    QString getFileType(const QString& extension) const;
    QString generateJobId() const;
    QIcon getStatusIcon(Status status) const;
    QIcon getFileTypeIcon(const QString& type) const;

private:
    QTreeWidget* m_treeWidget = nullptr;
    QMap<QString, FileItem> m_items;
    int m_jobCounter = 0;
};

#endif // FILELISTWIDGET_H
