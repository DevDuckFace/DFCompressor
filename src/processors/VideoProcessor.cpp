/**
 * @file VideoProcessor.cpp
 * @brief Video processor implementation using FFmpeg
 */

#include "VideoProcessor.h"
#include "Job.h"
#include "Settings.h"
#include "GPUDetector.h"
#include "Logger.h"

#include <QProcess>
#include <QRegularExpression>
#include <QFileInfo>
#include <QCoreApplication>

VideoProcessor::VideoProcessor()
{
    // Find FFmpeg
    // Find FFmpeg
    QString customPath = Settings::instance().ffmpegPath();
    if (!customPath.isEmpty() && QFileInfo::exists(customPath)) {
        m_ffmpegPath = customPath;
    } else {
        // Check for bundled FFmpeg
        QString appDir = QCoreApplication::applicationDirPath();
        QString bundledPath = appDir + "/ffmpeg/bin/ffmpeg.exe";
        if (QFileInfo::exists(bundledPath)) {
            m_ffmpegPath = bundledPath;
        } else {
            m_ffmpegPath = "ffmpeg";  // Use system PATH
        }
    }

    // Check for GPU encoders
    auto& settings = Settings::instance();
    if (settings.useGpu()) {
        GPUDetector detector;
        auto gpuInfo = detector.detect();
        m_hasNvenc = gpuInfo.hasNvenc && settings.useNvenc();
        m_hasNvdec = gpuInfo.hasNvdec && settings.useNvdec();
    }
}

VideoProcessor::~VideoProcessor() = default;

bool VideoProcessor::process(Job* job)
{
    if (!job) {
        m_lastError = "Invalid job";
        return false;
    }

    if (!checkFFmpeg()) {
        return false;
    }

    Logger::info(QString("Processing video: %1").arg(job->inputPath()));
    Logger::info(QString("FFmpeg path: %1").arg(m_ffmpegPath));
    reportProgress(5);

    // Get video duration for progress calculation
    double totalDuration = 0;
    {
        QString ffprobePath = QFileInfo(m_ffmpegPath).absolutePath() + "/ffprobe.exe";
        QProcess probe;
        probe.start(ffprobePath, {
            "-v", "error",
            "-show_entries", "format=duration",
            "-of", "default=noprint_wrappers=1:nokey=1",
            job->inputPath()
        });
        
        if (probe.waitForFinished(10000)) {
            QString output = probe.readAllStandardOutput().trimmed();
            totalDuration = output.toDouble();
            Logger::info(QString("Video duration: %1 seconds").arg(totalDuration));
        }
    }

    // Build FFmpeg command
    QStringList args = buildFFmpegArgs(job);

    Logger::info(QString("FFmpeg command: %1 %2")
        .arg(m_ffmpegPath)
        .arg(args.join(" ")));

    reportProgress(10);

    // Run FFmpeg
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(m_ffmpegPath, args);

    if (!process.waitForStarted(10000)) {
        m_lastError = QString("Failed to start FFmpeg: %1").arg(process.errorString());
        Logger::error(m_lastError);
        return false;
    }

    // Collect all output
    QByteArray allOutput;
    
    // Parse progress from FFmpeg output
    while (process.state() == QProcess::Running) {
        if (process.waitForReadyRead(100)) {
            QByteArray data = process.readAll();
            allOutput.append(data);
            QString output = QString::fromUtf8(data);
            
            // Parse time progress
            static QRegularExpression timeRegex(R"(time=(\d+):(\d+):(\d+)\.(\d+))");
            auto match = timeRegex.match(output);
            
            if (match.hasMatch() && totalDuration > 0) {
                int hours = match.captured(1).toInt();
                int minutes = match.captured(2).toInt();
                int seconds = match.captured(3).toInt();
                
                double currentTime = hours * 3600 + minutes * 60 + seconds;
                int progress = static_cast<int>(10 + (currentTime / totalDuration) * 85);
                
                reportProgress(qMin(progress, 95));
            }
        }
    }

    // Get any remaining output
    allOutput.append(process.readAll());
    process.waitForFinished(-1);

    if (process.exitCode() != 0) {
        QString outputStr = QString::fromUtf8(allOutput);
        m_lastError = QString("FFmpeg failed (exit code %1): %2")
            .arg(process.exitCode())
            .arg(outputStr.right(500));  // Last 500 chars
        Logger::error(m_lastError);
        return false;
    }

    // Update job with output size
    QFileInfo outputInfo(job->outputPath());
    if (outputInfo.exists() && outputInfo.size() > 0) {
        job->setOutputSize(outputInfo.size());
        reportProgress(100);
        Logger::info(QString("Video processed successfully: %1 (%2 bytes)")
            .arg(job->outputPath()).arg(outputInfo.size()));
        return true;
    }

    m_lastError = "Output file was not created or is empty";
    Logger::error(m_lastError);
    return false;
}

void VideoProcessor::setProgressCallback(std::function<void(int)> callback)
{
    m_progressCallback = callback;
}

bool VideoProcessor::checkFFmpeg()
{
    QProcess process;
    process.start(m_ffmpegPath, {"-version"});
    
    if (!process.waitForFinished(5000)) {
        m_lastError = "FFmpeg not found. Please install FFmpeg or set the path in settings.";
        return false;
    }
    
    return process.exitCode() == 0;
}

QStringList VideoProcessor::buildFFmpegArgs(Job* job)
{
    const auto& settings = Settings::instance();
    QStringList args;

    // Global options
    args << "-y";  // Overwrite output
    args << "-hide_banner";
    args << "-loglevel" << "info";
    args << "-stats";

    // Determine output container from output path
    QString outputExt = QFileInfo(job->outputPath()).suffix().toLower();
    QString codec = settings.videoCodec();
    
    // WebM only supports VP9 and AV1 - force compatible codec
    bool isWebM = (outputExt == "webm");
    bool useNvencEncoder = false;
    
    if (isWebM) {
        // WebM: must use VP9 or AV1 (software encoders, not NVENC)
        if (codec != "vp9" && codec != "av1") {
            codec = "vp9";  // Default to VP9 for WebM
        }
        // Don't use NVENC for WebM - use software encoder
        useNvencEncoder = false;
    } else {
        // MP4/MKV: can use NVENC for H.264 and HEVC only
        // AV1 NVENC has compatibility issues with CUDA hwaccel, use software encoder
        useNvencEncoder = m_hasNvenc && (codec == "h264" || codec == "hevc");
    }

    // Hardware decoding - only for NVENC encoders (not AV1)
    // Use hwaccel cuda but NOT hwaccel_output_format cuda to avoid format issues
    if (m_hasNvdec && useNvencEncoder) {
        args << "-hwaccel" << "cuda";
        // Don't use hwaccel_output_format cuda - let FFmpeg handle conversion
    }

    // Input
    args << "-i" << job->inputPath();

    // Video encoding
    if (codec == "copy") {
        args << "-c:v" << "copy";
    } else {
        // Select encoder based on codec and whether we can use NVENC
        QString encoder;
        if (codec == "hevc") {
            encoder = useNvencEncoder ? "hevc_nvenc" : "libx265";
        } else if (codec == "h264") {
            encoder = useNvencEncoder ? "h264_nvenc" : "libx264";
        } else if (codec == "vp9") {
            encoder = "libvpx-vp9";
        } else {
            encoder = "libx264";  // Fallback
        }
        
        args << "-c:v" << encoder;

        // CRF/Quality settings
        QString compressionMode = settings.videoCompressionMode();
        int crf = settings.videoCrf();
        
        if (compressionMode == "lossless") {
            crf = 0;
        } else if (compressionMode == "visually_lossless") {
            crf = 18;
        } else if (compressionMode == "high") {
            crf = 23;
        } else if (compressionMode == "medium") {
            crf = 28;
        } else if (compressionMode == "web") {
            crf = 32;
        }

        // Different encoders use different quality params
        if (useNvencEncoder) {
            args << "-cq" << QString::number(crf);
            args << "-preset" << "p4";  // NVENC preset
        } else if (codec == "vp9") {
            args << "-crf" << QString::number(crf);
            args << "-b:v" << "0";  // Use CRF mode for VP9
        } else {
            args << "-crf" << QString::number(crf);
            args << "-preset" << settings.videoPreset();
        }

        // Pixel format - only set when NOT using CUDA hw acceleration
        if (!useNvencEncoder) {
            args << "-pix_fmt" << "yuv420p";  // Standard 8-bit for compatibility
        }
    }

    // Audio encoding
    if (settings.preserveAudio()) {
        QString audioCodec = settings.audioCodec();
        
        if (audioCodec == "copy") {
            args << "-c:a" << "copy";
        } else if (audioCodec == "opus") {
            args << "-c:a" << "libopus";
            args << "-b:a" << QString("%1k").arg(settings.audioBitrate());
        } else if (audioCodec == "aac") {
            args << "-c:a" << "aac";
            args << "-b:a" << QString("%1k").arg(settings.audioBitrate());
        } else if (audioCodec == "flac") {
            args << "-c:a" << "flac";
        }
    } else {
        args << "-an";  // No audio
    }

    // Output
    args << job->outputPath();

    return args;
}

QString VideoProcessor::getVideoEncoder() const
{
    const auto& settings = Settings::instance();
    QString codec = settings.videoCodec();

    if (codec == "av1") {
        if (m_hasNvenc) {
            return "av1_nvenc";
        }
        return "libsvtav1";  // SVT-AV1 is faster than libaom
    } else if (codec == "hevc") {
        if (m_hasNvenc) {
            return "hevc_nvenc";
        }
        return "libx265";
    } else if (codec == "h264") {
        if (m_hasNvenc) {
            return "h264_nvenc";
        }
        return "libx264";
    } else if (codec == "vp9") {
        return "libvpx-vp9";
    }

    return "libx264";  // Fallback
}

QString VideoProcessor::getAudioEncoder() const
{
    const auto& settings = Settings::instance();
    QString codec = settings.audioCodec();

    if (codec == "opus") {
        return "libopus";
    } else if (codec == "aac") {
        return "aac";
    } else if (codec == "flac") {
        return "flac";
    }

    return "aac";  // Fallback
}

void VideoProcessor::reportProgress(int progress)
{
    if (m_progressCallback) {
        m_progressCallback(progress);
    }
}
