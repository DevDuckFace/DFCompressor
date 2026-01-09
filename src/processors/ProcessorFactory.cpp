/**
 * @file ProcessorFactory.cpp
 * @brief Processor factory implementation
 */

#include "ProcessorFactory.h"
#include "ImageProcessor.h"
#include "VideoProcessor.h"

std::unique_ptr<ImageProcessor> ProcessorFactory::createImageProcessor()
{
    return std::make_unique<ImageProcessor>();
}

std::unique_ptr<VideoProcessor> ProcessorFactory::createVideoProcessor()
{
    return std::make_unique<VideoProcessor>();
}
