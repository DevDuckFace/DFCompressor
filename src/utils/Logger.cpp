/**
 * @file Logger.cpp
 * @brief Logging utility implementation
 */

#include "Logger.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QDebug>

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::~Logger()
{
    shutdown();
}

void Logger::initialize()
{
    if (m_initialized) return;

    if (m_fileLogging) {
        QString logPath = getLogFilePath();
        QDir().mkpath(QFileInfo(logPath).absolutePath());
        
        m_logFile.setFileName(logPath);
        m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }

    m_initialized = true;
    info("Logger initialized");
}

void Logger::shutdown()
{
    if (!m_initialized) return;

    info("Logger shutting down");

    if (m_logFile.isOpen()) {
        m_logFile.close();
    }

    m_initialized = false;
}

void Logger::setLogLevel(LogLevel level)
{
    m_logLevel = level;
}

void Logger::setFileLogging(bool enabled)
{
    m_fileLogging = enabled;
}

void Logger::setConsoleLogging(bool enabled)
{
    m_consoleLogging = enabled;
}

void Logger::debug(const QString& message)
{
    instance().log(LogLevel::Debug, message);
}

void Logger::info(const QString& message)
{
    instance().log(LogLevel::Info, message);
}

void Logger::warning(const QString& message)
{
    instance().log(LogLevel::Warning, message);
}

void Logger::error(const QString& message)
{
    instance().log(LogLevel::Error, message);
}

void Logger::critical(const QString& message)
{
    instance().log(LogLevel::Critical, message);
}

void Logger::log(LogLevel level, const QString& message)
{
    if (level < m_logLevel) return;

    QMutexLocker locker(&m_mutex);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = levelToString(level);
    QString logLine = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);

    // Console output
    if (m_consoleLogging) {
        switch (level) {
            case LogLevel::Debug:
                qDebug().noquote() << logLine;
                break;
            case LogLevel::Info:
                qInfo().noquote() << logLine;
                break;
            case LogLevel::Warning:
                qWarning().noquote() << logLine;
                break;
            case LogLevel::Error:
            case LogLevel::Critical:
                qCritical().noquote() << logLine;
                break;
        }
    }

    // File output
    if (m_fileLogging && m_logFile.isOpen()) {
        QTextStream stream(&m_logFile);
        stream << logLine << "\n";
        stream.flush();
    }
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO ";
        case LogLevel::Warning:  return "WARN ";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT ";
    }
    return "?????";
}

QString Logger::getLogFilePath() const
{
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    return QString("%1/logs/mediaforge_%2.log").arg(logDir, date);
}
