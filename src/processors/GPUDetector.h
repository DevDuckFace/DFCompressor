/**
 * @file GPUDetector.h
 * @brief GPU detection header
 */

#ifndef GPUDETECTOR_H
#define GPUDETECTOR_H

#include <QString>

struct GPUInfo {
    bool hasNvidia = false;
    bool hasCuda = false;
    bool hasNvenc = false;
    bool hasNvdec = false;
    
    QString deviceName;
    int cudaCores = 0;
    int vramMB = 0;
    int computeCapabilityMajor = 0;
    int computeCapabilityMinor = 0;
    int driverVersion = 0;
    int cudaVersion = 0;
};

class GPUDetector
{
public:
    GPUDetector() = default;
    ~GPUDetector() = default;

    GPUInfo detect();
    
    static bool hasNvidiaGPU();
    static bool checkNvencSupport();
    static bool checkCudaSupport();

private:
    GPUInfo detectWithNvml();
    GPUInfo detectWithCuda();
    GPUInfo detectWithFFmpeg();
};

#endif // GPUDETECTOR_H
