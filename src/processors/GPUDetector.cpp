/**
 * @file GPUDetector.cpp
 * @brief GPU detection implementation
 */

#include "GPUDetector.h"
#include "Logger.h"

#include <QProcess>
#include <QRegularExpression>

#ifdef MEDIAFORGE_HAS_CUDA
#include <cuda_runtime.h>
#include <nvml.h>
#endif

GPUInfo GPUDetector::detect()
{
    GPUInfo info;

#ifdef MEDIAFORGE_HAS_CUDA
    info = detectWithCuda();
    if (info.hasNvidia) {
        Logger::info("GPU detected using CUDA API");
        return info;
    }
#endif

    // Try NVML
    info = detectWithNvml();
    if (info.hasNvidia) {
        Logger::info("GPU detected using NVML");
        return info;
    }

    // Fallback to FFmpeg detection
    info = detectWithFFmpeg();
    if (info.hasNvidia) {
        Logger::info("GPU detected using FFmpeg");
    }

    return info;
}

bool GPUDetector::hasNvidiaGPU()
{
    GPUDetector detector;
    auto info = detector.detect();
    return info.hasNvidia;
}

bool GPUDetector::checkNvencSupport()
{
    // Check using FFmpeg
    QProcess process;
    process.start("ffmpeg", {"-hide_banner", "-encoders"});
    
    if (process.waitForFinished(5000)) {
        QString output = process.readAllStandardOutput();
        return output.contains("nvenc") || output.contains("h264_nvenc");
    }
    
    return false;
}

bool GPUDetector::checkCudaSupport()
{
#ifdef MEDIAFORGE_HAS_CUDA
    int deviceCount = 0;
    cudaError_t error = cudaGetDeviceCount(&deviceCount);
    return error == cudaSuccess && deviceCount > 0;
#else
    return false;
#endif
}

GPUInfo GPUDetector::detectWithNvml()
{
    GPUInfo info;

#ifdef MEDIAFORGE_HAS_CUDA
    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS) {
        return info;
    }

    unsigned int deviceCount = 0;
    result = nvmlDeviceGetCount(&deviceCount);
    
    if (result != NVML_SUCCESS || deviceCount == 0) {
        nvmlShutdown();
        return info;
    }

    nvmlDevice_t device;
    result = nvmlDeviceGetHandleByIndex(0, &device);
    
    if (result == NVML_SUCCESS) {
        info.hasNvidia = true;
        info.hasCuda = true;

        // Get device name
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        if (nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE) == NVML_SUCCESS) {
            info.deviceName = QString::fromUtf8(name);
        }

        // Get memory info
        nvmlMemory_t memory;
        if (nvmlDeviceGetMemoryInfo(device, &memory) == NVML_SUCCESS) {
            info.vramMB = static_cast<int>(memory.total / (1024 * 1024));
        }

        // Get CUDA compute capability
        int major = 0, minor = 0;
        if (nvmlDeviceGetCudaComputeCapability(device, &major, &minor) == NVML_SUCCESS) {
            info.computeCapabilityMajor = major;
            info.computeCapabilityMinor = minor;
        }

        // Check encoder support (compute capability >= 3.0 typically has NVENC)
        info.hasNvenc = (major >= 3);
        info.hasNvdec = (major >= 3);
    }

    nvmlShutdown();
#endif

    return info;
}

GPUInfo GPUDetector::detectWithCuda()
{
    GPUInfo info;

#ifdef MEDIAFORGE_HAS_CUDA
    int deviceCount = 0;
    cudaError_t error = cudaGetDeviceCount(&deviceCount);
    
    if (error != cudaSuccess || deviceCount == 0) {
        return info;
    }

    cudaDeviceProp props;
    error = cudaGetDeviceProperties(&props, 0);
    
    if (error == cudaSuccess) {
        info.hasNvidia = true;
        info.hasCuda = true;
        info.deviceName = QString::fromUtf8(props.name);
        info.cudaCores = props.multiProcessorCount * 128; // Approximate
        info.vramMB = static_cast<int>(props.totalGlobalMem / (1024 * 1024));
        info.computeCapabilityMajor = props.major;
        info.computeCapabilityMinor = props.minor;
        
        // NVENC available on compute capability >= 3.0
        info.hasNvenc = (props.major >= 3);
        info.hasNvdec = (props.major >= 3);
    }
#endif

    return info;
}

GPUInfo GPUDetector::detectWithFFmpeg()
{
    GPUInfo info;

    // Try nvidia-smi first
    QProcess smiProcess;
    smiProcess.start("nvidia-smi", {
        "--query-gpu=name,memory.total,driver_version",
        "--format=csv,noheader,nounits"
    });

    if (smiProcess.waitForFinished(5000) && smiProcess.exitCode() == 0) {
        QString output = smiProcess.readAllStandardOutput().trimmed();
        QStringList parts = output.split(",");
        
        if (parts.size() >= 3) {
            info.hasNvidia = true;
            info.deviceName = parts[0].trimmed();
            info.vramMB = parts[1].trimmed().toInt();
            
            // Check NVENC support
            info.hasNvenc = checkNvencSupport();
            info.hasNvdec = info.hasNvenc; // Usually available together
        }
    }

    // Check for CUDA using FFmpeg
    if (!info.hasNvidia) {
        QProcess ffmpegProcess;
        ffmpegProcess.start("ffmpeg", {"-hide_banner", "-hwaccels"});
        
        if (ffmpegProcess.waitForFinished(5000)) {
            QString output = ffmpegProcess.readAllStandardOutput();
            
            if (output.contains("cuda") || output.contains("nvdec")) {
                info.hasNvidia = true;
                info.hasCuda = output.contains("cuda");
                info.hasNvdec = output.contains("nvdec") || output.contains("cuvid");
                info.deviceName = "NVIDIA GPU (detected via FFmpeg)";
                
                info.hasNvenc = checkNvencSupport();
            }
        }
    }

    return info;
}
