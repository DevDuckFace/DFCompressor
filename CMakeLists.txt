# ============================================================================
# DFCompressor - Advanced Media Converter & Compressor
# CMake Configuration File
# ============================================================================
cmake_minimum_required(VERSION 3.24)

project(DFCompressor
    VERSION 1.0.0
    DESCRIPTION "DFCompressor - Advanced Media Converter & Compressor with GPU Acceleration"
    LANGUAGES CXX
)

# ============================================================================
# C++ Standard Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Build Options
# ============================================================================
option(MEDIAFORGE_ENABLE_CUDA "Enable CUDA/NVENC GPU acceleration" ON)
option(MEDIAFORGE_ENABLE_TESTS "Build unit tests" ON)
option(MEDIAFORGE_USE_VCPKG "Use vcpkg for dependency management" OFF)

# ============================================================================
# Find Qt6
# ============================================================================
find_package(Qt6 6.6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    Multimedia
    MultimediaWidgets
    Svg
    Network
)

message(STATUS "Found Qt6 version: ${Qt6_VERSION}")

# ============================================================================
# Find CUDA (Optional)
# ============================================================================
if(MEDIAFORGE_ENABLE_CUDA)
    include(CheckLanguage)
    check_language(CUDA)
    
    if(CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
        find_package(CUDAToolkit)
        
        if(CUDAToolkit_FOUND)
            message(STATUS "CUDA Toolkit found: ${CUDAToolkit_VERSION}")
            set(MEDIAFORGE_HAS_CUDA TRUE)
            add_compile_definitions(MEDIAFORGE_HAS_CUDA=1)
        else()
            message(WARNING "CUDA Toolkit not found. GPU acceleration disabled.")
            set(MEDIAFORGE_HAS_CUDA FALSE)
        endif()
    else()
        message(WARNING "CUDA compiler not found. GPU acceleration disabled.")
        set(MEDIAFORGE_HAS_CUDA FALSE)
    endif()
else()
    set(MEDIAFORGE_HAS_CUDA FALSE)
endif()

# ============================================================================
# Find FFmpeg
# ============================================================================
# FFmpeg detection - using pkg-config or manual paths
find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(FFMPEG IMPORTED_TARGET
        libavcodec
        libavformat
        libavutil
        libswscale
        libswresample
        libavfilter
    )
endif()

if(NOT FFMPEG_FOUND)
    # Manual FFmpeg detection
    set(FFMPEG_ROOT "" CACHE PATH "FFmpeg installation root")
    
    if(FFMPEG_ROOT)
        set(FFMPEG_INCLUDE_DIRS ${FFMPEG_ROOT}/include)
        set(FFMPEG_LIBRARY_DIRS ${FFMPEG_ROOT}/lib)
        
        find_library(AVCODEC_LIBRARY avcodec HINTS ${FFMPEG_LIBRARY_DIRS})
        find_library(AVFORMAT_LIBRARY avformat HINTS ${FFMPEG_LIBRARY_DIRS})
        find_library(AVUTIL_LIBRARY avutil HINTS ${FFMPEG_LIBRARY_DIRS})
        find_library(SWSCALE_LIBRARY swscale HINTS ${FFMPEG_LIBRARY_DIRS})
        find_library(SWRESAMPLE_LIBRARY swresample HINTS ${FFMPEG_LIBRARY_DIRS})
        find_library(AVFILTER_LIBRARY avfilter HINTS ${FFMPEG_LIBRARY_DIRS})
        
        set(FFMPEG_LIBRARIES
            ${AVCODEC_LIBRARY}
            ${AVFORMAT_LIBRARY}
            ${AVUTIL_LIBRARY}
            ${SWSCALE_LIBRARY}
            ${SWRESAMPLE_LIBRARY}
            ${AVFILTER_LIBRARY}
        )
        
        if(AVCODEC_LIBRARY)
            set(FFMPEG_FOUND TRUE)
            message(STATUS "FFmpeg found at: ${FFMPEG_ROOT}")
        endif()
    endif()
endif()

if(NOT FFMPEG_FOUND)
    message(WARNING "FFmpeg not found. Video processing will be disabled.")
    message(STATUS "Set FFMPEG_ROOT to your FFmpeg installation path.")
else()
    add_compile_definitions(MEDIAFORGE_HAS_FFMPEG=1)
endif()

# ============================================================================
# Find libvips
# ============================================================================
if(PkgConfig_FOUND)
    pkg_check_modules(VIPS IMPORTED_TARGET vips)
endif()

if(NOT VIPS_FOUND)
    set(VIPS_ROOT "" CACHE PATH "libvips installation root")
    
    if(VIPS_ROOT)
        set(VIPS_INCLUDE_DIRS ${VIPS_ROOT}/include)
        set(VIPS_LIBRARY_DIRS ${VIPS_ROOT}/lib)
        
        find_library(VIPS_LIBRARY vips HINTS ${VIPS_LIBRARY_DIRS})
        
        if(VIPS_LIBRARY)
            set(VIPS_FOUND TRUE)
            set(VIPS_LIBRARIES ${VIPS_LIBRARY})
            message(STATUS "libvips found at: ${VIPS_ROOT}")
        endif()
    endif()
endif()

if(NOT VIPS_FOUND)
    message(WARNING "libvips not found. Image processing will be limited.")
    message(STATUS "Set VIPS_ROOT to your libvips installation path.")
else()
    add_compile_definitions(MEDIAFORGE_HAS_VIPS=1)
endif()

# ============================================================================
# Source Files
# ============================================================================
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/FileListWidget.cpp
    src/ui/FileListWidget.h
    src/ui/SettingsDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/ProgressWidget.cpp
    src/ui/ProgressWidget.h
    src/ui/PreviewWidget.cpp
    src/ui/PreviewWidget.h
    src/ui/DropZone.cpp
    src/ui/DropZone.h
    src/ui/ThemeManager.cpp
    src/ui/ThemeManager.h
)

set(CORE_SOURCES
    src/core/JobQueue.cpp
    src/core/JobQueue.h
    src/core/Job.cpp
    src/core/Job.h
    src/core/Settings.cpp
    src/core/Settings.h
    src/core/MediaInfo.cpp
    src/core/MediaInfo.h
)

set(PROCESSOR_SOURCES
    src/processors/ImageProcessor.cpp
    src/processors/ImageProcessor.h
    src/processors/VideoProcessor.cpp
    src/processors/VideoProcessor.h
    src/processors/GPUDetector.cpp
    src/processors/GPUDetector.h
    src/processors/ProcessorFactory.cpp
    src/processors/ProcessorFactory.h
)

set(UTIL_SOURCES
    src/utils/FileUtils.cpp
    src/utils/FileUtils.h
    src/utils/FormatUtils.cpp
    src/utils/FormatUtils.h
    src/utils/Logger.cpp
    src/utils/Logger.h
)

set(ALL_SOURCES
    src/main.cpp
    ${UI_SOURCES}
    ${CORE_SOURCES}
    ${PROCESSOR_SOURCES}
    ${UTIL_SOURCES}
)

# ============================================================================
# Resources
# ============================================================================
set(RESOURCES
    resources/resources.qrc
)

# ============================================================================
# Windows Application Resources
# ============================================================================
if(WIN32)
    set(WIN_RESOURCES
        resources/app.rc
    )
endif()

# ============================================================================
# Create Executable
# ============================================================================
add_executable(${PROJECT_NAME} WIN32
    ${ALL_SOURCES}
    ${RESOURCES}
    ${WIN_RESOURCES}
)

# ============================================================================
# Include Directories
# ============================================================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/processors
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

if(FFMPEG_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
endif()

if(VIPS_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${VIPS_INCLUDE_DIRS})
endif()

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Svg
    Qt6::Network
)

if(FFMPEG_FOUND)
    if(TARGET PkgConfig::FFMPEG)
        target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::FFMPEG)
    else()
        target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})
    endif()
endif()

if(VIPS_FOUND)
    if(TARGET PkgConfig::VIPS)
        target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::VIPS)
    else()
        target_link_directories(${PROJECT_NAME} PRIVATE ${VIPS_LIBRARY_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${VIPS_LIBRARIES})
    endif()
endif()

if(MEDIAFORGE_HAS_CUDA)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        CUDA::cudart
        CUDA::nvml
    )
endif()

# ============================================================================
# Compiler Definitions
# ============================================================================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:MEDIAFORGE_DEBUG>
    MEDIAFORGE_VERSION="${PROJECT_VERSION}"
    MEDIAFORGE_APP_NAME="${PROJECT_NAME}"
)

# ============================================================================
# Compiler Options
# ============================================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /utf-8
        /MP
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/GL>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Release>:-O3>
    )
endif()

# ============================================================================
# Windows Specific: Deploy Qt
# ============================================================================
if(WIN32)
    # Get Qt bin directory
    get_target_property(QT_QMAKE_LOCATION Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_LOCATION}" DIRECTORY)
    
    # Custom target to deploy Qt DLLs
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${QT_BIN_DIR}/windeployqt.exe"
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt dependencies..."
    )
    
    # Copy FFmpeg DLLs if available
    if(FFMPEG_FOUND AND FFMPEG_ROOT)
        file(GLOB FFMPEG_DLLS "${FFMPEG_ROOT}/bin/*.dll")
        foreach(DLL ${FFMPEG_DLLS})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL}"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            )
        endforeach()
    endif()
    
    # Copy libvips DLLs if available
    if(VIPS_FOUND AND VIPS_ROOT)
        file(GLOB VIPS_DLLS "${VIPS_ROOT}/bin/*.dll")
        foreach(DLL ${VIPS_DLLS})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL}"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            )
        endforeach()
    endif()
endif()

# ============================================================================
# Installation
# ============================================================================
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "===============================================")
message(STATUS " DFCompressor Configuration Summary")
message(STATUS "===============================================")
message(STATUS " Version:        ${PROJECT_VERSION}")
message(STATUS " C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS " Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS " Qt6:            ${Qt6_VERSION}")
message(STATUS " CUDA:           ${MEDIAFORGE_HAS_CUDA}")
message(STATUS " FFmpeg:         ${FFMPEG_FOUND}")
message(STATUS " libvips:        ${VIPS_FOUND}")
message(STATUS "===============================================")
message(STATUS "")
