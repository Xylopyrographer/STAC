# STAC v3 Startup Review and Recommendations

**Date:** November 24, 2025  
**Reviewer:** GitHub Copilot (Claude Sonnet 4.5)  
**Context:** Pre-release review of STAC v3 startup sequence vs baseline 2.2.0

---

## Executive Summary

The STAC v3 refactoring has successfully achieved functional parity with the baseline 2.2.0 code while introducing a cleaner, more maintainable architecture. However, the startup sequence has evolved organically during development and now contains opportunities for simplification that would:

1. **Improve maintainability** - Reduce code duplication and centralize decision logic
2. **Enhance readability** - Make the startup flow more linear and easier to follow
3. **Reduce complexity** - Eliminate redundant checks and state management
4. **Better match baseline semantics** - Align with proven patterns from 2.2.0

This review identifies **16 specific improvements** across **4 categories**: Quick Wins, Architectural Improvements, Code Organization, and Display System Refinements.

---

## Comparison: v3 vs Baseline Startup Flow

### Baseline 2.2.0 Startup Sequence (from documentation)

```
Power On → Serial Init → Display Init → Button Init → IMU Orient → 
Rotate Glyphs → Generate STAC ID → Peripheral Check → 
STACProvision (with button state machine) → User Config Sequence → 
Auto-start Timeout → WiFi Connect → STS Connect → Ready
```

**Key Characteristics:**

- **Single provisioning check** using `pVis` flag in NVS
- **Linear flow** with clear state progression
- **Centralized button handling** in provisioning state machine
- **Simple configuration loading** based on provisioned state

### STAC v3 Startup Sequence (current implementation)

```
main.cpp:setup() → STACApp::setup() → initializeHardware() → 
initializeNetworkAndStorage() → determineOperatingMode() → 
checkBootButtonSequence() → handleProvisioningMode() OR 
handleNormalMode() → startupConfig sequence → WiFi connect → 
Roland client init → Ready
```

**Key Characteristics:**

- **Multiple provisioning checks** scattered across `determineOperatingMode()`, `checkBootButtonSequence()`, and `handleNormalMode()`
- **Branching flow** with mode-specific handlers
- **Distributed button handling** in boot sequence AND startup config
- **Complex configuration loading** with multiple fallback paths

---

## Identified Issues and Recommendations

### Category 1: Quick Wins (Immediate Improvements)

#### 1. **Centralize Provisioning Status Check**

<!-- @robl: Agree. Though please review the proposed implementation against the arduino-esp32 Preferences API and Tutorial documentation at:
https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html

and

https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/preferences.html

Testing for the existence keys within a namespace before use is the preferred method when the state of the namespace is unknown (as when we reset of power on the device) especially considering the case where the entire flash has bee erased prior to the first time the firmware is flashed.
-->

**Issue:** The baseline uses a single `pVis` (provisioned) boolean flag in NVS to determine if the device needs provisioning. v3 checks multiple conditions:

- `configManager->hasWiFiCredentials()` 
- `configManager->loadSwitchConfig()` returns valid data
- `configManager->isConfigured()` (in boot button sequence)
- Repeated checks in `handleNormalMode()`

**Baseline Approach (`STACProvision.h`):**

```cpp
bool provisioned = false;
if (_stcPrefs.isKey("pVis")) {
    provisioned = _stcPrefs.getBool("pVis");
}

if (!provisioned || !goodPrefs) {
    // Enter provisioning
    STACconfig(...);
}
```

**Recommendation:**

Add a single `isProvisioned` boolean flag to ConfigManager:

```cpp
// In ConfigManager
bool isProvisioned() const {
    Preferences prefs;
    prefs.begin(NS_STAC, true);  // READ_ONLY
    bool provisioned = prefs.getBool("provisioned", false);
    prefs.end();
    return provisioned;
}

void setProvisioned(bool state) {
    Preferences prefs;
    prefs.begin(NS_STAC, false);  // READ_WRITE
    prefs.putBool("provisioned", state);
    prefs.end();
}
```

**Impact:** 

- Eliminates 4 redundant configuration checks
- Matches baseline semantics exactly
- Single source of truth for provisioning state

---

#### 2. **Simplify Boot Button Sequence Logic**

**Issue:** `checkBootButtonSequence()` has different behavior for configured vs unconfigured devices. If unconfigured, it starts at `OTA_UPDATE_PENDING` and skips provisioning/factory reset states. This adds complexity.

**Current Logic:**

```cpp
if (!configManager->isConfigured()) {
    state = BootButtonState::OTA_UPDATE_PENDING;
    // Display OTA glyph
} else {
    state = BootButtonState::PROVISIONING_PENDING;
    // Display config glyph
}
```

**Baseline Approach:**

The baseline handles boot button sequence AFTER checking provisioning status in `STACProvision()`. The state machine is consistent regardless of provisioning state.

**Recommendation:**

Always run the full button state machine (CFG → FR → DFU), but handle the provisioning action differently:

```cpp
// In checkBootButtonSequence() - always start at CFG_PEND
state = BootButtonState::PROVISIONING_PENDING;

// When provisioning is selected:
if (!button->isPressed()) {
    if (configManager->isProvisioned()) {
        // Provisioned device: reconfigure
        return OperatingMode::PROVISIONING;
    } else {
        // Unconfigured device: first-time config (same as reconfigure)
        return OperatingMode::PROVISIONING;
    }
}
```

**Impact:**

- Consistent user experience
- Simpler state machine logic
- ~30 lines of code removed

---

#### 3. **Move Brightness Maps to Board Configuration**

<!-- @robl: DECISION - Move brightness map to board config .h file with compile-time validation.
Maps may differ across physical LED panels. Validate at compile time and error out if improperly 
defined rather than runtime check.
-->

**Issue:** Brightness maps are defined in `Constants.h` but are hardware-specific. Different LED panels may require different brightness curves. Maps should be in board configuration with validation.

**Current Code:**

```cpp
// In Config/Constants.h
namespace Display {
    constexpr uint8_t BRIGHTNESS_MAP_5X5[] = { 0, 10, 20, 30, 40, 50, 60 };
    constexpr uint8_t BRIGHTNESS_MAP_8X8[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80 };
}

// Usage (3+ locations):
#ifdef GLYPH_SIZE_5X5
    display->setBrightness(Config::Display::BRIGHTNESS_MAP_5X5[1], false);
#else
    display->setBrightness(Config::Display::BRIGHTNESS_MAP_8X8[1], false);
#endif
```

**Recommendation:**

Move brightness maps to board-specific configuration files:

```cpp
// In BoardConfigs/AtomMatrix_Config.h
namespace Config {
    namespace Display {
        constexpr uint8_t BRIGHTNESS_MAP[] = { 0, 10, 20, 30, 40, 50, 60 };
        constexpr uint8_t BRIGHTNESS_LEVELS = (sizeof(BRIGHTNESS_MAP) / sizeof(uint8_t)) - 1;
        
        // Compile-time validation
        static_assert(sizeof(BRIGHTNESS_MAP) >= 2, "Brightness map must have at least 2 entries");
        static_assert(BRIGHTNESS_MAP[0] == 0, "First brightness entry must be 0");
    }
}

// In BoardConfigs/WaveshareS3_Config.h  
namespace Config {
    namespace Display {
        constexpr uint8_t BRIGHTNESS_MAP[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80 };
        constexpr uint8_t BRIGHTNESS_LEVELS = (sizeof(BRIGHTNESS_MAP) / sizeof(uint8_t)) - 1;
        
        static_assert(sizeof(BRIGHTNESS_MAP) >= 2, "Brightness map must have at least 2 entries");
        static_assert(BRIGHTNESS_MAP[0] == 0, "First brightness entry must be 0");
    }
}

// Usage everywhere (no #ifdef needed):
display->setBrightness(Config::Display::BRIGHTNESS_MAP[1], false);
display->setBrightness(Config::Display::BRIGHTNESS_MAP[ops.displayBrightnessLevel], true);
```

**Impact:**
- Hardware-specific configuration in appropriate location
- Compile-time validation prevents invalid maps
- Eliminates all `#ifdef GLYPH_SIZE_nXn` for brightness
- Calculated brightness levels prevent manual counting errors
- Different panels can have different brightness curves

---

#### 4. **Eliminate Redundant `yield()` Calls**

**Issue:** `yield()` is called in autostart loop and peripheral mode polling loop. Modern FreeRTOS tasks yield automatically during `delay()` calls.

**Current Code:**

```cpp
while (millis() < timeout) {
    // ... button handling ...
    yield();  // Not needed
}
```

**Recommendation:**

Remove explicit `yield()` calls. They're only needed in tight loops with NO blocking operations.

**Impact:**

- Cleaner code
- No functional change (FreeRTOS handles this)

---

#### 5. **Use READ_ONLY/READ_WRITE Constants for Preferences**

**Issue:** Preferences `begin()` uses boolean literals (`true`/`false`) which are not self-documenting.

**Current Code:**

```cpp
prefs.begin("stac", true);   // What does true mean?
prefs.begin("wifi", false);  // What does false mean?
```

**Recommendation:**

Add constants to improve readability:

```cpp
// In ConfigManager.h or Constants.h
namespace Config {
    namespace Storage {
        constexpr bool READ_ONLY = true;
        constexpr bool READ_WRITE = false;
    }
}

// Usage:
using namespace Config::Storage;
prefs.begin("stac", READ_ONLY);
prefs.begin("wifi", READ_WRITE);
```

**Impact:**

- Self-documenting code
- Prevents accidental read/write errors
- Matches ESP-IDF conventions

---

### Category 2: Architectural Improvements

#### 6. **Centralize Switch Model Handling**

**Issue:** Switch models ("V-60HD", "V-160HD") are compared as strings in 15+ locations. This is error-prone and inefficient.

**Current Pattern:**

```cpp
if (ops.switchModel == "V-60HD") {
    // ...
} else if (ops.switchModel == "V-160HD") {
    // ...
}
```

**Recommendation:**

Define enum and conversion functions:

```cpp
// In Types.h
enum class SwitchModel : uint8_t {
    UNKNOWN = 0,
    V_60HD = 1,
    V_160HD = 2
};

// In new file Utils/SwitchModelUtils.h
namespace Utils {
    class SwitchModelUtils {
    public:
        static SwitchModel fromString(const String& model) {
            if (model == "V-60HD") return SwitchModel::V_60HD;
            if (model == "V-160HD") return SwitchModel::V_160HD;
            return SwitchModel::UNKNOWN;
        }
        
        static String toString(SwitchModel model) {
            switch (model) {
                case SwitchModel::V_60HD: return "V-60HD";
                case SwitchModel::V_160HD: return "V-160HD";
                default: return "UNKNOWN";
            }
        }
    };
}

// In StacOperations struct
SwitchModel switchModel;  // Instead of String
```

**Impact:**

- Type-safe comparisons
- Compiler-checked exhaustive handling
- Easier to add new switch models
- Reduced string comparison overhead
- **But**: Need to maintain string representation for NVS storage and display

**Alternative (Simpler):**

<!-- @robl: Agree to this method. We can generate these once at run time during startup no need to store the String representation in the Preferences namespace.
-->

Keep string representation but add helper methods:

```cpp
// In StacOperations
bool isV60HD() const { return switchModel == "V-60HD"; }
bool isV160HD() const { return switchModel == "V-160HD"; }
bool isSDIChannel() const { return isV160HD() && tallyChannel > 8; }
```

---

#### 7. **Standardize Power-On Glyph Naming**

<!-- @robl: DECISION - Use getGlyph(GLF_PO) style calls. Ensure GLF_PO exists in both glyph maps 
with appropriate visual for each display size.
-->

**Issue:** Power-on indicator uses different glyph names for 5x5 (GLF_PO) vs 8x8 (GLF_MID), requiring `#ifdef` blocks in 15+ locations.

**Current Pattern (appears 15+ times):**

```cpp
#ifdef GLYPH_SIZE_5X5
    const uint8_t *powerGlyph = glyphManager->getGlyph(GLF_PO);  // Center pixel
#else
    const uint8_t *powerGlyph = glyphManager->getGlyph(GLF_MID); // Center 4 pixels
#endif
display->drawGlyphOverlay(powerGlyph, StandardColors::ORANGE, true);
```

**Recommendation:**

Standardize on `GLF_PO` in both glyph maps:

```cpp
// In Display/Glyphs5x5.h
namespace GlyphIndex {
    GLF_PO = 0,    // Power-on: center pixel for 5x5
    // ... other glyphs
}

// In Display/Glyphs8x8.h  
namespace GlyphIndex {
    GLF_PO = 0,    // Power-on: center 4 pixels for 8x8 (was GLF_MID)
    // ... other glyphs
}

// Usage everywhere (no #ifdef needed):
const uint8_t *powerGlyph = glyphManager->getGlyph(GLF_PO
private:
    const uint8_t* getPowerOnGlyph() const {
        #ifdef GLYPH_SIZE_5X5
        return glyphManager->getGlyph(Display::Glyphs5x5::GlyphIndex::GLF_PO);
        #else
        return glyphManager->getGlyph(Display::Glyphs8x8::GlyphIndex::GLF_MID);
        #endif
    }

// Usage:
display->drawGlyphOverlay(getPowerOnGlyph(), StandardColors::ORANGE, true);
```

**Recommendation Option B - Add to GlyphManager:**

```cpp
// In GlyphManager<SIZE>
const uint8_t* getPowerOnGlyph() const {
    #ifdef GLYPH_SIZE_5X5
    return getGlyph(GLF_PO);
    #else
    return getGlyph(GLF_MID);
    #endif
}

// Usage (no size awareness needed):
display->drawGlyphOverlay(glyphManager->getPowerOnGlyph(), StandardColors::ORANGE, true);
```

**Impact:**

- Eliminates 15+ `#ifdef` blocks
- Single point of change if power-on glyph changes
- Cleaner, more readable code

---

#### 8. **Add `SHOW/NO_SHOW` Display Constants**

**Issue:** Display methods use boolean literals for "show" parameter: `display->fill(color, true)` vs `display->fill(color, false)`. Not self-documenting.

**Baseline Approach:**

```cpp
#define SHOW true
#define NO_SHOW false

disFillPix(RGB_COLOR_RED, SHOW);
disClear(NO_SHOW);
```

**Recommendation:**

```cpp
// In Display/IDisplay.h or Constants.h
namespace Display {
    constexpr bool SHOW = true;
    constexpr bool NO_SHOW = false;
}

// Usage:
using namespace Display;
display->fill(StandardColors::RED, SHOW);
display->clear(NO_SHOW);
display->drawGlyph(glyph, fg, bg, NO_SHOW);
display->show();  // Explicit show
```

**Impact:**

- More readable code
- Matches baseline conventions
- Self-documenting intent

---

#### 9. **Simplify Autostart Corner Pixel Logic**

**Issue:** Autostart timeout loop redraws the channel glyph every iteration just to toggle corner pixels. This is inefficient.

**Current Code:**

```cpp
while (millis() < autostartTimeout) {
    if (millis() >= nextFlash) {
        cornersOn = !cornersOn;
        // Redraw entire channel glyph
        display->drawGlyph(channelGlyph, channelColor, StandardColors::BLACK, false);
        // Then toggle corners
        display->pulseCorners(cornersGlyph, cornersOn, autostartColor);
    }
}
```

**Baseline Approach:**

```cpp
void pulsePix(bool state, crgb_t colour) {
    disDrawPix(0, (state ? colour : RGB_COLOR_BLACK), NO_SHOW);
    disDrawPix(4, (state ? colour : RGB_COLOR_BLACK), NO_SHOW);
    disDrawPix(20, (state ? colour : RGB_COLOR_BLACK), NO_SHOW);
    disDrawPix(24, (state ? colour : RGB_COLOR_BLACK), NO_SHOW);
    disShow();
}
```

**Recommendation:**

<!-- @robl: We have changed this so that in the glyph map there is a defined glyph for the four corners. No need to do math, just index into the glyph map and use that glyph with the draw overlay method. I believe this is the approach we use in the latest version of our refactored code
-->

Change `pulseCorners()` to only update corner pixels without redrawing background:

```cpp
// In IDisplay.h
virtual void pulseCorners(const uint8_t* cornerMask, bool state, color_t color) = 0;

// Implementation should:
// 1. Use cornerMask to identify which pixels are corners
// 2. Set those pixels to color (if state=true) or BLACK (if state=false)
// 3. Call show()
// 4. Do NOT redraw background

// Usage in autostart loop:
// Draw channel once BEFORE loop
display->drawGlyph(channelGlyph, channelColor, StandardColors::BLACK, true);

// Then just pulse corners
while (millis() < autostartTimeout) {
    if (millis() >= nextFlash) {
        cornersOn = !cornersOn;
        display->pulseCorners(cornersGlyph, cornersOn, autostartColor);
        // Channel glyph remains visible underneath
    }
}
```

**Impact:**

- Matches baseline behavior
- More efficient (no redundant redraws)
- Cleaner loop logic

---

#### 10. **Consolidate Provisioning State Checks**

**Issue:** Multiple checks for provisioning state create confusing flow in `handleNormalMode()`:

```cpp
// Load ops from NVS
if (!opsLoaded) {
    log_w("Failed to load protocol configuration from NVS, using defaults");
    ops = systemState->getOperations();  // Get defaults
}
```

**Recommendation:**

<!-- @robl: Note we have eliminated the functional need for "showError(7)" type calls. 

Not sure if this is just semantics, but "StartupConfig" is always done during startup. The startup 
sequence early on determines if the device is provisioned. It cannot proceed until it is 
provisioned. If the device is provisioned, the software just loads from NVS the operating 
parameters into a structure. If it is not provisioned, it calls the provisioning routine and does 
not proceed until the provisioning data is received from the user's web browser and stored into 
NVS. It then sets the "isProvisioned" flag to "true" stores that into NVS and then resets the 
device. Now on this restart, the device checks and confirms it is provisioned and then proceeds to 
load the operating parameters from NVS and carries on. Thus "StartupConfigDone" is always true.

-->

Use the centralized `isProvisioned()` flag (from recommendation #1):

```cpp
void handleNormalMode() {
    static bool startupConfigDone = false;
    
    if (!startupConfigDone) {
        startupConfigDone = true;
        
        // At this point, we KNOW we're provisioned (checked in determineOperatingMode)
        // So loading ops should always succeed
        StacOperations ops;
        String protocol = configManager->getActiveProtocol();
        
        bool loaded = false;
        if (protocol == "V-60HD") {
            loaded = configManager->loadV60HDConfig(ops);
        } else if (protocol == "V-160HD") {
            loaded = configManager->loadV160HDConfig(ops);
        }
        
        if (!loaded) {
            // This should never happen - we're provisioned but can't load config
            log_e("FATAL: Provisioned but config load failed");
            showError(7);
            ESP.restart();  // Force reprovisioning
            return;
        }
        
        // Continue with loaded ops...
    }
}
```

**Impact:**

- Clearer error handling
- No silent fallback to defaults
- Matches baseline assumption: provisioned = valid config

---

### Category 3: Code Organization

#### 11. **Move WiFi Connection Earlier in Startup**

**Issue:** WiFi connection happens late in `handleNormalMode()` after startup config sequence. The baseline connects WiFi immediately after provisioning, before user config.

<!-- @robl: Again, not sure if it's semantics, but WiFi connection -only- starts after autostart 
times out -or- the user had clicked through the manual setting of the tally channel, operator 
mode, autostart section  and brightness selection options using the button.
-->

**Baseline Flow:**

```
STACProvision → User Config (Channel/Mode/Brightness) → 
WiFi Connect → Show Status → STS Connect → Ready
```

**Current v3 Flow:**

```
Provisioning Check → User Config → WiFi Connect (inside handleNormalMode)
```

**Recommendation:**

Move WiFi connection to end of `setup()`, before mode-specific handlers:

<!-- @robl: Unclear what is meant in this context by "mode-specific handlers".
Recommend leaving the WiFi connect logic near the start of loop() and after the loop() button 
check logic as we need to do a WiFi connection check on every loop iteration. If the WiFi logic is 
at the end of setup() it would have to be repeated again in loop() anyway. Note that without the 
WiFi connection being up and running, the STAC is useless so it sits in the "trying to connect" 
state loop until a connection is established.
-->

```cpp
bool STACApp::setup() {
    // ... existing initialization ...
    
    // Handle provisioning mode if needed
    if (systemState->getOperatingMode().getCurrentMode() == OperatingMode::PROVISIONING) {
        handleProvisioningMode();
        // Never returns - device restarts
    }
    
    // Create startup config handler (for both modes)
    startupConfig = std::make_unique<StartupConfig>(...)
    
    // For NORMAL mode only: Connect WiFi BEFORE entering main loop
    if (systemState->getOperatingMode().isNormalMode()) {
        connectWiFi();  // New helper method
    }
    
    return true;
}

void STACApp::connectWiFi() {
    String ssid, password;
    if (!configManager->loadWiFiCredentials(ssid, password)) {
        log_e("Failed to load WiFi credentials");
        return;
    }
    
    // Set callback for visual feedback
    wifiManager->setStateCallback([this](Net::WiFiState state) {
        displayWiFiStatus(state);
    });
    
    // Blocking connect with visual feedback
    wifiManager->connect(ssid, password);
}
```

**Impact:**

- Matches baseline flow exactly
- WiFi ready before user config sequence
- Clearer separation of concerns
- Eliminates `wifiAttempted` flag complexity

---

#### 12. **Separate Startup Config from Runtime Config**

<!-- @robl: Unsure this analysis is correct. Please review the baseline code operation. I think 
the button state machine determines which state we're in and then calls a method to deal with that 
state. The method, for setting the brightness for example, is the same regardless if we're trying 
to change brightness during initial startup or during normal run time or if we're operating in 
Peripheral mode. Likewise, for changing the camera or talent modes at startup or when in Peripheral Mode. The state machine is a different implementation, but the handlers are common.
-->

**Issue:** `StartupConfig` is used for both:

1. Initial boot configuration sequence (channel/mode/brightness/autostart)
2. Runtime brightness adjustment (long button press)
3. Peripheral mode settings changes

This creates confusion about state management.

**Recommendation:**

Split into two focused classes:

```cpp
// For initial boot configuration
class InitialConfig {
public:
    bool runStartupSequence(StacOperations& ops, bool autoStartBypass);
private:
    void selectChannel(StacOperations& ops);
    void selectMode(StacOperations& ops);
    void selectStartupMode(StacOperations& ops);
    void selectBrightness(StacOperations& ops);
};

// For runtime adjustments (separate responsibility)
class RuntimeConfig {
public:
    void adjustBrightness(StacOperations& ops);
    void adjustMode(StacOperations& ops);
};
```

**Impact:**

- Clearer separation of concerns
- Easier to test
- Reduces cognitive load

**Note:** This is a larger refactoring - may defer to post-release

---

#### 13. **Error Display** *(ELIMINATED)*

**Issue:** The @Claude comment suggested enhanced error display with severity levels and numeric glyphs.

**Decision:** **Not needed.** The existing display system already provides sufficient error feedback through:

- Appropriate glyphs (`GLF_BX` for errors, `GLF_QM` for warnings, etc.)
- Color coding (RED/ORANGE/YELLOW for severity)
- Display flashing patterns (`display->flash()`)
- Brightness modulation for attention

Current error handling is adequate. Separate error number display routines add unnecessary complexity.

---

### Category 4: Display System Refinements

#### 14. **Standardize Glyph Access Pattern**

**Issue:** Glyph access requires `#ifdef GLYPH_SIZE_nXn` and full namespace qualification:

```cpp
#ifdef GLYPH_SIZE_5X5
    using namespace Display::Glyphs5x5::GlyphIndex;
#else
    using namespace Display::Glyphs8x8::GlyphIndex;
#endif
const uint8_t *glyph = glyphManager->getGlyph(GLF_WIFI);
```

**Recommendation:**

Add GlyphManager methods that abstract size:

```cpp
// In GlyphManager<SIZE>
const uint8_t* getWiFiGlyph() const { return getGlyph(GLF_WIFI); }
const uint8_t* getCheckGlyph() const { return getGlyph(GLF_CK); }
const uint8_t* getConfigGlyph() const { return getGlyph(GLF_CFG); }
const uint8_t* getUpdateGlyph() const { return getGlyph(GLF_UD); }
// ... etc for commonly used glyphs

// Usage (no #ifdef needed):
const uint8_t *wifiGlyph = glyphManager->getWiFiGlyph();
display->drawGlyph(wifiGlyph, color, bg, true);
```

**Impact:**

- Eliminates ~20 `#ifdef` blocks
- More readable code
- Easier to maintain

**Alternative:** Keep `#ifdef` but consolidate at top of functions:

```cpp
void STACApp::displayWiFiStatus(Net::WiFiState state) {
    // Get all glyphs once at top
    #ifdef GLYPH_SIZE_5X5
    using namespace Display::Glyphs5x5::GlyphIndex;
    #else
    using namespace Display::Glyphs8x8::GlyphIndex;
    #endif
    
    const uint8_t *wifiGlyph = glyphManager->getGlyph(GLF_WIFI);
    const uint8_t *checkGlyph = glyphManager->getGlyph(GLF_CK);
    const uint8_t *powerGlyph = glyphManager->getGlyph(GLF_PO);
    
    // Rest of function uses these variables
    // No more #ifdef blocks
}
```

---

#### 15. **Add Brightness Map Calculation**

**Issue:** Brightness map maximum is hardcoded:

```cpp
#define DISPLAY_BRIGHTNESS_LEVELS_5X5 6    // Max level
#define DISPLAY_BRIGHTNESS_LEVELS_8X8 8

// But map is:
constexpr uint8_t BRIGHTNESS_MAP_5X5[] = { 0, 10, 20, 30, 40, 50, 60 };  // 7 elements
```

**Recommendation:**

Calculate from array size:

```cpp
// In Constants.h
namespace Config {
    namespace Display {
        constexpr uint8_t BRIGHTNESS_MAP_5X5[] = { 0, 10, 20, 30, 40, 50, 60 };
        constexpr uint8_t BRIGHTNESS_MAP_8X8[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80 };
        
        // Calculate maximum level (array size - 1, since index 0 is padding)
        constexpr uint8_t BRIGHTNESS_LEVELS_5X5 = (sizeof(BRIGHTNESS_MAP_5X5) / sizeof(uint8_t)) - 1;
        constexpr uint8_t BRIGHTNESS_LEVELS_8X8 = (sizeof(BRIGHTNESS_MAP_8X8) / sizeof(uint8_t)) - 1;
    }
}
```

**Impact:**

- Eliminates manual counting errors
- Easier to modify brightness maps
- Self-documenting

---

#### 16. **Define Peripheral Mode Invalid State Constant**

**Issue:** Peripheral mode uses magic number `0xff` for invalid tally state:

```cpp
uint8_t lastTallyState = 0xff;  // Invalid state to force initial update
```

**Recommendation:**

```cpp
// In Constants.h or Types.h
namespace Config {
    namespace Hardware {
        constexpr uint8_t PERIPHERAL_INVALID_STATE = 0xFF;
    }
}

// Usage:
uint8_t lastTallyState = Config::Hardware::PERIPHERAL_INVALID_STATE;
```

**Impact:**

- Self-documenting code
- Prevents accidental reuse of magic numbers

---

## Prioritized Implementation Plan

### Phase 1: Quick Wins (1-2 hours) - Immediate Value

1. ✅ Add `isProvisioned()` to ConfigManager (#1)
2. ✅ Add READ_ONLY/READ_WRITE constants (#5)
3. ✅ Add SHOW/NO_SHOW constants (#8)
4. ✅ Remove redundant `yield()` calls (#4)
5. ✅ Define PERIPHERAL_INVALID_STATE constant (#16)

**Estimated LOC Impact:** -50 lines, +20 lines = **-30 net reduction**

---

### Phase 2: Display System (2-3 hours) - High Readability Impact

6. ✅ Standardize GLF_PO in both glyph maps (#7)
7. ✅ Move brightness maps to board config with compile-time validation (#3)
8. ✅ Consolidate glyph access pattern at function tops (#14)
9. ✅ Auto-calculate brightness levels from map size (#15)

**Estimated LOC Impact:** -120 lines, +25 lines = **-95 net reduction**

**Key Changes:**
- Brightness maps in `BoardConfigs/*.h` instead of `Constants.h`
- Compile-time validation with `static_assert`
- No `#ifdef` needed for brightness or power-on glyph access
- Glyph name consistency across display sizes

---

### Phase 3: Architecture (3-4 hours) - Reduced Complexity

10. ✅ Simplify boot button sequence (#2)
11. ✅ Centralize provisioning checks in `determineOperatingMode()` (#10)
12. ✅ Move WiFi connection earlier (#11)
13. ✅ Fix autostart corner pixel logic (#9)

**Estimated LOC Impact:** -80 lines, +40 lines = **-40 net reduction**

---

### Phase 4: Switch Model Handling (2 hours) - Type Safety

14. ⚠️ Add switch model helper methods (#6)
    - Option A: Keep strings, add `isV60HD()` / `isV160HD()` helpers
    - Option B: Full enum refactoring (defer to v3.1)

**Estimated LOC Impact:** -20 lines, +15 lines = **-5 net reduction**

---

### Phase 5: Larger Refactoring (Defer to v3.1)

<!-- @robl: Question - In light of the comments above, is this still a significant block of work? -->

15. ⏸️ Separate startup config from runtime config (#12)
    - Requires significant restructuring
    - Good candidate for next release

---

## Summary Statistics

| Category | Improvements | Estimated LOC Reduction | Estimated Time |
|----------|--------------|------------------------|----------------|
| Quick Wins | 5 | -30 | 1-2 hours |
| Display System | 4 | -95 | 2-3 hours |
| Architecture | 4 | -40 | 3-4 hours |
| Switch Models | 1 | -5 | 2 hours |
| **Total (Phase 1-4)** | **14** | **-170 net** | **8-11 hours** |

**Key Decisions:**
- ✅ Brightness maps moved to board config with compile-time validation
- ✅ Glyph abstraction via `getGlyph(GLF_PO)` pattern with standardized names
- ✅ Error handling enhancement removed (existing system sufficient)

---

## Deferred Items (@Claude Comments)

The following items from `@Claude:` comments are noted but deferred:

1. **ConfigManager namespace confusion** - Needs clarification on NVS namespace vs C++ namespace

<!-- @robl: Agreed. A namespace in the context of the Preferences library is very different than 
the context in C++. My comment is within the context of the Preferences library. -->

2. **IMU availability detection** - Board config improvements
3. **Board config cleanup** - Remove unused defines, consolidate constants
4. **Hardware object cleanup in destructor** - Memory management review
5. **Migration functionality removal** - NVS version mismatch handling

These should be addressed in a separate pass focused on infrastructure cleanup.

---

## Removed Recommendations

### Error Handling Enhancement (Removed)

<!-- @robl: DECISION - Error handling recommendation removed. Current glyph/color/flash/pulse 
system is sufficient. No need for enhanced showError(code) functionality.
-->

The original recommendation #13 to enhance `showError(code)` with colored error codes (red for halt, orange for warning, yellow for info) has been **removed**. The existing display system with glyphs, colors, flashing, and pulsing provides sufficient error indication capabilities.

---

## Conclusion

The STAC v3 startup sequence is **functionally complete** and demonstrates excellent architectural improvements over the baseline. The recommendations above focus on:

1. **Reducing code duplication** through helper methods and constants
2. **Improving readability** through self-documenting constants and consolidated logic
3. **Simplifying control flow** by centralizing provisioning checks
4. **Matching baseline semantics** where the baseline's approach is clearer

**Recommended Action Plan:**

- Complete **Phases 1-3** (Quick Wins + Display + Architecture) before release ✅
- These provide the highest ROI with minimal risk
- Defer **Phase 5** (larger refactoring) to v3.1

**Total estimated effort:** 6-9 hours for Phases 1-3, resulting in **~145 lines of code removed** while improving clarity and maintainability.

