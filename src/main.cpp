/**
 * @file main.cpp
 * @brief DFCompressor Application Entry Point
 * 
 * Advanced Media Converter & Compressor with GPU Acceleration
 * Supports lossless compression for images and videos
 */

#include <QApplication>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QDir>
#include <QStandardPaths>

#include "MainWindow.h"
#include "ThemeManager.h"
#include "Settings.h"
#include "GPUDetector.h"
#include "Logger.h"

int main(int argc, char *argv[])
{
    // Enable High DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    QApplication app(argc, argv);
    
    // Application metadata
    QCoreApplication::setOrganizationName("DuckForge");
    QCoreApplication::setOrganizationDomain("duckforge.app");
    QCoreApplication::setApplicationName("DFCompressor");
    QCoreApplication::setApplicationVersion(MEDIAFORGE_VERSION);
    
    // Initialize logging
    Logger::instance().initialize();
    Logger::info("DFCompressor v" MEDIAFORGE_VERSION " starting...");
    
    // Load custom fonts
    QFontDatabase::addApplicationFont(":/fonts/Inter-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-Medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-SemiBold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-Bold.ttf");
    
    // Set default font
    QFont defaultFont("Inter", 10);
    defaultFont.setStyleStrategy(QFont::PreferAntialias);
    app.setFont(defaultFont);
    
    // Initialize settings
    Settings::instance().load();
    
    // Initialize theme manager
    ThemeManager::instance().initialize();
    ThemeManager::instance().applyTheme(Settings::instance().theme());
    
    // Detect GPU capabilities
    GPUDetector detector;
    auto gpuInfo = detector.detect();
    
    if (gpuInfo.hasNvidia) {
        Logger::info(QString("NVIDIA GPU detected: %1").arg(gpuInfo.deviceName));
        Logger::info(QString("CUDA Cores: %1, VRAM: %2 GB")
            .arg(gpuInfo.cudaCores)
            .arg(gpuInfo.vramMB / 1024.0, 0, 'f', 1));
        
        if (gpuInfo.hasNvenc) {
            Logger::info("NVENC hardware encoder available");
        }
    } else {
        Logger::info("No NVIDIA GPU detected. Using CPU processing.");
    }
    
    // Create and show main window
    MainWindow mainWindow;
    mainWindow.setGPUInfo(gpuInfo);
    mainWindow.show();
    
    Logger::info("Application initialized successfully");
    
    return app.exec();
}
