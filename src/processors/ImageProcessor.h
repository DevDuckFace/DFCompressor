/**
 * @file ImageProcessor.h
 * @brief Image processor header
 */

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QString>
#include <functional>

class Job;

class ImageProcessor
{
public:
    ImageProcessor();
    ~ImageProcessor();

    bool process(Job* job);
    QString lastError() const { return m_lastError; }

    void setProgressCallback(std::function<void(int)> callback);

private:
    bool processWithVips(Job* job);
    bool processWithQt(Job* job);
    bool convertWithExternalTool(Job* job);
    
    bool convertToJxl(const QString& input, const QString& output, bool lossless);
    bool convertToAvif(const QString& input, const QString& output, bool lossless);
    bool convertToWebp(const QString& input, const QString& output, bool lossless);
    bool convertToPng(const QString& input, const QString& output);
    
    void reportProgress(int progress);

private:
    QString m_lastError;
    std::function<void(int)> m_progressCallback;
    bool m_useVips = false;
};

#endif // IMAGEPROCESSOR_H
