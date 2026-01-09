/**
 * @file ProgressWidget.cpp
 * @brief Progress widget implementation
 */

#include "ProgressWidget.h"

#include <QLabel>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QFrame>

ProgressWidget::ProgressWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ProgressWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Header
    auto* header = new QLabel(tr("Processing Queue"));
    header->setStyleSheet("font-weight: 600; font-size: 14px; color: #ffffff; padding: 8px;");
    layout->addWidget(header);
    
    // Scroll area for jobs
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea {
            border: 1px solid #3d3d3d;
            border-radius: 8px;
            background: #1e1e1e;
        }
    )");
    
    auto* scrollContent = new QWidget;
    m_jobsLayout = new QVBoxLayout(scrollContent);
    m_jobsLayout->setContentsMargins(8, 8, 8, 8);
    m_jobsLayout->setSpacing(8);
    m_jobsLayout->addStretch();
    
    m_scrollArea->setWidget(scrollContent);
    layout->addWidget(m_scrollArea);
    
    setMaximumHeight(300);
}

void ProgressWidget::addJob(const QString& jobId, const QString& fileName)
{
    if (m_jobWidgets.contains(jobId)) return;
    
    auto* jobWidget = createJobWidget(jobId, fileName);
    m_jobWidgets[jobId] = jobWidget;
    
    // Insert before the stretch
    m_jobsLayout->insertWidget(m_jobsLayout->count() - 1, jobWidget);
}

void ProgressWidget::updateJob(const QString& jobId, int progress)
{
    if (!m_jobWidgets.contains(jobId)) return;
    
    auto* widget = m_jobWidgets[jobId];
    auto* progressBar = widget->findChild<QProgressBar*>();
    auto* statusLabel = widget->findChild<QLabel*>("statusLabel");
    
    if (progressBar) {
        progressBar->setValue(progress);
    }
    
    if (statusLabel) {
        statusLabel->setText(QString("%1%").arg(progress));
    }
}

void ProgressWidget::setJobCompleted(const QString& jobId)
{
    if (!m_jobWidgets.contains(jobId)) return;
    
    auto* widget = m_jobWidgets[jobId];
    auto* progressBar = widget->findChild<QProgressBar*>();
    auto* statusLabel = widget->findChild<QLabel*>("statusLabel");
    
    if (progressBar) {
        progressBar->setValue(100);
        progressBar->setStyleSheet(R"(
            QProgressBar {
                border: none;
                border-radius: 4px;
                background: #2d2d2d;
                height: 8px;
            }
            QProgressBar::chunk {
                background: #4CAF50;
                border-radius: 4px;
            }
        )");
    }
    
    if (statusLabel) {
        statusLabel->setText(tr("✓ Done"));
        statusLabel->setStyleSheet("color: #4CAF50; font-weight: 500;");
    }
}

void ProgressWidget::setJobFailed(const QString& jobId, const QString& error)
{
    if (!m_jobWidgets.contains(jobId)) return;
    
    auto* widget = m_jobWidgets[jobId];
    auto* progressBar = widget->findChild<QProgressBar*>();
    auto* statusLabel = widget->findChild<QLabel*>("statusLabel");
    
    if (progressBar) {
        progressBar->setStyleSheet(R"(
            QProgressBar {
                border: none;
                border-radius: 4px;
                background: #2d2d2d;
                height: 8px;
            }
            QProgressBar::chunk {
                background: #f44336;
                border-radius: 4px;
            }
        )");
    }
    
    if (statusLabel) {
        statusLabel->setText(tr("✗ Failed"));
        statusLabel->setToolTip(error);
        statusLabel->setStyleSheet("color: #f44336; font-weight: 500;");
    }
}

void ProgressWidget::clear()
{
    for (auto* widget : m_jobWidgets) {
        m_jobsLayout->removeWidget(widget);
        delete widget;
    }
    m_jobWidgets.clear();
}

QWidget* ProgressWidget::createJobWidget(const QString& jobId, const QString& fileName)
{
    Q_UNUSED(jobId)
    
    auto* frame = new QFrame;
    frame->setStyleSheet(R"(
        QFrame {
            background: #252525;
            border-radius: 6px;
            padding: 8px;
        }
    )");
    
    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);
    
    // Top row: filename and status
    auto* topRow = new QHBoxLayout;
    
    auto* nameLabel = new QLabel(fileName);
    nameLabel->setStyleSheet("color: #ffffff; font-weight: 500;");
    nameLabel->setMaximumWidth(300);
    nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    topRow->addWidget(nameLabel, 1);
    
    auto* statusLabel = new QLabel(tr("Starting..."));
    statusLabel->setObjectName("statusLabel");
    statusLabel->setStyleSheet("color: #888888;");
    topRow->addWidget(statusLabel);
    
    layout->addLayout(topRow);
    
    // Progress bar
    auto* progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(8);
    progressBar->setStyleSheet(R"(
        QProgressBar {
            border: none;
            border-radius: 4px;
            background: #2d2d2d;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            border-radius: 4px;
        }
    )");
    layout->addWidget(progressBar);
    
    return frame;
}
