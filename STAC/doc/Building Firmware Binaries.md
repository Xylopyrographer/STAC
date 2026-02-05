# Building Firmware Binaries

## Overview

**IMPORTANT:** Always use the custom build script to generate firmware binaries. Do not manually build or copy binaries.

The custom build script (`scripts/custom_targets.py`) automatically:
- Extracts version from `Device_Config.h`
- Generates properly named binaries using the agreed naming convention
- Creates both OTA and full flash binaries
- Copies binaries to the `bin/` directory

## Naming Convention

All firmware binaries follow this naming convention:

```
STAC_v<version>_<board>_<build>.bin       - OTA update binary (app only)
STAC_v<version>_<board>_<build>_FULL.bin  - Full flash binary (bootloader + partitions + app)
```

### Examples

- `STAC_v3.0.0-RC.9_ATOM_D3.bin` - ATOM Matrix, debug build, OTA
- `STAC_v3.0.0-RC.9_ATOM_D3_FULL.bin` - ATOM Matrix, debug build, full flash
- `STAC_v3.0.0-RC.9_ATOM_R.bin` - ATOM Matrix, release build, OTA
- `STAC_v3.0.0-RC.9_ATOM_R_FULL.bin` - ATOM Matrix, release build, full flash
- `STAC_v3.0.0-RC.9_WS_D3.bin` - Waveshare S3, debug build, OTA
- `STAC_v3.0.0-RC.9_WS_D3_FULL.bin` - Waveshare S3, debug build, full flash
- `STAC_v3.0.0-RC.9_WS_R.bin` - Waveshare S3, release build, OTA
- `STAC_v3.0.0-RC.9_WS_R_FULL.bin` - Waveshare S3, release build, full flash

### Board Codes

- `ATOM` - M5Stack ATOM Matrix (5x5 display)
- `WS` - Waveshare ESP32-S3-LCD (8x8 display)

### Build Codes

- `D3` - Debug build with LOG_LEVEL_DEBUG
- `R` - Release build (optimized, minimal logging)

## Building Firmware

### Build Single Environment

To build a specific environment with both OTA and full flash binaries:

```bash
cd /path/to/STAC
pio run -e <environment> -t merged -t ota
```

Examples:
```bash
# ATOM Matrix debug
pio run -e atom-matrix -t merged -t ota

# ATOM Matrix release
pio run -e atom-matrix-release -t merged -t ota

# Waveshare S3 debug
pio run -e waveshare-s3 -t merged -t ota

# Waveshare S3 release
pio run -e waveshare-s3-release -t merged -t ota
```

### Build All Environments

To build all firmware variants (8 binaries total):

```bash
cd /path/to/STAC
pio run -e atom-matrix -t merged -t ota
pio run -e atom-matrix-release -t merged -t ota
pio run -e waveshare-s3 -t merged -t ota
pio run -e waveshare-s3-release -t merged -t ota
```

Or use a shell loop:
```bash
for env in atom-matrix atom-matrix-release waveshare-s3 waveshare-s3-release; do
    pio run -e $env -t merged -t ota
done
```

## Binary Types

### OTA Binaries (without _FULL suffix)

- **Purpose:** Over-The-Air updates
- **Contents:** Application code only
- **Use case:** Updating already-flashed devices via WiFi
- **Size:** ~1.1-1.3 MB

### Full Flash Binaries (with _FULL suffix)

- **Purpose:** Initial device flashing or complete re-flash
- **Contents:** Bootloader + partition table + application code
- **Use case:** Web Serial flashing, esptool flashing, factory programming
- **Size:** ~1.2-1.4 MB
- **Flash offset:** 0x0 (writes to beginning of flash)

## Output Location

All binaries are automatically copied into `bin/` in the repository root.

## Custom Targets

The build script provides two custom targets:

### `-t merged`
Creates the full flash binary with bootloader and partitions.
- Uses `esptool merge-bin` (system) or `esptool.py merge_bin` (bundled)
- Fallback to Python merge if esptool unavailable
- Output: `STAC_v<version>_<board>_<build>_FULL.bin`

### `-t ota`
Creates the OTA update binary (app only).
- Copies the firmware binary with proper naming
- Output: `STAC_v<version>_<board>_<build>.bin`

## Version Management

The version is automatically extracted from:
```cpp
// include/Device_Config.h
#define STAC_SOFTWARE_VERSION "3.0.0-RC.9"
```

To change the version:
1. Update `STAC_SOFTWARE_VERSION` in `Device_Config.h`
2. Rebuild binaries - filenames will automatically update


## Notes

- **DO NOT** use `pio run` without `-t merged -t ota` for release binaries
- **DO NOT** manually copy or rename binaries
- **DO NOT** use old esptool.py syntax (use `esptool` with dashes, not `esptool.py` with underscores)
- The script automatically detects and uses system esptool if available
- Always verify the version in filenames matches `Device_Config.h`
- Clean builds may be needed after version changes: `pio run -e <env> -t clean`

## Troubleshooting

### Missing binaries
Ensure you're using both targets: `-t merged -t ota`

### Wrong version in filename
Rebuild after modifying `Device_Config.h`

### esptool errors
The script will automatically fall back to Python merge if esptool fails

### File not found errors
Check that all required build artifacts exist:
- `firmware.bin`
- `bootloader.bin`
- `partitions.bin`
