# STAC v3 NVS (Non-Volatile Storage) Namespace Structure

## Overview

STAC v3 uses the ESP32 Preferences library to store configuration data in Non-Volatile Storage (NVS). Starting with v3, the data is organized into six separate namespaces to logically separate different types of configuration, with protocol-specific namespaces for operational parameters.

All protocol namespaces use version tracking to support future migrations and updates.

---

## Namespaces

### 1. `wifi` Namespace
**Purpose:** WiFi network credentials for infrastructure (station) mode

| Key | Type | Description |
|-----|------|-------------|
| `version` | UInt8 | Configuration schema version (`NOM_PREFS_VERSION`) |
| `ssid` | String | WiFi network SSID |
| `password` | String | WiFi network password |
| `pmEnabled` | Bool | Peripheral mode enabled flag (early boot check) |

**Notes:**
- These credentials are for connecting to an existing WiFi network (infrastructure/station mode)
- Configured by user during initial provisioning via web interface
- The WiFi password is obfuscated before storing
- `pmEnabled` flag is checked early in boot process to determine operating mode
- Global `version` value

---

### 2. `switch` Namespace
**Purpose:** Video switch connection configuration (protocol-agnostic)

| Key | Type | Description |
|-----|------|-------------|
| `version` | UInt8 | Configuration schema version (`NOM_PREFS_VERSION`) |
| `model` | String | Video switch model identifier (e.g., "V-60HD", "V-160HD") |
| `ip` | UInt32 | Switch IP address (stored as 32-bit unsigned integer) |
| `port` | UInt16 | Switch TCP port number |
| `username` | String | Switch authentication username (V-160HD only) |
| `password` | String | Switch authentication password (V-160HD only) |

**Notes:**
- The `model` field determines which protocol-specific namespace is active
- IP address is stored as UInt32 and converted to/from IPAddress object
- Password is obfuscated before storing
- Username and password only used for V-160HD authentication
- This namespace is protocol-agnostic and applies to all switch models

---

### 3. `v60hd` Namespace (Protocol-Specific)
**Purpose:** Roland V-60HD protocol-specific operational parameters

| Key | Type | Description |
|-----|------|-------------|
| `tallyChannel` | UInt8 | Tally channel number being monitored (1-8) |
| `maxChannel` | UInt8 | Maximum available channels (typically 6 or 8) |
| `autoStart` | UInt8 | Auto-start monitoring on power-up (stored as 0/1) |
| `camOpMode` | Bool | Camera operator mode (true) vs talent mode (false) |
| `brightness` | UInt8 | Display brightness level (1-N based on device) |
| `pollInterval` | UInt32 | Status polling interval in milliseconds |

**V-60HD Notes:**
- Only present when configured for V-60HD operation
- No version key stored in this namespace (global version in wifi namespace)
- `autoStart` stored as UInt8 despite being boolean in application code

---

### 4. `v160hd` Namespace (Protocol-Specific)
**Purpose:** Roland V-160HD protocol-specific operational parameters

| Key | Type | Description |
|-----|------|-------------|
| `tallyChannel` | UInt8 | Tally channel number being monitored (1-16) |
| `maxHDMI` | UInt8 | Maximum HDMI channels (typically 8) |
| `maxSDI` | UInt8 | Maximum SDI channels (typically 8) |
| `channelBank` | String | Current channel bank ("hdmi\_" or "sdi\_") |
| `autoStart` | Bool | Auto-start monitoring on power-up |
| `camOpMode` | Bool | Camera operator mode (true) vs talent mode (false) |
| `brightness` | UInt8 | Display brightness level (1-N based on device) |
| `pollInterval` | UInt32 | Status polling interval in milliseconds |

**V-160HD Notes:**
- Only present when configured for V-160HD operation
- No version key stored in this namespace (global version in wifi namespace)
- Channel bank automatically determined from tally channel: channels 1-8 = "hdmi\_", 9-16 = "sdi\_"

---

### 5. `identity` Namespace
**Purpose:** STAC device identification

| Key | Type | Description |
|-----|------|-------------|
| `stacID` | String | Unique device identifier (e.g., "STAC-001") |

**Notes:**
- Displayed in web configuration interface
- Used for device identification and management

---

### 6. `peripheral` Namespace
**Purpose:** Peripheral mode configuration (when STAC acts as peripheral to another STAC)

| Key | Type | Description |
|-----|------|-------------|
| `version` | UInt8 | Configuration schema version (`PM_PREFS_VERSION`) |
| `pmCamMode` | Bool | Camera operator mode (true) vs talent mode (false) |
| `pmBrightness` | UInt8 | Display brightness level in peripheral mode (1-N based on device) |

**Peripheral Mode Notes:**
- Settings only apply when STAC is operating in peripheral mode
- Uses separate version tracking (PM_PREFS_VERSION) independent of NOM_PREFS_VERSION
- Key names prefixed with `pm` to distinguish from normal operating mode settings


---

## Protocol-Specific Architecture

### Design Rationale

Starting with v3, STAC uses protocol-specific namespaces to store operational parameters. This architectural change provides:

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

---

## Factory Reset Behaviour

Factory reset performs a **complete NVS partition erase** using `nvs_flash_erase()` followed by `nvs_flash_init()`. This removes all data from all namespaces:

1. `wifi` - WiFi credentials and peripheral mode flag
2. `switch` - Switch connection configuration
3. `v60hd` - V-60HD operational parameters (if present)
4. `v160hd` - V-160HD operational parameters (if present)
5. `identity` - STAC device ID
6. `peripheral` - Peripheral mode settings

This is more efficient and reliable than clearing namespaces individually, ensuring no orphaned data remains in NVS

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

*Document Version: 3.0*  
*Last Updated: 2026-02-05*  


<!-- EOF -->
