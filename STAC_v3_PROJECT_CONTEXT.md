# STAC v3 Project Context

**Version:** v3.0.0  
**Branch:** `v3-arduino-gfx-test` (Arduino_GFX migration)  
**Updated:** February 4, 2026  
**Status:** ✅ READY FOR RELEASE. All critical fixes complete: ESP32-S3 bootloader offset corrected, factory reset logic fixed, boot button double-flash eliminated, WiFi and V-160HD password obfuscation implemented. Release binaries built for all 6 devices (commit a82b60e). Production ready.

---

## Recent Development Activity

### Password Obfuscation (Feb 4, 2026)

**✅ COMPLETE:** WiFi and V-160HD switch passwords now obfuscated in NVS storage

**Security Enhancement:**
Implemented XOR-based password obfuscation to prevent casual browsing of credentials in NVS flash storage while maintaining full functionality for network connections and switch authentication.

**Implementation:**
- Added `obfuscatePassword()` and `deobfuscatePassword()` helper methods to ConfigManager
- Uses device MAC address as XOR key (unique per device)
- Passwords XORed byte-by-byte with cycling MAC bytes, then hex-encoded for storage
- Works with passwords of any length (key cycles when password exceeds 6 bytes)

**Code Changes:**

1. **Added helper methods** [ConfigManager.h](STAC/include/Storage/ConfigManager.h#L267-281):
   ```cpp
   String obfuscatePassword( const String &password );
   String deobfuscatePassword( const String &obfuscated );
   ```

2. **Updated WiFi credential storage** [ConfigManager.cpp](STAC/src/Storage/ConfigManager.cpp#L28,43):
   ```cpp
   // Save: XOR with MAC + hex encode
   prefs.putString( KEY_PASSWORD, obfuscatePassword( password ) );
   
   // Load: Hex decode + XOR with MAC
   password = deobfuscatePassword( obfuscated );
   ```

3. **Updated switch credential storage** [ConfigManager.cpp](STAC/src/Storage/ConfigManager.cpp#L110,133):
   ```cpp
   // Save V-160HD password obfuscated
   prefs.putString( KEY_PASSWORD, obfuscatePassword( password ) );
   
   // Load V-160HD password deobfuscated
   password = deobfuscatePassword( obfuscated );
   ```

**Obfuscation Algorithm:**
```cpp
// Each byte XORed with MAC[i % 6], result converted to 2-digit hex
For password "MyPass" with MAC 94:b9:7e:a8:f8:00:
  'M' ^ 0x94 = 0xD9 → "d9"
  'y' ^ 0xb9 = 0xC0 → "c0"
  'P' ^ 0x7e = 0x2E → "2e"
  ... (continues for full password length)
```

**Security Characteristics:**
- **Not cryptographic encryption** - provides obfuscation against casual NVS inspection
- Device-specific: Each device uses its unique MAC as key
- Reversible: Required for WiFi connection and switch authentication
- Hex encoding doubles stored size (each byte → 2 hex chars)
- Empty passwords stored as empty string

**Testing:**
- ✅ Verified on ATOM Matrix (MAC: 94:b9:7e:a8:f8:00)
- ✅ WiFi password obfuscated in "wifi" namespace
- ✅ V-160HD password obfuscated in "switch" namespace
- ✅ Both passwords successfully deobfuscated and used for connections
- ✅ V-60HD configuration unaffected (no password field)

**NVS Storage After Implementation:**
| Namespace | Key | Storage Format |
|-----------|-----|----------------|
| wifi | password | Hex-encoded XOR obfuscated |
| switch | password | Hex-encoded XOR obfuscated (V-160HD only) |
| v60hd | (none) | No password for V-60HD |

**Release Binaries Built (Feb 4, 2026):**
All 6 devices built successfully with password obfuscation (commit a82b60e):
- **ATOM Matrix:** FULL 1.2M, OTA 1.2M
- **StickC Plus:** FULL 1.3M, OTA 1.2M
- **T-Display:** FULL 1.3M, OTA 1.2M
- **Waveshare S3:** FULL 1.2M, OTA 1.1M
- **T-QT:** FULL 1.3M, OTA 1.2M
- **AIPI-Lite:** FULL 1.3M, OTA 1.2M

---

### Boot Button Double-Flash Fix (Feb 4, 2026)

**✅ COMPLETE:** Eliminated duplicate flash animation for unconfigured devices entering provisioning mode

**Problem:**
When an unconfigured device entered provisioning mode via boot button sequence:
1. User holds button past peripheral mode selection
2. RED config icon appears and flashes 4 times (boot button sequence)
3. User releases button
4. **Display flashed 4 MORE times before starting to pulse** ❌

**Root Cause:**
The `handleProvisioningMode()` function always flashed the config glyph 4 times for unprogrammed devices, regardless of entry method:
- **Boot button sequence:** Already flashed 4 times when button released
- **Auto-entry on startup:** Hadn't flashed yet (needed the flash)

**Solution:** Track provisioning entry method to skip duplicate flash

**Code Changes:**

1. **Added tracking flag** [STACApp.h](STAC/include/Application/STACApp.h#L101):
   ```cpp
   bool provisioningFromBootButton;  // Track if provisioning entered via boot button
   ```

2. **Updated `handleProvisioningMode()` signature** [STACApp.h](STAC/include/Application/STACApp.h#L185):
   ```cpp
   void handleProvisioningMode( bool fromBootButton = false );
   ```

3. **Set flag in `determineOperatingMode()`** [STACApp.cpp](STAC/src/Application/STACApp.cpp#L410):
   ```cpp
   if ( bootMode == OperatingMode::PROVISIONING ) {
       provisioningFromBootButton = true;
       return OperatingMode::PROVISIONING;
   }
   ```

4. **Conditional flash in `handleProvisioningMode()`** [STACApp.cpp](STAC/src/Application/STACApp.cpp#L1048-1053):
   ```cpp
   // For unprogrammed devices NOT from boot button, flash before pulsing
   // (if from boot button, the boot sequence already did the flashing)
   if ( !wasProvisioned && !fromBootButton ) {
       delay( 500 );  // Hold static glyph for 500ms
       display->flash( 4, 250, normalBrightness );  // Flash 4 times
       display->setBrightness( normalBrightness );
   }
   ```

**Behavior After Fix:**
- **Boot button → provisioning:** 4 flashes (boot sequence) → pulse ✅
- **Auto-entry → provisioning:** 4 flashes (handleProvisioningMode) → pulse ✅
- No duplicate flashes in either scenario

**Testing:**
- ✅ Verified on ATOM Matrix (unconfigured device)
- ✅ Boot button sequence shows single set of 4 flashes
- ✅ Immediately transitions to pulsing animation
- ✅ Auto-entry path still flashes correctly (not tested but logic verified)

---

### ESP32-S3 Bootloader Offset Fix (Feb 4, 2026)

**✅ COMPLETE:** Critical bootloader offset fix for ESP32-S3 devices

**Problem:** Waveshare ESP32-S3 Matrix failed to boot with error:
```
invalid header: 0xff
ESP-ROM:esp32s3-20210327
rst:0x7 (TG0WDT_SYS_RST),boot:0x8 (SPI_FAST_FLASH_BOOT)
```

**Root Cause:**
- ESP32-S3 family requires bootloader at flash offset **0x0** (not 0x1000)
- Build system (custom_targets.py) was using 0x1000 for all ESP32 chip families
- ESP32-S3 ROM bootloader expected bootloader at 0x0, found 0xFF padding instead
- Caused continuous boot loop on all ESP32-S3 devices

**ESP32 Bootloader Offset Requirements:**
| Chip Family | Bootloader Offset |
|-------------|-------------------|
| ESP32 classic | 0x1000 |
| ESP32-S2 | 0x1000 |
| **ESP32-S3** | **0x0** |
| ESP32-C3 | 0x0 |
| ESP32-C6 | 0x0 |
| ESP32-H2 | 0x0 |

All families use: Partitions at 0x8000, App at 0x10000

**Solution:** Updated [custom_targets.py](STAC/scripts/custom_targets.py#L118-130) with chip detection:

```python
# Determine bootloader offset based on chip type
chip = (board.get("build.mcu") or "esp32").lower()

if chip in ("esp32s3", "esp32c3", "esp32c6", "esp32h2"):
    OFF_BOOTLOADER = 0x0     # Newer generation chips
else:
    OFF_BOOTLOADER = 0x1000  # ESP32 classic, ESP32-S2

OFF_PARTITIONS = 0x8000
OFF_BOOT_APP0 = 0xE000
OFF_APP = 0x10000
```

**Additional Fix:** T-QT Flash Size Configuration
- T-QT hardware has 4MB flash (not 8MB default)
- Added `board_upload.flash_size = 4MB` to [platformio.ini](STAC/platformio.ini#L331,306)
- Prevents "Detected size(4096k) smaller than binary header(8192k)" error

**Rebuild Results:**
- ✅ Waveshare S3: Build c876f7 (4MB flash, 0x0 bootloader) - boots successfully
- ✅ T-QT: Build c876f7 (4MB flash, 0x0 bootloader) - boots successfully  
- ✅ AIPI-Lite: Build 72b63a (16MB flash, 0x0 bootloader) - boots successfully
- ✅ ATOM Matrix: Build a52050 (4MB flash, 0x1000 bootloader) - boots successfully
- ✅ StickC Plus: Build a0f2ed (4MB flash, 0x1000 bootloader) - boots successfully
- ✅ T-Display: Build 78c577 (4MB flash, 0x1000 bootloader) - boots successfully

**Key Learnings:**
- ESP32 chip families have different bootloader offset requirements
- esptool merge-bin handles padding automatically based on specified offsets
- FULL binaries ALWAYS flash at 0x0 regardless of internal structure
- Flash size must match hardware to prevent boot failures

---

### Repository Cleanup & Build System Updates (Feb 3-4, 2026)

**✅ COMPLETE:** Major repository cleanup, documentation reorganization, and fresh release build

**Day 1: Archive & Documentation (Feb 3, 2026)**
1. **Archive Management:**
   - Created `STAC3DevArchive/` for obsolete development files (40+ items)
   - Moved purple-tagged files preserving directory structure
   - Archived font generation scripts, old build scripts, backup files
   - Preserved IMU calibration tool in `STAC/doc/Utility/IMU_Calibrator/`

2. **Documentation Reorganization:**
   - `Documentation/` → User-facing documentation only
   - `STAC/Documentation/` → `STAC/doc/` (GitHub convention for developer docs)
   - Separated Developer/ and User/ documentation by audience
   - Consolidated HARDWARE_CONFIG documentation (removed _B variant)

3. **Build Configuration Cleanup:**
   - Removed 5 calibration environments from platformio.ini (182 lines removed)
   - Cleaned up build filter exclusions for main_calibrate.cpp
   - Removed platformio_backup.ini
   - Updated .gitignore: `*.bin`, `*.backup`, added archive paths

**Day 2: Board Config & Release Build (Feb 4, 2026)**
1. **File Cleanup:**
   - Removed empty directories (Hardware/LED, Network/WebServer)
   - Deleted TestConfig.h and .bak files (StartupConfig.tpp.bak, STACApp.cpp.bak, GlyphManager.cpp.bak)
   - Removed NVS_Namespace_Structure.md.bak

2. **Board Configuration Standardization:**
   - Renamed `TQT_Config.h` → `LilygoTQT_Config.h` (consistency with other boards)
   - Updated 13 references across platformio.ini, HARDWARE_CONFIG.md, DEVELOPER_GUIDE.md
   - Fixed escaped quotes in platformio.ini: `\"BoardConfigs/LilygoTQT_Config.h\"`
   - Used `git mv` to preserve file history

3. **Build System:**
   - Created `build_release_all.sh` at repository root
   - Automated build process: clean → FULL binary → OTA binary for all 6 devices
   - Removes old binaries before building (prevents version confusion)
   - Changes to STAC/ directory for platformio.ini access
   - **Updated custom_targets.py to output binaries to repository root `bin/` folder**
   - Binaries now placed alongside user documentation (not buried in STAC/bin/)

4. **Fresh v3.0.0 Release Binaries:**
   - Successfully built all 12 binaries (FULL + OTA for 6 devices)
   - **Binaries output to repository root `bin/` structure**
   - Each device has dedicated folder: `bin/<Device Name>/`
   - **ATOM Matrix verified booting successfully with build c85a55**

**Release Artifacts (Feb 4, 2026 Build):**
- `bin/M5Stack ATOM Matrix/` - FULL: 1.3M, OTA: 1.2M
- `bin/Waveshare ESP32-S3 Matrix/` - FULL: 1.2M, OTA: 1.1M  
- `bin/M5Stack StickC Plus/` - FULL: 1.3M, OTA: 1.2M
- `bin/Lilygo T-Display/` - FULL: 1.3M, OTA: 1.2M
- `bin/Lilygo T-QT/` - FULL: 1.3M, OTA: 1.2M
- `bin/AI PI-Lite/` - FULL: 1.3M, OTA: 1.2M

**Key Improvements:**
- ✅ Clean separation of active vs. archived development files
- ✅ Clear documentation audience boundaries (users vs. developers)
- ✅ Streamlined build configuration (no calibration environments)
- ✅ Consistent board naming conventions (LilygoTQT_Config.h)
- ✅ Automated release build script
- ✅ Build system outputs to repository root bin/ folder
- ✅ All 6 devices verified building successfully
- ✅ Hardware verification: ATOM Matrix boots correctly with new binaries

**Note on Binary Anonymization:**
- Attempted to anonymize username paths (`/Users/robl/` → `/Users/user/`) in binaries
- Discovered ESP32 firmware binaries contain embedded SHA256 checksums
- Modifying binaries post-build invalidates checksums, causing boot loop
- Username paths are from precompiled ESP32 framework debug symbols (not project code)
- Decision: Accepted username presence in binaries as non-security issue
- Reverted anonymization changes from custom_targets.py

---

### Version 3.0.0 Release (Feb 2, 2026)

**✅ COMPLETE:** STAC v3.0.0 officially released with production binaries

**Changes in v3.0.0:**
1. **Build System:**
   - Fixed merged binary bootloader offset (0x1000 instead of 0x0) - resolves "flash read err, 1000" boot failures
   - Reorganized bin/ folder structure by device type (ATOM Matrix/, M5Stack StickC Plus/, etc.)
   - Updated .gitignore to track documentation/structure while excluding .bin files
   - Build script now removes old binaries before creating new ones (prevents version confusion)

2. **Device Configurations:**
   - **T-QT:** Removed GPIO 0 button configuration (boot strapping pin conflict with bootloader entry)
   - **All devices:** Removed unused `-DBOARD_HAS_PSRAM` build flags (no code usage)

3. **Documentation:**
   - Created M5StickC Plus Peripheral Mode pinout reference with GPIO 32/33 mapping
   - Updated bin/README.md with new folder structure
   - Created TFT Display Mockups (actual-size HTML/canvas rendering)

4. **Binary Organization:**
   - Each device has dedicated folder: `bin/<Device Name>/`
   - User Guide Supplements and images tracked in repository
   - Binary files excluded from version control
   - Naming: `STAC_v3.0.0_<BOARD>_<BUILD>.bin` and `..._FULL.bin`

---

### Waveshare S3 Performance Optimization (Jan 31, 2026)

**✅ COMPLETE:** Waveshare S3 startup and mode switching now matches ATOM Matrix speed

**Problem:** Waveshare S3 exhibited slower startup and mode switching compared to ATOM Matrix:
- Noticeable delay during device boot showing POG (orange power glyph)
- Longer transition when switching between Normal ↔ Peripheral modes
- Checkmark confirmation displayed longer than expected during mode changes
- Root causes identified through timing analysis

**Investigation Findings:**
1. **IMU Polling Loops:** QMI8658_IMU.cpp contained polling loops with 10ms delays waiting for sensor data, while MPU6886_IMU.cpp reads data immediately
2. **Library Behavior:** QMI8658 library functions (getDataReady(), getAccelerometer()) return instantly via I2C register reads - no inherent waiting required
3. **Display Timing:** TFT reset delays (400ms) not applicable to Waveshare (LED matrix, no TFT_RST defined)
4. **Restart Cleanup:** Mode toggle restart missing display shutdown (setBrightness(0)) before ESP.restart()

**Solution 1: Remove Unnecessary IMU Polling** ([QMI8658_IMU.cpp](STAC/src/Hardware/Sensors/QMI8658_IMU.cpp))

Removed three polling loops with 10ms delays:
- **begin()** line 36-47: Removed "wait for data ready" polling loop after enableAccelerometer()
- **getOrientation()** line 64-72: Removed accelerometer read polling loop - read directly
- **getRawAcceleration()** line 112-120: Removed accelerometer read polling loop - read directly

**Before:**
```cpp
// Wait for data to be ready
uint32_t timeout = millis() + DATA_WAIT_TIMEOUT_MS;
while ( !sensor.getDataReady() ) {
    if ( millis() > timeout ) {
        log_e( "Timeout waiting for QMI8658 data ready" );
        return false;
    }
    delay( 10 );  // ← Unnecessary delay
}
```

**After:**
```cpp
// QMI8658 is ready immediately after enable (no polling needed like MPU6886)
sensor.enableAccelerometer();
initialized = true;
```

**Solution 2: Standardize Restart Behavior** ([STACApp.cpp](STAC/src/Application/STACApp.cpp#L1523-L1530), [STACApp.h](STAC/include/Application/STACApp.h#L217-L225))

Added `restartDevice()` helper method to ensure consistent display cleanup across all restart scenarios:

```cpp
void STACApp::restartDevice( uint16_t delayMs ) {
    delay( delayMs );
    display->setBrightness( 0 );  // Turn off display before restart
    ESP.restart();
    // Never returns
}
```

**Applied to 5 restart locations:**
- OTA success (1000ms delay)
- OTA failed (3000ms delay)
- Provisioning complete (1000ms delay)
- Mode toggle (default 1500ms) ← **Fixed missing display shutdown**
- Reduces code duplication by 20 lines

**Solution 3: Correct FLAT Orientation** ([WaveshareS3_Config.h](STAC/include/BoardConfigs/WaveshareS3_Config.h#L89))

Adjusted FLAT orientation mapping:
- **Before:** `Orientation::ROTATE_90` (incorrect - USB on right side)
- **After:** `Orientation::ROTATE_180` (correct - baseline at USB port)
- All tilt rotations (0°, 90°, 180°, 270°) remain correct

**Results:**
- ✅ Startup speed now matches ATOM Matrix (removed ~100-200ms IMU polling overhead)
- ✅ Mode switching speed matches ATOM Matrix (eliminated extra delays)
- ✅ Checkmark confirmation displays for identical duration on both devices
- ✅ Code quality improved with restartDevice() helper (DRY principle)
- ✅ FLAT orientation displays correctly with baseline at USB port
- ✅ Flash size reduced by 380 bytes (removed timeout checking code)

---

### Factory Reset Behavior Unification (Jan 30, 2026)

**✅ COMPLETE:** Browser-initiated factory reset now matches button-initiated behavior exactly

**Problem:** Inconsistent factory reset behavior between initiation methods:
- **Button-initiated:** Show factory reset glyph (red) → Flash once → Halt forever (no restart)
- **Browser-initiated:** Clear NVS → Restart immediately with "Please wait for restart..." message
- User expectation: Both methods should behave identically
- Web confirmation page incorrectly suggested device would restart

**Solution:** Unified code path for both factory reset methods

**Implementation ([STACApp.cpp](STAC/src/Application/STACApp.cpp#L1107-L1144)):**

**Before (browser-initiated):**
```cpp
else if ( result.type == Net::WebConfigServer::PortalResultType::FACTORY_RESET ) {
    log_i( "Factory reset requested from web portal" );
    configServer.end();
    Utils::InfoPrinter::printReset();
    
    if ( !configManager->clearAll() ) {
        log_e( "Factory reset failed - NVS clear unsuccessful" );
    }
    
    // Restart immediately (web-based reset doesn't wait for button)
    delay( 1000 );
    ESP.restart();  // ← DIFFERENT BEHAVIOR
    // Never returns
}
```

**After (unified behavior):**
```cpp
else if ( result.type == Net::WebConfigServer::PortalResultType::FACTORY_RESET ) {
    log_i( "Factory reset requested from web portal" );
    configServer.end();

    // Show factory reset glyph (matching button-initiated behavior)
    const uint8_t *frGlyph = glyphManager->getGlyph( Display::GLF_FR );
    display->drawGlyph( frGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, Config::Display::SHOW );
    
    Utils::InfoPrinter::printReset();
    
    if ( !configManager->clearAll() ) {
        log_e( "Factory reset failed - NVS clear unsuccessful" );
    }
    else {
        log_i( "Factory reset complete" );
    }

    // Brief pause
    delay( Config::Timing::GUI_PAUSE_MS );

    // Flash display once to confirm (baseline behavior)
    uint8_t brightness = Config::Display::BRIGHTNESS_MAP[ 1 ];
    display->flash( 1, 500, brightness );

    // Park here forever showing the factory reset glyph (matching button behavior)
    while ( true ) {
        #if defined(BUTTON_B_PIN)
        if ( buttonB->isPressed() ) {
            log_i( "Button B pressed after factory reset - restarting" );
            display->setBrightness( 0 );
            ESP.restart();
        }
        #endif
        yield();
    }
    // Never returns
}
```

**Web Confirmation Page Update ([WebConfigPages.h](STAC/include/Network/WebConfigPages.h#L1088-L1093)):**

**Before:**
```html
<h1>Factory Reset Complete</h1>
<p>All configuration data has been erased.</p>
<p>The STAC will now restart with factory default settings.</p>
<p><strong>Please wait for restart...</strong></p>
```

**After:**
```html
<h1>Factory Reset Complete</h1>
<p>All configuration data has been erased.</p>
<p><strong>Remove power or press the reset button to setup the device.</strong></p>
```

**Unified Behavior:**

Both button-initiated and browser-initiated factory reset now execute identical sequence:
1. Show factory reset glyph (GLF_FR) in red
2. Print reset notification to serial
3. Clear all NVS data via ConfigManager
4. Brief pause (GUI_PAUSE_MS)
5. Flash display once at brightness level 1 (500ms)
6. Halt forever in infinite loop showing factory reset glyph
7. Optional: Button B press triggers restart (M5StickC Plus only)

**Benefits:**
- ✅ Consistent user experience regardless of reset method
- ✅ Single code path eliminates maintenance burden
- ✅ Web confirmation page accurately reflects behavior
- ✅ Matches documented baseline behavior from v2
- ✅ User must manually power cycle or press reset button

**Modified Files:**
- `src/Application/STACApp.cpp` Lines 1107-1144 (browser-initiated path)
- `include/Network/WebConfigPages.h` Lines 1088-1093 (confirmation page)

**Testing:**
- ✅ Button-initiated factory reset: Shows glyph, flashes, halts
- ✅ Browser-initiated factory reset: Shows glyph, flashes, halts
- ✅ User confirmed: "Works like a charm"

**Commits:** TBD

---

### Web Portal Clipboard Paste Fix - Hybrid Desktop/Mobile Approach (Jan 29-30, 2026)

**✅ COMPLETE:** Web configuration portal paste functionality now works reliably on both desktop and mobile browsers

**Problem:** 
- **Desktop (Microsoft Edge):** "Paste Settings" button showed "Clipboard is empty" alert despite valid JSON in clipboard
  - User workflow: Click "Paste Settings" → Alert appears → Dismiss alert → Press Cmd+V → Error chime, nothing happens
  - Clipboard verified to contain valid JSON (could paste into text editor)
- **Mobile (Firefox):** Alert instructed users to "Use Ctrl+V (or Cmd+V on Mac)" which doesn't exist on mobile devices
  - Native mobile paste works via tap-and-hold menu, not keyboard shortcuts
  - Event-driven approach triggered desktop-style alert on mobile

**Root Cause Analysis:**

**Desktop Issue - Timing/Flow Problem:**
```javascript
// OLD CODE - useExecCommandFallback()
setTimeout(function() {
  document.execCommand('paste');  // Blocked by browser security
  
  setTimeout(function() {
    const jsonStr = pasteArea.value;  // Empty (paste was blocked)
    document.body.removeChild(pasteArea);  // ← REMOVED HERE
    
    if (!jsonStr.trim()) {
      alert('...use Ctrl+V to paste.');  // ← Alert shown, but textarea gone
      return;
    }
  }, 100);
}, 50);
```

**Problem:** Textarea removed before user could manually paste. By the time alert dismissed and user pressed Cmd+V, target element no longer existed.

**Mobile Issue - Wrong Instructions:**
- Alert message referenced keyboard shortcuts (Ctrl+V/Cmd+V) not available on mobile
- Users should tap-and-hold in textarea to access native paste menu
- Event-driven approach works but instructions were misleading

**Solution:** Hybrid approach based on device detection

**Implementation ([WebConfigPages.h](STAC/include/Network/WebConfigPages.h#L766-840)):**

```javascript
function useExecCommandFallback() {
  const isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
  
  if (isMobile) {
    // Mobile: Use execCommand with timeout (original method)
    const pasteArea = document.createElement('textarea');
    pasteArea.style.position = 'fixed';
    pasteArea.style.opacity = '0';
    document.body.appendChild(pasteArea);
    pasteArea.focus();
    
    setTimeout(function() {
      document.execCommand('paste');
      
      setTimeout(function() {
        const jsonStr = pasteArea.value;
        document.body.removeChild(pasteArea);
        
        if (!jsonStr.trim()) {
          alert('Clipboard is empty or paste failed.\n\nTip: After clicking Paste Settings, use your device\'s paste function.');
          return;
        }
        
        processClipboardData(jsonStr);
      }, 100);
    }, 50);
  } else {
    // Desktop: Use paste event listener
    const pasteArea = document.createElement('textarea');
    pasteArea.style.position = 'fixed';
    pasteArea.style.opacity = '0';
    document.body.appendChild(pasteArea);
    pasteArea.focus();
    
    let cleanedUp = false;
    
    function cleanup() {
      if (!cleanedUp) {
        cleanedUp = true;
        if (pasteArea.parentNode) {
          document.body.removeChild(pasteArea);
        }
      }
    }
    
    // Listen for paste event
    pasteArea.addEventListener('paste', function(event) {
      event.preventDefault();
      const jsonStr = event.clipboardData.getData('text/plain');
      cleanup();
      
      if (!jsonStr.trim()) {
        alert('Clipboard is empty');
        return;
      }
      
      processClipboardData(jsonStr);
    });
    
    // Listen for Escape key to cancel
    pasteArea.addEventListener('keydown', function(event) {
      if (event.key === 'Escape' || event.keyCode === 27) {
        cleanup();
      }
    });
    
    // Timeout cleanup after 30 seconds
    setTimeout(function() {
      cleanup();
    }, 30000);
    
    alert('Ready to paste.\n\nUse Ctrl+V (or Cmd+V on Mac) to paste your settings.\n\nPress Escape to cancel.');
  }
}
```

**Key Features:**

**Desktop Browsers (Chrome, Edge, Firefox, Safari):**
- Event-driven paste listener captures clipboard data from `event.clipboardData`
- Textarea remains focused until paste occurs or user cancels
- Escape key handler allows cancellation
- 30-second timeout prevents orphaned textarea
- Cleanup function ensures proper removal
- No premature textarea removal - stays alive until paste event fires

**Mobile Browsers (iOS Safari, Android Chrome, Firefox):**
- Uses original `execCommand('paste')` method with timeout
- Simpler approach works better with native mobile paste mechanisms
- Mobile-appropriate alert message (references device paste function, not keyboard)
- Compatible with tap-and-hold paste menu

**Benefits:**
- ✅ Desktop: Resolves Microsoft Edge "clipboard empty" issue
- ✅ Desktop: Works with keyboard shortcuts (Ctrl+V/Cmd+V)
- ✅ Desktop: Allows cancellation via Escape key
- ✅ Mobile: Works with native paste UI (tap-and-hold menu)
- ✅ Cross-platform: Paste event fires regardless of trigger method
- ✅ More reliable than polling/timeout on desktop
- ✅ Maintains compatibility with mobile execCommand behavior

**Modified Files:**
- `include/Network/WebConfigPages.h` Lines 766-840

**Testing:**
- ✅ Microsoft Edge (desktop): Paste functionality restored
- ✅ Mobile Firefox: Works with native paste menu
- ✅ User confirmed: "Much better 👍!"

**Commits:** TBD

---

### Documentation/User/ Added to .gitignore (Jan 30, 2026)

**✅ COMPLETE:** User documentation folder excluded from git tracking

**Change:**
Added `Documentation/User/` to [.gitignore](STAC/.gitignore#L18-19) to prevent tracking of user-facing documentation and images.

**Files Excluded:**
- STAC3 Users Guide.md
- STAC3WebImages/
- images/ (ClippyCopy.png, ConfirmLoadChange.png, FactoryReset.png, Info.png, etc.)

**Modified Files:**
- `.gitignore` Line 18-19

**Commits:** TBD

---

### TFT Brightness Digit Centering Fix (Jan 25, 2026)

**✅ COMPLETE:** Brightness digits now properly centered within black rectangle using same formula as tally number display

**Problem:** On TFT displays (especially T-QT 128×128), brightness digits exhibited multiple positioning issues:
- Digits positioned at bottom of black rectangle instead of centered vertically
- Digit "1" biased to left of rectangle
- Digit "3" offset downward relative to other digits
- Pixels spilling outside black rectangle on left edge
- Scale=2 made digits too large for the rectangle

**Root Cause Analysis:**
- **Vertical positioning:** Formula `cy - y1 + (h/2)` was incorrect for Adafruit GFX baseline positioning
  - `y1` is negative (offset from baseline to top of bounding box)
  - Cursor `y` is the baseline position, not top-left corner
  - Formula placed baseline too low in the rectangle
- **Horizontal bias:** Different centering approach than proven tally digit display
  - `cx - (w/2) - x1` worked differently in small rectangle vs full canvas
  - Font x1 offset had more relative impact in confined space
- **Scale too large:** Scale=2 made 24pt font too large for 50×75px rectangle
  - Caused pixel spillover outside black square
  - Limited room for proper centering adjustments

**Solution:** Use identical centering formula as `drawLargeDigit()` (tally numbers) which was confirmed working correctly

**Implementation ([DisplayTFT.cpp](STAC/src/Hardware/Display/TFT/DisplayTFT.cpp#L535-565)):**

**Before (brightness digit overlay - GLF_0-9):**
```cpp
uint8_t scale = (_rotation == 1 || _rotation == 3) ? 1 : 2;  // Scale varies by rotation

// Center both horizontally and vertically accounting for font offsets
int16_t x = cx - (w / 2) - x1;        // Custom horizontal formula
int16_t y = cy - y1 + (h / 2);        // INCORRECT vertical formula
```

**After (same formula as tally display):**
```cpp
uint8_t scale = 1;  // Fixed scale for consistent sizing

// Use same centering formula as drawLargeDigit for consistency
uint16_t canvasW = _canvas->width();
uint16_t canvasH = _canvas->height();
int16_t x = (canvasW - w) / 2 - x1;  // Account for x1 offset (important for narrow chars like "1")
int16_t y = (canvasH - h) / 2 + h;   // Baseline adjustment (same as tally digits)
```

**Key Changes:**
1. **Consistent formula:** Both brightness and tally digits use `(displayH - h) / 2 + h` for vertical centering
2. **Fixed scale:** Changed from rotation-dependent (1 or 2) to fixed `scale = 1`
   - Makes digits smaller and fit properly within rectangle
   - Prevents pixel spillover
3. **Full canvas reference:** Use full canvas dimensions for centering (not cx/cy center point)
   - More reliable positioning
   - Matches proven tally digit approach

**Results:**
- ✅ All digits properly centered both horizontally and vertically
- ✅ No pixel spillover outside black rectangle
- ✅ Consistent vertical alignment across all digits (including "3")
- ✅ "1" no longer biased to left
- ✅ Works correctly on all display sizes (135×240 and 128×128)

**Testing:**
- Verified on LilyGo T-QT (128×128) hardware
- Compiled successfully on all 4 TFT targets
- Generated updated OTA binaries for all 6 device types

**Modified Files:**
- `src/Hardware/Display/TFT/DisplayTFT.cpp` Lines 535-565

**Commits:** TBD

---

### TFT Display Dynamic Sizing - Checkerboard Patterns & OTA Binaries (Jan 25, 2026)

**✅ COMPLETE:** All TFT display elements now scale dynamically to fit display dimensions without edge clipping

**Problem:** Fixed-size checkerboard blocks (24px, 20px), black square (50×80px), and orange square (16px/20px) caused asymmetric edge clipping on different display sizes:
- M5StickC Plus (135×240): 15px clipped edges on checkerboard
- AIPI-Lite/T-QT (128×128): 8px clipped edges on checkerboard
- Brightness digits not properly centered within black square
- Orange square used rotation-based sizing, inconsistent across devices

**Solution:** Runtime calculation of all element sizes based on display dimensions with proper centering

**Implementation:**

**1. Checkerboard Patterns (GLF_DF, GLF_CBD) - Lines 240-277, 392-430:**
```cpp
// Calculate block size for perfect fit (5 blocks per smallest dimension)
const uint8_t TARGET_BLOCKS = 5;
uint16_t minDim = min(w, h);
uint8_t blockSize = minDim / TARGET_BLOCKS;
blockSize = constrain(blockSize, 10, 32);  // Visibility constraints

// Center the grid
int xOffset = (w - (cols × blockSize)) / 2;
int yOffset = (h - (rows × blockSize)) / 2;
```

**2. Black Square (GLF_EN) - Lines 503-524:**
```cpp
// Calculate block size matching checkerboard
uint8_t blockSize = constrain(minDim / 5, 10, 32);

// Black square as 2×3 checkerboard blocks (aligned to grid)
int16_t boxW = blockSize * 2;
int16_t boxH = blockSize * 3;
```

**3. Brightness Digit Centering - Lines 543-558:**
```cpp
// Use same centering formula as drawLargeDigit for consistency
uint16_t canvasW = _canvas->width();
uint16_t canvasH = _canvas->height();
int16_t x = (canvasW - w) / 2 - x1;   // Account for x1 offset (important for narrow chars like "1")
int16_t y = (canvasH - h) / 2 + h;    // Baseline adjustment (same as tally digits)
```

**4. Orange Square (GLF_PO) - Lines 569-584:**
```cpp
// Size square as 12% of smallest dimension
uint16_t minDim = min(w, h);
uint16_t squareSize = minDim * 12 / 100;
squareSize = max(squareSize, (uint16_t)12);  // Minimum 12px
if (squareSize % 2 == 0) squareSize++;  // Odd for centering
```

**Results by Display Size:**

| Display | Block Size | Black Square | Orange Square | Result |
|---------|-----------|--------------|---------------|--------|
| M5StickC+ (135×240) | 27px | 54×81px | 16px | Perfect fit, centered |
| LilyGo T-Display (135×240) | 27px | 54×81px | 16px | Perfect fit, centered |
| LilyGo T-QT (128×128) | 25px | 50×75px | 15px | Perfect fit, centered |
| AIPI-Lite (128×128) | 25px | 50×75px | 15px | Perfect fit, centered |

**OTA Binary Generation:**

Created release builds and extracted OTA binaries for all TFT display devices:

```bash
# Build all TFT display targets
pio run -e m5stickc-plus-release -e lilygo-t-display-release \
        -e aipi-lite-release -e lilygo-t-qt-release

# Extract OTA binaries to bin/OTA_TFT_Displays/
```

**Build Statistics:**
- **M5StickC Plus:** RAM 15.5%, Flash 65.7%, Binary 1.28MB
- **LilyGo T-Display:** RAM 15.5%, Flash 65.9%, Binary 1.28MB
- **LilyGo T-QT:** RAM 15.0%, Flash 64.9%, Binary 1.26MB
- **AIPI-Lite:** RAM 15.2%, Flash 15.2%, Binary 1.27MB
- **ATOM Matrix:** RAM 15.7%, Flash 62.7%, Binary 1.19MB
- **Waveshare S3:** RAM 15.0%, Flash 61.2%, Binary 1.17MB

**Library Dependency Fix:**
- Updated `lilygo-t-display-release` and `aipi-lite-release` from LovyanGFX to Arduino_GFX
- Added `-ULITTLE_FOOT_PRINT` flag for text rendering compatibility

**Documentation:**
- Created [bin/OTA_TFT_Displays/README.md](STAC/bin/OTA_TFT_Displays/README.md) with:
  - Dynamic sizing feature description
  - Device specifications and build info
  - OTA update instructions
  - Expected visual appearance per display size

**Modified Files:**
- `src/Hardware/Display/TFT/DisplayTFT.cpp` Lines 240-277, 392-430, 503-524, 543-558, 569-584
- `platformio.ini` Lines 289-310, 403-420
- `bin/OTA_TFT_Displays/README.md` (NEW/UPDATED)

**Testing:**
- ✅ All four TFT devices compile successfully
- ✅ OTA binaries validated on hardware (confirmed by user)
- ✅ Memory usage within acceptable limits
- ✅ Brightness digits properly centered horizontally and vertically
- ✅ Black square scales with display, aligns with checkerboard grid

**Commits:** 
- 7ec3a6c: Initial dynamic sizing for checkerboard patterns and orange square
- 96d522d: Brightness digit centering and dynamic black square sizing

---

### Unconfigured Peripheral Mode Support (Jan 25, 2026)

**✅ COMPLETE:** Peripheral mode now accessible without device configuration

**Problem:** Previous implementation required devices to be fully configured (WiFi + switch settings) before entering peripheral mode, limiting flexibility for users who only need wired tally functionality.

**Solution:** Reordered boot sequence logic to allow peripheral mode entry regardless of configuration state, with clear visual feedback indicating configuration status.

**Implementation Changes:**

**1. Boot Sequence Reordering ([STACApp.cpp](STAC/src/Application/STACApp.cpp#L404-435)):**
- `determineOperatingMode()` now checks button sequence **BEFORE** provisioning state
- Unconfigured devices can enter peripheral mode via button hold at boot
- Provisioning check only blocks Normal mode path (not Peripheral mode)
- Logic flow: Button sequence → Peripheral mode check → Provisioning check → Normal mode

**2. Visual Feedback Enhancement ([STACApp.cpp](STAC/src/Application/STACApp.cpp#L1570-1610)):**
- **Entering Peripheral mode:** GREEN "P" glyph (always - P mode works without config)
- **Exiting to Normal (configured):** GREEN "N" glyph (ready for normal operation)
- **Exiting to Normal (unconfigured):** RED "N" glyph (warning - needs configuration first)
- Checkmark confirmation displays for 1.5 seconds before restart (fixed display timing issue)

**3. Status Reporting ([InfoPrinter.h](STAC/include/Utils/InfoPrinter.h#L190-210)):**

**Serial Monitor Output:**
```
    >>> DEVICE NOT CONFIGURED <<<
     Operating in Peripheral Mode
    Receiving tally via GROVE port
    Tally Mode: Talent
    Brightness Level: 1
=======================================
```

**4. Web Portal Integration ([WebConfigServer.cpp](STAC/src/Network/WebConfigServer.cpp#L422-448)):**

**Info Display for Unconfigured P Mode:**
```
  --------------------------------------
    >>> DEVICE NOT CONFIGURED <<<
     Operating in Peripheral Mode
    Receiving tally via GROVE port
  --------------------------------------
    Tally Mode: Talent
    Brightness Level: 1
```

**User Experience:**

**Fresh Device → Peripheral Mode:**
1. Hold button at power-on
2. GREEN "P" flashing → Release
3. Green checkmark (1.5s) → Restart
4. Device operates in Peripheral mode (reads tally via GROVE)
5. Serial/web show "DEVICE NOT CONFIGURED" status

**Unconfigured P Mode → Provisioning:**
1. Hold button at power-on
2. RED "N" flashing (warning: not configured) → Release
3. Green checkmark → Restart
4. Device enters Provisioning mode (web portal)

**Configured Device Toggling:**
1. Hold button → GREEN "P" or GREEN "N" → Release
2. Checkmark confirmation → Mode switches

**Technical Notes:**
- Peripheral mode settings (camera/talent mode, brightness) persist independently in NVS
- Operating mode flag (`pmEnabled`) stored separately from provisioning state
- No code changes needed to peripheral mode operation itself - only boot sequence logic
- Checkmark display fixed: single `show()` call after preparing complete buffer

**Modified Files:**
- `src/Application/STACApp.cpp` - Reordered mode determination, visual feedback colors, checkmark fix
- `include/Utils/InfoPrinter.h` - Added unconfigured state to peripheral mode serial output
- `src/Network/WebConfigServer.cpp` - Special case for unconfigured peripheral mode in info display

**Testing:**
- ✅ Unconfigured device enters/exits peripheral mode correctly
- ✅ Configured device toggling between Normal/Peripheral modes
- ✅ RED "N" warning shows when exiting P mode without configuration
- ✅ GREEN glyphs confirm successful operations
- ✅ Checkmark visible for full 1.5 seconds
- ✅ Serial output and web portal info display correct status

**Commit:** [pending]

---

### Web Portal Info Display Enhancement & UI Refinements (Jan 24, 2026)

**✅ COMPLETE:** Enhanced web portal info display with NVS integration, UI improvements, form default corrections

**Info Display Enhancement:**

**Problem:** Info modal showed static "Not configured" message even when device was fully provisioned and configured.

**Solution:** Enhanced WebConfigServer to load actual configuration from NVS via ConfigManager integration.

**Implementation ([WebConfigServer.cpp](STAC/src/Network/WebConfigServer.cpp)):**
- Added ConfigManager include for NVS access
- Enhanced `buildIndexPage()` to load actual device configuration:
  - WiFi credentials (SSID, password)
  - Switch configuration (model, IP, port, username, password)
  - StacOperations (tally channel, brightness, auto-start, polling interval)
  - Operating mode (Normal/Peripheral)
- Detects provisioning state - shows "*** Not Configured ***" when not provisioned
- Properly formats V-60HD vs V-160HD channel display (HDMI/SDI notation)
- Uses monospaced `<pre>` tag for formatted display

**UI Refinements ([WebConfigPages.h](STAC/include/Network/WebConfigPages.h)):**

**Button & Modal Changes:**
- Button label: "Show Geek Info" → "Show Info"
- Modal title: "Geek Info" → "STAC Info"
- Clipboard confirmation: "Geek info copied to clipboard!" → "Information copied to the clipboard."

**Text Alignment Fix:**
- Added `text-align: left;` to `.modal-text` CSS class
- Removed centering to preserve monospaced formatting
- Space-based column alignment now displays correctly

**V-160HD Form Default Corrections:**

**Problem:** V-160HD configuration form showed incorrect default credentials.

**Solution:** Updated LAN Username/Password defaults to match Roland V-160HD factory settings:
- LAN Username: "admin" → "user"
- LAN Password: "admin" → "0000"

**Implementation:**
- Updated HTML input field default values (lines 433, 436)
- Updated JavaScript fallback values (lines 869, 870)
- Ensures consistent defaults across HTML and JavaScript

**Modified Files:**
- `src/Network/WebConfigServer.cpp` - Added ConfigManager integration for dynamic info display
- `include/Network/WebConfigPages.h` - UI refinements, V-160HD form defaults

**Testing:**
- ✅ Build and upload to ATOM Matrix successful
- ✅ Device boots and connects to WiFi in Normal mode
- ✅ Info display ready for verification with provisioned device
- ✅ V-160HD form defaults ready for testing

**User Experience Impact:**
- **Info Display:** Shows actual device configuration (WiFi, switch, tally settings, operating mode)
- **Clarity:** "Not configured" only appears when device truly not provisioned
- **Professionalism:** Improved button labels and confirmation messages
- **Readability:** Left-aligned monospaced text maintains proper formatting
- **Convenience:** Correct factory defaults pre-filled for V-160HD initial configuration

**Commit:** [pending]

---

### Comprehensive Code Cleanup & Developer Documentation Enhancement (Jan 24, 2026)

**✅ COMPLETE:** Dead code removal, code consolidation, build fixes, and developer guide improvements

**Code Review & Cleanup:**

Performed systematic code review across entire codebase identifying and removing obsolete code from the LovyanGFX to Arduino_GFX migration and other legacy components.

**Files Deleted (11 files, ~1,350 lines removed):**

**Obsolete LovyanGFX Integration:**
- `include/Hardware/Display/TFT/LGFX_M5StickCPlus.h` (~150 lines)
- `include/Hardware/Display/TFT/LGFX_LilygoTDisplay.h` (~150 lines)
- `include/Hardware/Display/TFT/LGFX_STAC.h` (~250 lines)

**Unused Calibration System:**
- `src/main_calibrate.cpp` (~650 lines)
- `include/Calibration/CalibrationDisplay.h` (~40 lines)
- `include/Calibration/CalibrationDisplayLED.h` (~90 lines)
- `include/Calibration/CalibrationDisplayTFT.h` (~150 lines)
- `include/Calibration/` directory removed

**Obsolete Fonts:**
- `include/Hardware/Display/TFT/FreeSansBold10pt7b.h` (~255 lines)
- `include/Hardware/Display/TFT/FreeSansBold18pt7b.h` (~485 lines)

**Obsolete Development Tools:**
- `src/main_display_offset.cpp.disabled` (~170 lines)

**Board Config Cleanup (4 files):**
- Removed dead LED matrix defines from TFT board configs:
  - `M5StickCPlus_Config.h` - Removed PIN_DISPLAY_DATA placeholder (-3 lines)
  - `LilygoTDisplay_Config.h` - Removed PIN_DISPLAY_DATA placeholder (-2 lines)
  - `TQT_Config.h` - Removed PIN_DISPLAY_DATA placeholder (-2 lines)
  - `AIPI_Lite_Config.h` - Removed entire commented LED matrix section (-48 lines)

**Code Consolidation:**

**Display Position Mapping (Display5x5.cpp, Display8x8.cpp):**
- **Problem:** Duplicate serpentine/row-by-row xyToPosition logic in both 5×5 and 8×8 display classes
- **Solution:** Created shared `xyToPositionHelper(x, y, matrixSize, serpentine)` in DisplayBase.h
- **Result:** Eliminated 26 lines of duplicate code, improved maintainability

**Implementation:**
```cpp
// include/Hardware/Display/DisplayBase.h
inline uint8_t xyToPositionHelper(uint8_t x, uint8_t y, uint8_t matrixSize, bool serpentine) {
    if (serpentine && (y % 2 != 0)) {
        return y * matrixSize + (matrixSize - 1 - x);
    } else {
        return y * matrixSize + x;
    }
}

// Usage in Display5x5.cpp and Display8x8.cpp
uint8_t Display5x5::xyToPosition(uint8_t x, uint8_t y) const {
    return xyToPositionHelper(x, y, 5, serpentine);
}
```

**Version Management Improvement:**

**Problem:** STAC_SOFTWARE_VERSION hardcoded in Device_Config.h with FIXME comment about using build system version.

**Solution:** 
- Removed hardcoded version from Device_Config.h
- Updated Constants.h to include build_info.h and use BUILD_VERSION
- Made PIN_DISPLAY_DATA conditional for TFT boards (build fix)

**Modified Files:**
- `include/Device_Config.h` - Removed STAC_SOFTWARE_VERSION define and FIXME (-5 lines)
- `include/Config/Constants.h` - Added build_info.h include, uses BUILD_VERSION, conditional DISPLAY_DATA (+5 lines)
- `include/Hardware/Display/DisplayBase.h` - Added xyToPositionHelper() function (+22 lines)
- `src/Hardware/Display/Matrix5x5/Display5x5.cpp` - Now uses helper (-13 lines)
- `src/Hardware/Display/Matrix8x8/Display8x8.cpp` - Now uses helper (-13 lines)
- `src/Application/STACApp.cpp` - Removed obsolete LGFX_STAC reference (-2 lines)

**Build Verification:**

Successfully tested builds on 3 platforms after all changes:
- ✅ `atom-matrix` - ESP32, 5×5 LED matrix, 67.9% flash, 15.7% RAM
- ✅ `m5stickc-plus` - ESP32, TFT + AXP192 PMU, 71.2% flash, 15.5% RAM
- ✅ `lilygo-t-display` - ESP32, TFT + PWM backlight, 71.5% flash, 15.6% RAM

**Developer Documentation Enhancement:**

**Added "Quick Start: Key Extension Points" section to DEVELOPER_GUIDE.md:**

Comprehensive reference guide for developers extending STAC functionality, covering:

**Core Application Classes:**
- **STACApp** - Main controller with mode handlers (normal, peripheral, provisioning)
- **ConfigManager** - Persistent configuration (WiFi, switch, protocols, device identity)

**Hardware Abstraction Layer:**
- **IDisplay** - Display interface (LED matrix & TFT implementations)
- **IIMU** - IMU sensor interface (MPU6886, QMI8658)
- **IRolandClient** - Video switcher protocol interface (V-60HD, V-160HD)
- **GlyphManager** - Glyph storage with automatic orientation rotation

**State Management:**
- **TallyStateManager** - Tally state tracking with callbacks
- **OperatingModeManager** - System mode management
- **SystemState** - Centralized state coordination

**Network Layer:**
- **WiFiManager** - WiFi connection and AP management
- **WebConfigServer** - Web portal for provisioning and OTA updates

**Extension Scenarios:**
- Adding a new board configuration
- Adding a new Roland switcher model
- Adding custom operating modes
- Customizing tally display behavior
- Adding persistent configuration parameters

**Benefits:**
- Developers get quick overview of where to extend functionality
- Clear identification of extension points in each class
- Complements existing comprehensive Doxygen documentation in headers
- Reduces time to understand codebase for new contributors

**Code Quality Impact:**

- **Total Lines Removed:** ~1,350 lines of dead/obsolete code
- **Files Deleted:** 11 files plus 1 directory
- **Code Duplication:** Eliminated duplicate xyToPosition implementations
- **Build Health:** All test platforms compile successfully
- **Maintainability:** Cleaner codebase, better organized, well documented
- **Developer Experience:** Clear extension points and comprehensive guides

**Modified Files:**
- `Documentation/Developer/DEVELOPER_GUIDE.md` - Added ~400 line "Quick Start" section

**Commit:** [pending]

---

### TFT Display Reset Optimization & Peripheral Mode Testing (Jan 23, 2026)

### TFT Display Reset Optimization & Peripheral Mode Testing (Jan 23, 2026)

**✅ COMPLETE:** Early LCD reset before IMU initialization, peripheral mode verified

**Display Reset Timing Optimization:**

**Problem:** TFT displays were reset AFTER IMU orientation detection, potentially causing electrical noise during sensitive sensor readings.

**Solution:** Moved LCD reset to before IMU initialization for electrical silence during sensor readings.

**Implementation ([STACApp.cpp](STAC/src/Application/STACApp.cpp#L155-183)):**
```cpp
#if defined(TFT_RST) && (TFT_RST >= 0)
unsigned long tftResetStartTime = millis();
pinMode(TFT_RST, OUTPUT);
digitalWrite(TFT_RST, HIGH);
delay(10);
digitalWrite(TFT_RST, LOW);  // Assert reset - LCD now electrically quiet
log_i("TFT reset asserted (LCD silent during IMU init)");
#endif

// IMU initialization (~100ms) - LCD in reset state

#if defined(TFT_RST) && (TFT_RST >= 0)
unsigned long elapsedResetTime = millis() - tftResetStartTime;
const unsigned long MIN_RESET_HOLD_MS = 200;  // Conservative for all controllers
if (elapsedResetTime < MIN_RESET_HOLD_MS) {
    delay(MIN_RESET_HOLD_MS - elapsedResetTime);
}
digitalWrite(TFT_RST, HIGH);  // Release reset
delay(200);  // Stabilization
log_i("TFT reset released and stabilized");
#endif
```

**Key Benefits:**
- **Electrical silence:** LCD in reset state during IMU sensor readings (reduces EMI/noise)
- **Proper timing:** Conservative 200ms reset hold time works for all TFT controllers:
  - ST7735: 50ms minimum
  - ST7789: 120ms minimum  
  - GC9A01: 200ms minimum
- **Time efficient:** IMU init (~100ms) provides most of reset hold time
- **Clean architecture:** Hardware reset in app init, driver init in display class
- **User-friendly:** When device rotated and reset, new orientation detected reliably

**Reset Sequence (Industry Standard):**
- HIGH (10ms) → LOW (assert reset) → wait for hold time → HIGH (release) → stabilize (200ms)
- All TFT controllers use active-low reset (LOW = reset asserted)

**Modified Files:**
- `src/Application/STACApp.cpp` - Added early TFT reset logic with timing
- `src/Hardware/Display/TFT/DisplayTFT.cpp` - Removed redundant explicit reset

**Tested Platforms:**
- ✅ M5StickC Plus (ST7789)
- ✅ LilyGo T-QT (GC9A01)
- ✅ AIPI-Lite (ST7735)

**Peripheral Mode Verification:**

**✅ VERIFIED:** Peripheral mode working on ATOM Matrix hardware

Successfully tested I2C communication via Grove port (PIN_TALLY_STATUS_0/1) between two ATOM devices. Peripheral mode functionality confirmed operational after refactor to button-based mode selection.

**Commit:** 0288855

---

### LilyGo T-QT Board Support & Peripheral Mode Refactor (Jan 23, 2026)

**✅ COMPLETE:** Added T-QT board support with peripheral mode capability

**Hardware Specifications:**
- **MCU:** ESP32-S3 (FN4R2: 4MB Flash + 2MB PSRAM, or N8: 8MB Flash)
- **Display:** 0.85" GC9A01 TFT, 128×128 pixels (native, circular)
- **Backlight:** GPIO 10, active-low (LOW = ON, HIGH = OFF)
- **Buttons:** GPIO 0 (left/reset), GPIO 47 (right/primary)
- **Peripheral Connector:** GPIO42/43 (PIN_TALLY_STATUS_0/1)
- **Battery Monitor:** GPIO 4 (ADC)

**Key Technical Fixes:**

1. **Display Positioning (Arduino_GFX GC9A01):**
   - Issue: Content displayed in bottom-right quadrant after 180° rotation
   - Root cause: Driver treating 128×128 as windowed 240×240 framebuffer
   - Solution: Set all offset parameters to 0 for native 128×128 display
   ```cpp
   gfx = new Arduino_GC9A01(
       bus, TFT_RST, rotation, true,
       128, 128,        // Native dimensions
       0, 0, 0, 0       // All offsets = 0
   );
   ```

2. **Active-Low Backlight Support:**
   - Added TFT_BACKLIGHT_ON configuration (HIGH/LOW)
   - Automatic PWM inversion in updateBacklight() when TFT_BACKLIGHT_ON == LOW
   - Backward compatible: boards without define default to active-high

3. **Question Mark Icon Enlarged:**
   - Scale increased from (1,2) to (2,3) for landscape/portrait
   - Global change affecting all boards for better visibility during error conditions

**Peripheral Mode Architecture Changes:**

**Removed:** Jumper-based peripheral mode detection (legacy hardware)
- Deleted `PeripheralMode` class and associated pin defines (PIN_PM_CHECK_OUT/IN)
- Removed `PM_CHECK_TOGGLE_COUNT` constant
- Removed `createPeripheralDetector()` from InterfaceFactory
- Removed `peripheralDetector` member from STACApp

**Retained:** GROVE port output functionality (actual peripheral connector)
- Kept `GrovePort` class for driving PIN_TALLY_STATUS_0/1
- Peripheral mode selection now 100% button-based in software
- Simplified Constants.h to only include TALLY_STATUS pins

**Rationale:**
STAC v3 replaced hardware jumper detection with button-based mode selection. The old detection mechanism was creating build errors on boards without jumper pins. Removal simplifies the codebase and eliminates hardware dependencies while maintaining full peripheral mode functionality through GPIO outputs.

**Template Updates:**
- TEMPLATE_NewBoard_Config.h updated to v1.1 (2026-01-23)
- Added TFT_BACKLIGHT_ON documentation and usage examples
- Added peripheral mode section with PIN_TALLY_STATUS_0/1
- Removed jumper detection references
- Added revision history tracking

**Build Environments:**
- `lilygo-t-qt` (debug)
- `lilygo-t-qt-release` (release)

**Status:**
- ✅ Display working (positioning, rotation, brightness)
- ✅ Build compiles successfully
- ⏳ Peripheral mode output pins untested (hardware verification pending)

---

### Startup Visual Feedback System (Jan 22, 2026)

**✅ COMPLETE:** Unified visual feedback sequence across all hardware platforms

**Visual Feedback Sequence:**

All devices show consistent startup indicators:
1. **Orange power glyph** - "Hardware initializing" (during `initializeHardware()`)
2. **Green power glyph** - "All systems ready" (after `initializeNetworkAndStorage()`, 750ms hold)
3. **Mode determination** - Check provisioning status, boot button state
4. **Mode-specific display** - Config glyph, normal operation, etc.

**Provisioned Device - Normal Boot:**
- Orange → Green (750ms) → Normal operation

**Provisioned Device - Button Held at Boot:**
- Orange → Green (750ms) → Boot button sequence:
  - **PMode devices (M5StickC Plus):** P/N toggle (green) → flash 4× → config (orange) → flash 4×
  - **Non-PMode devices (aipi-lite, LilyGo):** Config (orange) → flash 4×
- Clear display → Redraw config glyph → Pulsing animation (128↔42 brightness)

**Unprogrammed Device (Never Configured):**
- Orange → Green (750ms) → Skip boot button sequence
- Clear display → Config glyph (RED, not orange)
- Flash 4× (500ms static + 4×250ms intervals) → Pulsing animation

**Key Architectural Changes:**

**1. Mode Determination Reordering ([STACApp.cpp](STAC/src/Application/STACApp.cpp#L377-415)):**
```cpp
OperatingMode STACApp::determineOperatingMode() {
    // Check provisioned status FIRST - unprogrammed devices skip boot button UI
    if (!configManager->isProvisioned()) {
        log_i("Device not provisioned, entering provisioning mode");
        return OperatingMode::PROVISIONING;
    }
    
    // Boot button sequence (only for provisioned devices)
    OperatingMode bootMode = checkBootButtonSequence();
    if (bootMode == OperatingMode::PROVISIONING) return OperatingMode::PROVISIONING;
    
    // ... other checks
}
```

**2. Green Glyph Timing ([STACApp.cpp](STAC/src/Application/STACApp.cpp#L48)):**
- Moved to show AFTER `initializeNetworkAndStorage()` completes
- Shows BEFORE `determineOperatingMode()` is called
- Consistent meaning: "All systems initialized successfully" regardless of mode

**3. Unprogrammed Device Flash ([STACApp.cpp](STAC/src/Application/STACApp.cpp#L1022-1026)):**
```cpp
// For unprogrammed devices, flash the config glyph before pulsing
if (!wasProvisioned) {
    delay(500);  // Hold static glyph for 500ms
    display->flash(4, 250, normalBrightness);  // Flash 4 times at 250ms intervals
    display->setBrightness(normalBrightness);  // Restore brightness after flash
}
```

**Display Implementation:**
- **TFT displays:** Flash via brightness modulation (0→128 at 250ms intervals)
- **LED matrices:** Flash via brightness modulation (same timing)
- Both use `DisplayBase::flash()` - works identically across all hardware

**Benefits:**
- **Consistent UX:** Same visual feedback on all platforms (M5StickC Plus, aipi-lite, LilyGo, ATOM Matrix)
- **Clear state indication:** Users immediately understand device status
- **Logical flow:** Green = successful init, then mode-specific behavior
- **Guard clause pattern:** Unprogrammed devices skip unnecessary UI cleanly

**Tested:**
- ✅ aipi-lite (ESP32-S3): Normal boot, button boot, unprogrammed
- ⏳ M5StickC Plus: Pending test (P/N toggle expected to work with new timing)
- ⏳ ATOM Matrix: Pending test (LED flash expected to match TFT behavior)

**Build Size:**
- aipi-lite (build d288ae): Flash 1372271 bytes (16.5%), RAM 49900 bytes (15.2%)

---

### Optimized Font System & Icon Implementation (Jan 21, 2026)

**✅ COMPLETE:** Single optimized font with vector graphics icons, automated extraction tooling

**Font Optimization:**
- Created `extract_font_glyphs.py` script to extract minimal character subset from FreeSansBold24pt7b
- **Characters extracted:** space, *, +, -, 0-9, ?, A, C, N, P, S, T (21 glyphs total)
- **New font:** STACSansBold24pt7b with contiguous range 0x20-0x34
- **Size:** ~2038 bytes bitmap data (down from ~8143 bytes in full FreeSansBold24pt7b)
- **Flash savings:** ~6KB by eliminating dual-font approach
- Automated extraction ensures consistency and reproducibility

**Character Mapping:**
Added `remapCharToSTACSansFont()` function to map standard ASCII to font positions:
- Digits 0-9 → 0x24-0x2D (positions 4-13)
- Question mark → 0x2E (position 14)
- Letters A, C, N, P, S, T → 0x2F-0x34 (positions 15-20)
- Symbols *, +, - → 0x21-0x23 (positions 1-3)

**Bug Fixes:**
- **Brightness digit positioning:** Fixed "4" rendering outside black box
  - Root cause: Digits were printed as standard ASCII without remapping
  - Solution: Use `remapCharToSTACSansFont()` before printing digits
  - Now correctly maps '4' (0x34) to font position 0x28 instead of accessing 'T' glyph
- **Question mark icon:** Replaced placeholder circle with actual font character
  - Implemented `drawQuestionIcon()` using remapped '?' from font
  - Properly centered using getTextBounds() with remapping

**Vector Graphics Icons:**
- **Checkmark:** 14px thick strokes, ~78px tall (previous session's work)
- **Question mark:** Now uses font character (this session)
- **Error X:** Existing vector implementation (unchanged)

**Implementation Details:**
```cpp
// Digit overlay (brightness display) - now with remapping
char digitStr[2] = {static_cast<char>('0' + digit), '\0'};
char remappedDigit = remapCharToSTACSansFont(digitStr[0]);
_canvas->print(remappedDigit);

// Question mark icon - uses font character
char qMark = remapCharToSTACSansFont('?');
_canvas->print(qMark);
```

**Files Created:**
- `scripts/extract_font_glyphs.py` - Automated font extraction tool

**Files Modified:**
- `include/Hardware/Display/TFT/STACSansBold24pt7b.h` - New optimized font with remapping function
- `include/Hardware/Display/TFT/FreeSansBold24pt7b.h` - Restored to original (source for extraction)
- `src/Hardware/Display/TFT/DisplayTFT.cpp` - Single font usage, fixed digit rendering, implemented question icon

**Benefits:**
- **Single font file:** Eliminates dual-font complexity and memory overhead
- **Smaller flash footprint:** ~6KB savings from optimized character set
- **Automated tooling:** Reproducible font extraction for future updates
- **Consistent rendering:** All characters properly mapped and centered
- **Mixed approach:** Font for text, vector graphics for icons - best of both worlds

---

### Web Portal Clipboard Paste Enhancement (Jan 23, 2026)

**✅ COMPLETE:** Hybrid clipboard API approach for cross-browser compatibility

**Problem:**
- Original implementation used deprecated `document.execCommand('paste')` 
- Worked on mobile browsers (with manual Ctrl+V/Cmd+V)
- Failed on desktop browsers with "Clipboard access not supported" error
- Modern Clipboard API (`navigator.clipboard.readText()`) requires HTTPS or localhost
- ESP32 web server runs on HTTP (192.168.6.14), blocking modern API

**Solution: Hybrid Approach with Graceful Fallback**

Implemented three-function system in [WebConfigPages.h](STAC/include/Network/WebConfigPages.h#L746-810):

**1. Primary Function - `loadFromClipboard()`:**
```javascript
function loadFromClipboard() {
  // Try modern API first (HTTPS/localhost)
  if (navigator.clipboard && navigator.clipboard.readText) {
    navigator.clipboard.readText()
      .then(processClipboardData)
      .catch(error => {
        console.log('Clipboard API failed, using fallback:', error.message);
        useExecCommandFallback();
      });
  } else {
    // Browser lacks modern API - use fallback
    useExecCommandFallback();
  }
}
```

**2. Fallback Function - `useExecCommandFallback()`:**
```javascript
function useExecCommandFallback() {
  const pasteArea = document.createElement('textarea');
  pasteArea.style.position = 'fixed';
  pasteArea.style.opacity = '0';
  document.body.appendChild(pasteArea);
  pasteArea.focus();
  
  setTimeout(function() {
    document.execCommand('paste');
    setTimeout(function() {
      const jsonStr = pasteArea.value;
      document.body.removeChild(pasteArea);
      processClipboardData(jsonStr);
    }, 100);
  }, 50);
}
```

**3. Common Processing - `processClipboardData()`:**
- JSON validation
- Model verification (V-60HD/V-160HD)
- Structure validation (wifi, switch objects)
- Error handling with user-friendly messages

**Behavior by Platform:**

| Browser Type | Environment | Result |
|--------------|-------------|--------|
| Desktop Chrome/Firefox/Safari | HTTP (ESP32 AP) | Modern API fails → Fallback triggers → Browser paste UI popup |
| Mobile Safari/Chrome | HTTP (ESP32 AP) | No modern API → Fallback triggers → Browser paste UI popup |
| Desktop (any) | HTTPS/localhost | Modern API works → Immediate paste (if available in future) |

**User Experience:**
1. User clicks "Paste Settings" button in web portal
2. Browser shows native paste UI popup (mobile/desktop)
3. User taps/clicks "Paste" in popup (or presses Ctrl+V/Cmd+V)
4. JSON configuration loaded and validated
5. Form fields populated automatically

**Files Modified:**
- `include/Network/WebConfigPages.h` - Implemented hybrid clipboard approach

**Benefits:**
- **Cross-platform compatibility:** Works on all browsers (mobile and desktop)
- **Graceful degradation:** Tries modern API, falls back automatically
- **Future-proof:** Will use modern API if HTTPS ever added
- **User-friendly:** Native browser paste UI is familiar and accessible
- **No breaking changes:** Maintains backward compatibility with existing workflow

**Testing:**
- Validated on ATOM Matrix (build 5f464a)
- Confirmed working on desktop browsers (Safari/Chrome/Firefox)
- Confirmed working on mobile browsers (iOS Safari, Chrome)
- Configuration import/export cycle tested successfully

---

### Vector Graphics Checkmark Implementation (Jan 21, 2026)

**✅ COMPLETE:** Replaced font-based checkmark with vector graphics implementation

**Problem:**
- Checkmark glyph in STACSansBold24pt7b font rendered as garbled diagonal lines
- Multiple attempts to fix bitmap data failed (manual creation, PNG conversion, git restore)
- Fixed font structure issues (dimensions 28×34→24×28, array size 1200→1976 bytes)
- Attempted professional font tools (Adafruit fontconvert, TTF2GFX) - all produced corrupt output
- Root cause never definitively identified despite 10+ hours of debugging

**Solution: Vector Graphics Approach**
- Removed checkmark glyph from font (range changed from 0x20-0x34 to 0x20-0x33)
- Implemented `drawCheckIcon()` using vector primitives (matching proven error X icon approach)
- Checkmark drawn as two thick strokes using parallel `drawLine()` calls

**Final Implementation:**
```cpp
// Short stroke (bottom-left arm): 14px thick, ~24px length
for (int i = 0; i < 14; i++) {
    _canvas->drawLine(cx - 36 + i, cy + 6, cx - 10 + i, cy + 30, rgb);
}

// Long stroke (ascending arm): 14px thick, ~78px length  
for (int i = 0; i < 14; i++) {
    _canvas->drawLine(cx - 10 + i, cy + 30, cx + 56 + i, cy - 48, rgb);
}
```

**Checkmark Specifications:**
- **Thickness:** 14 pixels (bold, highly visible)
- **Height:** ~78 pixels (scales with display)
- **Width:** ~66 pixels horizontal span
- **Rendering:** Clean, no artifacts, visually balanced with 24pt bold font

**Benefits:**
- No font dependency or bitmap data
- Full control over appearance and scaling
- Simple, maintainable code (8 lines vs 1900-byte bitmap array)
- Zero memory overhead (draws on demand)
- Matches proven vector graphics approach used for error X icon

**Files Modified:**
- `include/Hardware/Display/TFT/STACSansBold24pt7b.h` - Removed checkmark glyph, updated remapping
- `src/Hardware/Display/TFT/DisplayTFT.cpp` - Implemented vector graphics `drawCheckIcon()`

**Lessons Learned:**
- Font-based approach unreliable for custom glyphs despite perfect structure
- Vector graphics preferred for icons (control, simplicity, no tooling dependencies)
- "Perfect is the enemy of good" - vector solution took 30 minutes vs 10+ hours of font debugging

---

### Font System Refactoring & Config Diagnostics (Jan 21, 2026)

**✅ COMPLETE:** Global font remapping helper and config loading diagnostics

**Font Remapping Refactor:**
- Created global `remapCharToSTACSansFont(uint16_t unicode)` helper in STACSansBold24pt7b.h
- Replaced class member `DisplayTFT::remapCharToFont()` with global inline function
- Updated all 6 call sites across DisplayTFT.cpp to use global helper
- **Benefit:** Reusable anywhere in codebase, zero overhead (inline), single source of truth
- **Maps:** Standard ASCII (A=0x41, etc.) → font positions (A=0x2E, etc.)

**Letter Rendering Fix:**
- Fixed letters C, T, S, A, P, N not displaying correctly
- Added `remapCharToSTACSansFont(letter)` call before printing in DisplayTFT.cpp:306
- Letters now render properly on display

**Config Loading Diagnostics:**
- Enhanced `handleNormalMode()` config loading with detailed logging (STACApp.cpp:589-625)
- Added success logging: `"Loaded configuration: channel=%d, model=%s, autoStart=%s"`
- Added failure logging: `"Failed to load protocol configuration from NVS for %s"`
- Added critical error: `"CRITICAL: No active protocol found and config load failed!"`
- Explicit default construction on failure: `ops = StacOperations();`
- **Purpose:** Diagnose potential channel reset bug when switching operating modes
- **Status:** User unable to reproduce reported bug with Normal→P→Normal sequence

**Startup Banner Reformatting:**
- Changed access URL display from single line to two lines for better readability
- **Old:** `Access: http://stac.local (or http://192.168.6.14)`
- **New:** 
  ```
  Access: http://stac.local
      or: http://192.168.6.14
  ```

**Files Modified:**
- `include/Hardware/Display/TFT/STACSansBold24pt7b.h` - Global remapping function
- `src/Hardware/Display/TFT/DisplayTFT.cpp` - Updated to use global remapping, letter fix
- `include/Hardware/Display/TFT/DisplayTFT.h` - Removed class member declaration
- `src/Application/STACApp.cpp` - Enhanced config loading diagnostics
- `include/Utils/InfoPrinter.h` - Reformatted startup banner URLs

---

### Display Flash Function Correction (Jan 20, 2026)

**✅ COMPLETE:** Corrected flash() implementation to match documented specification

**Discovered Issue:**
While reviewing boot button sequence code, found that `flash()` implementation did not match specification in "Documentation/Developer/Flashing a Display.md":
- **Documented:** Flash defined as "removal of content (OFF/black) then return content (ON/visible)"
- **Implemented:** Pattern was reversed - ON first, then OFF
- **History:** Incorrect pattern existed since original DisplayBase creation (commit d05809b)
- **Scope:** Both LED (DisplayBase) and TFT (DisplayTFT) implementations had same bug

**Corrected Implementation:**
```cpp
void DisplayBase::flash(uint8_t times, uint16_t interval, uint8_t brightness) {
    // Flash per documentation:
    // - Remove content (OFF) for interval ms
    // - Return content (ON at brightness) for interval ms
    // - Repeat 'times'
    for (uint8_t i = 0; i < times; i++) {
        setBrightness(0, true);          // OFF: Remove content (black)
        delay(interval);
        setBrightness(brightness, true);  // ON: Return content
        delay(interval);
    }
}
```

**Pattern Change:**
- **Old (incorrect):** ON → OFF → ON → OFF
- **New (correct):** OFF → ON → OFF → ON

**Brightness Management:**
- Brightness parameter is **required** (not optional)
- Caller explicitly passes brightness level for flash
- Caller responsible for restoring brightness after flash() returns
- Design rationale: Allows flashing at different brightness than current display, works consistently across display types

**Boot Button Sequence Refinement:**
- **Flashing = Armed state** - indicates "release button now to execute this action"
- Each state shows glyph + flash to indicate it's armed and ready
- Removed extra flashing during state transitions (was causing double flash)
- Pattern: Initial state flashes → hold past timer → next state shows new glyph and flashes → repeat

**Files Modified:**
- `src/Hardware/Display/DisplayBase.cpp` (lines 88-101): Corrected flash() to OFF→ON pattern
- `src/Hardware/Display/TFT/DisplayTFT.cpp` (lines 453-465): Corrected flash() to OFF→ON pattern
- `src/Application/STACApp.cpp` (lines 1540-1625): Added brightness restoration after flash(), refined state transitions

**Testing:**
- ✅ LilyGo T-Display: Verified OFF→ON flash pattern visually superior
- ✅ Boot button sequence: Single flash per armed state (no double flashing)
- ✅ P/N mode toggle: Flash indicates armed, release executes
- ✅ Provisioning state: Flash indicates armed, release enters portal
- ✅ Factory reset state: Flash indicates armed, release performs reset

---

### Arduino_GFX Font Integration - FreeSansBold24pt7b (Jan 20, 2026)

**✅ COMPLETE:** Migrated from built-in font to Adafruit GFX FreeSansBold24pt7b for improved text rendering

**Motivation:**
- Built-in Arduino_GFX font (6×8 pixels scaled) appeared blocky and pixelated
- FreeSansBold24pt7b provides smoother appearance when scaled down vs scaling up smaller fonts
- Better readability for channel numbers and brightness levels on TFT displays

**Implementation:**
- **Font File:** Downloaded FreeSansBold24pt7b.h from Adafruit GFX Fonts repository (~8.8KB)
- **Header Modification:** Changed `#include <Adafruit_GFX.h>` to `#include "gfxfont.h"` for Arduino_GFX compatibility
- **Text Rendering:** Uses `setFont()`, `setTextSize()`, `getTextBounds()`, and `setCursor()` for positioned text
- **Scaling Strategy:** Larger base font (24pt) scaled down for smoother appearance
- **Centering Fix:** Account for x1 offset from `getTextBounds()` to properly center narrow characters like "1"

**Scale Factors (Portrait/Landscape):**
- Letter glyphs (P, N, C, T, A, S): 3/2 (reduced from 4/3)
- Channel numbers: Single digit 3/2, Double digit 2/1 (reduced from 4-3/3-2)
- Large digit display: 3/2 (reduced from 4/3)
- Brightness overlay: 2/1 (reduced from 3/2) - smallest to fit within black box

**Centering Algorithm:**
```cpp
int16_t x1, y1;
uint16_t w, h;
_canvas->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
int16_t x = (displayWidth - w) / 2 - x1;  // Account for x1 offset
int16_t y = (displayHeight - h) / 2 + h;  // Baseline adjustment
```

**Code Changes:**
- `FreeSansBold24pt7b.h` (NEW): Font data and glyph definitions
- `DisplayTFT.cpp` Line 12: Added font include
- `DisplayTFT.cpp` Lines 290-315: Letter glyph rendering with font
- `DisplayTFT.cpp` Lines 498-524: Brightness digit overlay with font
- `DisplayTFT.cpp` Lines 620-646: `drawLargeDigit()` with font
- `DisplayTFT.cpp` Lines 650-686: `drawChannelNumber()` with font
- `platformio.ini` Line 197: Added `-ULITTLE_FOOT_PRINT` to m5stickc-plus
- `platformio.ini` Line 244: Updated m5stickc-plus-release to Arduino_GFX with `-ULITTLE_FOOT_PRINT`

**Character Set:**
- ASCII 0x20 to 0x7E (95 printable characters)
- Currently using only digits 0-9 for channel numbers and brightness levels
- Future optimization: Could create minimal font with just digits (10 chars) to save ~7-8KB flash

**Platforms Tested:**
- ✅ M5StickC Plus (ST7789 135×240)
- ✅ AIPI-Lite (ST7735S 128×128)

**Quality Assessment:**
- Better than built-in font (significantly smoother)
- Not as smooth as LovyanGFX anti-aliased rendering (expected - Arduino_GFX uses monochrome bitmaps)
- Quality acceptable for brief display duration

**Issues Resolved:**
- ✅ Font too large: Reduced all scale factors by 1
- ✅ Brightness digit overflow: Extra reduction to scale 1/2 for proper fit in black box
- ✅ Digit "1" off-center: Fixed by accounting for x1 offset in centering calculation

---

### Arduino_GFX TFT Migration - AIPI-Lite & LilyGo T-Display (Jan 19, 2026)

**✅ COMPLETE:** AIPI-Lite and LilyGo T-Display migrated to Arduino_GFX

**Platforms Tested:**
- ✅ ATOM Matrix (LED 5×5)
- ✅ M5StickC Plus (ST7789 TFT 135×240)
- ✅ Waveshare ESP32-S3 (ST7789 TFT 135×240)
- ✅ AIPI-Lite (ST7735S TFT 128×128)
- ✅ LilyGo T-Display (ST7789 TFT 135×240)

**Implementation:**
- **AIPI-Lite ST7735 Support:** Extended `ArduinoGFX_STAC.h` to support ST7735S panels (`IPS=false`)
- **No-IMU Board Pattern:** Use `DISPLAY_PHYSICAL_ROTATION` to set display rotation directly
- **NullIMU:** Returns `Orientation::ROTATE_0` (base glyph orientation, no rotation)
- **Framebuffer Clearing:** Clear display in all 4 rotations during initialization to eliminate stale controller memory
- **Rotation Offsets:** `TFT_ROTATION_OFFSETS` array for proper centering when panel < controller memory

**Key Code Changes:**
- `ArduinoGFX_STAC.h`: Added ST7735S panel support with conditional `IPS=false`
- `STACApp.cpp`: Moved `#if IMU_HAS_IMU` guard inside `imu->begin()` block for orientation LUT code
- `STACApp.cpp`: Added `#ifdef DISPLAY_PHYSICAL_ROTATION` to override IMU orientation for no-IMU boards
- `DisplayTFT.cpp`: Added framebuffer clearing loop in all 4 rotations during `begin()`
- `NullIMU.h`: Implemented all required IIMU interface methods, returns `ROTATE_0`
- `AIPI_Lite_Config.h`: Added `DISPLAY_PHYSICAL_ROTATION 3` (270° for upright display)
- `LilygoTDisplay_Config.h`: Added `DISPLAY_PHYSICAL_ROTATION 0` and `TFT_ROTATION_OFFSETS`

**Display Configurations:**
- **AIPI-Lite:** 128×128 ST7735S, rotation 3, no offsets needed
- **LilyGo T-Display:** 135×240 ST7789, rotation 0, rotation-specific offsets {52,40}, {40,53}, {53,40}, {40,52}

**Issues Resolved:**
- ✅ White border artifacts: Fixed by clearing framebuffer in all rotations on init
- ✅ IMU orientation code: Properly guarded with `#if IMU_HAS_IMU`
- ✅ ST7735 panel: Extended ArduinoGFX_STAC.h to support both ST7789 and ST7735S
- ✅ Display rotation: Simplified using library rotation instead of IMU override

**Build Sizes:**
- AIPI-Lite: 1.39 MB (71.4% flash)
- LilyGo T-Display: 1.39 MB (71.3% flash)

**Boot Button Testing:** All boot button sequences tested and verified on all 5 platforms

---

### Button B Reset Functionality (Jan 19, 2026)

**✅ COMPLETE:** Button B reset working in all states including error conditions

**Implementation:**
- Changed from edge-triggered (`wasPressed()`/`wasReleased()`) to level-triggered (`isPressed()`)
- More reliable detection - resets when button held down, not dependent on timing
- `handleButtonB()` called in all error display loops and normal operation
- Works in all error states: connection timeout (orange X), no-reply threshold (purple X), unknown error (red X)

**Button B Locations:**
- Main loop: Called every iteration for normal operation
- Error states: Connection timeout, no-reply error, unknown error
- Web config server: Reset check callback during provisioning mode
- Factory reset: Reset after factory reset completes
- Boot autostart: Cancel/reset during autostart countdown

**Code Changes:**
- `handleButtonB()`: Changed to `isPressed()` (level-triggered)
- Added `handleButtonB()` after connection timeout orange X display
- Changed web config reset callback to `isPressed()`
- Changed factory reset button check to `isPressed()`

**Tested:** ✅ M5StickC Plus (reset works in connection timeout error state)

---

### Boot Button State Machine Implementation (Jan 19, 2026)

**✅ COMPLETE:** Boot button sequence with spec-compliant timing and device restart on mode change

**Implementation:**
- 500ms static glyph display + 2000ms flash (4×500ms) = 2500ms per state
- Uses `display->flash(4, 250, nvsBrightness)` method (backlight/brightness control only)
- Button checked with `!button->isPressed()` immediately after flash returns
- Mode toggle triggers `ESP.restart()` for clean hardware re-initialization
- Orange power pixel properly displayed in peripheral mode (all tally states in Talent mode)
- Green power-on glyph confirmation shown on both LED and TFT displays (750ms)
- TFT backlight turned ON during hardware initialization (after orange power glyph)

**State Machine Sequence:**
1. **PMODE_PENDING** (0-2.5s): P or N glyph → toggle peripheral/normal mode
2. **PROVISIONING_PENDING** (2.5-5s): CFG glyph (orange/red) → force provisioning mode
3. **FACTORY_RESET_PENDING** (5-7.5s): FR glyph (red) → clear all NVS and park

**Key Design Decision:**
- After mode toggle, device calls `ESP.restart()` instead of trying to switch modes inline
- Ensures correct hardware initialization for new mode
- Normal startup animations run automatically on restart
- No need for `skipStartupAnimation` flags or complex state tracking

**Display API Usage:**
```cpp
// Peripheral mode orange power pixel on green background:
display->clear(NO_SHOW);
display->fill(GREEN, NO_SHOW);
display->drawGlyphOverlay(powerGlyph, ORANGE, NO_SHOW);
display->show();
```

**Code Changes:**
- `checkBootButtonSequence()`: 500ms delay + flash(4, 250) for each state
- `handlePeripheralMode()`: Removed skipStartupAnimation parameter
- Boot sequence: Shows checkmark, calls `ESP.restart()` on mode toggle
- Peripheral tally display: Added orange power pixel to NO_TALLY/ERROR state (Talent mode)
- Created `showConfirmationCheckmark()` helper function
- TFT backlight: Turn ON in `initializeHardware()` after orange power glyph
- Green power glyph: Extended to all display types (removed LED-only ifdef)

**Tested:** ✅ ATOM Matrix (all states: P/N toggle, reconfiguration, factory reset)  
**Tested:** ✅ M5StickC Plus (boot sequence visible, mode toggle with restart)

**Build Size:**
- ATOM Matrix: 67.9% Flash, 15.7% RAM
- M5StickC Plus: 71.1% Flash, 15.5% RAM

---

### esp_timer Button Polling (Jan 18, 2026)

**✅ COMPLETE:** Automatic button polling using esp_timer replaces manual read() calls

**Implementation:**
- esp_timer polls buttons at 2ms interval (500Hz)
- Timer runs on Core 0, main application runs on Core 1
- Polls both `button` (main) and `buttonB` (reset, when BUTTON_B_PIN defined)
- Static callback function with static pointers for C-style timer callback access

**Code Changes:**
- Added `buttonPollTimer` (esp_timer_handle_t) member to STACApp
- Added `startButtonPolling()` method to create and start timer
- Removed all manual `button->read()` calls from main loop
- Removed stabilization loops from button initialization
- Simplified wait-for-release loops: `while (button->isPressed()) { delay(10); }`
- Timer starts after button->begin(), followed by 1.5× debounce delay (~30ms)

**Benefits:**
- Button state always current without explicit polling
- Cleaner code with fewer scattered read() calls
- More reliable button detection
- XP_Button methods (isPressed, wasPressed, pressedFor) always valid

**Tested:** ✅ ATOM Matrix, ✅ M5StickC Plus (both buttons working)

**Build Size:**
- M5StickC Plus: 71% Flash, 15.5% RAM
- ATOM Matrix: 67.9% Flash, 15.7% RAM

---

### Boot Button Sequence & Text Rendering (Jan 17, 2026)

**✅ COMPLETE:** Boot button sequence now matches ATOM implementation from commit c8c0bdd

**Boot Button Sequence Behavior:**
- Button held at boot → 4 fast flashes of P/N glyph (125ms intervals)
- Button released during flashes → Toggle PMode, show checkmark, enter new mode
- Button held past 2 seconds → Advance to PROVISIONING_PENDING (CFG glyph)
- Button held past 4 seconds → Advance to FACTORY_RESET_PENDING (FR glyph)
- Button released at any state → Execute that state's action

**Text Rendering on TFT:**
- ⚠️ Arduino_GFX `print()` method not displaying text on M5StickC Plus TFT
- ✅ Workaround: Shape-based rendering using `fillRect()` primitives for P/N letters
- Canvas operations (fillRect, fillCircle, fill) confirmed working
- Text rendering issue isolated to font/print subsystem - root cause unknown

**Shape-Based Letter Rendering (DisplayTFT.cpp):**
```cpp
// Letter P using fillRect primitives
case GLF_P:
    _canvas->fillRect(ox, oy - 15 * scale, 5 * scale, 30 * scale, fg);  // Vertical stem
    _canvas->fillRect(ox, oy - 15 * scale, 15 * scale, 5 * scale, fg);  // Top bar
    _canvas->fillRect(ox + 10 * scale, oy - 15 * scale, 5 * scale, 14 * scale, fg);  // Right
    _canvas->fillRect(ox, oy - 6 * scale, 15 * scale, 5 * scale, fg);  // Bottom of loop

// Letter N using fillRect + diagonal loop
case GLF_N:
    _canvas->fillRect(ox, oy - 15 * scale, 4 * scale, 30 * scale, fg);  // Left vertical
    _canvas->fillRect(ox + 16 * scale, oy - 15 * scale, 4 * scale, 30 * scale, fg);  // Right
    for (int i = 0; i < 30 * scale; i++) {
        int16_t x = ox + 4 * scale + (i * 16 / 30);
        _canvas->fillRect(x, oy - 15 * scale + i, 3, 1, fg);  // Diagonal
    }
```

**Known TFT Timing Issue:**
- LCD display updates take significantly longer than LED matrix
- Initial 4-flash sequence takes ~1.35 seconds (250ms + 4×250ms)
- May need to adjust timing constants for TFT displays
- User perception: flashing appears too slow to catch button release

**Peripheral Mode Display:**
- ✅ Orange square overlay implemented for PROGRAM/PREVIEW/UNSELECTED states
- Error states do NOT show orange square (matches ATOM behavior)

---

### Previous: Arduino_GFX Text Rendering (Jan 17, 2026)

**Playground Testing Results:**
- Created minimal test environment (`v3-arduino-gfx-playground` branch)
- All 8 text rendering tests passing in isolated environment
- Full STAC application has text rendering issues (print() not visible)

**Key Configuration Requirements:**
1. **IPS Parameter:** MUST be `true` for M5StickC Plus (prevents color inversion)
2. **Color Mode:** RGB (default) - NOT BGR for correct RGB565 color mapping
3. **Build Flag:** `-ULITTLE_FOOT_PRINT` required to enable text rendering
4. **Display Offsets:** 52/40/53/40 (col1, row1, col2, row2)
5. **AXP192 PMIC:** Proper initialization critical to prevent device bricking

**Known Issues Remaining:**
- ⚠️ Font-based text rendering not working in full application (using shape workaround)
- ⚠️ TFT display update timing slower than LED matrix (affects flash sequences)
- ⚠️ Button B soft reset behavior needs testing during "big orange X" display

**Branches:**
- `v3-arduino-gfx-test` - Full STAC with shape-based text rendering ✅
- `v3-arduino-gfx-playground` - Minimal test environment ✅
- `v3-lovyangfx-offset-fix` - Investigation branch (archived) ❌

---

## Quick Reference

### Supported Hardware

| Board | Display | IMU | MCU | Status |
|-------|---------|-----|-----|--------|
| M5Stack ATOM Matrix | 5×5 LED | MPU6886 | ESP32-PICO-D4 | ✅ Tested |
| Waveshare ESP32-S3-Matrix | 8×8 LED | QMI8658 | ESP32-S3 | ✅ Tested |
| M5StickC Plus | 135×240 TFT | MPU6886 | ESP32-PICO-D4 | ✅ Tested |
| LilyGO T-Display | 135×240 TFT | None | ESP32 | ✅ Tested |
| AIPI-Lite | 128×128 TFT | None | ESP32-S3 | ✅ Tested |
| **LilyGo T-QT** | **0.85" GC9A01 128×128 TFT** | **None** | **ESP32-S3** | **✅ Tested** |

### Key Files to Edit

| Purpose | File |
|---------|------|
| Select board | `include/Device_Config.h` |
| Board settings | `include/BoardConfigs/<Board>_Config.h` |
| Version number | `include/Device_Config.h` (FIRMWARE_VERSION) |
| Build config | `platformio.ini` |

### Common Commands

```bash
cd /Users/robl/Documents/PlatformIO/Projects/STAC3/STAC

# Build
pio run -e atom-matrix              # Debug build
pio run -e atom-matrix-release      # Release build

# Upload & Monitor
pio run -e atom-matrix -t upload && pio device monitor -b 115200

# Generate release binaries
pio run -e atom-matrix-release -t merged -t ota
```

---

## Project Overview

**STAC** = **S**mart **T**ally **At**om **C**lient

A WiFi-enabled tally light system for Roland video switchers (V-60HD, V-160HD) using ESP32-based hardware.

### Operating Modes

1. **Normal Mode** - WiFi client polling Roland switch for tally state
2. **Peripheral Mode** - Receives tally via GROVE port from master STAC
3. **Provisioning Mode** - Unified web portal with captive portal for WiFi/switch configuration and OTA updates

---

## Architecture

```
┌─────────────────────────────────────────┐
│        Application Layer                │
│  STACApp - Main application logic       │
├─────────────────────────────────────────┤
│        State Management                 │
│  SystemState, TallyState, OpMode        │
├─────────────────────────────────────────┤
│     Network & Storage Layer             │
│  WiFi, Roland Protocol, Web, OTA, NVS   │
├─────────────────────────────────────────┤
│   Hardware Abstraction Layer (HAL)      │
│  Display, IMU, Button, GROVE Port       │
├─────────────────────────────────────────┤
│        Configuration Layer              │
│  Device_Config.h, Board Configs         │
└─────────────────────────────────────────┘
```

### Key Components

**Hardware Layer:**
- `IDisplay` interface → `Display5x5`, `Display8x8`, `DisplayTFT`
- `IIMU` interface → `MPU6886_IMU`, `QMI8658_IMU`
- `GlyphManager` → Orientation-aware glyph rotation
- Factory pattern for hardware creation

**Network Layer:**
- `WiFiManager` → Connection management
- `RolandClientBase` → `V60HDClient`, `V160HDClient`
- `WebConfigServer` → Web-based configuration (no captive portal)
- `DNSServer` → Local hostname resolution (stac.local)

**State Management:**
- `SystemState` → Central state coordinator
- `TallyStateManager` → PROGRAM, PREVIEW, UNSELECTED, NO_TALLY
- `OperatingModeManager` → Mode handling

**Storage:**
- `ConfigManager` → NVS-based configuration
- Namespaces: `stac`, `wifi`, `switch`, `v60hd`, `v160hd`, `peripheral`

---

## File Structure

```
STAC3/
├── STAC/
│   ├── platformio.ini
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Application/STACApp.cpp
│   │   ├── Hardware/
│   │   │   ├── Display/
│   │   │   │   ├── Matrix5x5/Display5x5.cpp
│   │   │   │   ├── Matrix8x8/Display8x8.cpp
│   │   │   │   └── TFT/DisplayTFT.cpp
│   │   │   ├── Sensors/MPU6886_IMU.cpp, QMI8658_IMU.cpp
│   │   │   └── Power/AXP192.cpp
│   │   ├── Network/
│   │   │   ├── WiFiManager.cpp
│   │   │   ├── Protocol/V60HDClient.cpp, V160HDClient.cpp
│   │   │   └── Web/WebConfigServer.cpp, OTAUpdateServer.cpp
│   │   ├── State/SystemState.cpp, TallyStateManager.cpp
│   │   └── Storage/ConfigManager.cpp
│   └── include/
│       ├── Device_Config.h           # Board selection
│       ├── BoardConfigs/             # Per-board settings
│       ├── Config/Constants.h        # Global constants
│       └── Hardware/Display/
│           ├── Glyphs5x5.h           # 5×5 glyphs + rotation LUTs
│           ├── Glyphs8x8.h           # 8×8 glyphs + rotation LUTs
│           └── TFT/                  # TFT display support
└── Documentation/
    ├── Developer/
    │   ├── DEVELOPER_GUIDE.md
    │   ├── HARDWARE_CONFIG.md
    │   ├── Detailed Change Log.md
    │   └── TFT_Display_Support_Context.md
    └── User/
        └── STAC Users Guide.md
```

---

## Adding New Hardware

### New LED Matrix Board

1. Create `include/BoardConfigs/NewBoard_Config.h` (copy from TEMPLATE)
2. Define display pins, LED type, IMU type, button GPIO
3. Include appropriate glyph header (`Glyphs5x5.h` or `Glyphs8x8.h`)
4. Add environment to `platformio.ini`
5. Add `#elif` in `Device_Config.h`

### New TFT Board

1. Create board config with `DISPLAY_TYPE_TFT` define
2. Create LovyanGFX config in `include/Hardware/Display/TFT/`
3. Configure backlight (PMU or PWM)
4. See `TFT_Display_Support_Context.md` for details

---

## Key Patterns

### Display Independence

```cpp
// Board config includes size-appropriate glyph header
#include "Hardware/Display/Glyphs5x5.h"  // Defines GLYPH_WIDTH=5

// Type aliases are auto-defined
using GlyphManagerType = GlyphManager<5>;
using StartupConfigType = StartupConfig<5>;

// Application code is display-agnostic
display->showGlyph(Display::GLF_CHECK);
```

### Configuration Flow

```cpp
// 1. Startup: Load from NVS into SystemState
configManager->loadSwitchConfig(...);
systemState->setOperations(ops);

// 2. Runtime: Access from SystemState (not NVS)
auto ops = systemState->getOperations();

// 3. Changes: Update both
systemState->setOperations(newOps);
configManager->saveSwitchConfig(newOps);
```

### Button B (Reset) Pattern (TFT boards)

```cpp
// STACApp.h
std::unique_ptr<XP_Button> buttonB;
esp_timer_handle_t buttonPollTimer;  // Polls both buttons

// STACApp.cpp - init (buttons are polled automatically by esp_timer at 2ms interval)
buttonB = std::make_unique<XP_Button>(BUTTON_B_PIN, ACTIVE_LOW, enablePullup);
buttonB->begin();
startButtonPolling();  // Starts esp_timer, polls button and buttonB

// STACApp.cpp - loop (no manual read() needed - timer handles it)
if (buttonB->wasPressed()) {
    ESP.restart();
}
```

---

## Build Configuration

### Environments

| Environment | Board | Build Type | Debug Level |
|-------------|-------|------------|-------------|
| `atom-matrix` | ATOM Matrix | debug | 3 (INFO) |
| `atom-matrix-release` | ATOM Matrix | release | 0 (NONE) |
| `waveshare-s3` | Waveshare S3 | debug | 3 (INFO) |
| `waveshare-s3-release` | Waveshare S3 | release | 0 (NONE) |
| `m5stickc-plus` | M5StickC Plus | debug | 3 (INFO) |
| `lilygo-t-display` | T-Display | debug | 3 (INFO) |
| `aipi-lite` | AIPI-Lite | debug | 3 (INFO) |

### Version String Format

- **Release:** `3.0.0-RC.9 (b26d99)`
- **Debug:** `3.0.0-RC.9 (6928a8) D3`

---

## Testing Checklist

### Core Functionality
- [ ] Display: Colors correct, rotation follows IMU
- [ ] Button: Short press, long press (1.5s), boot sequences
- [ ] WiFi: Connect, reconnect, timeout handling
- [ ] Tally: PROGRAM (red), PREVIEW (green), UNSELECTED

### Operating Modes
- [ ] Normal: Poll Roland switch, display tally
- [ ] Peripheral: GROVE input, mirror master
- [ ] Provisioning: Captive portal, config save
- [ ] OTA: Upload firmware, restart
- [ ] Factory Reset: Clear NVS, restart

### Boot Button Sequence (HAS_PERIPHERAL_MODE_CAPABILITY)
| Hold Time | Action | Display | Color |
|-----------|--------|---------|-------|
| 0-2 sec | Toggle Peripheral Mode | P (enable) or N (disable) | Green |
| 2-4 sec | Unified Portal (Setup/OTA) | Gear icon | Orange (provisioned) / Red (not provisioned) |
| 4-6 sec | Factory Reset | Factory reset icon | Red |

### Boot Button Sequence (Non-Peripheral Boards)
| Hold Time | Action | Display | Color |
|-----------|--------|---------|-------|
| 0-2 sec | Unified Portal (Setup/OTA) | Gear icon | Orange (provisioned) / Red (not provisioned) |
| 2-4 sec | Factory Reset | Factory reset icon | Red |

---

## Recent Changes

### January 16, 2026 - M5StickC Plus Display Offset Calibration Tool (v3.0.0-RC.24+)

**Created Display Offset Adjustment Tool:**
- Standalone tool using framebuffer rectangle approach to find ST7789 viewport offsets
- **Key insight:** ST7789 controller has 240×320 internal framebuffer, M5StickC Plus has 135×240 physical display
- Physical display is a window into the larger framebuffer - position controlled by panel offset_x/offset_y
- Tool "lies" to LovyanGFX by reporting panel as 240×320 (full framebuffer), then draws movable 240×135 test rectangle
- Gray background fills framebuffer areas outside physical viewport
- Visual markers: Green border (2px), red corner squares (15×15), blue crosshair (center)
- Keyboard controls: U/D/L/R shift rectangle through framebuffer space, C confirms position
- When rectangle aligns perfectly with physical display = correct offset found

**Tool Architecture:**
- Separate PlatformIO environment: `display-offset-tool`
- Build filter: `-<*> +<main_display_offset.cpp>` (standalone, no STAC dependencies)
- Embedded LGFX class with ST7789 configuration (panel_width/height = 240×320, offset_x/y = 0)
- Configuration constants: ROTATION, PHYSICAL_WIDTH, PHYSICAL_HEIGHT, test_offset_x/y
- Similar pattern to IMU calibration tool (version-controlled, development-time utility)

**Testing Results:**
- **Rotation 3 (USB LEFT, landscape 240×135):** Verified at **{40, 52}** ✅
  - Only 1 pixel different from original config value {40, 53}
  - Blue crosshair centered, red corners slightly clipped by green border (expected behavior)
  - Original config was nearly perfect for this rotation
- **Rotation 1 (USB RIGHT, landscape 240×135):** Display shifted up and left ⚠️
  - Current config value {45, 57} needs adjustment
  - Work incomplete - requires further testing to find correct offset
- **Rotations 0, 2, FLAT:** Already working correctly (240×135 portrait, verified in earlier sessions)

**Outstanding Issues:**
- Autostart corner flashing not working after revert (needs investigation)
- Rotation 1 display shift requires offset adjustment
- Green rectangle visibility: May be partially hidden by M5StickC Plus device shell

**Files Created:**
- `src/main_display_offset.cpp`: Display offset finder tool (170 lines)
- `platformio.ini`: Added `[env:display-offset-tool]` environment (lines 219-227)

**Files Modified:**
- `include/BoardConfigs/M5StickCPlus_Config.h`: Updated rotation 3 offset from {40, 53} to {40, 52}
- `platformio.ini`: Excluded `main_display_offset.cpp` from main build
- `src/Application/STACApp.cpp`: Reverted debug mode (removed static corner square display bypass)

**Implementation Details:**
- Framebuffer config: cfg.panel_width = 240, cfg.panel_height = 320 (not physical 135×240)
- Zero panel offset: cfg.offset_x = 0, cfg.offset_y = 0 (test_offset represents what we're finding)
- Test pattern drawing:
  ```cpp
  lcd->fillScreen(0x18C3);  // Gray background
  lcd->fillRect(test_offset_x, test_offset_y, PHYSICAL_WIDTH, PHYSICAL_HEIGHT, 0x0000);  // Black rectangle
  lcd->drawRect(test_offset_x - 2, test_offset_y - 2, PHYSICAL_WIDTH + 4, PHYSICAL_HEIGHT + 4, 0x07E0);  // Green border
  lcd->fillRect(corners..., 15, 15, 0xF800);  // Red corner squares
  lcd->fillRect(crosshair..., 4, PHYSICAL_xxx, 0x001F);  // Blue crosshair
  ```
- Keyboard input processing: U/D/L/R adjusts test_offset_x/y and redraws pattern, C prints results

**Architecture Benefits:**
- Self-discovering: No manual framebuffer analysis needed
- Visual feedback: See exactly where framebuffer viewport aligns with physical display
- Interactive: Real-time adjustment with immediate visual response
- Version-controlled: Config values committed to git
- One-time use: Only needed during hardware bring-up or when adding new rotation support

**Next Steps:**
- Fix autostart corner flashing issue
- Test rotation 1 with offset tool to find correct values
- Update `TFT_ROTATION_OFFSETS` with final values for all rotations
- Document display offset calibration procedure
- Consider creating similar tools for other TFT boards if needed

**Status:** Tool created and partially validated. Rotation 3 confirmed correct, rotation 1 needs adjustment.

---

### January 15, 2026 - IMU Pattern Detection Refactoring (v3.0.0-RC.24)

**Refactored Pattern Detection to Base Class:**
- Moved duplicated pattern detection logic from MPU6886_IMU and QMI8658_IMU to IIMU base class
- Created `detectOrientationFromPattern()` as protected static method in IIMU
- Moved tolerance constants (ACCL_SCALE, LOW_TOL, HIGH_TOL, MID_TOL) to base class
- Both IMU implementations now use identical shared pattern detection logic

**Benefits Achieved:**
- ✅ Single source of truth for pattern-to-enum mappings
- ✅ Guaranteed consistency across all IMU implementations
- ✅ Easier maintenance - changes only needed in one location
- ✅ Reduced code duplication (~60 lines removed)

**Files Modified:**
- `include/Hardware/Sensors/IIMU.h`: Added shared detection method + constants (+51 lines)
- `include/Hardware/Sensors/MPU6886_IMU.h`: Removed duplicate constants (-5 lines)
- `include/Hardware/Sensors/QMI8658_IMU.h`: Removed duplicate constants (-5 lines)
- `src/Hardware/Sensors/MPU6886_IMU.cpp`: Use shared method (-24 lines)
- `src/Hardware/Sensors/QMI8658_IMU.cpp`: Use shared method (-27 lines)
- `include/Device_Config.h`: Version bump to v3.0.0-RC.24

**Validation:**
- ✅ ATOM Matrix (MPU6886) build: SUCCESS
- ✅ Waveshare S3 (QMI8658) build: SUCCESS
- ✅ ATOM Matrix runtime test: Pattern detection working correctly
- ✅ Waveshare S3 runtime test: Pattern detection working correctly
- ✅ Both devices showing correct orientation with shared detection method

**Result:**
- Code duplication eliminated
- TODO item from pattern-based calibration work completed
- System ready for future IMU implementations (LSM6DSO, ICM-20948, etc.)

---

### January 14-15, 2026 - IMU Calibration v3.0: Pattern-Based Methodology Complete & Validated

**Pattern-Based Calibration Completed (Jan 14):**
- Replaced complex formula-based calculation with empirical pattern matching
- Corner identification breaks 4-fold rotational symmetry
- Pattern sequence validation: 3→0→1→2 or 2→3→0→1 for each 90° CW rotation
- Calibration tool successfully generated working configs for both devices

**CRITICAL RUNTIME BUG DISCOVERED (Jan 14 evening):**
- **Problem:** Waveshare showed incorrect orientations despite successful calibration
- **Root Cause:** QMI8658_IMU.cpp used HARDCODED orientation mappings incompatible with calibration tool
  - Calibration tool: Pattern-based detection (X>0 → ROTATE_180, Y>0 → ROTATE_270, etc.)
  - Runtime code: Different hardcoded logic (X>0 → ROTATE_90, Y>0 → ROTATE_180, etc.)
  - Complete mismatch between calibration and runtime pattern-to-enum assignments
- **Solution:** Replaced hardcoded logic with pattern-based detection matching calibration tool:
  ```cpp
  // Pattern 0: (+1, 0) → ROTATE_180
  // Pattern 1: (0, -1) → ROTATE_90
  // Pattern 2: (-1, 0) → ROTATE_0
  // Pattern 3: (0, +1) → ROTATE_270
  ```
- **Lesson Learned:** Calibration and runtime MUST use identical pattern-to-enum mappings

**Pattern Array Corrections (Jan 14):**
- Fixed PATTERN_Z_AWAY indexing: Reversed array so consecutive 90° CW rotations increment by +1
- Changed from `{{0,-1,0,1},{1,0,-1,0}}` to `{{1,0,-1,0},{0,-1,0,1}}`
- Empirically validated on both ATOM Matrix and Waveshare

**FLAT/UNKNOWN Handling (Jan 14):**
- Fixed calculation to use home position LUT instead of always using lut[0]
- Changed from `lut[4] = lut[0]` to `lut[4] = lut[orientationEnums[0]]`
- Runtime code simplified to use LUT array directly

**Empirical Validation Results:**
- ✅ ATOM Matrix (MPU6886, 5×5, Corner #0): All orientations correct
- ✅ Waveshare ESP32-S3-Matrix (QMI8658, 8×8, Corner #1): All orientations correct
- ✅ Both devices tested with all 4 rotations + FLAT orientation
- ✅ Pattern increment validated: Each 90° CW rotation increments pattern by +1

**Documentation Created (Jan 15):**
- **IMU_Calibration_Methodology.md** - Comprehensive methodology documentation:
  - Pattern-based approach explanation
  - Runtime pattern detection requirements
  - Display LUT configuration
  - FLAT/UNKNOWN handling
  - Empirical validation results
  - Critical bug documentation (hardcoded vs pattern-based)
  - Implementation notes and code duplication issue
- **Publishing_IMU_Methodology.md** - Community sharing strategy

**Code Quality Issue Identified:**
- Pattern detection logic duplicated in MPU6886_IMU.cpp and QMI8658_IMU.cpp
- Proposed refactoring: Move to IMUBase::detectOrientationFromPattern() protected method
- Added to TODO list (MEDIUM priority - working code, optimization not urgent)

**Files Modified:**
- `src/main_calibrate.cpp`: Pattern array corrections, FLAT/UNKNOWN fix
- `src/Hardware/Sensors/QMI8658_IMU.cpp`: Pattern-based detection (critical fix)
- `src/Application/STACApp.cpp`: Simplified LUT lookup
- `include/BoardConfigs/AtomMatrix_Config.h`: Updated calibration data
- `include/BoardConfigs/WaveshareS3_Config.h`: Updated calibration data
- `Documentation/Developer/IMU_Calibration_Methodology.md`: New comprehensive doc
- `Documentation/Developer/Publishing_IMU_Methodology.md`: Sharing strategy
- `Documentation/Developer/Detailed Change Log.md`: v3.0.0-RC.10 entry + TODO items

**Result:**
- Pattern-based calibration methodology fully validated on 2 devices with 2 different IMUs
- Critical calibration/runtime mismatch bug fixed
- Comprehensive documentation created for community sharing
- System ready for production use

---

### January 13, 2026 - IMU Calibration Tool - Arbitrary Home Position Support Complete

**Issue:** Calibration tool required USB DOWN as home position, failed with other orientations
- Validation test with USB LEFT as home revealed three bugs:
  1. Reverse mapping showed home at 90° instead of 0°
  2. Step 6 prompt required double ENTER press
  3. Display rotation LUT values wrong for FLAT/UNKNOWN orientations

**Root Cause Analysis:**
- deviceHome offset not applied to reverse mapping calculation
- Step 6 instruction case used `break` instead of `return`, falling through to waitForEnter()
- FLAT/UNKNOWN incorrectly used home vertical orientation instead of lut[0]
- Key insight: When device is physically flat, getOrientation() always returns ROTATE_0, regardless of which vertical position was chosen as calibration home

**Fixes Applied:**
- Reverse mapping: `physicalAngle = ((physPos - deviceHome) * 90 + 360) % 360`
- Step 6 prompt: Changed to `return` to skip waitForEnter()
- Instructions: Made hardware-agnostic (removed "USB port" references)
- FLAT/UNKNOWN: Use `lut[0]` instead of `lut[orientationEnums[deviceHome]]`

**Testing:**
- USB LEFT as home with Bottom-Left pixel corner: All orientations correct ✅
- Generates LUT matching working manual config ✅
- Physical angle logging shows 0° for chosen home ✅

**Files Modified:**
- `src/main_calibrate.cpp`: Fixed reverse mapping, double ENTER, FLAT calculation
- All three bugs resolved, calibration tool now works for any home position choice

**Commit:** be14e79

---

### January 13-14, 2026 - Calibration Tool v3.0: Pattern-Based Implementation Complete

**Problem Discovery (Evening Jan 13):**
- Testing calibration tool with USB RIGHT as home revealed fundamental issue
- Generated DIFFERENT axis remap: `X=(acc.x), Y=((-acc.y))` vs expected `X=((-acc.y)), Y=((-acc.x))`
- Axis remap should be physical property of mounting (invariant), not dependent on home choice
- Root cause: Accelerometer pattern has 4-fold rotational symmetry
  - Pattern: `X:(-1,0,+1,0), Y:(0,-1,0,+1)` looks identical when rotated 90°, 180°, 270°
  - Different measurement sequences can match the same pattern with different axis remaps

**Solution Part 1: Corner Identification (Jan 13 evening):**
- Use display pixel 0 as absolute reference to break rotational symmetry
- Sequential corner lighting with Y/N prompts
- ✅ **Major Milestone: Axis remap now INVARIANT**
  - Both USB RIGHT and USB DOWN home produce: `X=((-acc.y)), Y=((-acc.x))`
  - Rotational symmetry successfully broken
- ❌ LUT calculation formula still incorrect (multiple iterations attempted)
- Save point: commit f24997d

**Solution Part 2: Complete Pattern-Based Rewrite (Jan 14):**
- Replaced complex formula-based calculation with pattern matching approach
- Insight from user analysis (XP IMU Analysis 2.md): Patterns are cyclical sequences
- New algorithm:
  1. Determine Z-axis direction → select pattern table (Z+away vs Z+toward)
  2. Identify pattern number at each rotation using `identifyPatternNumber(x, y, z)`
  3. Validate sequence: patterns increment by +1 (Z+away) or -1 (Z+toward)
  4. Find axis remap that reproduces identified pattern numbers
  5. Calculate LUT using modular arithmetic: `lutAngle = (cornerOffset*90 - physicalAngle + 360) % 360`
- ✅ **Complete rewrite successful - pattern-based approach working**
  - Simpler and more intuitive than formula-based calculation
  - Pattern sequence validation catches measurement errors
  - Better debugging: shows pattern numbers at each step
  - Backward compatible config output format

**Documentation Created:**
- `IMU_Display_Reference_Tables.md`: 8 rotation tables demonstrating 4-fold symmetry
- `XP IMU Analysis 2.md`: Pattern-based approach proposal (updated with Z-axis requirement)
  - Critical insight: X/Y patterns alone insufficient - Z-axis determines which pattern table
  - Pattern matching simpler than complex formulas

**Files Modified:**
- `src/main_calibrate.cpp`: Complete rewrite (~200 lines changed)
  - Added `identifyPatternNumber()` function with Z-axis parameter
  - Pattern matching against PATTERN_Z_AWAY and PATTERN_Z_TOWARD tables
  - Sequence validation confirms rotation direction
  - Clearer output showing pattern numbers and calculation steps

**Critical Bug Fix (Jan 14 afternoon):**
- ❌ **Runtime mismatch discovered:** Calibration tool simulated different enum assignments than runtime code
- Root cause: Calibration used hardcoded thresholds (0.5f, 0.7f), runtime uses MPU6886_IMU.h constants
  - Runtime: `MID_TOL = 500.0f`, `HIGH_TOL = 900.0f` (after ACCL_SCALE=1000)
  - Calibration simulation: `abs(boardY) > 0.7f` vs runtime `abs(scaledAccY) > MID_TOL`
  - Different thresholds → different enum values → incorrect LUT mapping
- ✅ **Fixed:** Updated calibration to use EXACT same thresholds and scaling as runtime
  - Added runtime constants to calibration simulation
  - Exact match: `abs(scaledAccX) < HIGH_TOL && abs(scaledAccY) > MID_TOL`
  - Pattern sequence bug also fixed: `expectedIncrement = zPointsAway ? 3 : 1` (was inverted)
- ✅ **Runtime validation:** All orientations report correctly, display matches at all rotations
- ✅ **FLAT/UNKNOWN fix:** Changed to use same LUT as physical 0° (no math needed)

**Result:**
- Pattern-based calibration tool v3.0 complete and validated
- Calibration output matches runtime behavior exactly
- USB DOWN home position tested and working
- FLAT orientation uses correct LUT (same as 0°)
- Ready for production use
- `include/BoardConfigs/AtomMatrix_Config.h`: Latest test configuration

**Implementation Status:**
- ✅ Corner identification UI working perfectly
- ✅ Pattern identification and validation
- ✅ Axis remap detection (invariant)
- ✅ LUT calculation (pattern-based)
- ✅ Compiles successfully (350KB flash, 7% RAM)
- ⏸️ Ready for hardware testing

**Next Steps:**
- Test v3.0 calibration tool on ATOM Matrix
- Test on Waveshare ESP32-S3-Matrix
- Validate generated configs produce correct display rotation
- Consider runtime code simplification using pattern-based approach (future enhancement)

**Commits:** 
- f24997d: Corner identification breakthrough (axis remap invariant)
- 9c9ed5e: Pattern-based implementation complete

---

### January 12, 2026 - IMU Orientation System Complete - Waveshare Working, Calibration Tool Updated

**Session 1: Critical Bug Discovery & Architecture Refinement**
- **CRITICAL BUG FIX:** GlyphManager was receiving raw physical orientation instead of mapped display orientation
  - STACApp.cpp was reading IMU orientation twice:
    1. First read: Applied DEVICE_ORIENTATION_TO_LUT_MAP for logging (correct)
    2. Second read: Passed raw orientation to GlyphManager (WRONG - bypassed mapping!)
  - Result: LUT mapping only affected log output, not actual glyph rotation
  - Fix: Consolidated to single IMU read, store mapped orientation in variable, use for both logging and GlyphManager
  - Code duplication eliminated, single source of truth for display orientation
  - Commit: 2ff4679 "Consolidate IMU orientation detection - fix GlyphManager bug"

- **Waveshare ESP32-S3-Matrix Z-axis flip handled:**
  - QMI8658 IMU has Z+ pointing AWAY from display (vs AtomMatrix Z+ toward display)
  - Requires Y-axis rotation LUT swap: Physical 0°→LUT_180, Physical 180°→LUT_0
  - X-axis rotations unchanged: Physical 90°→LUT_90, Physical 270°→LUT_270
  - DEVICE_ORIENTATION_TO_LUT_MAP config:
    ```cpp
    { Orientation::ROTATE_180,   // Physical 0°   → LUT_ROTATE_180
      Orientation::ROTATE_90,    // Physical 90°  → LUT_ROTATE_90  
      Orientation::ROTATE_0,     // Physical 180° → LUT_ROTATE_0   
      Orientation::ROTATE_270,   // Physical 270° → LUT_ROTATE_270 
      Orientation::ROTATE_180,   // FLAT          → same as 0°
      Orientation::ROTATE_180 }  // UNKNOWN       → same as 0°
    ```
  - Insight: IMU_AXIS_REMAP handles coordinate transformation, LUT mapping handles rotation offset

- **Board-specific LUT mapping architecture:**
  - Replaced simple rotation offset with per-board LUT mapping table
  - Each board defines DEVICE_ORIENTATION_TO_LUT_MAP[6] array
  - Maps each physical orientation (0°/90°/180°/270°/FLAT/UNKNOWN) to display LUT
  - Supports both Z-axis orientations (toward/away from display)
  - FLAT and UNKNOWN use home position LUT (typically same as 0°)

- **Architectural separation - Physical vs Display:**
  - IMU layer: Returns RAW physical orientation based on accelerometer only
    * No offset application at IMU level
    * Axis remapping (IMU_AXIS_REMAP_X/Y/Z) handles coordinate transformation
    * Physical orientation logging always accurate
  - Application layer: Applies board-specific LUT mapping
    * STACApp.cpp looks up display orientation using DEVICE_ORIENTATION_TO_LUT_MAP
    * Passes mapped orientation to GlyphManager
    * Logging shows both LUT name and physical orientation

- **Complete nomenclature refactoring:**
  - Eliminated UP/DOWN/LEFT/RIGHT terminology throughout codebase
  - Replaced with rotation degrees: ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270, FLAT, UNKNOWN
  - Updated: Orientation enum, all LUT names, GlyphManager, DisplayTFT, IMU drivers
  - LUT names: LUT_ROTATE_0/90/180/270 (instead of LUT_UP/RIGHT/DOWN/LEFT)
  - Clearer intent: Rotation angle, not arbitrary direction

- **Enhanced debug logging:**
  - Shows LUT being used: "LUT being used: LUT_ROTATE_180"
  - Shows physical orientation: "Physical device orientation: 0°"
  - Removed confusing "Display rotation applied" log (redundant with LUT name)
  - Raw IMU values logged in QMI8658_IMU.cpp: "Raw IMU: acc.x=..., boardX=..., boardY=..."

- **Testing results (Session 1):**
  - Waveshare ESP32-S3-Matrix: All orientations (0°/90°/180°/270°/FLAT) display correctly ✅
  - Physical orientation detection accurate at all positions ✅
  - Display rotation matches physical device orientation ✅
  - FLAT and UNKNOWN use home orientation LUT ✅

**Session 2: Calibration Tool Update & End-to-End Validation**
- **Calibration tool updated to match new architecture:**
  - Removed obsolete IMU_ROTATION_OFFSET detection logic (~70 lines)
  - Added Z-axis orientation detection based on FLAT_UP measurement
    * If FLAT_UP Z < 0: Z+ points AWAY from display
    * If FLAT_UP Z > 0: Z+ points TOWARD display
  - Generates DEVICE_ORIENTATION_TO_LUT_MAP in proper C++ array format
    * Z+ away: Outputs Y-axis swapped LUTs (0°↔180°)
    * Z+ toward: Outputs direct LUT mapping
  - Retained axis remapping detection (X/Y sensor mapping via delta analysis)
  - Output ready for copy/paste into board config files
  - Commit: 9596562 "Update calibration tool to output DEVICE_ORIENTATION_TO_LUT_MAP"

- **End-to-end validation complete:**
  - Ran calibration tool on Waveshare ESP32-S3-Matrix
  - Tool output matched existing board configuration exactly ✅
  - Uploaded main STAC application
  - Tested all orientations (0°, 90°, 180°, 270°, FLAT)
  - All orientations display glyphs correctly ✅
  - IMU detection, LUT mapping, and display rotation all working perfectly ✅

- **Files modified (both sessions):**
  - src/Application/STACApp.cpp: Consolidated IMU read, removed duplication, cleaner logging
  - include/Config/Types.h: Orientation enum ROTATE_* values
  - include/Hardware/Display/Glyphs8x8.h & Glyphs5x5.h: LUT_ROTATE_* names
  - src/Hardware/Display/GlyphManager.cpp: Updated to ROTATE_* enum
  - src/Hardware/Sensors/QMI8658_IMU.cpp: Returns raw orientation, enhanced logging
  - src/Hardware/Sensors/MPU6886_IMU.cpp: Returns raw orientation
  - include/BoardConfigs/WaveshareS3_Config.h: DEVICE_ORIENTATION_TO_LUT_MAP with Z-flip swap
  - include/BoardConfigs/AtomMatrix_Config.h: DEVICE_ORIENTATION_TO_LUT_MAP (direct mapping)
  - src/Hardware/Display/TFT/DisplayTFT.cpp: Updated to ROTATE_* enum
  - src/main_calibrate.cpp: Z-axis detection, DEVICE_ORIENTATION_TO_LUT_MAP output

- **Key insights from debugging:**
  - System appeared to work in logs but display was wrong
  - Root cause: Logging code path vs execution code path diverged
  - LUT mapping was only applied for logging, not for actual GlyphManager
  - Classic case of code duplication hiding critical bug
  - Consolidated to single code path with single source of truth

- **Production ready:**
  - Complete IMU orientation system working on Waveshare ESP32-S3-Matrix ✅
  - Calibration tool generates correct configuration ✅
  - Board config applies correct LUT mapping ✅
  - End-to-end workflow validated ✅
  - Architecture clean, maintainable, and documented ✅

- **Next steps:**
  - Test AtomMatrix with current architecture (expected to work with existing config)
  - Clean up obsolete code (OrientationOffset enum, applyOrientationOffset() function)
  - Consider removing IMU_ROTATION_OFFSET from board config files (no longer used)

### January 8, 2026 - OTA Update UX Improvements & Build System Enhancements
- **OTA Progress Tracking (RC.11):**
  - Added JavaScript XMLHttpRequest-based upload with real-time progress bar
  - Visual progress: 0-100% animated green bar, percentage text display
  - Works across all browsers including mobile (Android Chrome/Edge tested)
  - Flash impact: +2.7KB for JavaScript code
- **Symbol Encoding Fix (RC.11):**
  - Replaced UTF-8 symbols (✓, ✗, ⚠️) with HTML entities (&#10004;, &#10008;, &#9888;)
  - Fixes rendering issues on Android WebView and captive portals
  - Universal browser compatibility without font dependencies
- **Version Display Enhancement (RC.11):**
  - Changed from hardcoded "3.0.0-unified" to `STAC_SOFTWARE_VERSION` macro
  - Version now updates correctly after OTA firmware updates
  - Single source of truth in `Device_Config.h`
- **Build Number & Git Info Display (RC.12):**
  - Serial console: Renamed "Build:" to "Git:" to avoid confusion with build number
  - Web portal: Now displays `BUILD_FULL_VERSION` (e.g., "3.0.0-RC.12 (7259c9)")
  - Added Git commit info to web portal matching serial console format
  - Build number = MD5 hash of source files (tracks code changes)
  - Git commit = repository SHA (tracks commits)
- **Build System Automation (RC.13):**
  - Fixed `build_version.py` to auto-generate `build_info.h` before compilation
  - Version changes in `Device_Config.h` now automatically update build artifacts
  - Script runs immediately when loaded, ensures build_info.h exists before compilation starts
  - Eliminated manual build_info.h regeneration requirement
- **Captive Portal Platform Detection (RC.14):**
  - Enhanced JavaScript detection for macOS, iOS, Android, Windows, and Linux
  - **iOS captive portal:** Full functionality (Setup + Maintenance tabs) - file uploads work natively
  - **macOS captive portal:** Setup tab only + notice - file uploads don't work in captive portal WebView
  - **Android captive portal:** Setup tab only + notice - file uploads don't work in WebView
  - **Desktop browsers (all platforms):** Full functionality when user manually navigates to portal
  - Updated notice text to mention Safari for macOS users
  - Fixed stuck "Opening in browser..." issue - now always shows appropriate content
  - Detection logic: Uses `CaptiveNetworkSupport` presence + platform detection (iPhone/iPad/iPod vs Mac OS X)
  - Validated on: iOS (iPhone), macOS (Safari captive portal + manual Safari), Android (Chrome), Windows (Edge/Chrome), Debian 13 (Firefox)

### January 9, 2026 - Configuration Import/Export & Captive Portal Removal
- **Configuration Import/Export Feature:**
  - Added JSON-based configuration export/import for multi-device deployment
  - Export options: Copy to clipboard or download as file (`stac-v60hd-config.json` / `stac-v160hd-config.json`)
  - Import options: Upload file or paste from clipboard
  - HTTP-compatible clipboard paste using `execCommand()` (works without HTTPS)
  - Model mismatch validation with user confirmation dialog
  - Auto-updates export JSON as user types in forms
  - Enables quick cloning of settings across multiple STAC devices
- **Captive Portal Removal:**
  - Abandoned captive portal approach - iOS has no intuitive exit method, Android/iOS still show "no internet" warnings
  - DNS server changed from wildcard redirect (`*`) to hostname-only (`stac.local`)
  - Users now manually connect to STAC AP, then navigate to http://stac.local or http://192.168.6.14
  - Updated boot banner: "Access: http://stac.local (or http://192.168.6.14)"
  - Removed captive portal detection endpoints (simplified from 8+ endpoints to just 1 for macOS compatibility)
  - Simpler, more reliable user experience across all platforms
- **Geek Info Modal:**
  - Added "Show Geek Info" button on Maintenance tab
  - Displays Device, MAC, Firmware, Git commit, Core version, SDK version
  - Modal dialog with Copy and OK buttons
  - Line breaks properly rendered in display and preserved when copied
- **WebPortal → WebConfig Rename:**
  - Renamed `WebPortalServer` → `WebConfigServer` (no longer a "portal" since captive portal removed)
  - Renamed `WebPortalPages` → `WebConfigPages`
  - Updated all namespace references: `WebPortal` → `WebConfig`
  - Updated include guards, file headers, comments for consistency
  - Removed dead code: `openInBrowser()` function, portal-notice div
- **UX Improvements:**
  - Page title: "STAC Portal" → "STAC Setup"
  - Import: Replaced textarea with "Paste Settings" button
  - Export: "Copy Settings" and "Save Settings" buttons (below Configure STAC)
  - Button order: Configure STAC, then Reset/Back on second row
  - Factory reset confirmation dialog
  - Landing page simplified (removed captive portal exit instructions)
- **Technical Details:**
  - Version: 3.0.0-RC.22
  - Branch: v3-config-import-export (from v3-unified-portal at commit 1534119)
  - Flash: 67.8% (1,319,735 bytes), RAM: 15.7% (51,348 bytes)
  - Tested on: iOS, Android, macOS
  - Git commits: RC.19-RC.22 progression

### January 8, 2026 - Multi-Platform Glyph Compatibility Fixes & Complete Platform Validation
- **Waveshare ESP32-S3-Matrix (8×8 LED):** Added GLF_X alias to GLF_BX (existing Big X glyph at index 18)
  - Boot button sequence uses GLF_X for error states
  - 8×8 glyph set already had Big X icon, just needed alias for compatibility
  - Build: Flash 65.7% (1,267,347 bytes), RAM 15.0% (49,192 bytes)
  - Tested: Unified portal working, boot button sequence visual feedback confirmed
- **M5StickC Plus (TFT LCD):** Added GLF_N glyph support to TFT display system
  - TFT displays render letter glyphs (C, T, A, S, P) using `drawString()` with FreeSansBold font
  - GLF_N was missing when boot sequence changed from GLF_P_CANCEL to GLF_N in commit 164772e
  - Added GLF_N constant (index 36) to `GlyphsTFT.h` to match LED matrix glyph sets
  - Added GLF_N to BASE_GLYPHS array and drawGlyph() switch case to render 'N'
  - Build: Flash 72.1% (1,402,067 bytes), RAM 15.7% (51,368 bytes)
  - Tested: Boot button sequence shows proper glyphs (N, gear, factory reset icons)
- **AIPI-Lite (TFT LCD):** Validated with GLF_N support
  - Build: Flash 16.7% (1,393,195 bytes), RAM 15.4% (50,376 bytes)
  - Tested: Boot button sequence, unified portal, all features working
- **LilyGO T-Display (TFT LCD):** Validated with GLF_N support
  - Build: Flash 72.4% (1,408,063 bytes), RAM 15.7% (51,572 bytes)
  - Tested: Boot button sequence (P, gear, factory reset glyphs), provisioning mode with captive portal, factory reset, configuration save/load, WiFi connection
  - Serial monitor confirmed: startup config sequence, web portal operation, successful configuration submission, restart and normal mode operation
- **Key insight:** TFT and LED matrix displays have fundamentally different architectures:
  - LED matrix: Include `Glyphs5x5.h` or `Glyphs8x8.h` with bitmap glyph data arrays
  - TFT: Include `GlyphsTFT.h` with stub arrays (just index values), render glyphs as graphics primitives
  - Both use same glyph index constants for application code compatibility
- **All five platforms validated:** ATOM Matrix (5×5 LED), Waveshare S3 (8×8 LED), M5StickC Plus (TFT), AIPI-Lite (TFT), LilyGO T-Display (TFT)
- **Production ready:** Unified portal fully operational across entire hardware ecosystem
- Git commits: a96ab82 (GLF_X alias), c88705c (GLF_N support)

### January 7-8, 2026 - Unified Web Portal with Captive Portal Support & Cross-Platform Testing
- **Major architectural change:** Combined provisioning and OTA into single unified web portal
- Replaced separate `WebConfigServer` and `OTAUpdateServer` with unified `WebPortalServer`
- Deleted 6 obsolete files: WebConfigServer.{h,cpp}, OTAUpdateServer.{h,cpp}, WebConfigPages.h, OTAUpdatePages.h (1,521 lines removed)
- **Key Features:**
  - Tabbed interface: "Setup" tab (WiFi + Roland configuration) and "Maintenance" tab (Firmware Update + Factory Reset)
  - Auto-popup captive portal on iOS/iPadOS/Android/Windows devices
  - DNSServer integration for DNS redirect to 192.168.6.14
  - Captive portal detection endpoints: `/hotspot-detect.html` (iOS), `/generate_204` (Android), `/connecttest.txt` (Windows)
  - mDNS support: Access portal at **http://stac.local** (hostname: "stac")
  - Fallback direct access: `http://192.168.6.14`
- **Platform-Specific Behavior:**
  - **iOS/iPadOS captive portal:** Auto-redirect to Safari after 1.5s (captive portal restrictions bypass), full functionality in Safari
  - **iOS/iPadOS caching:** After first successful setup, iOS caches network as "known good" and won't show captive portal on reconnection (user must use browser or "forget" network)
  - **Android captive portal:** Setup tab only with instructions to open browser for OTA (WebView blocks file input for security, navigation attempts blocked)
  - **Android browser (Chrome/Firefox):** Full functionality, both tabs visible, file upload works
  - **Desktop browsers:** Full functionality with both tabs
  - Browser detection: Identifies Chrome/Firefox/Safari vs WebView (checks User-Agent for browser name + absence of `wv`/`WebView`)
- **iOS/iPadOS Compatibility Fixes:**
  - Removed UTF-8 special characters (displayed incorrectly on iOS)
  - Replaced JavaScript `confirm()` dialogs with HTML5 required checkbox (JavaScript dialogs blocked in iOS captive portal)
  - Added `inputmode="numeric"` and `pattern="[0-9]*"` for numeric keyboards on mobile devices (both attributes required for iOS reliability)
  - Added `inputmode="decimal"` for IP address fields (numeric keyboard with decimal point)
  - Auto-redirect button uses `<a href target="_blank">` instead of JavaScript `window.location` (Android best practice)
- **Form Validation (matching v2.x implementation):**
  - IP address pattern: `^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$`
  - SSID: Required, max 32 characters
  - Password: Optional (allows open WiFi), max 63 characters (corrected from 64)
  - Port: Required, 1-65535
  - Max channels (V-60HD/V-160HD): Required, 1-8
  - Poll interval: Required, 175-2000ms
  - V-160HD LAN credentials: Required, max 32 characters each
- **UX Improvements:**
  - Factory reset: Simplified to single button click (removed checkbox confirmation requirement)
  - Android captive portal: Step-by-step instructions to open browser at http://stac.local or http://192.168.6.14
  - All platforms: Consistent mDNS + IP fallback guidance
- **Form Defaults:**
  - V-60HD: port=80, maxChan=6, poll=300ms
  - V-160HD: port=80, poll=300ms, LAN user="admin", LAN pass="admin", HDMI/SDI channels=8
- **Boot Button Sequence Updates:**
  - 0-2 sec: Toggle Peripheral Mode (if capable) - shows green N glyph when exiting to normal mode
  - 2-4 sec: Unified Portal (Setup/OTA) - shows orange/red gear icon
  - 4-6 sec: Factory Reset - shows red factory reset icon
- **Display Improvements:**
  - Added GLF_N glyph for 5×5 and 8×8 displays (clearer than slash-through-P for normal mode)
  - Peripheral mode exit sequence: green N flash → checkmark → clear → channel number
  - Fixed display artifacts when transitioning from peripheral mode to normal mode (added clear() calls)
  - Web-based factory reset calls `ESP.restart()` directly (no orange glyph artifact)
- **Documentation:**
  - Updated DEVELOPER_GUIDE.md with unified portal architecture and platform-specific captive portal behavior
  - Updated "Installing STAC v3.x Unified Portal.md" with comprehensive platform-specific notes section
  - Documented iOS caching behavior, Android WebView restrictions, mDNS compatibility requirements
  - Serial monitor shows "Portal: Captive (auto-popup)" and "Fallback: http://192.168.6.14"
- **Testing:** Validated on iOS/iPadOS (captive portal + Safari, caching behavior confirmed), Android (captive portal + Chrome browser), desktop browsers
- **Memory:** Flash 67.1% (1,305,735 bytes), RAM 15.7% (51,348 bytes)

### December 1, 2025 - TFT Display Startup Artifact Fix
- Fixed display artifacts (stale pixels) showing during soft reset on TFT displays
- **Root cause:** LCD driver memory persists through ESP32 soft reset; hidden offset regions retain stale data
- **Solution:** Unified approach for all TFT panels - clear full driver memory before normal `fillScreen()`
- **M5StickC Plus (ST7789):** Disabled LDO2 (backlight) in AXP192 init - let display code turn it on
- Common fixes for all TFT boards:
  - Early backlight OFF in `main.cpp` before any other init
  - Backlight OFF before all `ESP.restart()` calls
  - Removed LGFX Light_PWM backlight management - control directly via LEDC/analogWrite or PMU
  - Added `PreRestartCallback` to WebConfigServer and OTAUpdateServer
  - Clear full LCD driver memory via `setWindow()` to address hidden offset regions:
    - ST7735/ST7735S: 132×162 memory for various display sizes
    - ST7789: 240×320 memory for various display sizes
    - ILI9341/ILI9342: 240×320 memory
    - GC9A01: 240×240 memory
- Changed AIPI-Lite backlight control from `ledcAttach/ledcWrite` to simpler `analogWrite()`

### November 30, 2025 - AIPI-Lite TFT Display Working
- Added full support for AIPI-Lite board (ESP32-S3 + 128×128 ST7735S TFT)
- **Key finding:** Display labeled ST7789 but requires ST7735S driver
- Added `Panel_ST7735S` support to unified LGFX_STAC.h configuration
- Added `TFT_ROTATION_OFFSET` for board-specific rotation adjustment
- Added `_lcd->setSwapBytes(true)` after init for correct RGB565 colors
- Added configurable `TFT_READABLE` and `TFT_BUS_SHARED` options
- LovyanGFX requires develop branch (1.2.9+) for ESP32-S3/ESP-IDF 5.5 compatibility
- AIPI-Lite specific settings: offset_rotation=2, rotation_offset=3, 27MHz SPI, BGR

### January 12, 2026 - IMU Calibration Redesign - Corner-Based Rotation Method
- **Redesigned calibration tool with corner-based approach:**
  - Previous method used edge-based pixel placement (TOP/RIGHT/BOTTOM/LEFT) - too confusing
  - New method: Light single pixel at buffer index 0, rotate device to place at 4 corners
  - Clearer instructions: "Rotate device until pixel is at TOP-LEFT corner"
  - Measurements taken at: TOP-LEFT (0°), TOP-RIGHT (90°), BOTTOM-RIGHT (180°), BOTTOM-LEFT (270°)
- **Added automatic OFFSET detection:**
  - Analyzes gravity magnitude at each rotation: |g| = √(accX² + accY² + accZ²)
  - Rotation with strongest gravity = device UP (vertical, home position)
  - Calculates OFFSET automatically - no manual adjustment needed
  - Outputs: `#define IMU_ROTATION_OFFSET OrientationOffset::OFFSET_270`
- **Waveshare ESP32-S3-Matrix calibration completed:**
  - Axis mapping: X=((-acc.y)), Y=((-acc.x)), Z=(acc.z)
  - Detected OFFSET: OFFSET_270 (gravity magnitude 1.399g at 270° measurement)
  - DEVICE_ORIENTATION_MAP: Maps display Orientation enum to physical device degrees
  - Board-specific mapping: { "90°", "270°", "0°", "180°", "FLAT", "UNKNOWN" }
- **Enhanced orientation logging:**
  - Dual logging: "Display rotation applied" vs "Physical device orientation"
  - Display rotation: Glyph rotation (0°/90°/180°/270°) applied to buffer
  - Physical orientation: Actual device position (0°/90°/180°/270°/FLAT)
  - DEVICE_ORIENTATION_MAP bridges Display enum to physical degrees
- **Fixed FLAT orientation detection:**
  - QMI8658_IMU.cpp: Now returns Orientation::FLAT when Z-axis dominant
  - Previously returned RIGHT (causing FLAT to show as 180°)
  - FLAT special handling: Uses OFFSET rotation directly for display
- **Identified fundamental calibration methodology issue:**
  - Current approach has double rotation compensation (OFFSET + runtime adjustment)
  - Calibration measurements show device was not consistently vertical
  - Measurement 0: Z=1.085g (device flat), Measurement 3: Z=0.000g (device vertical)
  - Root cause: Calibration reference frame based on pixel position, not device home
  - OFFSET compensates for sensor mounting relative to calibration frame, not device frame
- **Outstanding issues to resolve:**
  - Display rotation working correctly (90° compensation applied in STACApp.cpp)
  - Physical orientation logging inverted (shows 270° when device at 0°)
  - FLAT orientation 180° out of phase (should match home rotation)
  - End-to-end methodology needs revision to eliminate trial-and-error
  - Suspect Z-axis mapping or sensor coordinate system mismatch
  - Need to verify SensorLib returns axes in expected order
- **Next steps:**
  - Verify raw sensor axis order from QMI8658 via SensorLib
  - Add debug output showing which physical direction each axis points
  - Redesign calibration to explicitly define device home position (vertical, USB down)
  - Ensure device held vertical during all measurements (gravity in XY plane, Z≈0)
  - Validate Z-axis is perpendicular to display after remapping
- **Files modified:**
  - `src/main_calibrate.cpp`: Redesigned state machine for corner-based rotation
  - `include/Calibration/CalibrationDisplayLED.h`: Added `showPixelAtIndex()`
  - `include/BoardConfigs/WaveshareS3_Config.h`: Applied calibration values
  - `src/Application/STACApp.cpp`: Added FLAT handling and dual orientation logging
  - `src/Hardware/Sensors/QMI8658_IMU.cpp`: Fixed FLAT detection
- **Testing status:** Waveshare rotation working, but calibration methodology under review

### January 11, 2026 - IMU Calibration System Complete
- **Identified fundamental IMU orientation issue:**
  - Current implementation assumed 4 possible IMU orientations (0°/90°/180°/270° rotation)
  - Reality: IMU can be mounted in 8 configurations (2 Z-axis directions × 4 rotations)
    * Z+ toward display (FORWARD) or Z+ away from display (AFT)
    * Rotated 0°, 90°, 180°, or 270° relative to board Home position
  - Different boards (ATOM vs Waveshare) have different physical mountings
  - See `IMU_Analysis.md` for detailed analysis
- **Implemented interactive calibration tool:**
  - Development-time offline calibration utility (not runtime NVS-based)
  - New PlatformIO environments: `atom-matrix-calibrate`, `waveshare-s3-calibrate`, `m5stickc-plus-calibrate`, `lilygo-t-display-calibrate`, `aipi-lite-calibrate`
  - Minimal builds: Exclude Network, State, Storage, Application layers (343KB flash, 23KB RAM)
  - Interactive procedure via serial monitor:
    1. Place board in Home position (USB down)
    2. Tool shows reference marker (white pixel on LED / arrow on TFT)
    3. User reports marker location (Top/Bottom/Left/Right)
    4. Rotate board 90° clockwise, repeat 3 more times
  - Algorithm analyzes accelerometer data at each rotation:
    * Identifies dominant sensor axis (X/Y/Z) at each position
    * Determines axis polarity (+ or -)
    * Detects standard vs swapped axis mapping
    * Calculates rotation offset from marker progression
  - Outputs complete board config values with proper syntax:
    ```cpp
    #define IMU_AXIS_REMAP_X    (acc.x)      // or (-acc.y), etc.
    #define IMU_AXIS_REMAP_Y    (acc.y)      // or (acc.x), etc.
    #define IMU_AXIS_REMAP_Z    (acc.z)
    #define IMU_ROTATION_OFFSET OrientationOffset::OFFSET_0     // or OFFSET_90/180/270
    ```
  - Removed unused IMU_FACE_DIRECTION (not implemented in code)
- **Axis remapping integration:**
  - Added `getRawAcceleration()` to IIMU interface (MPU6886, QMI8658)
  - Updated IMU drivers to read and apply `IMU_AXIS_REMAP_X/Y/Z` from board config
  - Axis remapping expressions evaluated at runtime (compile-time defined, runtime evaluated)
  - Supports standard mapping (X→X, Y→Y) and swapped mapping (X→Y, Y→X)
  - Supports polarity inversions (e.g., -acc.y)
  - Changed `IMU_ORIENTATION_OFFSET` → `IMU_ROTATION_OFFSET` for clarity
- **Display rotation validation:**
  - Display now correctly rotates based on physical device orientation
  - ATOM Matrix requires `OFFSET_90` (raw IMU LEFT becomes display UP)
  - Tested all 4 rotations (0°/90°/180°/270°) plus FLAT detection
- **Enhanced debugging and logging:**
  - Updated `Orientation` enum documentation in Types.h
    * Clarifies enum represents which display edge should be "up" for correct character display
    * Documents rotation degrees for each value (UP=0°, DOWN=180°, LEFT=270°, RIGHT=90°)
    * Explains difference between display orientation (after offset) and device orientation
  - Enhanced debug logging in IMU drivers:
    * Shows both direction names AND rotation degrees (e.g., "UP (0°)", "RIGHT (90°)")
    * Format: `Display orientation: raw=LEFT (270°), offset=1, corrected=UP (0°)`
  - User-facing log shows physical device orientation:
    * Reports device rotation in degrees: "Device orientation: 0°" (USB down - home)
    * Mapping: Display UP→Device 0°, DOWN→180°, LEFT→90°, RIGHT→270°, FLAT→FLAT
    * More meaningful than abstract directions (UP/DOWN/LEFT/RIGHT)
- **Implementation details:**
  - Display-agnostic architecture: `CalibrationDisplay` base class
  - `CalibrationDisplayLED`: Shows white pixel at top-center (5×5, 8×8 matrices)
  - `CalibrationDisplayTFT`: Shows upward arrow marker (TFT displays)
  - `main_calibrate.cpp`: 9-state interactive state machine
  - Display clears on completion
  - Build system: Excluded main_calibrate.cpp from normal builds (prevents linker conflicts)
- **Testing and validation:**
  - ATOM Matrix (MPU6886): Standard mapping (acc.x, acc.y, acc.z) with OFFSET_90
  - Display rotation matches device orientation in all positions
  - Device orientation logging verified accurate: 0°/90°/180°/270°/FLAT
  - End-to-end validation: Calibration tool → board config → runtime → correct display rotation
- **Architecture achievement:**
  - Self-discovering IMU calibration system
  - Manufacturing-agnostic solution (same IMU chip, different physical mounting)
  - Solves the 8-orientation problem (2 Z-axis directions × 4 rotations)
  - Ready for Waveshare testing (expected: different axis mapping)
- **Documentation:** `Documentation/Developer/IMU_CALIBRATION_TOOL.md`
- **Benefits:**
  - Self-discovering: No manual sensor orientation analysis
  - Manufacturing-agnostic: Works regardless of PCB assembly variations
  - Comprehensive: Handles both axis remapping and rotation offset
  - Version-controlled: Config values committed to git, survive flash erases
  - One-time: Only needed during hardware bring-up
  - Supports both LED matrix (pixel marker) and TFT displays (visual marker)
- **Benefits:**
  - Self-discovering: No manual sensor orientation analysis
  - Manufacturing-agnostic: Works regardless of PCB assembly variations
  - Version-controlled: Config values committed to git, survive flash erases
  - One-time: Only needed during hardware bring-up

### January 13, 2026 - IMU Calibration Tool LUT Rotation Direction Fix
- **Critical bug discovered in calibration tool LUT calculation:**
  - Display rotation LUTs (e.g., `LUT_ROTATE_90`) rotate pixel content in the **same** direction as their name
  - When device physically rotates clockwise, content must rotate **counter-clockwise** to stay upright
  - Original formula produced direct mapping (Physical 90° → LUT_90) which caused upside-down display at 90°/270°
  - Required manual correction: swapping 90°↔270° in output before use
- **Root cause identified:**
  - Tool was calculating: `LUT[rotation] = (rotation - deviceHome + displayOffset) % 360`
  - This assumes LUTs rotate content opposite to their name (they don't)
  - Git history showed commit 2ff4679 had wrong comment: "AtomMatrix has Z+ toward display" (actually Z+ away)
  - Previous direct mapping worked by accident only for specific Z-axis orientations
- **Solution implemented:**
  - Fixed formula to invert rotation direction: `LUT[rotation] = (360 - rotation + displayOffset + deviceHome) % 360`
  - The `360 - rotation` term inverts rotation so content rotates counter to device
  - Example: Physical 90° CW → `(360 - 90 + 0 + 0) % 360 = 270` → LUT_270 (90° CCW)
  - Now produces correct output matching manually-validated working configurations
- **Files modified:**
  - `src/main_calibrate.cpp`: Fixed LUT calculation formula with rotation inversion
  - `src/main_calibrate.cpp`: Added detailed header comment explaining rotation inversion logic
  - `src/main_calibrate.cpp`: Added runtime note in output explaining LUT inversion
  - `Documentation/Developer/IMU_Calibration_Methodology.md`: Updated formula with critical insight section
- **Testing status:**
  - Formula mathematically validated against working ATOM Matrix configuration
  - Calibration tool built and uploaded successfully
  - Awaiting full end-to-end testing (scheduled for tomorrow)
- **Expected outcome:**
  - Tool will now produce directly copy/paste-able configuration
  - No manual LUT swapping required
  - Same calibration methodology works for all devices regardless of Z-axis direction

### January 13, 2026 - IMU Calibration Tool Complete - Production Ready ✅
- **Major achievement: Calibration tool produces directly usable copy/paste configuration**
  - Both ATOM Matrix and Waveshare ESP32-S3-Matrix fully validated end-to-end
  - No manual adjustments needed - output works immediately in main application
  - Physical orientation logging matches actual device position (0°, 90°, 180°, 270°, FLAT)
  
- **Root cause discovered: LUT indexing paradigm mismatch**
  - **Original (wrong) assumption:** `DEVICE_ORIENTATION_TO_LUT_MAP` indexed by physical rotation angles
    - Array index 0 = physical 0°, index 1 = physical 90°, etc.
  - **Actual runtime behavior:** Indexed by `Orientation` enum values from `getOrientation()`
    - Array index = enum value returned by accelerometer logic, NOT physical angle
  - **Why it matters:** For boards with axis remapping, enum values ≠ physical angles
    - Example: Waveshare at physical 0° (USB DOWN) → boardX=-1g → `getOrientation()` returns `ROTATE_270` (enum 3)
    - Runtime looks up `LUT[3]`, so LUT[3] must contain the rotation for physical 0°, not physical 270°

- **Calibration tool comprehensive fix:**
  - **Simulates `getOrientation()` during calibration** using identical threshold logic (0.5g, 0.7g)
  - **Builds enum mapping:** For each measurement, determines which `Orientation` enum `getOrientation()` will return
  - **Constructs LUT by enum index:** `lut[enumValue] = rotation` instead of `lut[physicalPosition] = rotation`
  - **Generates reverse mapping:** `ORIENTATION_ENUM_TO_PHYSICAL_ANGLE` for debug logging
  - **LUT rotation formula:** `(360 - physicalAngle + displayOffset + deviceHome*90) % 360` (inverted for CCW rotation)

- **Code changes implemented:**
  - `src/main_calibrate.cpp` (lines 495-530): Added `orientationEnums[4]` calculation simulating `getOrientation()`
  - `src/main_calibrate.cpp` (lines 547-615): Rewrote `printConfiguration()` to build enum-indexed LUT
  - `src/main_calibrate.cpp`: Added reverse mapping output for physical angle debugging
  - `src/Application/STACApp.cpp` (lines 190-205): Use reverse mapping to log actual physical orientation
  - `include/BoardConfigs/AtomMatrix_Config.h`: Added `ORIENTATION_ENUM_TO_PHYSICAL_ANGLE` (enums match angles)
  - `include/BoardConfigs/WaveshareS3_Config.h`: Added `ORIENTATION_ENUM_TO_PHYSICAL_ANGLE` (enums offset from angles)

- **Validation results - ATOM Matrix:**
  - Calibration output: enum 0→physical 0°, enum 1→physical 90°, enum 2→physical 180°, enum 3→physical 270°
  - All vertical orientations display correctly upright
  - Logs show correct physical angles: "Physical device orientation: 0°" when USB DOWN
  - FLAT orientation: baseline aligned to home position (USB DOWN)

- **Validation results - Waveshare ESP32-S3-Matrix:**
  - Calibration output: enum 0→physical 90°, enum 1→physical 180°, enum 2→physical 270°, enum 3→physical 0°
  - All vertical orientations display correctly upright  
  - Logs show correct physical angles: "Physical device orientation: 0°" when USB DOWN
  - FLAT orientation: baseline aligned to home position (USB DOWN)
  - **Confirms:** Enum values differ from physical angles due to axis remap, but LUT mapping is now correct

- **Technical insights gained:**
  - LUT is runtime-indexed: `displayRotation = lutMap[detectedOrientation]` where `detectedOrientation` is enum
  - `getOrientation()` returns enum based on which accelerometer axis is dominant (>0.7g threshold)
  - Different axis remappings → different enum-to-physical mappings → different LUT contents (but same visual result)
  - Display offset: Determines relationship between pixel 0 corner and home position
  - Home position: Arbitrary choice - any vertical orientation can serve as reference frame
  - Copy/paste now works perfectly: No understanding of LUT internals required by user

- **Production status:**
  - ✅ Calibration tool validated on 2 platforms with different IMU sensors and axis remappings
  - ✅ Output works immediately without manual corrections
  - ✅ Debug logging shows meaningful physical orientations
  - ✅ Tool is generic - works regardless of home position choice
  - ✅ Ready for use on additional hardware platforms

### January 12, 2026 - IMU Calibration Tool - LUT Rotation Direction Fixed
- **Version bump:** RC.22 → RC.23
- **Enhanced build_all.sh script:**
  - Added ATOM Matrix to release build sequence
  - Added `-t clean` flag to all environments for guaranteed clean builds
  - Ensures no stale artifacts from previous builds
- **Fixed merged binary generation:**
  - Corrected bootloader offset in `custom_targets.py`
  - Changed from 0x1000 to 0x0 (esptool pads to 0x1000 automatically)
  - Prevents 0xFF padding that corrupts ESP32 image header
  - All `_FULL.bin` files now have valid 0xE9 magic byte
- **IMU improvements for Waveshare QMI8658:**
  - Added 100ms settling delay after IMU initialization
  - Clarified debug message: "Initial orientation" → "Display orientation"
  - Fixed FLAT orientation handling: Returns RIGHT raw value (becomes DOWN after OFFSET_90)
  - Matches vertical USB-down home position when board is horizontal

### January 10, 2026 - Waveshare IMU Orientation Fix & Startup LED Enhancements
- **Fixed QMI8658 axis remapping** on Waveshare ESP32-S3-Matrix
  - Root cause: Incorrect sign mapping in axis transformation
  - Physical sensor orientation: +X=UP (away from USB), +Y=LEFT (rear view)
  - Correct mapping: Board X (RIGHT) = -Sensor Y, Board Y (UP) = Sensor X
  - With OFFSET_90, all edge orientations now display correctly
- **Improved startup LED feedback sequence:**
  - Orange POG displays immediately after `display->begin()` 
  - Green POG shows for 750ms at end of successful setup
  - Red POG shows on system halt for error indication
  - Removed early LED clearing (caused RMT channel conflict)
- **Cleaned up debug logging:**
  - Converted startup messages to `log_i()` (debug-only, suppressed in release builds)
  - Removed non-existent `Logger.h` include, uses Arduino ESP32 built-in logging

### November 29, 2025 - Provisioning Mode Color Fix
- Button held at boot: ORANGE if provisioned, RED if not
- Pulsing display during provisioning now uses correct color
- Not provisioned: skips factory reset state (already at defaults)
- Matches baseline v2.x behavior

### November 28, 2025 - TFT Support Merged to v3_RC
- Added M5StickC Plus and LilyGO T-Display support
- TFT display via LovyanGFX with sprite buffering
- AXP192 PMU driver for backlight control
- Button B (reset) support for TFT boards
- GLF_FR glyph (factory reset icon)
- Fixed GPIO pullup error on input-only pins

### November 28, 2025 - STS Emulator v1.2.0
- Added per-STAC sequential state mode
- Each STAC gets unique state (first 3 guaranteed different)
- States are static by default (optional cycling)
- Fixed duplicate class definition bug

### Key Documentation
- **TFT Details:** `Documentation/Developer/TFT_Display_Support_Context.md`
- **Full History:** `Documentation/Developer/Detailed Change Log.md`

---

## Known Issues

### Expected Warnings
- `RMT channel not initialized. Call initRMT first` - Ignore (LiteLED RMT DMA not available on ESP32-PICO)

### GPIO Notes
- GPIO 39 (ATOM Matrix, M5StickC Plus) is input-only, no internal pullup
- Use `BUTTON_NEEDS_EXTERNAL_PULLUP` flag in board config
