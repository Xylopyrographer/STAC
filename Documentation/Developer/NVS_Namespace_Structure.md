# STAC v3 NVS (Non-Volatile Storage) Namespace Structure

## Overview

STAC v3 uses the ESP32 Preferences library to store configuration data in Non-Volatile Storage (NVS). Starting with v3.0.0-RC.9, the data is organized into **six separate namespaces** to logically separate different types of configuration, with protocol-specific namespaces for operational parameters.

All protocol namespaces use version tracking to support future migrations and updates.

---

## Namespaces

### 1. `wifi` Namespace
**Purpose:** WiFi network credentials for infrastructure (station) mode

| Key | Type | Description |
|-----|------|-------------|
| `ssid` | String | WiFi network SSID |
| `password` | String | WiFi network password |

**Notes:**
- These credentials are for connecting to an **existing WiFi network** (infrastructure/station mode)
- Configured by user during initial provisioning via web interface
- Access Point mode credentials are hardcoded in firmware:
  - SSID: Dynamically generated from STAC ID (e.g., "STAC-12345")
  - Password: "1234567890" (hardcoded constant)

---

### 2. `switch` Namespace
**Purpose:** Video switch connection configuration (protocol-agnostic)

| Key | Type | Description |
|-----|------|-------------|
| `model` | String | Video switch model identifier (e.g., "V-60HD", "V-160HD") |
| `ip` | String | Switch IP address (IPv4 format string) |
| `port` | UInt16 | Switch TCP port number |
| `username` | String | Switch authentication username |

**Notes:**
- The `model` field determines which protocol-specific namespace is active
- Password is currently not stored (uses empty password for authentication)
- This namespace is protocol-agnostic and applies to all switch models

---

### 3. `v60hd` Namespace (Protocol-Specific)
**Purpose:** Roland V-60HD protocol-specific operational parameters

| Key | Type | Description |
|-----|------|-------------|
| `version` | UInt8 | Configuration schema version (currently 1) |
| `tallyChannel` | UInt8 | Tally channel number being monitored (1-8) |
| `maxChannel` | UInt8 | Maximum available channels (typically 6 or 8) |
| `autoStart` | Bool | Auto-start monitoring on power-up |
| `camOpMode` | Bool | Camera operator mode (true) vs talent mode (false) |
| `brightness` | UInt8 | Display brightness level (1-5) |
| `pollInterval` | UInt16 | Status polling interval in milliseconds |

**V-60HD Notes:**
- Simple channel structure: channels 1-8
- No channel banks (unlike V-160HD)
- `maxChannel` is typically 6 or 8 depending on V-60HD variant
- Only present when configured for V-60HD operation

---

### 4. `v160hd` Namespace (Protocol-Specific)
**Purpose:** Roland V-160HD protocol-specific operational parameters

| Key | Type | Description |
|-----|------|-------------|
| `version` | UInt8 | Configuration schema version (currently 1) |
| `tallyChannel` | UInt8 | Tally channel number being monitored (1-8) |
| `maxHDMI` | UInt8 | Maximum HDMI channels (typically 8) |
| `maxSDI` | UInt8 | Maximum SDI channels (typically 8) |
| `channelBank` | String | Current channel bank ("hdmi_" or "sdi_") |
| `autoStart` | Bool | Auto-start monitoring on power-up |
| `camOpMode` | Bool | Camera operator mode (true) vs talent mode (false) |
| `brightness` | UInt8 | Display brightness level (1-5) |
| `pollInterval` | UInt16 | Status polling interval in milliseconds |

**V-160HD Notes:**
- Dual-bank channel structure: HDMI channels (1-8) and SDI channels (1-8)
- `channelBank` determines which physical input bank the `tallyChannel` refers to
- Bank auto-correction: if `tallyChannel` > 8, automatically sets bank based on channel number
- Only present when configured for V-160HD operation

---

### 5. `identity` Namespace
**Purpose:** STAC device identification

| Key | Type | Description |
|-----|------|-------------|
| `stacID` | String | Unique device identifier (e.g., "STAC-001") |

**Notes:**
- Displayed in web configuration interface
- Used for device identification and management
- Set during initial provisioning

---

### 6. `peripheral` Namespace
**Purpose:** Peripheral mode configuration (when STAC acts as peripheral to another STAC)

| Key | Type | Description |
|-----|------|-------------|
| `cameraMode` | Bool | Camera operator mode (true) vs talent mode (false) |
| `brightness` | UInt8 | Display brightness level in peripheral mode (1-5) |

**Peripheral Mode Notes:**
- Peripheral mode uses separate settings from normal operation
- Camera/talent mode can be configured independently for peripheral operation
- Settings only apply when STAC is operating in peripheral mode
- Leader STAC controls which peripheral device is active

---

## Protocol-Specific Architecture

### Design Rationale

Starting with v3.0.0-RC.9, STAC uses **protocol-specific namespaces** to store operational parameters. This architectural change provides:

1. **Clean Separation:** Each protocol's complete parameter set is isolated in its own namespace
2. **No Dead Keys:** No unused or invalid keys for inactive protocol models
3. **Extensibility:** Easy to add new protocols (ATEM, VMix, TriCaster, etc.) without affecting existing ones
4. **Protocol Independence:** Switch between protocols without parameter conflicts or data pollution
5. **Maintainability:** Each protocol implementation is self-contained and easier to maintain

### Active Protocol Selection

- The `switch.model` key determines which protocol namespace is active
- When `model` = "V-60HD", operational parameters are stored in/loaded from `v60hd` namespace
- When `model` = "V-160HD", operational parameters are stored in/loaded from `v160hd` namespace
- ConfigManager automatically loads from the correct protocol namespace based on the switch model

### Migration from Legacy Configuration

STAC automatically migrates configurations from the legacy `operations` namespace (used in pre-RC.9 versions) to the appropriate protocol-specific namespace on first boot after update:

**Migration Process:**
1. On startup, checks for existence of old `operations` namespace
2. If found, reads `switchModel` and all operational parameters
3. Calls appropriate protocol save method (`saveV60HDConfig` or `saveV160HDConfig`)
4. Clears old `operations` namespace to free NVS space
5. Logs migration to serial console

This migration is **transparent** and **preserves all existing configuration data**.

---

## Factory Reset Behavior

Factory reset clears **all six namespaces**:
1. `stac` (internal firmware state)
2. `wifi`
3. `switch`
4. `v60hd` (if present)
5. `v160hd` (if present)
6. `peripheral`

**After Factory Reset:**
- STAC displays the "Configuration Required" icon (red C glyph in 5x5 mode)
- STAC starts in Access Point mode for initial provisioning
- All settings must be reconfigured through the web interface
- STAC remains parked in infinite yield loop until manually reset (power cycle)

**Factory Reset Availability:**
- Factory reset is **only available when STAC is configured**
- Boot button sequence automatically skips factory reset option when unconfigured
- Documented in User's Guide: *"You cannot do a Factory Reset if the red Configuration Required icon is displayed as the STAC is already in its factory default state."*

---

## Comparison to Baseline v2.x

| v2.x Namespace | v2.x Purpose | v3 Equivalent |
|----------------|--------------|---------------|
| `STCPrefs` | All general settings | Split into `wifi`, `switch`, `v60hd`/`v160hd`, `identity` |
| `PModePrefs` | Peripheral mode settings | `peripheral` |

**Key Improvements in v3:**
- More granular organization for better separation of concerns
- Protocol-specific namespaces for better extensibility
- Explicit identity namespace for device management
- Support for multiple protocols without parameter conflicts
- No "dead" configuration keys for unused protocol models
- Cleaner migration path for future protocols

---

## Implementation Details

### ConfigManager API

The `Storage::ConfigManager` class provides protocol-specific methods:

#### V-60HD Protocol
```cpp
bool saveV60HDConfig(const StacOperations& ops);
bool loadV60HDConfig(StacOperations& ops);
```

#### V-160HD Protocol
```cpp
bool saveV160HDConfig(const StacOperations& ops);
bool loadV160HDConfig(StacOperations& ops);
```

#### Protocol Detection
```cpp
String getActiveProtocol();                    // Returns current switch model
bool hasProtocolConfig(const String& protocol); // Checks if protocol namespace exists
```

#### Peripheral Mode
```cpp
bool savePeripheralSettings(bool cameraMode, uint8_t brightness);
bool loadPeripheralSettings(bool& cameraMode, uint8_t& brightness);
```

#### Other Configuration Methods
```cpp
// Switch configuration
bool loadSwitchConfig(String& model, IPAddress& ip, uint16_t& port, 
                     String& username, String& password);
bool saveSwitchConfig(const String& model, const IPAddress& ip, uint16_t port,
                     const String& username, const String& password);

// WiFi configuration
bool loadWiFiConfig(String& ssid, String& password);
bool saveWiFiConfig(const String& ssid, const String& password);

// Device identity
bool loadStacID(String& stacID);
bool saveStacID(const String& stacID);

// Factory reset
void clearAll();  // Clears all namespaces
```

### Application Usage Pattern

```cpp
// Load configuration based on active protocol
String protocol = configManager->getActiveProtocol();
if (protocol == "V-60HD") {
    configManager->loadV60HDConfig(ops);
} else if (protocol == "V-160HD") {
    configManager->loadV160HDConfig(ops);
}

// Save configuration based on switch model in operations struct
if (ops.switchModel == "V-60HD") {
    configManager->saveV60HDConfig(ops);
} else if (ops.switchModel == "V-160HD") {
    configManager->saveV160HDConfig(ops);
}
```

### NVS Key Constraints
- Maximum key length: 15 characters (ESP32 NVS limitation)
- All keys use camelCase naming convention
- String values limited by NVS (typically up to 1984 bytes)
- STAC uses much shorter strings in practice

---

## Future Protocol Extensions

The protocol-specific namespace architecture is designed to easily support additional switcher protocols:

### Potential Future Protocols

| Protocol | Namespace | Description |
|----------|-----------|-------------|
| `atem` | Blackmagic ATEM | ATEM switcher family support |
| `vmix` | vMix | vMix software switcher support |
| `tricaster` | NewTek TriCaster | TriCaster hardware switcher |
| Custom | User-defined | Customer-specific protocols |

### Adding a New Protocol

To add a new protocol (e.g., ATEM):

1. **Define Namespace:** Add `NS_ATEM = "atem"` constant in ConfigManager
2. **Define Keys:** Add protocol-specific key constants (e.g., `KEY_ATEM_PROGRAM_INPUT`)
3. **Implement Methods:** Create `saveATEMConfig()` and `loadATEMConfig()` methods
4. **Update Factory Reset:** Add ATEM namespace to `clearAll()`
5. **Update Web Interface:** Add ATEM model selection and configuration fields
6. **Create Protocol Client:** Implement ATEM protocol client class
7. **Update Documentation:** Document ATEM-specific keys and behavior

Each protocol is fully self-contained and does not affect existing protocols.

---

## Version History

### Version 2.0 (v3.0.0-RC.9+)
- **Breaking Change:** Migrated from unified `operations` namespace to protocol-specific namespaces
- Added `v60hd` namespace for V-60HD protocol parameters
- Added `v160hd` namespace for V-160HD protocol parameters
- Removed `operations` namespace (automatically migrated on first boot)
- Added automatic migration logic for existing configurations
- Updated all save/load methods to use protocol-specific namespaces
- Updated factory reset to clear new namespaces

### Version 1.0 (v3.0.0-RC.1 to RC.8)
- Initial v3 namespace structure
- Used unified `operations` namespace for all protocols
- Five namespaces: `wifi`, `switch`, `operations`, `identity`, `peripheral`

---

*Document Version: 2.0*  
*Last Updated: 2025-11-21*  
*STAC Firmware Version: v3.0.0-RC.9+*  
*Author: STAC Development Team*
