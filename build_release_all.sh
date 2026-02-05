#!/bin/bash
# Build all release binaries (FULL merged + OTA) for all devices
# Removes old binaries first

set -e  # Exit on any error

ENVS=(
    "atom-matrix-release"
    "waveshare-s3-release"
    "m5stickc-plus-release"
    "lilygo-t-display-release"
    "lilygo-t-qt-release"
    "aipi-lite-release"
)

echo "=================================================="
echo "STAC v3 Release Binary Builder"
echo "=================================================="

# Remove old binaries
echo ""
echo "Removing old binaries..."
find bin -name "*.bin" -type f -delete
echo "✓ Old binaries removed"

echo ""
echo "Building Release Binaries for All Devices"
echo "=================================================="

# Change to STAC directory where platformio.ini is located
cd STAC

for env in "${ENVS[@]}"; do
    echo ""
    echo "Building $env..."
    echo "--------------------------------------------------"
    
    # Clean build
    echo "Cleaning..."
    pio run -e "$env" -t clean
    
    # Build FULL merged binary
    echo "Creating FULL binary..."
    pio run -e "$env" -t merged
    
    # Build OTA binary
    echo "Creating OTA binary..."
    pio run -e "$env" -t ota
    
    echo "✓ $env complete"
done

# Return to repository root
cd ..

echo ""
echo "=================================================="
echo "All Release Binaries Built Successfully!"
echo "=================================================="
echo ""
echo "Binaries are located in:"
ls -lh bin/*/STAC_*.bin 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
