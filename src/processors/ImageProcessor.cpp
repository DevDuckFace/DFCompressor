/**
 * @file ImageProcessor.cpp
 * @brief Image processor implementation
 */

#include "ImageProcessor.h"
#include "Job.h"
#include "Settings.h"
#include "Logger.h"

#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QCoreApplication>

#ifdef MEDIAFORGE_HAS_VIPS
extern "C" {
#include <vips/vips.h>
}
#endif

ImageProcessor::ImageProcessor()
{
#ifdef MEDIAFORGE_HAS_VIPS
    m_useVips = true;
    if (VIPS_INIT("mediaforge") != 0) {
        Logger::warning("Failed to initialize libvips, falling back to Qt");
        m_useVips = false;
    }
#endif
}

ImageProcessor::~ImageProcessor()
{
#ifdef MEDIAFORGE_HAS_VIPS
    if (m_useVips) {
        vips_shutdown();
    }
#endif
}

bool ImageProcessor::process(Job* job)
{
    if (!job) {
        m_lastError = "Invalid job";
        return false;
    }

    Logger::info(QString("Processing image: %1").arg(job->inputPath()));
    reportProgress(5);

    bool success = false;

#ifdef MEDIAFORGE_HAS_VIPS
    if (m_useVips) {
        success = processWithVips(job);
    } else {
        success = processWithQt(job);
    }
#else
    success = processWithQt(job);
#endif

    if (success) {
        QFileInfo outputInfo(job->outputPath());
        job->setOutputSize(outputInfo.size());
        reportProgress(100);
        Logger::info(QString("Image processed successfully: %1").arg(job->outputPath()));
    }

    return success;
}

void ImageProcessor::setProgressCallback(std::function<void(int)> callback)
{
    m_progressCallback = callback;
}

void ImageProcessor::reportProgress(int progress)
{
    if (m_progressCallback) {
        m_progressCallback(progress);
    }
}

bool ImageProcessor::processWithVips(Job* job)
{
#ifdef MEDIAFORGE_HAS_VIPS
    const auto& settings = Settings::instance();
    QString outputFormat = job->outputFormat().toLower();
    bool lossless = settings.imageCompressionMode() == "lossless";

    reportProgress(20);

    VipsImage* image = nullptr;

    // Load image
    if (vips_image_new_from_file(job->inputPath().toUtf8().constData(),
                                  "access", VIPS_ACCESS_SEQUENTIAL,
                                  nullptr) == nullptr) {
        m_lastError = "Failed to load image with libvips";
        return false;
    }

    reportProgress(40);

    // Save with appropriate format
    int result = -1;

    if (outputFormat == "jxl") {
        result = vips_jxlsave(image, job->outputPath().toUtf8().constData(),
                              "lossless", lossless ? TRUE : FALSE,
                              "effort", settings.jpegXlEffort(),
                              nullptr);
    } else if (outputFormat == "avif") {
        result = vips_heifsave(image, job->outputPath().toUtf8().constData(),
                               "compression", VIPS_FOREIGN_HEIF_COMPRESSION_AV1,
                               "lossless", lossless ? TRUE : FALSE,
                               "speed", settings.avifSpeed(),
                               nullptr);
    } else if (outputFormat == "webp") {
        result = vips_webpsave(image, job->outputPath().toUtf8().constData(),
                               "lossless", lossless ? TRUE : FALSE,
                               "Q", settings.imageQuality(),
                               "effort", settings.webpMethod(),
                               nullptr);
    } else if (outputFormat == "png") {
        result = vips_pngsave(image, job->outputPath().toUtf8().constData(),
                              "compression", 9,
                              nullptr);
    } else {
        m_lastError = QString("Unsupported output format: %1").arg(outputFormat);
        g_object_unref(image);
        return false;
    }

    g_object_unref(image);

    reportProgress(90);

    if (result != 0) {
        m_lastError = QString("Failed to save image: %1").arg(vips_error_buffer());
        vips_error_clear();
        return false;
    }

    return true;
#else
    Q_UNUSED(job)
    return processWithQt(job);
#endif
}

bool ImageProcessor::processWithQt(Job* job)
{
    const auto& settings = Settings::instance();
    QString outputFormat = job->outputFormat().toLower();
    QString outputPath = job->outputPath();

    Logger::info(QString("processWithQt: Input=%1, Output=%2, Format=%3")
        .arg(job->inputPath()).arg(outputPath).arg(outputFormat));

    reportProgress(20);

    // Load image
    QImage image;
    if (!image.load(job->inputPath())) {
        m_lastError = QString("Failed to load image: %1").arg(job->inputPath());
        Logger::error(m_lastError);
        return false;
    }

    Logger::info(QString("Image loaded: %1x%2, format=%3")
        .arg(image.width()).arg(image.height()).arg(image.format()));
    reportProgress(50);

    // Ensure output directory exists
    QFileInfo outputInfo(outputPath);
    QDir outputDir = outputInfo.absoluteDir();
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            m_lastError = QString("Failed to create output directory: %1").arg(outputDir.absolutePath());
            Logger::error(m_lastError);
            return false;
        }
    }

    // For advanced formats (JXL, AVIF, WebP), try vips first
    if (outputFormat == "jxl" || outputFormat == "avif" || outputFormat == "webp") {
        Logger::info("Trying vips for advanced format...");
        if (convertWithExternalTool(job)) {
            return true;
        }
        Logger::warning("Vips failed, falling back to Qt if possible");
    }

    reportProgress(80);

    // Determine quality
    int quality = -1; // -1 means default
    if (outputFormat == "jpg" || outputFormat == "jpeg") {
        quality = settings.imageQuality();
    } else if (outputFormat == "png") {
        quality = 100; // For PNG, this affects compression (100 = maximum compression)
    } else if (outputFormat == "webp") {
        quality = settings.imageCompressionMode() == "lossless" ? 100 : settings.imageQuality();
    }

    // Use QImage::save directly - simpler and more reliable
    Logger::info(QString("Saving with QImage::save, format=%1, quality=%2").arg(outputFormat).arg(quality));
    
    bool saved = false;
    if (outputFormat == "jpg" || outputFormat == "jpeg") {
        saved = image.save(outputPath, "JPG", quality);
    } else if (outputFormat == "png") {
        saved = image.save(outputPath, "PNG", quality);
    } else if (outputFormat == "bmp") {
        saved = image.save(outputPath, "BMP");
    } else if (outputFormat == "webp") {
        saved = image.save(outputPath, "WEBP", quality);
    } else {
        // Try with format string
        saved = image.save(outputPath, outputFormat.toUpper().toUtf8().constData(), quality);
    }

    if (!saved) {
        m_lastError = QString("Failed to save image to: %1").arg(outputPath);
        Logger::error(m_lastError);
        // Clean up empty file if created
        if (QFile::exists(outputPath)) {
            QFile::remove(outputPath);
        }
        return false;
    }

    // Verify output
    QFileInfo finalInfo(outputPath);
    if (!finalInfo.exists() || finalInfo.size() == 0) {
        m_lastError = "Output file is empty or missing";
        Logger::error(m_lastError);
        if (QFile::exists(outputPath)) {
            QFile::remove(outputPath);
        }
        return false;
    }

    Logger::info(QString("Image saved successfully: %1 (%2 bytes)")
        .arg(outputPath).arg(finalInfo.size()));

    return true;
}

bool ImageProcessor::convertWithExternalTool(Job* job)
{
    const auto& settings = Settings::instance();
    QString outputFormat = job->outputFormat().toLower();
    bool lossless = settings.imageCompressionMode() == "lossless";

    // Try using ImageMagick or other tools
    QProcess process;
    QStringList args;

    QString appDir = QCoreApplication::applicationDirPath();
    QString vipsPath = settings.vipsPath(); 
    // Auto-detect bundled vips if not set
    if (vipsPath.isEmpty()) {
        QString bundledVips = appDir + "/vips/bin/vips.exe";
        if (QFileInfo::exists(bundledVips)) {
            vipsPath = bundledVips;
        }
    }

    // Utilize VIPS CLI command: vips copy input output[options]
    if (!vipsPath.isEmpty() && QFileInfo::exists(vipsPath)) {
        args << "copy" << job->inputPath();
        
        QString outputArg = job->outputPath();
        
        // Add encoder options
        QString options;
        if (outputFormat == "jxl") {
            options = QString("effort=%1").arg(settings.jpegXlEffort());
            if (lossless) {
                 options += ",lossless=true";
            } else {
                 options += QString(",Q=%1").arg(settings.imageQuality());
            }
        } else if (outputFormat == "avif") {
            options = QString("speed=%1").arg(settings.avifSpeed());
             if (lossless) {
                 options += ",lossless=true";
            } else {
                 options += QString(",Q=%1").arg(settings.imageQuality());
            }
        } else if (outputFormat == "webp") {
             if (lossless) {
                 options = "lossless=true";  // No leading comma!
            } else {
                 options = QString("Q=%1").arg(settings.imageQuality());
            }
        } else if (outputFormat == "png") {
            // Use vips for PNG too if falling back here
            options = "compression=9";
        }
        
        if (!options.isEmpty()) {
            outputArg += "[" + options + "]";
        }
        
        args << outputArg;

        // CRITICAL: Set up environment so vips can find its DLLs
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString vipsDir = QFileInfo(vipsPath).absolutePath(); // vips/bin
        QString path = env.value("PATH");
        env.insert("PATH", vipsDir + ";" + path);
        process.setProcessEnvironment(env);
        process.setWorkingDirectory(vipsDir); 

        Logger::info(QString("Running vips: %1 %2").arg(vipsPath).arg(args.join(" ")));
        process.start(vipsPath, args);
        
    } else {
        // Fallback to legacy tools (avifenc/cjxl) only if vips not found...
        // ... (keep existing fallback logic but likely won't be hit if vips is bundled) ...
         if (outputFormat == "avif") {
            // Use avifenc if available
            args.clear();
            args << job->inputPath();
            if (lossless) {
                args << "--lossless";
            } else {
                args << "--min" << "0" << "--max" << QString::number(63 - settings.imageQuality() * 63 / 100);
            }
            args << "--speed" << QString::number(settings.avifSpeed());
            args << job->outputPath();
    
            process.start("avifenc", args);
        } else if (outputFormat == "jxl") {
            // ... cjxl logic ...
            args.clear();
            args << job->inputPath() << job->outputPath();
             // ... options ...
             if (lossless) args << "-d" << "0";
             else args << "-d" << QString::number((100 - settings.imageQuality()) / 10.0);
             args << "-e" << QString::number(settings.jpegXlEffort());
            process.start("cjxl", args);
        } else {
             m_lastError = "Vips not found and no other tool available.";
             return false;
        }
    }

    if (!process.waitForStarted(5000)) {
        m_lastError = QString("Failed to start external tool (%1)").arg(process.errorString());
        Logger::error(m_lastError);
        return false;
    }

    if (!process.waitForFinished(600000)) {  // 10 minute timeout
        process.kill();
        m_lastError = "External tool timed out";
        Logger::error(m_lastError);
        return false;
    }

    if (process.exitCode() != 0) {
        QString stdErr = QString::fromUtf8(process.readAllStandardError());
        QString stdOut = QString::fromUtf8(process.readAllStandardOutput());
        m_lastError = QString("External tool failed (Code %1): %2. Output: %3")
            .arg(process.exitCode()).arg(stdErr).arg(stdOut);
        Logger::error(m_lastError);
        return false;
    }
    
    // Check if output file was actually created and has size
    QFileInfo outInfo(job->outputPath());
    if (!outInfo.exists() || outInfo.size() == 0) {
        m_lastError = "Tool finished but output file is empty or missing";
        Logger::error(m_lastError);
        return false;
    }

    return true;
}

bool ImageProcessor::convertToJxl(const QString& input, const QString& output, bool lossless)
{
    Q_UNUSED(input)
    Q_UNUSED(output)
    Q_UNUSED(lossless)
    // Implementation using cjxl or libvips
    return true;
}

bool ImageProcessor::convertToAvif(const QString& input, const QString& output, bool lossless)
{
    Q_UNUSED(input)
    Q_UNUSED(output)
    Q_UNUSED(lossless)
    // Implementation using avifenc or libvips
    return true;
}

bool ImageProcessor::convertToWebp(const QString& input, const QString& output, bool lossless)
{
    Q_UNUSED(input)
    Q_UNUSED(output)
    Q_UNUSED(lossless)
    // Implementation using cwebp or libvips
    return true;
}

bool ImageProcessor::convertToPng(const QString& input, const QString& output)
{
    Q_UNUSED(input)
    Q_UNUSED(output)
    // Implementation using pngquant or libvips
    return true;
}
