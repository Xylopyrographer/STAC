# IMU Calibration Tool

## Overview

The IMU calibration tool is a development utility that determines the physical mounting configuration of your IMU sensor. It generates the necessary configuration values for your board config header file.

## Problem Statement

IMUs can be mounted in 8 different orientations:
- **Z-axis direction**: FORWARD (Z+ toward display) or AFT (Z+ away from display)
- **Rotation**: 0°, 90°, 180°, or 270° clockwise relative to board home position

Different board manufacturers may mount IMUs in different orientations, making it impossible to hardcode a single configuration that works for all boards.

## Solution

This interactive tool:
1. Shows a reference marker on the display
2. Asks you to report where the marker appears
3. Repeats this at 4 rotations (0°, 90°, 180°, 270°)
4. Calculates the IMU configuration from your responses
5. Outputs values to copy into your board config file

## Usage

### 1. Build and Upload

Choose the calibration environment for your board:


### 2. Open Serial Monitor

```bash
pio device monitor -b 115200
```

### 3. Follow On-Screen Instructions

The tool will guide you through:
1. Holding board in HOME position (USB port DOWN)
2. Observing where a reference marker appears
3. Rotating board 90° clockwise
4. Repeating for 180° and 270° rotations

### 4. Copy Configuration Values

At the end, the tool outputs values like:

```cpp
// IMU Configuration (from calibration tool)
#define IMU_AXIS_REMAP_X    (-acc.y)
#define IMU_AXIS_REMAP_Y    (acc.x)
#define IMU_AXIS_REMAP_Z    (acc.z)
#define IMU_FACE_DIRECTION  IMU_FACE_AFT
#define IMU_ROTATION_OFFSET OFFSET_90
```

Copy these into your board config header file (e.g., `include/BoardConfigs/YourBoard_Config.h`).

### 5. Rebuild Normal Firmware

After updating your board config:

```bash
pio run -e your-board -t upload
```

## Display Types

### LED Matrix (ATOM, Waveshare)
- Shows bright white pixel at top-center position
- Very easy to see which edge it's on

### TFT Display (M5StickC Plus, LilyGO, AIPI)
- Shows white arrow pointing up from top edge
- Clear visual indicator

## Troubleshooting

### "Config not declared" errors
- Make sure you're building a calibration environment, not a regular one
- Calibration environments are: `<board>-calibrate`

### IMU not initialized
- Check your board config has IMU pins defined
- Some boards (LilyGO T-Display, AIPI-Lite) don't have IMUs

### Display shows nothing
- Check LED_DATA pin or TFT configuration in board config
- Verify backlight is enabled (TFT boards)

## Implementation Notes

### Minimal Build
Calibration environments exclude:
- Network code (WiFi, web server, OTA)
- State management
- Storage (NVS)
- Application logic

Only includes:
- Display drivers
- IMU drivers
- Calibration logic

This keeps binary size small and compilation fast.

### Data Collection
The tool reads raw accelerometer values at each rotation and correlates them with user-reported marker positions. The calculation algorithm will:
1. Determine which sensor axis corresponds to which board axis
2. Calculate polarity (+ or -) for each axis
3. Detect Z-axis direction (FORWARD/AFT)
4. Calculate rotation offset

## References

- Board configs: `include/BoardConfigs/`
- IMU drivers: `src/Hardware/Sensors/`

