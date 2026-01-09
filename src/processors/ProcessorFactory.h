/**
 * @file ProcessorFactory.h
 * @brief Processor factory header
 */

#ifndef PROCESSORFACTORY_H
#define PROCESSORFACTORY_H

#include <memory>

class ImageProcessor;
class VideoProcessor;

class ProcessorFactory
{
public:
    static std::unique_ptr<ImageProcessor> createImageProcessor();
    static std::unique_ptr<VideoProcessor> createVideoProcessor();
};

#endif // PROCESSORFACTORY_H
