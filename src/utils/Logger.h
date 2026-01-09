/**
 * @file Logger.h
 * @brief Logging utility header
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QMutex>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

class Logger
{
public:
    static Logger& instance();

    void initialize();
    void shutdown();

    void setLogLevel(LogLevel level);
    void setFileLogging(bool enabled);
    void setConsoleLogging(bool enabled);

    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);
    static void critical(const QString& message);

    void log(LogLevel level, const QString& message);

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QString levelToString(LogLevel level) const;
    QString getLogFilePath() const;

private:
    QFile m_logFile;
    QMutex m_mutex;
    LogLevel m_logLevel = LogLevel::Info;
    bool m_fileLogging = true;
    bool m_consoleLogging = true;
    bool m_initialized = false;
};

#endif // LOGGER_H
