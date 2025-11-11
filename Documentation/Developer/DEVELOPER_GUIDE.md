# STAC Developer Guide

A comprehensive guide for developers working on STAC or building similar embedded systems projects.

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Project Structure](#project-structure)
- [Design Patterns](#design-patterns)
- [Adding Features](#adding-features)
- [Testing](#testing)
- [Code Style](#code-style)
- [Debugging](#debugging)
- [Common Tasks](#common-tasks)

---

## Architecture Overview

STAC uses a layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────────┐
│        Application Layer                │
│  (STACApp, Mode Handlers)               │
├─────────────────────────────────────────┤
│        State Management                 │
│  (TallyState, OperatingMode, System)    │
├─────────────────────────────────────────┤
│     Network & Storage Layer             │
│  (WiFi, ConfigManager, Web Server)      │
├─────────────────────────────────────────┤
│   Hardware Abstraction Layer (HAL)      │
│  (Display, IMU, Button, Interfaces)     │
├─────────────────────────────────────────┤
│        Configuration Layer              │
│  (Device_Config, Board Configs)         │
└─────────────────────────────────────────┘
```

### Key Principles

1. **Interface-Based Design**
   - All hardware accessed through interfaces (IDisplay, IIMU, IButton)
   - Enables testing with mocks
   - Easy to add new hardware implementations

2. **Compile-Time Configuration**
   - Hardware selection via `Device_Config.h`
   - Zero runtime overhead
   - Type-safe configuration
   - Compiler catches errors

3. **Factory Pattern**
   - Factories create appropriate implementations based on configuration
   - Application code doesn't know concrete types
   - Easy to extend with new hardware

4. **State Management**
   - Centralized state in SystemState
   - State change callbacks for coordination
   - Clear state transitions

5. **Dependency Injection**
   - Dependencies passed to constructors
   - Makes testing easier
   - Loose coupling between components

---

## Project Structure
```
STAC/                          # PlatformIO project (primary    
├── platformio.ini             # Build configuration
├── src/
│   ├── main.cpp               # Entry point
│   ├── Application/
│   │   └── STACApp.cpp        # Main application
│   ├── Hardware/
│   │   ├── Display/
│   │   │   ├── Matrix5x5/
│   │   │   │   └── Display5x5.cpp
│   │   │   └── Matrix8x8/
│   │   │       └── Display8x8.cpp
│   │   ├── Sensors/
│   │   │   ├── MPU6886_IMU.cpp
│   │   │   └── QMI8658_IMU.cpp
│   │   ├── Input/
│   │   │   └── ButtonHandler.cpp
│   │   └── Interface/
│   │       ├── GrovePort.cpp
│   │       └── PeripheralMode.cpp
│   ├── Network/
│   │   └── WiFiManager.cpp
│   ├── Storage/
│   │   └── ConfigManager.cpp
│   └── State/
│       ├── TallyStateManager.cpp
│       ├── OperatingModeManager.cpp
│       └── SystemState.cpp
└── include/
│   ├── Device_Config.h        # User edits this
│   ├── BoardConfigs/          # Board-specific configs
│   │   ├── AtomMatrix_Config.h
│   │   └── WaveshareS3_Config.h
│   ├── Config/
│   │   ├── Constants.h        # System constants
│   │   └── Types.h            # Common types
│   ├── Hardware/              # Hardware interfaces & headers
│   ├── Network/               # Network headers
│   ├── Storage/               # Storage headers
│   ├── State/                 # State management headers
│   └── Application/           # Application headers
└── Documentation/             # All documentation
```

### File Organization Rules

**Headers (.h):** `include/` directory

- Interface definitions
- Class declarations
- Inline functions
- Template definitions

**Implementations (.cpp):** `src/` directory

- Class implementations
- Non-inline functions

**Why this split?**

- PlatformIO standard
- Clean separation
- Faster compilation (headers included less often)

---

## Design Patterns

### 1. Interface Pattern (Abstract Base Classes)

**When:** Hardware abstraction, testability

**Example:**

```cpp
// IDisplay.h - Interface
class IDisplay {
public:
    virtual ~IDisplay() = default;
    virtual bool begin() = 0;
    virtual void fill(color_t color, bool show = true) = 0;
    // ... more methods
};

// Display5x5.h - Concrete implementation
class Display5x5 : public IDisplay {
public:
    bool begin() override { /* actual implementation */ }
    void fill(color_t color, bool show = true) override { /* ... */ }
};
```

**Benefits:**

- Polymorphism
- Testing with mocks
- Easy to add implementations

### 2. Factory Pattern

**When:** Creating hardware-specific implementations

**Example:**

```cpp
// DisplayFactory.h
class DisplayFactory {
public:
    static std::unique_ptr<IDisplay> create() {
        #if defined(GLYPH_SIZE_5X5)
            return std::make_unique<Display5x5>(...);
        #elif defined(GLYPH_SIZE_8X8)
            return std::make_unique<Display8x8>(...);
        #endif
    }
};

// Usage in application
auto display = DisplayFactory::create();  // Gets correct type
```

**Benefits:**

- Compile-time selection
- Application code doesn't know concrete types
- Easy to extend

### 3. Singleton Pattern (Careful Use)

**When:** Single system-wide instance needed (WiFi, Config)

**Example:**

```cpp
// Not a true singleton, but managed at application level
class STACApp {
private:
    std::unique_ptr<WiFiManager> wifiManager;  // Only one
    std::unique_ptr<SystemState> systemState;  // Only one
};
```

**Why not global singletons?**

- Harder to test
- Hidden dependencies
- Initialization order issues

**Better:** Pass references where needed

### 4. Observer Pattern (Callbacks)

**When:** Component needs to notify others of changes

**Example:**

```cpp
// TallyStateManager.h
class TallyStateManager {
public:
    using StateChangeCallback = std::function<void(TallyState, TallyState)>;
    
    void setStateChangeCallback(StateChangeCallback callback) {
        this->callback = callback;
    }
    
    bool setState(TallyState newState) {
        if (newState != currentState) {
            callback(currentState, newState);  // Notify observers
            currentState = newState;
        }
    }
};

// Usage
tallyState.setStateChangeCallback([](TallyState old, TallyState new) {
    log_i("State changed: %d -> %d", old, new);
});
```

**Benefits:**

- Loose coupling
- Multiple observers possible
- Event-driven architecture

### 5. Strategy Pattern (Mode Handlers)

**When:** Different behaviors based on mode

**Example:**

```cpp
void STACApp::loop() {
    switch (systemState->getOperatingMode().getCurrentMode()) {
        case OperatingMode::NORMAL:
            handleNormalMode();
            break;
        case OperatingMode::PERIPHERAL:
            handlePeripheralMode();
            break;
        case OperatingMode::PROVISIONING:
            handleProvisioningMode();
            break;
    }
}
```

**Could be improved with:**

```cpp
// Strategy pattern (future refactor)
class OperatingModeHandler {
public:
    virtual void handle() = 0;
};

class NormalModeHandler : public OperatingModeHandler { ... };
class PeripheralModeHandler : public OperatingModeHandler { ... };
```

---

## Adding Features

### Adding a New Hardware Platform

See [HARDWARE_CONFIG.md](HARDWARE_CONFIG.md) for detailed steps.

**Quick summary:**

1. Create `BoardConfigs/YourBoard_Config.h`
2. Define all pins and settings
3. Add to `Device_Config.h`
4. Test thoroughly
5. Submit PR with documentation

### Adding a New Display Type

**Example: Adding a TFT display**

**Step 1:** Create interface implementation

```cpp
// include/Hardware/Display/TFT/TFTDisplay.h
#include "../IDisplay.h"
#include <TFT_eSPI.h>

class TFTDisplay : public IDisplay {
public:
    TFTDisplay(uint8_t width, uint8_t height);
    bool begin() override;
    void fill(color_t color, bool show = true) override;
    // ... implement all IDisplay methods
private:
    TFT_eSPI tft;
};
```

**Step 2:** Implement in .cpp

```cpp
// src/Hardware/Display/TFT/TFTDisplay.cpp
#include "Hardware/Display/TFT/TFTDisplay.h"

TFTDisplay::TFTDisplay(uint8_t width, uint8_t height)
    : tft()
{
}

bool TFTDisplay::begin() {
    tft.init();
    tft.setRotation(1);
    return true;
}

void TFTDisplay::fill(color_t color, bool show) {
    tft.fillScreen(color);
}
// ... rest of implementation
```

**Step 3:** Add to factory

```cpp
// include/Hardware/Display/DisplayFactory.h
#if defined(GLYPH_SIZE_5X5)
    #include "Matrix5x5/Display5x5.h"
    using DisplayImpl = Display5x5;
#elif defined(GLYPH_SIZE_8X8)
    #include "Matrix8x8/Display8x8.h"
    using DisplayImpl = Display8x8;
#elif defined(DISPLAY_TYPE_TFT)  // NEW
    #include "TFT/TFTDisplay.h"
    using DisplayImpl = TFTDisplay;
#endif
```

**Step 4:** Update board config

```cpp
// BoardConfigs/TFTBoard_Config.h
#define DISPLAY_TYPE_TFT
#define DISPLAY_TFT_WIDTH 240
#define DISPLAY_TFT_HEIGHT 320
// ... TFT-specific settings
```

### Adding a New IMU

**Example: Adding BMP280 (just accelerometer)**

**Step 1:** Create implementation

```cpp
// include/Hardware/Sensors/BMP280_IMU.h
#include "IIMU.h"
#include <Adafruit_BMP280.h>

class BMP280_IMU : public IIMU {
public:
    BMP280_IMU(uint8_t sclPin, uint8_t sdaPin);
    bool begin() override;
    Orientation getOrientation() override;
    bool isAvailable() const override;
    const char* getType() const override { return "BMP280"; }
private:
    Adafruit_BMP280 sensor;
    bool initialized;
};
```

**Step 2:** Add to factory

```cpp
// include/Hardware/Sensors/IMUFactory.h
#elif defined(IMU_TYPE_BMP280)  // NEW
    #include "BMP280_IMU.h"
    using IMUImpl = BMP280_IMU;
#endif
```

**Step 3:** Add to board config

```cpp
#define IMU_TYPE_BMP280
#define IMU_HAS_IMU true
// ... I2C pins
```

### Adding a State Machine State

**Example: Adding a "Calibration" operating mode**

**Step 1:** Add to enum

```cpp
// include/Config/Types.h
enum class OperatingMode : uint8_t {
    NORMAL,
    PERIPHERAL,
    PROVISIONING,
    CALIBRATION  // NEW
};
```

**Step 2:** Add handler

```cpp
// src/Application/STACApp.cpp
void STACApp::handleCalibrationMode() {
    // Calibration logic here
    log_i("Running calibration...");
}
```

**Step 3:** Add to loop

```cpp
void STACApp::loop() {
    switch (systemState->getOperatingMode().getCurrentMode()) {
        case OperatingMode::NORMAL:
            handleNormalMode();
            break;
        case OperatingMode::CALIBRATION:  // NEW
            handleCalibrationMode();
            break;
        // ...
    }
}
```

**Step 4:** Add trigger

```cpp
// In handleButton() or wherever appropriate
if (specialCondition) {
    systemState->getOperatingMode().setMode(OperatingMode::CALIBRATION);
}
```

---

## Testing

### Unit Testing (Future)

STAC is designed for testability but doesn't yet have unit tests.

**What's testable:**

- State managers (TallyStateManager, OperatingModeManager)
- Utility functions
- Configuration parsing

**What requires mocking:**

- Hardware interfaces (IDisplay, IIMU, IButton)
- Network (WiFiManager)
- Storage (ConfigManager)

**Example test structure:**

```cpp
// test/test_tally_state.cpp
#include <unity.h>
#include "State/TallyStateManager.h"

void test_state_transitions() {
    TallyStateManager mgr;
    
    TEST_ASSERT_EQUAL(TallyState::NO_TALLY, mgr.getCurrentState());
    
    mgr.setState(TallyState::PREVIEW);
    TEST_ASSERT_EQUAL(TallyState::PREVIEW, mgr.getCurrentState());
}
```

### Hardware-in-the-Loop Testing

**Required:**

- Test on actual hardware
- Both ATOM Matrix and Waveshare
- All operating modes

**Test checklist:**

- [ ] Startup animation
- [ ] Button input (short press, long press)
- [ ] IMU orientation detection
- [ ] Display colors correct
- [ ] Normal mode operation
- [ ] Peripheral mode operation
- [ ] GROVE port I/O
- [ ] WiFi connection
- [ ] Configuration save/load
- [ ] State transitions
- [ ] Error handling

### Integration Testing

**Two-device test:**

1. Device A in Normal mode
2. Device B in Peripheral mode
3. Connect GROVE ports
4. Button press on A should update display on B

**Expected behavior:**

- NO_TALLY → PREVIEW → PROGRAM → UNSELECTED → NO_TALLY
- B mirrors A's display
- <2ms latency

---

## Code Style

### Naming Conventions

**Classes:** PascalCase

```cpp
class DisplayFactory { ... };
class TallyStateManager { ... };
```

**Functions/Methods:** camelCase

```cpp
void updateDisplay();
bool isConnected() const;
```

**Variables:** camelCase

```cpp
int buttonPressCount;
unsigned long lastUpdate;
```

**Constants:** UPPER_SNAKE_CASE or constexpr

```cpp
#define MAX_RETRIES 5
constexpr int BUFFER_SIZE = 128;
```

**Namespaces:** PascalCase

```cpp
namespace STAC {
namespace Hardware {
    // ...
}
}
```

**Private members:** camelCase (no prefix/suffix)

```cpp
class Example {
private:
    int internalCounter;    // Good
    bool isInitialized;     // Good
    // NOT: m_counter, _initialized, counter_, etc.
};
```

### File Organization

**Header guards:** `#ifndef STAC_COMPONENT_NAME_H`

```cpp
#ifndef STAC_DISPLAY_FACTORY_H
#define STAC_DISPLAY_FACTORY_H
// ...
#endif
```

**Include order:**

1. Own header (in .cpp files)
2. Standard library
3. Third-party libraries
4. Project headers

```cpp
#include "MyClass.h"        // Own header first

#include <Arduino.h>        // Standard/system
#include <memory>

#include <LiteLED.h>        // Third-party

#include "Config/Types.h"   // Project headers
#include "Hardware/IDisplay.h"
```

### Comments

**Function documentation:**

```cpp
/**
 * @brief Initialize the WiFi subsystem
 * 
 * Configures WiFi hardware and prepares for connection.
 * Must be called before connect() or startAP().
 * 
 * @return true if initialization succeeded
 */
bool begin();
```

**Inline comments:**

```cpp
// Use for brief explanations
int retryCount = 0;  // Number of connection attempts

/* Use for longer explanations
   that span multiple lines and
   need more detail */
```

**TODO comments:**

```cpp
// TODO: Implement Roland V-160HD protocol
// FIXME: Button debounce needs tuning
// HACK: Temporary workaround for GPIO issue
```

### Formatting

**Indentation:** 4 spaces (no tabs)

**Braces:** Opening on same line

```cpp
if (condition) {
    doSomething();
}
```

**Line length:** Aim for 100 characters, hard limit 120

**Whitespace:**

```cpp
// Good
int value = calculateValue(param1, param2);
if (value > threshold) {
    process(value);
}

// Bad
int value=calculateValue(param1,param2);
if(value>threshold){
    process(value);
}
```

---

## Debugging

### Serial Logging

**Log levels (ESP32):**

```cpp
log_e("Error: %s", errorMsg);       // ERROR
log_w("Warning: %d", value);        // WARNING
log_i("Info: connected");           // INFO
log_d("Debug: x=%d", x);            // DEBUG
log_v("Verbose: entering loop");    // VERBOSE
```

**Setting log level in platformio.ini:**

```ini
[debug_level]
build_flags =
    -DCORE_DEBUG_LEVEL=3  ; 3 = INFO, 4 = DEBUG, 5 = VERBOSE
```

**Conditional logging:**

```cpp
#if CORE_DEBUG_LEVEL >= 4
    log_d("Detailed debug info: %d, %d, %d", a, b, c);
#endif
```

### Common Issues

**Problem:** Device keeps resetting

- **Check:** Power supply (USB may not provide enough current)
- **Check:** Display brightness (lower to reduce current)
- **Debug:** Add `log_i()` at start of `setup()` to see if it reaches there

**Problem:** WiFi won't connect

- **Check:** SSID/password correct
- **Check:** 2.4GHz network (ESP32 doesn't support 5GHz)
- **Debug:** Enable WiFi debug: `-DCORE_DEBUG_LEVEL=5`

**Problem:** Button not responding

- **Check:** GPIO pin number correct
- **Check:** Active low/high setting
- **Debug:** Add `log_d()` in `ButtonHandler::update()`

**Problem:** IMU orientation wrong

- **Fix:** Adjust `IMU_ORIENTATION_OFFSET` (0-3)
- **Debug:** Log raw accelerometer values

**Problem:** Display wrong colors

- **Fix:** Toggle `DISPLAY_COLOR_ORDER_RGB` ↔ `DISPLAY_COLOR_ORDER_GRB`
- **Test:** Use `display->fill(StandardColors::RED, true)` to verify

### Using `esp32_exception_decoder`

When you get a crash with a stack trace:

```
Guru Meditation Error: Core  0 panic'ed (LoadProhibited)
PC: 0x400d1234 SP: 0x3ffb2340
Backtrace: 0x400d1234 0x400d5678 0x400d9abc
```

The `esp32_exception_decoder` filter (enabled in platformio.ini) automatically decodes this to:

```
PC: 0x400d1234: loop() at main.cpp:45
Backtrace:
  0x400d1234: loop() at main.cpp:45
  0x400d5678: loopTask() at core.cpp:123
```

Shows exactly where the crash occurred!

### Hardware Debugging Tools

**Multimeter:**

- Check voltage levels (should be 3.3V)
- Check continuity for jumper wires
- Measure current draw

**Logic Analyzer:**

- Analyze I2C communication (IMU)
- Debug WS2812 data signal
- Check GROVE port signals

**Oscilloscope:**

- Verify WS2812 timing
- Check button debounce
- Power supply stability

---

## Common Tasks

### Changing Default Settings

**Brightness:**

```cpp
// BoardConfigs/YourBoard_Config.h
#define DISPLAY_BRIGHTNESS_DEFAULT 30  // Was 20
```

**Button timing:**

```cpp
#define BUTTON_DEBOUNCE_MS 50          // Was 25
#define TIMING_BUTTON_SELECT_MS 2000   // Was 1500 (long press)
```

**WiFi timeout:**

```cpp
#define TIMING_WIFI_CONNECT_TIMEOUT_MS 30000  // Was 60000 (30 seconds)
```

### Adding a New Color

```cpp
// include/Hardware/Display/Colors.h
namespace StandardColors {
    // ... existing colors ...
    constexpr color_t PINK = makeRGB(255, 192, 203);  // NEW
}
```

### Changing Tally Color Scheme

```cpp
// include/Hardware/Display/Colors.h
namespace STACColors {
    constexpr color_t PROGRAM = StandardColors::YELLOW;  // Was RED
    constexpr color_t PREVIEW = StandardColors::BLUE;    // Was GREEN
    // ...
}
```

### Adding a Configuration Parameter

**Step 1:** Add to types

```cpp
// include/Config/Types.h
struct StacOperations {
    // ... existing fields ...
    uint8_t newParameter;  // NEW
};
```

**Step 2:** Add to ConfigManager

```cpp
// src/Storage/ConfigManager.cpp
bool ConfigManager::saveOperations(const StacOperations& ops) {
    // ... existing saves ...
    prefs.putUChar("newParam", ops.newParameter);  // NEW
}

bool ConfigManager::loadOperations(StacOperations& ops) {
    // ... existing loads ...
    ops.newParameter = prefs.getUChar("newParam", 42);  // NEW (with default)
}
```

**Step 3:** Use in application

```cpp
// src/Application/STACApp.cpp
void STACApp::someFunction() {
    uint8_t value = systemState->getOperations().newParameter;
    // Use value...
}
```

### Profiling Performance

**Measure execution time:**

```cpp
unsigned long startTime = micros();
someFunction();
unsigned long elapsed = micros() - startTime;
log_i("someFunction() took %lu microseconds", elapsed);
```

**Measure loop frequency:**

```cpp
void loop() {
    static unsigned long lastLoop = 0;
    static uint32_t loopCount = 0;
    
    loopCount++;
    
    if (millis() - lastLoop > 1000) {
        log_i("Loop frequency: %lu Hz", loopCount);
        loopCount = 0;
        lastLoop = millis();
    }
    
    // Rest of loop...
}
```

---

## Contributing

### Workflow

1. **Fork** the repository
2. **Create branch** from `dev`: `feature/my-new-feature`
3. **Make changes** and test thoroughly
4. **Commit** with clear messages
5. **Push** to your fork
6. **Create PR** to `dev` branch

### Commit Messages

**Format:**

```
Type: Short description

Longer explanation if needed.

- Bullet points for details
- Multiple changes listed
```

**Types:**

- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation
- `refactor:` Code restructure (no functionality change)
- `test:` Adding tests
- `chore:` Maintenance

**Examples:**

```
feat: Add support for WS2815 RGBW LEDs

- Update color handling for RGBW
- Add RGBW config option
- Test on hardware

fix: Button debounce not working on GPIO 39

GPIO 39 requires external pullup. Added config option
BUTTON_NEEDS_EXTERNAL_PULLUP for boards with input-only pins.
```

### Pull Request Checklist

Before submitting PR:

- [ ] Code compiles without warnings
- [ ] Tested on hardware (both boards if applicable)
- [ ] Documentation updated
- [ ] Code follows style guide
- [ ] Commit messages clear
- [ ] No debugging code left in
- [ ] Configuration files updated if needed

---

## Resources

### ESP32 Documentation
- [ESP32 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [PlatformIO ESP32](https://docs.platformio.org/en/latest/platforms/espressif32.html)

### Libraries
- [LiteLED Documentation](https://github.com/Xylopyrographer/LiteLED)
- [XP_Button Documentation](https://github.com/Xylopyrographer/XP_Button)

### Hardware
- [M5Stack ATOM Matrix](https://docs.m5stack.com/en/core/atom_matrix)
- [Waveshare ESP32-S3-Matrix](https://www.waveshare.com/wiki/ESP32-S3-Matrix)

### Tools
- [PlatformIO IDE](https://platformio.org/platformio-ide)
- [Fritzing](https://fritzing.org/) - Circuit diagrams
- [Doxygen](https://www.doxygen.nl/) - Documentation generator

---

**Happy coding!** 🚀

**Last Updated:** 2025-11-10 
**Version:** 2.3.0


<!-- //  --- EOF --- // -->
