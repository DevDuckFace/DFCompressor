# DFCompressor

A powerful, GPU-accelerated media compression tool for Windows.

![DFCompressor Logo](resources/icons/logo.png)

## Features

- **Image Compression**: Convert and compress images to modern formats
  - AVIF (best compression)
  - WebP (excellent compression, wide support)
  - PNG, JPG (classic formats)
  
- **Video Compression**: Re-encode videos with hardware acceleration
  - H.265/HEVC (best compression, NVENC accelerated)
  - H.264/AVC (most compatible, NVENC accelerated)
  - VP9 (for WebM containers)
  
- **GPU Acceleration**: Utilizes NVIDIA NVENC for fast video encoding
- **Batch Processing**: Process multiple files at once
- **Modern UI**: Clean, intuitive dark-themed interface

## Requirements

- Windows 10 or later (64-bit)
- Qt 6.10+ (for building)
- CMake 3.20+
- MinGW or MSVC compiler
- NVIDIA GPU with NVENC support (optional, for GPU acceleration)

## Building from Source

### Prerequisites

1. Install Qt 6.10+ with MinGW
2. Install CMake 3.20+
3. Clone this repository

### Download Dependencies

Run the setup script to download FFmpeg and Libvips:

```powershell
powershell -ExecutionPolicy Bypass -File .\setup_libs.ps1
```

### Build

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

To create the installer:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Package
```

The installer will be created in `installer/output/`.

## Project Structure

```
DFCompressor/
├── src/
│   ├── core/           # Core classes (Settings, Job, JobQueue)
│   ├── processors/     # Image and Video processors
│   ├── ui/             # UI components (MainWindow, dialogs)
│   └── utils/          # Utilities (Logger, GPU detection)
├── resources/
│   ├── icons/          # Application icons
│   ├── fonts/          # Custom fonts
│   └── styles/         # QSS stylesheets
├── installer/          # Inno Setup scripts
└── build/              # Build output (not tracked in git)
```

## Supported Formats

### Input Formats
- **Images**: PNG, JPG, JPEG, WebP, AVIF, HEIC, TIFF, BMP, GIF
- **Videos**: MP4, MKV, AVI, MOV, WebM, WMV, FLV

### Output Formats
- **Images**: AVIF, WebP, PNG, JPG
- **Videos**: MP4, MKV, WebM

## License

MIT License - see [LICENSE](installer/LICENSE.txt)

## Credits

- Built with [Qt](https://www.qt.io/)
- Video processing powered by [FFmpeg](https://ffmpeg.org/)
- Image processing powered by [libvips](https://libvips.github.io/libvips/)

