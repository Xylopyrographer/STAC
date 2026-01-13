# IMU Calibration Methodology

## Overview

This document describes the methodology for calibrating IMU orientation and display rotation for STAC devices. The approach is purely empirical and deterministic, requiring no heuristics or string parsing.

## Fundamental Principles

### Right-Hand Rule Coordinate System

All IMUs follow the right-hand rule:

- X, Y, Z axes are orthogonal
- When flat: Z+ typically points up (away from table)
- Rotation about Z-axis follows right-hand rule (curl fingers in rotation direction, thumb points along positive axis)

### Z-Axis Direction Determines Rotation Pattern

The key insight: **The sign of the Z-axis when the device is horizontal determines which rotation pattern the device will exhibit.**

#### Pattern 1: Z+ Points Away (Standard Pattern)

**Setup:** Z+ away from user, X+ points up, Y+ points right

| Rotation | X-axis Direction | Y-axis Direction | accelX | accelY |
|----------|------------------|------------------|---------|---------|
| 0°       | UP              | RIGHT            | -1g     | 0g      |
| 90°      | RIGHT           | DOWN             | 0g      | -1g     |
| 180°     | DOWN            | LEFT             | +1g     | 0g      |
| 270°     | LEFT            | UP               | 0g      | +1g     |

**Pattern:** X: (-1, 0, +1, 0), Y: (0, -1, 0, +1)

#### Pattern 2: Z+ Points Toward (Reversed Pattern)

**Setup:** Z+ toward user, X+ points up, Y+ points left

| Rotation | X-axis Direction | Y-axis Direction | accelX | accelY |
|----------|------------------|------------------|---------|---------|
| 0°       | UP              | LEFT             | +1g     | 0g      |
| 90°      | RIGHT           | UP               | 0g      | +1g     |
| 180°     | DOWN            | RIGHT            | -1g     | 0g      |
| 270°     | LEFT            | DOWN             | 0g      | -1g     |

**Pattern:** X: (+1, 0, -1, 0), Y: (0, +1, 0, -1)

### Z-Axis Measurement Rule

When device is **horizontal with display facing up:**

- **Z ≈ -1g** → Z+ points DOWN (into table) → When vertical, Z+ points **away** from user → Use Pattern 1
- **Z ≈ +1g** → Z+ points UP (away from table) → When vertical, Z+ points **toward** user → Use Pattern 2

## Calibration Algorithm

### Step 1: Measure Device Flat (Display Up)

1. Place device flat on table, display facing up
2. Record: `Z_flat`
3. Determine pattern:
   - If `Z_flat < -0.5g` → Use Pattern 1 (Z+ away)
   - If `Z_flat > +0.5g` → Use Pattern 2 (Z+ toward)

### Step 2: Measure Four Vertical Rotations

For rotations 0°, 90°, 180°, 270° (clockwise when viewed from above):

1. Hold device **vertical** at each rotation
2. Record raw sensor values: `sensor.x`, `sensor.y`, `sensor.z`
3. Normalize to pattern values:
   - Find dominant axis: `|accel| > 0.7g`
   - Normalize to ±1 based on sign
   - Non-dominant axis → 0

Result: Four measurements, each normalized to one of:
- (-1, 0), (0, -1), (+1, 0), (0, +1)

### Step 3: Match Pattern to Find Axis Remapping

Test all 8 possible axis remappings:

1. `(sensor.x, sensor.y)`
2. `(sensor.x, -sensor.y)`
3. `(-sensor.x, sensor.y)`
4. `(-sensor.x, -sensor.y)`
5. `(sensor.y, sensor.x)`
6. `(sensor.y, -sensor.x)`
7. `(-sensor.y, sensor.x)`
8. `(-sensor.y, -sensor.x)`

For each remapping:

- Apply to all 4 measurements
- Normalize to ±1 or 0
- Compare to expected pattern (Pattern 1 or Pattern 2)
- Count matches

**The correct remapping produces 4/4 matches** with the expected pattern.

This gives us:

- `IMU_AXIS_REMAP_X`
- `IMU_AXIS_REMAP_Y`
- `IMU_AXIS_REMAP_Z` (typically just `acc.z` or `-acc.z`)

### Step 4: Identify Device Home Rotation

From Step 3, we now know which of the 4 rotations produces the pattern's starting point.

For Pattern 1: Device home is the rotation where (boardX ≈ -1g, boardY ≈ 0g)

For Pattern 2: Device home is the rotation where (boardX ≈ +1g, boardY ≈ 0g)

**Device home** = rotation index (0, 1, 2, or 3) where this occurs.

### Step 5: Measure Display Rotation Offset

1. Put device at **device home rotation**
2. Light **pixel 0** (display buffer index 0)
3. Ask user: "Which corner is the lit pixel in?"
   - Top-Left (TL)
   - Top-Right (TR)
   - Bottom-Right (BR)
   - Bottom-Left (BL)

4. Calculate display offset:
   - TL → `display_offset = 0°` (no rotation needed)
   - TR → `display_offset = 270°` (need +90° CW LUT)
   - BR → `display_offset = 180°` (need +180° LUT)
   - BL → `display_offset = 90°` (need +270° CW LUT)

### Step 6: Calculate LUT Mapping

**CRITICAL INSIGHT**: Display rotation LUTs rotate content in the **same** direction as their name (e.g., `LUT_ROTATE_90` rotates pixels 90° clockwise). But when the device rotates, we need to rotate the content in the **opposite** direction to keep it upright. Therefore, the LUT calculation must **invert** the rotation direction.

For each device rotation (0°, 90°, 180°, 270°):

```
LUT[rotation] = (360 - rotation + display_offset + device_home) % 360
```

The `360 - rotation` term inverts the rotation direction so that content rotates counter to the device.

Example:

- Device home = 0° (device at rotation index 0)
- Display offset = 0° (pixel 0 appeared at top-left)
- LUT[0°] = (360 - 0 + 0 + 0) % 360 = 0° → `ROTATE_0`
- LUT[90°] = (360 - 90 + 0 + 0) % 360 = 270° → `ROTATE_270`
- LUT[180°] = (360 - 180 + 0 + 0) % 360 = 180° → `ROTATE_180`
- LUT[270°] = (360 - 270 + 0 + 0) % 360 = 90° → `ROTATE_90`

This produces the correct inverse mapping where device rotating CW makes content rotate CCW.

## Output Format

The calibration tool outputs directly copy/paste-able configuration:

```cpp
// IMU Configuration (from calibration tool)
#define IMU_AXIS_REMAP_X    ((-acc.y))  // Example
#define IMU_AXIS_REMAP_Y    ((-acc.x))  // Example
#define IMU_AXIS_REMAP_Z    (acc.z)     // Example

// Display Rotation LUT
// Device home at physical 0°, display offset = 90°
#define DEVICE_ORIENTATION_TO_LUT_MAP { \
    Orientation::ROTATE_90,   /* Physical 0° → LUT_90 */ \
    Orientation::ROTATE_180,  /* Physical 90° → LUT_180 */ \
    Orientation::ROTATE_270,  /* Physical 180° → LUT_270 */ \
    Orientation::ROTATE_0,    /* Physical 270° → LUT_0 */ \
    Orientation::ROTATE_90,   /* FLAT → same as device home */ \
    Orientation::ROTATE_90    /* UNKNOWN → same as device home */ \
}
```

## Why This Works

### Pure Empirical Approach
- No string parsing or heuristics
- No assumptions about IMU mounting orientation
- Works for any combination of axis swaps and inversions

### Self-Validating
- Pattern matching must produce 4/4 correct results
- If no remapping achieves 4/4, measurements were incorrect

### Deterministic
- Only ONE axis remapping produces the correct pattern
- Only ONE LUT configuration produces correct display rotation
- No ambiguity or guesswork

### Complete Coverage
- Handles all 8 possible axis remappings (X/Y swaps with inversions)
- Handles all 4 possible device home positions
- Handles all 4 possible display mounting orientations
- Total: 8 × 4 × 4 = 128 possible configurations, all handled correctly

## References

- Right-hand rule: https://en.wikipedia.org/wiki/Right-hand_rule
- Coordinate system rotations: https://en.wikipedia.org/wiki/Rotation_matrix
- STAC IMU orientation system: `ARCHITECTURE_LINK_LAYER.md`

<!-- EOF -->
