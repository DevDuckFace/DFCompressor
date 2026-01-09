# Script to download and setup FFmpeg and Libvips dependencies
$ErrorActionPreference = "Stop"

$thirdPartyDir = Join-Path $PSScriptRoot "third_party"
$ffmpegDir = Join-Path $thirdPartyDir "ffmpeg"
$vipsDir = Join-Path $thirdPartyDir "vips"

# Create directories
if (!(Test-Path $thirdPartyDir)) { New-Item -ItemType Directory -Force -Path $thirdPartyDir | Out-Null }
if (Test-Path $ffmpegDir) { Remove-Item -Path $ffmpegDir -Recurse -Force -ErrorAction SilentlyContinue }
New-Item -ItemType Directory -Force -Path $ffmpegDir | Out-Null
if (Test-Path $vipsDir) { Remove-Item -Path $vipsDir -Recurse -Force -ErrorAction SilentlyContinue }
New-Item -ItemType Directory -Force -Path $vipsDir | Out-Null

# --- FFmpeg ---
Write-Host "Downloading FFmpeg..." -ForegroundColor Cyan
$ffmpegUrl = "https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-win64-gpl.zip"
$ffmpegZip = Join-Path $thirdPartyDir "ffmpeg_v2.zip"

if (Test-Path $ffmpegZip) { Remove-Item $ffmpegZip -Force -ErrorAction SilentlyContinue }
& curl.exe -L -o "$ffmpegZip" "$ffmpegUrl"

if (Test-Path $ffmpegZip) {
    if ((Get-Item $ffmpegZip).Length -gt 0) {
        Write-Host "Extracting FFmpeg..." -ForegroundColor Yellow
        Expand-Archive -Path $ffmpegZip -DestinationPath $thirdPartyDir -Force
        
        # Flatten folder structure
        $extractedItems = Get-ChildItem -Path $thirdPartyDir -Filter "ffmpeg-master-latest-win64-gpl" -Directory
        if ($extractedItems) {
            Copy-Item -Path "$($extractedItems.FullName)\bin" -Destination $ffmpegDir -Recurse -Force
            Remove-Item -Path $extractedItems.FullName -Recurse -Force
        }
        Remove-Item -Path $ffmpegZip -Force
        Write-Host "FFmpeg setup complete." -ForegroundColor Green
    }
    else {
        Write-Host "ERROR: FFmpeg download failed (empty file)." -ForegroundColor Red
        exit 1
    }
}

# --- Libvips ---
Write-Host "Downloading Libvips..." -ForegroundColor Cyan
$vipsUrl = "https://github.com/libvips/libvips/releases/download/v8.16.0/vips-dev-w64-all-8.16.0.zip"
$vipsZip = Join-Path $thirdPartyDir "vips_v2.zip"

if (Test-Path $vipsZip) { Remove-Item $vipsZip -Force -ErrorAction SilentlyContinue }
& curl.exe -L -o "$vipsZip" "$vipsUrl"

if (Test-Path $vipsZip) {
    if ((Get-Item $vipsZip).Length -gt 0) {
        Write-Host "Extracting Libvips..." -ForegroundColor Yellow
        Expand-Archive -Path $vipsZip -DestinationPath $vipsDir -Force
        
        # Flatten folder structure
        $extractedItems = Get-ChildItem -Path $vipsDir -Filter "vips-dev-*" -Directory
        if ($extractedItems) {
            Copy-Item -Path "$($extractedItems.FullName)\bin" -Destination $vipsDir -Recurse -Force
            Remove-Item -Path $extractedItems.FullName -Recurse -Force
        }
        
        Remove-Item -Path $vipsZip -Force
        Write-Host "Libvips setup complete." -ForegroundColor Green
    }
    else {
        Write-Host "ERROR: Libvips download failed (empty file)." -ForegroundColor Red
        exit 1
    }
}
