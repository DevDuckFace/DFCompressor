/**
 * @file ProgressWidget.h
 * @brief Progress widget for job monitoring
 */

#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QMap>
#include <QLabel>
#include <QProgressBar>

class ProgressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressWidget(QWidget *parent = nullptr);
    ~ProgressWidget() = default;

    void addJob(const QString& jobId, const QString& fileName);
    void updateJob(const QString& jobId, int progress);
    void setJobCompleted(const QString& jobId);
    void setJobFailed(const QString& jobId, const QString& error);
    void clear();

private:
    void setupUI();
    QWidget* createJobWidget(const QString& jobId, const QString& fileName);

private:
    QScrollArea* m_scrollArea = nullptr;
    QVBoxLayout* m_jobsLayout = nullptr;
    QMap<QString, QWidget*> m_jobWidgets;
};

#endif // PROGRESSWIDGET_H
