# STAC v3 NVS (Non-Volatile Storage) Namespace Structure

## Overview

STAC v3 uses the ESP32 Preferences library to store configuration data in Non-Volatile Storage (NVS). The data is organized into five separate namespaces to logically separate different types of configuration.

All namespaces use version tracking to support future migrations and updates.

---

## Namespaces

### 1. `wifi` - WiFi Network Credentials

Stores the WiFi network connection information.

**Keys:**
- `ssid` (String) - WiFi network SSID (max 32 characters)
- `password` (String) - WiFi network password (max 63 characters, can be empty for open networks)
- `version` (UChar) - Namespace version number for migration tracking

**Notes:**
- This namespace must exist and have a valid SSID for the STAC to be considered "configured"
- Used by `ConfigManager::hasWiFiCredentials()` to determine provisioning state

---

### 2. `switch` - Roland Switch Configuration

Stores connection and authentication details for the Roland video switch.

**Keys:**
- `model` (String) - Switch model: "V-60HD" or "V-160HD"
- `ip` (UInt) - Switch IP address stored as 32-bit unsigned integer
- `port` (UShort) - Switch HTTP port number (default: 80, V-160HD: 8080)
- `username` (String) - LAN username for authentication (V-160HD only, typically "user")
- `password` (String) - LAN password for authentication (V-160HD only)
- `version` (UChar) - Namespace version number for migration tracking

**Notes:**
- IP address is stored as a uint32_t and converted to/from IPAddress type
- V-60HD typically uses port 80
- V-160HD typically uses port 8080
- Username/password are only used for V-160HD LAN control authentication

---

### 3. `operations` - Operating Parameters

Stores runtime operating parameters that control STAC behavior.

**Keys:**
- `switchModel` (String) - Copy of switch model for convenience ("V-60HD" or "V-160HD")
- `tallyChannel` (UChar) - Active tally channel number being monitored (1-16)
- `maxChannelCount` (UChar) - Maximum tally channel for V-60HD (1-8)
- `channelBank` (String) - Channel bank identifier for V-160HD ("hdmi_" or "sdi_")
- `maxHDMI` (UChar) - Maximum HDMI channel for V-160HD (1-8)
- `maxSDI` (UChar) - Maximum SDI channel for V-160HD (1-8)
- `autoStart` (Bool) - Auto-start mode enabled flag
- `camOpMode` (Bool) - Camera operator mode flag (true = Camera Operator, false = Talent)
- `brightness` (UChar) - Display brightness level (1-6)
- `pollInterval` (ULong) - Status polling interval in milliseconds (175-2000, default: 300)
- `version` (UChar) - Namespace version number for migration tracking

**Notes:**
- Channel numbering: V-60HD uses 1-8, V-160HD uses 1-8 for HDMI, 9-16 for SDI
- channelBank is automatically set based on tallyChannel for V-160HD (>8 = "sdi_")
- Brightness maps to physical LED brightness values via BRIGHTNESS_MAP_5X5 or BRIGHTNESS_MAP_8X8
- Poll interval practical increments are 50ms steps

---

### 4. `identity` - STAC Identity

Stores the unique identifier for this STAC unit.

**Keys:**
- `stacid` (String) - Unique STAC identifier in format "STAC-XXXXXX"

**Notes:**
- Generated from MAC address (last 3 bytes, reversed): `STAC-` + 6 hex digits
- Example: MAC `94:B9:7E:A8:F8:00` → STAC ID `STAC-00F8A8`
- Used for WiFi AP SSID during provisioning and as network hostname
- Generated once and persists across factory resets in baseline v2.x
- **v3 Behavior:** Currently regenerated after factory reset (differs from baseline)

---

### 5. `peripheral` - Peripheral Mode Settings

Stores settings specific to Peripheral Mode operation.

**Keys (from baseline v2.x):**
- `pmPrefVer` (UShort) - Peripheral mode preferences version
- `pmct` (Bool) - Peripheral mode camera/talent display mode (false = Talent, true = Camera Operator)
- `pmbrightness` (UChar) - Peripheral mode display brightness level (1-6)

**Notes:**
- Only used when STAC operates in Peripheral Mode (jumper installed)
- Independent settings from normal operating mode
- Retains brightness and tally mode separately from normal operations
- Factory defaults: Talent mode, brightness level 1
- **v3 Status:** Namespace is cleared during factory reset but storage implementation is pending

---

## Factory Reset Behavior

When a factory reset is performed via the boot button sequence:

**Namespaces Cleared:**
1. `wifi` - WiFi credentials removed
2. `switch` - Switch configuration removed
3. `operations` - Operating parameters reset to defaults
4. `identity` - STAC ID removed (v3 behavior - differs from baseline)
5. `peripheral` - Peripheral mode settings reset to defaults

**Post-Reset State:**
- STAC returns to unconfigured state
- Requires provisioning via web interface
- Operating parameters reset to:
  - Tally Channel: 1 (HDMI 1 for V-160HD)
  - Tally Mode: Camera Operator
  - Startup Mode: Standard (not autostart)
  - Brightness: Level 1
  - Poll Interval: 300ms
- Peripheral mode settings reset to:
  - Tally Mode: Talent
  - Brightness: Level 1

---

## Version Tracking

Each namespace (except `identity`) includes a `version` key (`UChar`) that stores the current schema version number defined in `Config::NVS::NOM_PREFS_VERSION`.

**Purpose:**
- Enables future schema migrations
- Allows detection of outdated configuration data
- Supports backward compatibility during firmware updates

**Migration Process:**
- On startup, `ConfigManager::checkAndMigrateConfig()` checks version numbers
- If version mismatch is detected, migration logic can update the schema
- Currently used to detect and handle legacy configurations

---

## Related Files

**Implementation:**
- `STAC/include/Storage/ConfigManager.h` - Namespace and key definitions
- `STAC/src/Storage/ConfigManager.cpp` - Storage operations implementation
- `STAC/src/Application/STACApp.cpp` - Factory reset namespace clearing

**Configuration:**
- `STAC/include/Config/Constants.h` - Version constants and defaults

**Baseline Reference:**
- `STAC_220_rel/STAC/STACLib/STACUtil.h` - Baseline factory reset (STCPrefs, PModePrefs)
- `STAC_220_rel/STAC/STACLib/STACPeripheral.h` - Baseline peripheral mode storage

---

## Baseline v2.x Comparison

**Namespace Mapping:**

| v2.x Baseline | v3 Current | Contents |
|---------------|------------|----------|
| `STCPrefs` | `wifi`, `switch`, `operations`, `identity` | All normal mode configuration combined in v2.x, separated in v3 |
| `PModePrefs` | `peripheral` | Peripheral mode settings (same concept, different implementation) |

**Key Differences:**
1. v3 uses separate namespaces for logical separation of concerns
2. v3 includes explicit version tracking in each namespace
3. v3 separates identity from operations (STAC ID in separate namespace)
4. v2.x preserved STAC ID across factory reset; v3 currently regenerates it

---

*Last Updated: 2025-11-21*  
*STAC Version: 3.0.0-RC.9*
