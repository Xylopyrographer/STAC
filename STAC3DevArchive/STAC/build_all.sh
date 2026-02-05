#!/bin/bash
# Build all platform release binaries with clean builds

cd /Users/robl/Documents/PlatformIO/Projects/STAC3/STAC

echo "Building ATOM Matrix..."
pio run -e atom-matrix-release -t clean -t merged -t ota

echo "Building Waveshare S3..."
pio run -e waveshare-s3-release -t clean -t merged -t ota

echo "Building M5StickC Plus..."
pio run -e m5stickc-plus-release -t clean -t merged -t ota

echo "Building LilyGO T-Display..."
pio run -e lilygo-t-display-release -t clean -t merged -t ota

echo "Building AIPI-Lite..."
pio run -e aipi-lite-release -t clean -t merged -t ota

echo ""
echo "All builds complete!"
echo ""
ls -lh bin/
