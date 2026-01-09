# MediaForge Build Script for Windows
# Requires: Visual Studio 2022 OR Qt MinGW + Ninja
# Usage: .\build.ps1 [-Clean] [-Package] [-NoCuda] [-QtPath "C:\Qt\..."]

param(
    [string]$QtPath = "",
    [string]$FFmpegPath = "",
    [string]$VipsPath = "",
    [string]$BuildType = "Release",
    [switch]$Clean,
    [switch]$Package,
    [switch]$NoCuda
)

$ErrorActionPreference = "Stop"

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host " DFCompressor Build Script" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Check prerequisites
Write-Host "Checking prerequisites..." -ForegroundColor Yellow

# Check CMake
if (!(Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: CMake not found. Please install CMake 3.24 or higher." -ForegroundColor Red
    exit 1
}
$cmakeVersion = cmake --version | Select-Object -First 1
Write-Host "  CMake: $cmakeVersion" -ForegroundColor Green

# Check/Find Qt
if ([string]::IsNullOrEmpty($QtPath) -or !(Test-Path $QtPath)) {
    # Try to auto-detect
    $possiblePaths = @("C:\Qt\6.10.1\mingw_64", "C:\Qt\6.8.0\mingw_64", "C:\Qt\6.7.0\mingw_64", "C:\Qt\6.6.0\msvc2022_64")
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $QtPath = $path
            Write-Host "Auto-detected Qt at: $QtPath" -ForegroundColor Green
            break
        }
    }
}

if (!(Test-Path $QtPath)) {
    Write-Host "ERROR: Qt not found. Please set -QtPath to your Qt installation" -ForegroundColor Red
    exit 1
}
Write-Host "  Qt: $QtPath" -ForegroundColor Green

# Detect Build System (MinGW vs MSVC)
$isMinGW = $QtPath -like "*mingw*"
$isMSVC = $QtPath -like "*msvc*"

if ($isMinGW) {
    Write-Host "MinGW detected. Configuring environment..." -ForegroundColor Yellow
    
    # Path to MinGW and Ninja in standard Qt installation
    $mingwBin = "C:\Qt\Tools\mingw1310_64\bin" 
    $ninjaDir = "C:\Qt\Tools\Ninja"
    
    if (Test-Path $mingwBin) {
        $env:PATH = "$mingwBin;$ninjaDir;$env:PATH"
        Write-Host "  Added MinGW to PATH: $mingwBin" -ForegroundColor Green
    }
    else {
        Write-Host "WARNING: MinGW compiler not found at default location ($mingwBin). Build might fail." -ForegroundColor Red
    }
    
    if (!(Test-Path "$ninjaDir\ninja.exe")) {
        Write-Host "WARNING: Ninja not found at $ninjaDir. Build might fail." -ForegroundColor Red
    }
}
elseif ($isMSVC) {
    # Check Visual Studio
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsPath = & $vsWhere -latest -property installationPath
        Write-Host "  Visual Studio: $vsPath" -ForegroundColor Green
    }
    else {
        Write-Host "WARNING: vswhere not found, assuming VS is installed" -ForegroundColor Yellow
    }
}

# Setup directories
$projectRoot = $PSScriptRoot
$buildDir = Join-Path $projectRoot "build"
$binDir = Join-Path $buildDir "bin"

# Adjust bin dir for MSVC (creates Release subdir)
if (-not $isMinGW) {
    $binDir = Join-Path $binDir $BuildType
}

# Clean if requested
if ($Clean) {
    Write-Host ""
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path $buildDir) {
        Remove-Item -Recurse -Force $buildDir
    }
}

# Create build directory
if (!(Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

# Configure CMake arguments
$cmakeArgs = @()

if ($isMinGW) {
    $cmakeArgs += "-G", "Ninja"
    $cmakeArgs += "-DCMAKE_BUILD_TYPE=$BuildType"
    $cmakeArgs += "-DCMAKE_PREFIX_PATH=`"$QtPath`""
    $cmakeArgs += "-DCMAKE_MAKE_PROGRAM=`"C:\Qt\Tools\Ninja\ninja.exe`""
    $cmakeArgs += "-DCMAKE_C_COMPILER=`"C:\Qt\Tools\mingw1310_64\bin\gcc.exe`""
    $cmakeArgs += "-DCMAKE_CXX_COMPILER=`"C:\Qt\Tools\mingw1310_64\bin\g++.exe`""
}
else {
    # Default to Visual Studio
    $cmakeArgs += "-G", "Visual Studio 17 2022"
    $cmakeArgs += "-A", "x64"
    $cmakeArgs += "-DCMAKE_PREFIX_PATH=`"$QtPath`""
    $cmakeArgs += "-DCMAKE_BUILD_TYPE=$BuildType"
}

if ($FFmpegPath -and (Test-Path $FFmpegPath)) {
    $cmakeArgs += "-DFFMPEG_ROOT=`"$FFmpegPath`""
    Write-Host "  FFmpeg: $FFmpegPath" -ForegroundColor Green
}

if ($VipsPath -and (Test-Path $VipsPath)) {
    $cmakeArgs += "-DVIPS_ROOT=`"$VipsPath`""
    Write-Host "  libvips: $VipsPath" -ForegroundColor Green
}

if ($NoCuda) {
    $cmakeArgs += "-DMEDIAFORGE_ENABLE_CUDA=OFF"
    Write-Host "  CUDA: Disabled" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Configuring CMake..." -ForegroundColor Yellow

Push-Location $buildDir
try {
    $cmakeCmd = "cmake $($cmakeArgs -join ' ') `"$projectRoot`""
    Write-Host "  Command: cmake $cmakeArgs" -ForegroundColor Gray
    
    # Run CMake Configure
    $proc = Start-Process -FilePath "cmake" -ArgumentList ($cmakeArgs + $projectRoot) -PassThru -NoNewWindow -Wait
    if ($proc.ExitCode -ne 0) {
        Write-Host "ERROR: CMake configuration failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Building..." -ForegroundColor Yellow
    
    # Run CMake Build
    cmake --build . --config $BuildType --parallel
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Build failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host "  Executable: $binDir\DFCompressor.exe" -ForegroundColor Cyan
    
    # Package if requested
    if ($Package) {
        Write-Host ""
        Write-Host "Creating installer..." -ForegroundColor Yellow
        
        $innoSetup = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
        if (!(Test-Path $innoSetup)) {
            Write-Host "WARNING: Inno Setup not found. Skipping installer creation." -ForegroundColor Yellow
            $innoPath2 = "${env:LocalAppData}\Programs\Inno Setup 6\ISCC.exe"
            if (Test-Path $innoPath2) {
                & $innoPath2 "$projectRoot\installer\setup.iss"
                Write-Host "Installer created: installer\output\DFCompressor_Setup_*.exe" -ForegroundColor Green
            }
        }
        else {
            & $innoSetup "$projectRoot\installer\setup.iss"
            Write-Host "Installer created: installer\output\DFCompressor_Setup_*.exe" -ForegroundColor Green
        }
    }
    
}
finally {
    Pop-Location
}

Write-Host ""
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host " Build Complete!" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
