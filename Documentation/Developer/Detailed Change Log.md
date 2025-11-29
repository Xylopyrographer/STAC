
## Detailed Change Log

Intended for developers and others interested in the nitty gritty.

---

## Version 3.0.0 Development

### v3.0.0-RC.9 + TFT Support (November 2025)

**STS Emulator v1.2.0 (November 28, 2025)**
- Added per-STAC sequential state mode
- Each STAC IP gets unique state in connection order
- First 3 STACs guaranteed different states (UNSELECTED, SELECTED, ONAIR)
- States are static by default (no cycling) for easier visual testing
- Optional per-STAC cycling available
- Fixed duplicate STSEmulator class definition

**TFT Display Support (November 26-28, 2025)**
- Added M5StickC Plus support (ST7789V2 TFT, AXP192 PMU)
- Added LilyGO T-Display support (ST7789V TFT, PWM backlight)
- DisplayTFT class with LovyanGFX sprite-based rendering
- AXP192 PMU driver for power and backlight control
- Button B (reset) support for TFT boards via XP_Button polling
- GLF_FR glyph (factory reset icon - bars with X)
- Fixed GPIO pullup error on input-only pins (NEEDS_EXTERNAL_PULLUP flag)
- See `TFT_Display_Support_Context.md` for full details

**NVS Version Management Refactoring (November 25, 2025)**
- Simplified from complex migration to baseline v2.x approach
- Single global `NOM_PREFS_VERSION` stored only in wifi namespace
- Removed ~150 LOC of migration code
- Manual factory reset required for schema changes

**Board Configuration Cleanup (November 25, 2025)**
- Centralized global constants to Constants.h
- Removed 6 redundant defines from each board config
- Created TEMPLATE_NewBoard_Config.h and TEMPLATE_GlyphsMxN.h
- Type-safe enums for IMU orientation offset

**Architectural Refactoring (November 25, 2025)**
- Eliminated all `#ifdef` blocks in application code
- Moved rotation LUTs into glyph headers
- Added dimension-agnostic type aliases (GlyphManagerType, StartupConfigType)
- Template-based design for future display sizes

**Pre-Release Code Quality (November 24-25, 2025)**
- ~200 LOC reduction across 6 phases
- isProvisioned() helper, storage constants, brightness maps to board configs
- Separate glyph headers per display size (Glyphs5x5.h, Glyphs8x8.h)
- Switch model helpers (isV60HD, isV160HD)
- Duplicate NVS reads eliminated

**Glyph-Based Refactoring (November 22, 2025)**
- Eliminated 18 magic numbers (corner pixels, colors)
- Created GLF_CORNERS glyphs for 5×5 and 8×8
- Moved pulseCorners to DisplayBase
- Consolidated all colors to StandardColors namespace

**Custom Build Script (November 22, 2025)**
- Automated firmware binary generation
- Naming: STAC_v{version}_{board}_{build}.bin
- PlatformIO targets: `pio run -t merged -t ota`

**Code Refactoring (November 21, 2025)**
- DisplayBase class eliminates ~180 lines per display type
- RolandClientBase eliminates ~100 lines between protocols
- StateManagerBase<T> template eliminates ~140 lines
- NetworkClient.flush() → clear() for arduino-esp32 v3.3.2+

**Build Versioning Enhancement (November 21, 2025)**
- Automatic extraction from PlatformIO configuration
- Clean version for release: "3.0.0-RC.9 (b26d99)"
- Annotated version for dev: "3.0.0-RC.9 (6928a8) D3"

**Factory Reset Fix (November 21, 2025)**
- Fixed to match baseline v2.x behavior (single flash, infinite park)
- Skip factory reset when unconfigured (matches User's Guide)

### v3.0.0-RC.1 to RC.9 (November 2025)

**RC.9 Fixes**
- V-160HD error handling improvements
- HTTP timeout optimization
- Polling improvements

**RC.2 to RC.8 Fixes**
- V-160HD startup sequence bug fixes
- Channel display fixes
- Color corrections
- Serial output improvements

**RC.1 Features**
- Critical error recovery fix
- STS Emulator utility
- Web UI enhancements

### Phase 9: Web Config & OTA (November 2025)

- Glyph management system
- Roland protocol support (V-60HD, V-160HD)
- Web configuration captive portal
- OTA firmware updates
- Boot button sequence with visual feedback
- Performance optimizations (server startup timing)

### Architecture Overhaul (October-November 2025)

**Complete Rewrite from v2.x**
- Migrated from Arduino IDE (.ino) to PlatformIO project
- Object-oriented architecture with interfaces
- Hardware Abstraction Layer (HAL) for display, IMU, buttons
- Factory pattern for hardware creation
- Centralized state management (SystemState, TallyState, OpMode)
- ConfigManager for NVS storage
- Support for multiple hardware platforms (ATOM Matrix, Waveshare S3)

---

### Version 2.2.0
* Refactored pretty much the entire code base. Not an insignificant effort as you know.
* Moved things about in and inside the `STACLib` "`.h`" files.
* Add support for the Roland V-160HD Streaming Video Switch.
* Rearranged items in the serial port info dump & added a few more needed for the V-160HD.
* Replace the FastLED library with the LiteLED library.
    - See docs in the LiteLED library.
* Replace the JC\_Button with the XP\_Button library.
    - See docs in the XP\_Button library.
* Add short "How to" to the serial monitor out when doing an OTA update.
* migrate to arduino-esp32 core v2.0.14.

### Version 2.1.1

* Bug fix - STAC ID.
    * A unique STAC ID using the last 4 bytes of the ESP-32 chip MAC address, in byte reverse order, is used as the ID suffix. For a chip MAC ending in `4D:3C:2B:1A` the STAC ID would be `STAC-1A2B3C4D` with there always being eight characters after the `-`.  However, with the method used, all trailing zeros of the MAC are removed, resulting in ID's with less than eight characters. Not the desired outcome. (Trailing zero's become leading zero's which are ignored by the String HEX formatting function.)

    * Solution: Use a different method to retrieve the chip MAC and then format it using `snprintf()`. Add new function to generate the STAC ID to `/STACLib/STACUtil.h`

* Internal improvements
    * Code changes throughout to convert all in-line `#include` files to functions, with appropriate function calls replacing them.

### Version 2.1

* Add the ability to do OTA firmware updates via a web browser.
    * routines are in `STACUtil.h`. 
    * implemented using the `WebServer` library.
* Use the `ESPmDNS` library when the STAC is operating a WiFi access point for setup & firmware updates.
    * operational details are in the *Users Guide.*
* Move all HTML code for setup and OTA updates to `STACsuPages.h` and `STACotaPages.h` respectively.
    * make all HTML code HTML5 compliant.
* Refactor the initialization code in `STACProvision.h`.
* Refactor `getCreds()` in `STACUtil.h` to use the WebServer library.
* Refactor the display drawing routines to increase speed by reducing the number of calls to `FastLED.show()`.
    * Add a function parameter to optionally refresh the display.
    * Revise all display calls throughout the code to match match the above.
    * Rename functions that draw to the display using `x` & `y`  coordinates to `drawPixXY()` due to changing overloads for the `drawPix()` functions, 
* Set the display brightness using a LUT that maps a brightness level to an absolute value that is passed to `FastLED`.
    * the brightness LUT contains a max of 9 levels.
    * as it is a table, brightness can follow a curve.
* Add the ability when in Peripheral Mode to change the Tally Display Mode to either Camera Operator or Talent.
    * operational details are in the *Users Guide.*
* Rename `STACDisplay.h`, `STACGlyph.h` and `STACIMU.h` to `STACDisplay5.h`, `STACGlyph5.h` and `STACIMU5.h` respectively.
    * the "`5`" indicating use for 5 x 5 matrix displays.
    * except that the "`5`" for the `STACIMU5.h` file means it is for the IMU type used in the ATOM MATRIX.
* Move `updateBrightness()` to `STACOpModes.h`
* Modify layout of the serial data dumps.
* Code changes throughout to make the code more modular and to convert a number of routines to be functions rather than in-line `#includes`.
* Address a number of housekeeping items.

### Version 2.0

* Require arduino-esp32 core 2.0.3 or greater
    * there are irreconcilable breaking changes in WiFiClient from core 1.0.6

* Replace M5Stack ATOM libraries:
  * M5Stack are making library changes that are not backward compatible.
  * use `JC_Button` library for the display button;
  * use `I2C_MPU6886` library for the IMU;
  * use `FastLED` library for the display;
    * replace "`M5.dis.X`" functions with bespoke display drawing routines.
  * directly initialize `Serial`.
  
* Break code into separate `.h` files by function group into a new STACLib folder.
  * add appropriate `#include` statements to `STAC.ino`.

* Rewrite of the `loop()` tally display code to better facilitate error handling.
  * error handling should now be done in `loop()`.

* Add user configurable polling interval via the web config page.
  * add polling interval as an NVS item.
  * increment the NVS `NOM_PREFS_VERSION`.

* Correct improper use of the `Preferences` library.

* No longer reformats the NVS when doing a factory reset.
  * clears all `Preferences` name spaces instead.

* Change layout of the startup data dump.
  * display the arduino-esp32 core version used.
  * display the ESP-IDF SDK version used. 
  * show the polling interval.
  * items above the "`-----`" line are hard coded.
  * items above the "`=-=-=`" line are web config items.
  * items below the "`=-=-=`" line are run-time configurable.

* Add macros for setting the tally state on the GROVE connector.

* Set WiFi hostname to the STAC ID.

* Set the normal operating mode default polling interval to 300 ms.

* Set the Peripheral Mode polling interval to 0 ms.

* Remove left over debug code in `parseForm()`.

* Change all `drawGlyph()` `colors[]` parameter data type to `const int`.

* Remove `buttonClicked()` function in favour of native `JC_Button` library calls.
  * remove all references to `btnWas` & `btnNow`.

* Change `stIP` to use data type `IPAddress` instead of `char[]`.

* Fix the "set" order in `STACWiFi.h` so setting the hostname works with core v2.0.3.

* Fix a coding error in `getCreds()` in `STACWeb.h` that greatly improves error handling.

* Add `fetchInfo()` function to `STACUtil.h`.
  * interesting for debugging; not used in normal operation.

* With core v2.0.3 & greater, the default WiFi authentication/encryption 
    mode of the STAC when being provisioned is now WAP2-PSK (AES).

* Rework of the error handling in `loop()`.

* Add "BigOrangeX" display error as notification that there is no STS connection at all (Camera operator mode only).
  
* Add filtering for erroneous `stClient` connection cases.
  * require eight consecutive errors of the same type before the glyph for that error type is displayed.
  * exception is when there is no connection at all to the STS (aka "BigOrangeX"), for which the error glyph is always displayed immediately.
  * all error counters are reset after an error glyph is displayed,

* Add setting of STS re-poll interval on errors by error type, independent of the re-poll interval; currently set as:
  * 1500 ms for "BigOrangeX"
  * 1000 ms for unknown "BigRedX"
  * 50 ms for all other errors

* Add `stClient.stop()` if we have an STS connection but the STS response timed out.

* Display the arduino-esp32 core version used on the STAC web config form.

* Display the ESP-IDF SDK version used on the STAC web config form.

* Add STAC software build number.
    * displayed in brackets after the software number on the Serial data dump and on the STAC web config form.
    * see `Documentation/Creating the build number.md` for details.
