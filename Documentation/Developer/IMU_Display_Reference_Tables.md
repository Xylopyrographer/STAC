# IMU and Display Reference Tables

## IMU Accelerometer Readings During Rotation

All tables show accelerometer readings as you rotate the IMU **clockwise** (as viewed from your perspective) in 90° increments.

**Perspective:**

- **Tables 1-4:** You are looking at the IMU with +Z pointing **away** from you (into the page)
- **Tables 5-8:** You are looking at the IMU with +Z pointing **toward** you (out of the page)
- "Clockwise" always means clockwise rotation as you see it from where you're standing

**Rotation Transform (CW about Z-axis pointing away):**

- At 0°: (X, Y) = (X₀, Y₀)
- At 90° CW: (X, Y) = (Y₀, -X₀)
- At 180°: (X, Y) = (-X₀, -Y₀)
- At 270° CW: (X, Y) = (-Y₀, X₀)

**Rotation Transform (CW about Z-axis pointing toward):**

- At 0°: (X, Y) = (X₀, Y₀)
- At 90° CW: (X, Y) = (-Y₀, X₀)
- At 180°: (X, Y) = (-X₀, -Y₀)
- At 270° CW: (X, Y) = (Y₀, -X₀)

---

### Table 1: +Z Away, +X Right (Standard Orientation)

**Initial State:** +Z points away from user, +X points right, +Y points down

| Rotation | X Value | Y Value | Description |
|----------|---------|---------|-------------|
| 0°       | +1      | 0       | +X points RIGHT |
| 90° CW   | 0       | +1      | +X points DOWN |
| 180°     | -1      | 0       | +X points LEFT |
| 270° CW  | 0       | -1      | +X points UP |

**Pattern:** `X: (+1, 0, -1, 0)`, `Y: (0, +1, 0, -1)`

---

### Table 2: +Z Away, +X Down

**Initial State:** +Z points away from user, +X points down, +Y points left

| Rotation | X Value | Y Value | Description |
|----------|---------|---------|-------------|
| 0°       | 0       | +1      | +X points DOWN |
| 90° CW   | -1      | 0       | +X points LEFT |
| 180°     | 0       | -1      | +X points UP |
| 270° CW  | +1      | 0       | +X points RIGHT |

**Pattern:** `X: (0, -1, 0, +1)`, `Y: (+1, 0, -1, 0)`

---

### Table 3: +Z Away, +X Left

**Initial State:** +Z points away from user, +X points left, +Y points up

| Rotation | X Value | Y Value | Description |
|----------|---------|---------|-------------|
| 0°       | -1      | 0       | +X points LEFT |
| 90° CW   | 0       | -1      | +X points UP |
| 180°     | +1      | 0       | +X points RIGHT |
| 270° CW  | 0       | +1      | +X points DOWN |

**Pattern:** `X: (-1, 0, +1, 0)`, `Y: (0, -1, 0, +1)`

---

### Table 4: +Z Away, +X Up

**Initial State:** +Z points away from user, +X points up, +Y points right

| Rotation | X Value | Y Value | Description |
|----------|---------|---------|-------------|
| 0°       | 0       | -1      | +X points UP |
| 90° CW   | +1      | 0       | +X points RIGHT |
| 180°     | 0       | +1      | +X points DOWN |
| 270° CW  | -1      | 0       | +X points LEFT |

**Pattern:** `X: (0, +1, 0, -1)`, `Y: (-1, 0, +1, 0)`

---

### Table 5: +Z Toward, +X Right (Reversed Z)

**Initial State:** +Z points toward user, +X points right, +Y points up

| Rotation | X Value | Y Value | Description |
|----------|---------|---------|-------------|
| 0°       | +1      | 0       | +X points RIGHT |
| 90° CW   | 0       | -1      | +X points UP |
| 180°     | -1      | 0       | +X points LEFT |
| 270° CW  | 0       | +1      | +X points DOWN |

**Pattern:** `X: (+1, 0, -1, 0)`, `Y: (0, -1, 0, +1)`

**Note:** Same X pattern as Table 1, but Y pattern is inverted!

---

### Table 6: +Z Toward, +X Down

**Initial State:** +Z points toward user, +X points down, +Y points right

| Rotation | X Value | Y Value | Description |
|----------|---------|---------|-------------|
| 0°       | 0       | -1      | +X points DOWN |
| 90° CW   | +1      | 0       | +X points RIGHT |
| 180°     | 0       | +1      | +X points UP |
| 270° CW  | -1      | 0       | +X points LEFT |

**Pattern:** `X: (0, +1, 0, -1)`, `Y: (-1, 0, +1, 0)`

---

### Table 7: +Z Toward, +X Left

**Initial State:** +Z points toward user, +X points left, +Y points down

| Rotation | X Value | Y Value | Description |
|----------|---------|---------|-------------|
| 0°       | -1      | 0       | +X points LEFT |
| 90° CW   | 0       | +1      | +X points DOWN |
| 180°     | +1      | 0       | +X points RIGHT |
| 270° CW  | 0       | -1      | +X points UP |

**Pattern:** `X: (-1, 0, +1, 0)`, `Y: (0, +1, 0, -1)`

**Note:** Same X pattern as Table 3, but Y pattern is inverted!

---

### Table 8: +Z Toward, +X Up

**Initial State:** +Z points toward user, +X points up, +Y points left

| Rotation | X Value | Y Value | Description |
|----------|---------|---------|-------------|
| 0°       | 0       | +1      | +X points UP |
| 90° CW   | -1      | 0       | +X points LEFT |
| 180°     | 0       | -1      | +X points DOWN |
| 270° CW  | +1      | 0       | +X points RIGHT |

**Pattern:** `X: (0, -1, 0, +1)`, `Y: (+1, 0, -1, 0)`

---

## Pattern Analysis

### Z+ Away from User (Tables 1-4)

- **Table 1 (+X Right):** `X: (+1, 0, -1, 0)`, `Y: (0, +1, 0, -1)`
- **Table 2 (+X Down):** `X: (0, -1, 0, +1)`, `Y: (+1, 0, -1, 0)`
- **Table 3 (+X Left):** `X: (-1, 0, +1, 0)`, `Y: (0, -1, 0, +1)`
- **Table 4 (+X Up):** `X: (0, +1, 0, -1)`, `Y: (-1, 0, +1, 0)`

**Observation:** All four patterns are rotations of each other! This is the **4-fold rotational symmetry** that caused the calibration problem.

### Z+ Toward User (Tables 5-8)

- **Table 5 (+X Right):** `X: (+1, 0, -1, 0)`, `Y: (0, -1, 0, +1)`
- **Table 6 (+X Down):** `X: (0, +1, 0, -1)`, `Y: (-1, 0, +1, 0)`
- **Table 7 (+X Left):** `X: (-1, 0, +1, 0)`, `Y: (0, +1, 0, -1)`
- **Table 8 (+X Up):** `X: (0, -1, 0, +1)`, `Y: (+1, 0, -1, 0)`

**Observation:** Same X patterns as Z+ away, but Y patterns are inverted! This allows distinguishing Z direction.

---

## Display LUT Rotation Tables

### Baseline Assumption

- **`LUT_0 (ROTATE_0)`:** Display buffer pixel 0 is at the **top-left** corner
- Physical display rotation is **clockwise** (CW) when viewing from the front
- Content rotation is **counter-clockwise** (CCW) to compensate and keep characters upright

---

### Display Rotation Compensation Table

| Physical Display Rotation | Pixel 0 Location | Content Rotation Needed | LUT Name | Orientation Enum |
|---------------------------|------------------|-------------------------|----------|------------------|
| 0° (baseline) | Top-Left (TL) | 0° | `LUT_ROTATE_0` | `Orientation::ROTATE_0` |
| 90° CW | Top-Right (TR) | -90° (= +270° CCW) | `LUT_ROTATE_270` | `Orientation::ROTATE_270` |
| 180° | Bottom-Right (BR) | -180° (= +180°) | `LUT_ROTATE_180` | `Orientation::ROTATE_180` |
| 270° CW | Bottom-Left (BL) | -270° (= +90° CCW) | `LUT_ROTATE_90` | `Orientation::ROTATE_90` |

---

### LUT Naming Convention

In our implementation:

| Content Rotation | LUT Enum Name | Rotation Direction | Notes |
|------------------|---------------|-------------------|--------|
| 0° | `Orientation::ROTATE_0` | No rotation | Baseline |
| -90° (or +270° CCW) | `Orientation::ROTATE_270` | Rotate pixels 270° CW | Compensates for 90° CW physical rotation |
| -180° (or +180°) | `Orientation::ROTATE_180` | Rotate pixels 180° | Compensates for 180° physical rotation |
| -270° (or +90° CCW) | `Orientation::ROTATE_90` | Rotate pixels 90° CW | Compensates for 270° CW physical rotation |

---

### Visual Example: Character 'A' Display

```
Physical 0° (pixel 0 at TL):          Physical 90° CW (pixel 0 at TR):
┌─────┐                               ┌─────┐
│A    │  ← Use LUT_ROTATE_0           │    A│  ← Use LUT_ROTATE_270
│     │                               │     │     (content rotated -90°)
└─────┘                               └─────┘

Physical 180° (pixel 0 at BR):        Physical 270° CW (pixel 0 at BL):
┌─────┐                               ┌─────┐
│     │  ← Use LUT_ROTATE_180         │     │  ← Use LUT_ROTATE_90
│    A│     (content rotated -180°)   │A    │     (content rotated -270° = +90°)
└─────┘                               └─────┘
```

---

## Key Insights

1. **IMU Symmetry Problem:** With Z-axis alone, we cannot distinguish between Tables 1-4 because they're all rotations of the same pattern. This is why corner identification is needed.

2. **Z-Direction Detection:** Comparing Y pattern polarity tells us if Z points away (Tables 1-4) or toward (Tables 5-8) the user.

3. **Corner Identification Breaks Symmetry:** By identifying which physical corner appears as top-left, we establish an absolute reference frame that makes the axis remap calculation unique and invariant.

4. **LUT Naming Convention:** `LUT_ROTATE_N` means "rotate pixel content N degrees clockwise to compensate for physical rotation." The number represents the **content rotation**, not the physical device rotation.

5. **Inverse Relationship:** 
   - Physical rotation 90° CW → Use `LUT_ROTATE_270` (rotate content -90° or +270°)
   - Physical rotation 270° CW → Use `LUT_ROTATE_90` (rotate content -270° or +90°)


<!-- EOF -->
