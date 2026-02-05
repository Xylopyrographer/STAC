# STAC Developer Guide

A comprehensive guide for developers working on STAC or building similar embedded systems projects.

## Table of Contents

- [STAC Developer Guide](#stac-developer-guide)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Initial Setup](#initial-setup)
    - [Building the Project](#building-the-project)
  - [Quick Start: Key Extension Points](#quick-start-key-extension-points)
    - [Core Application Classes](#core-application-classes)
      - [**STACApp** - Main Application Controller](#stacapp---main-application-controller)
      - [**ConfigManager** - Persistent Configuration Storage](#configmanager---persistent-configuration-storage)
    - [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal)
      - [**IDisplay** - Display Interface](#idisplay---display-interface)
      - [**IIMU** - Inertial Measurement Unit Interface](#iimu---inertial-measurement-unit-interface)
      - [**IRolandClient** - Video Switcher Protocol Interface](#irolandclient---video-switcher-protocol-interface)
      - [**GlyphManager** - Glyph Storage and Rotation](#glyphmanager---glyph-storage-and-rotation)
    - [State Management](#state-management)
      - [**TallyStateManager** - Tally State Tracking](#tallystatemanager---tally-state-tracking)
      - [**OperatingModeManager** - System Mode Management](#operatingmodemanager---system-mode-management)
      - [**SystemState** - Centralized State Container](#systemstate---centralized-state-container)
    - [Network Layer](#network-layer)
      - [**WiFiManager** - WiFi Connection Management](#wifimanager---wifi-connection-management)
      - [**WebConfigServer** - Web-Based Configuration Portal](#webconfigserver---web-based-configuration-portal)
    - [Factory Classes](#factory-classes)
      - [**DisplayFactory**](#displayfactory)
      - [**IMUFactory**](#imufactory)
      - [**RolandClientFactory**](#rolandclientfactory)
    - [Utility Classes](#utility-classes)
      - [**StartupConfig** - Interactive Configuration at Boot](#startupconfig---interactive-configuration-at-boot)
    - [Configuration Files](#configuration-files)
      - [**Device\_Config.h** - User Hardware Selection](#device_configh---user-hardware-selection)
      - [**Board Configs** - Hardware-Specific Settings](#board-configs---hardware-specific-settings)
    - [Common Extension Scenarios](#common-extension-scenarios)
      - [Adding a New Board](#adding-a-new-board)
      - [Adding a New Roland Switcher Model](#adding-a-new-roland-switcher-model)
      - [Adding a Custom Operating Mode](#adding-a-custom-operating-mode)
      - [Customizing Tally Display Behavior](#customizing-tally-display-behavior)
      - [Adding Persistent Configuration Parameters](#adding-persistent-configuration-parameters)
  - [Architecture Overview](#architecture-overview)
    - [Key Principles](#key-principles)
  - [Project Structure](#project-structure)
    - [File Organization Rules](#file-organization-rules)
  - [Design Patterns](#design-patterns)
    - [1. Interface Pattern (Abstract Base Classes)](#1-interface-pattern-abstract-base-classes)
    - [2. Factory Pattern](#2-factory-pattern)
    - [3. Singleton Pattern (Careful Use)](#3-singleton-pattern-careful-use)
    - [4. Observer Pattern (Callbacks)](#4-observer-pattern-callbacks)
    - [5. Strategy Pattern (Mode Handlers)](#5-strategy-pattern-mode-handlers)
  - [Adding Features](#adding-features)
    - [Adding a New Hardware Platform](#adding-a-new-hardware-platform)
    - [Adding Support for a New Roland Video Switcher](#adding-support-for-a-new-roland-video-switcher)
    - [Adding a New Display Type](#adding-a-new-display-type)
    - [Adding a New IMU](#adding-a-new-imu)
    - [Adding a State Machine State](#adding-a-state-machine-state)
  - [Testing](#testing)
    - [Unit Testing (Future)](#unit-testing-future)
    - [Hardware-in-the-Loop Testing](#hardware-in-the-loop-testing)
    - [Integration Testing](#integration-testing)
  - [Code Style](#code-style)
    - [Naming Conventions](#naming-conventions)
    - [File Organization](#file-organization)
    - [Comments](#comments)
    - [Formatting](#formatting)
  - [Debugging](#debugging)
    - [Serial Logging](#serial-logging)
    - [Common Issues](#common-issues)
    - [Using `esp32_exception_decoder`](#using-esp32_exception_decoder)
    - [Hardware Debugging Tools](#hardware-debugging-tools)
  - [Common Tasks](#common-tasks)
    - [Using InfoPrinter for Serial Output](#using-infoprinter-for-serial-output)
    - [Changing Default Settings](#changing-default-settings)
    - [Adding a New Color](#adding-a-new-color)
    - [Changing Tally Color Scheme](#changing-tally-color-scheme)
    - [Adding a Configuration Parameter](#adding-a-configuration-parameter)
    - [Profiling Performance](#profiling-performance)
  - [Contributing](#contributing)
    - [Workflow](#workflow)
    - [Commit Messages](#commit-messages)
    - [Pull Request Checklist](#pull-request-checklist)
  - [Resources](#resources)
    - [ESP32 Documentation](#esp32-documentation)
    - [Libraries](#libraries)
    - [Tools](#tools)
- [STAC Developer Guide](#stac-developer-guide)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Initial Setup](#initial-setup)
    - [Building the Project](#building-the-project)
  - [Quick Start: Key Extension Points](#quick-start-key-extension-points)
    - [Core Application Classes](#core-application-classes)
      - [**STACApp** - Main Application Controller](#stacapp---main-application-controller)
      - [**ConfigManager** - Persistent Configuration Storage](#configmanager---persistent-configuration-storage)
    - [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal)
      - [**IDisplay** - Display Interface](#idisplay---display-interface)
      - [**IIMU** - Inertial Measurement Unit Interface](#iimu---inertial-measurement-unit-interface)
      - [**IRolandClient** - Video Switcher Protocol Interface](#irolandclient---video-switcher-protocol-interface)
      - [**GlyphManager** - Glyph Storage and Rotation](#glyphmanager---glyph-storage-and-rotation)
    - [State Management](#state-management)
      - [**TallyStateManager** - Tally State Tracking](#tallystatemanager---tally-state-tracking)
      - [**OperatingModeManager** - System Mode Management](#operatingmodemanager---system-mode-management)
      - [**SystemState** - Centralized State Container](#systemstate---centralized-state-container)
    - [Network Layer](#network-layer)
      - [**WiFiManager** - WiFi Connection Management](#wifimanager---wifi-connection-management)
      - [**WebConfigServer** - Web-Based Configuration Portal](#webconfigserver---web-based-configuration-portal)
    - [Factory Classes](#factory-classes)
      - [**DisplayFactory**](#displayfactory)
      - [**IMUFactory**](#imufactory)
      - [**RolandClientFactory**](#rolandclientfactory)
    - [Utility Classes](#utility-classes)
      - [**StartupConfig** - Interactive Configuration at Boot](#startupconfig---interactive-configuration-at-boot)
    - [Configuration Files](#configuration-files)
      - [**Device\_Config.h** - User Hardware Selection](#device_configh---user-hardware-selection)
      - [**Board Configs** - Hardware-Specific Settings](#board-configs---hardware-specific-settings)
    - [Common Extension Scenarios](#common-extension-scenarios)
      - [Adding a New Board](#adding-a-new-board)
      - [Adding a New Roland Switcher Model](#adding-a-new-roland-switcher-model)
      - [Adding a Custom Operating Mode](#adding-a-custom-operating-mode)
      - [Customizing Tally Display Behavior](#customizing-tally-display-behavior)
      - [Adding Persistent Configuration Parameters](#adding-persistent-configuration-parameters)
  - [Architecture Overview](#architecture-overview)
    - [Key Principles](#key-principles)
  - [Project Structure](#project-structure)
    - [File Organization Rules](#file-organization-rules)
  - [Design Patterns](#design-patterns)
    - [1. Interface Pattern (Abstract Base Classes)](#1-interface-pattern-abstract-base-classes)
    - [2. Factory Pattern](#2-factory-pattern)
    - [3. Singleton Pattern (Careful Use)](#3-singleton-pattern-careful-use)
    - [4. Observer Pattern (Callbacks)](#4-observer-pattern-callbacks)
    - [5. Strategy Pattern (Mode Handlers)](#5-strategy-pattern-mode-handlers)
  - [Adding Features](#adding-features)
    - [Adding a New Hardware Platform](#adding-a-new-hardware-platform)
    - [Adding Support for a New Roland Video Switcher](#adding-support-for-a-new-roland-video-switcher)
    - [Adding a New Display Type](#adding-a-new-display-type)
    - [Adding a New IMU](#adding-a-new-imu)
    - [Adding a State Machine State](#adding-a-state-machine-state)
  - [Testing](#testing)
    - [Unit Testing (Future)](#unit-testing-future)
    - [Hardware-in-the-Loop Testing](#hardware-in-the-loop-testing)
    - [Integration Testing](#integration-testing)
  - [Code Style](#code-style)
    - [Naming Conventions](#naming-conventions)
    - [File Organization](#file-organization)
    - [Comments](#comments)
    - [Formatting](#formatting)
  - [Debugging](#debugging)
    - [Serial Logging](#serial-logging)
    - [Common Issues](#common-issues)
    - [Using `esp32_exception_decoder`](#using-esp32_exception_decoder)
    - [Hardware Debugging Tools](#hardware-debugging-tools)
  - [Common Tasks](#common-tasks)
    - [Using InfoPrinter for Serial Output](#using-infoprinter-for-serial-output)
    - [Changing Default Settings](#changing-default-settings)
    - [Adding a New Color](#adding-a-new-color)
    - [Changing Tally Color Scheme](#changing-tally-color-scheme)
    - [Adding a Configuration Parameter](#adding-a-configuration-parameter)
    - [Profiling Performance](#profiling-performance)
  - [Contributing](#contributing)
    - [Workflow](#workflow)
    - [Commit Messages](#commit-messages)
    - [Pull Request Checklist](#pull-request-checklist)
  - [Resources](#resources)
    - [ESP32 Documentation](#esp32-documentation)
    - [Libraries](#libraries)
    - [Tools](#tools)


<a name="getting-started"></a>
## Getting Started

<a name="prerequisites"></a>
### Prerequisites

Before working on STAC, ensure you have:

1. **PlatformIO** - Install via VS Code extension or CLI
   - VS Code Extension: Search for "PlatformIO IDE" in Extensions marketplace
   - CLI: `pip install platformio`

2. **Git** - For version control and repository management

3. **Supported Hardware** (optional for development, required for deployment):
   - M5Stack ATOM Matrix (LED matrix display)
   - M5Stack StickC Plus (TFT display with IMU)
   - LilyGo T-Display (TFT display)
   - LilyGo T-QT (small TFT display)
   - Waveshare ESP32-S3 (LED matrix display)
   - AIPI-Lite (custom board with LED matrix)

<a name="initial-setup"></a>
### Initial Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Xylopyrographer/STAC.git
   cd STAC
   ```

2. **Configure your build environment:**
   
   STAC uses `platformio.ini` for build configuration, but this file is gitignored to allow each developer to customize their multi-device setup. A template is provided:
   
   ```bash
   cp platformio.ini.example platformio.ini
   ```

3. **Customize platformio.ini for your hardware:**
   
   The example file includes two devices (ATOM Matrix and T-Display). To enable additional devices:
   
   - Uncomment the `[env:device-name]` section for your board
   - Adjust `default_envs` in the `[platformio]` section if desired
   - See comments in `platformio.ini.example` for device-specific requirements

4. **Verify dependencies:**
   
   PlatformIO will automatically download all required libraries on first build. Core dependencies include:
   - LiteLED (LED matrix control)
   - XP_Button (button handling)
   - Arduino_GFX (TFT display support for some boards)

<a name="building-the-project"></a>
### Building the Project

**Basic Build (Debug):**
```bash
pio run -e atom-matrix
```

**Upload to Device:**
```bash
pio run -e atom-matrix -t upload
```

**Build Release Version:**

Each device has a `-release` variant that optimizes for size and speed:
```bash
pio run -e atom-matrix-release
```

**Generate Merged Binary (for distribution):**

STAC includes a custom PlatformIO target that creates a single flashable binary:
```bash
pio run -e atom-matrix-release -t merged
```

This creates a complete firmware image in `bin/<Device Name>/` that can be flashed with:
```bash
esptool.py write_flash 0x0 bin/ATOM_Matrix/STAC_v3.x.x_ATOM_Matrix_FULL.bin
```

**Monitor Serial Output:**
```bash
pio device monitor -e atom-matrix
```

**Clean Build:**
```bash
pio run -e atom-matrix -t clean
```

**Building for Multiple Devices:**

If you're developing for multiple devices simultaneously, you can:
- Build all configured environments: `pio run`
- Build specific set: `pio run -e atom-matrix -e m5stickc-plus`

**Build System Details:**

- `scripts/build_version.py` - Auto-generates build number before each compile
- `scripts/custom_targets.py` - Registers the `merged` target for binary generation
- `include/build_info.h` - Auto-generated file with version info (gitignored)

**Next Steps:**

- Review [Configuration Files](#configuration-files) to understand board setup
- See [Common Extension Scenarios](#common-extension-scenarios) for adding features
- Check [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal) for interface details


<a name="quick-start-key-extension-points"></a>
## Quick Start: Key Extension Points

This section provides a quick reference to the main classes and methods developers need to understand when extending STAC functionality. All classes listed below have comprehensive Doxygen documentation in their header files.

<a name="core-application-classes"></a>
### Core Application Classes

<a name="stacapp-main-application-controller"></a>
#### **STACApp** - Main Application Controller
**Location:** `include/Application/STACApp.h`, `src/Application/STACApp.cpp`

The central orchestrator that coordinates all subsystems. Key methods for extension:

- **`handleNormalMode()`** - Main tally monitoring loop
  - Polls Roland switcher at configured interval
  - Updates tally state and display
  - Handles button inputs during normal operation

- **`handlePeripheralMode()`** - Peripheral device mode (HDMI/SDI channels)
  - High-speed polling (2ms default)
  - Button-configurable settings (camera mode, brightness)
  - Settings persistence across power cycles

- **`handleProvisioningMode()`** - WiFi provisioning and configuration
  - Unified web portal (setup + OTA updates)
  - Web portal with OS auto-detection
  - Visual feedback with pulsing glyph

- **`handleButton()`, `handleButtonB()`** - Button event handlers
  - Short press: Toggle camera/talent mode (normal mode)
  - Long press: Access startup config (peripheral mode)
  - Boot sequence: Timed holds for portal/factory reset

**Extension Points:**
- Add new operating modes by extending the switch statement in `loop()`
- Customize button behavior in existing mode handlers
- Add new state transitions in mode change handlers

---

<a name="configmanager-persistent-configuration-storage"></a>
#### **ConfigManager** - Persistent Configuration Storage
**Location:** `include/Storage/ConfigManager.h`, `src/Storage/ConfigManager.cpp`

Manages all configuration data in ESP32 Non-Volatile Storage (NVS). Key method groups:

**WiFi Configuration:**
- `saveWiFiCredentials()` / `loadWiFiCredentials()` - Network credentials
- `hasWiFiCredentials()` / `isProvisioned()` - Check if configured
- `clearWiFiCredentials()` - Factory reset WiFi settings

**Switch Configuration:**
- `saveSwitchConfig()` / `loadSwitchConfig()` - Roland switcher settings (IP, port, channel, model, poll interval)
- `hasProtocolConfig()` - Check if switch is configured

**Protocol-Specific:**
- `saveV60HDConfig()` / `loadV60HDConfig()` - V-60HD specific settings
- `saveV160HDConfig()` / `loadV160HDConfig()` - V-160HD specific settings (HDMI/SDI bank)
- `getActiveProtocol()` - Get current switcher model

**Device Identity:**
- `saveStacID()` / `loadStacID()` - Unique device identifier (hex-encoded MAC)
- `savePMode()` / `loadPMode()` - Camera/Talent mode preference

**Extension Points:**
- Add new configuration parameters by following the existing save/load pattern
- Use namespaces to organize related settings (e.g., "wifi", "switch", "v60hd")
- Leverage NVS key-value storage for any persistent data needs

---

<a name="hardware-abstraction-layer-hal"></a>
### Hardware Abstraction Layer (HAL)

<a name="idisplay-display-interface"></a>
#### **IDisplay** - Display Interface
**Location:** `include/Hardware/Display/IDisplay.h`

Abstract interface for all display types (LED matrices and TFT screens). Key methods:

- `begin()` - Initialize display hardware
- `fill(color, show)` - Fill entire display with color
- `setPixel(x, y, color, show)` - Set individual pixel
- `showGlyph(glyphIndex, color, show)` - Display a glyph (0-31)
- `showDigit(digit, color, show)` - Display a numeric digit (0-9)
- `setBrightness(level)` - Adjust display brightness (0-255)
- `clear(show)` - Clear display to black
- `show()` - Update physical display (for buffered displays)

**Implementations:**
- `Display5x5` - 5×5 LED matrix (ATOM Matrix)
- `Display8x8` - 8×8 LED matrix (Waveshare ESP32-S3)
- `DisplayTFT` - TFT LCD displays (M5StickC Plus, Lilygo T-Display, etc.)

**Extension Points:**
- Create new display implementation by inheriting from `IDisplay`
- Add to `DisplayFactory::create()` with appropriate `#if defined()` check
- Define board-specific display settings in `BoardConfigs/*.h`

---

<a name="iimu-inertial-measurement-unit-interface"></a>
#### **IIMU** - Inertial Measurement Unit Interface
**Location:** `include/Hardware/Sensors/IIMU.h`

Abstract interface for IMU sensors (accelerometer/gyroscope). Key methods:

- `begin()` - Initialize IMU hardware
- `getOrientation()` - Get device orientation (ROTATE_0/90/180/270)
- `isAvailable()` - Check if IMU is present
- `getType()` - Get IMU model name

**Implementations:**
- `MPU6886_IMU` - M5Stack IMU (M5Atom, M5StickC)
- `QMI8658_IMU` - Waveshare IMU (ESP32-S3)

**Extension Points:**
- IMU orientation drives glyph rotation via `GlyphManager`
- Add new IMU by inheriting from `IIMU` and adding to `IMUFactory`
- Orientation detection can be customized per IMU model

---

<a name="irolandclient-video-switcher-protocol-interface"></a>
#### **IRolandClient** - Video Switcher Protocol Interface
**Location:** `include/Network/Protocol/IRolandClient.h`

Abstract interface for Roland video switcher communication. Key types and methods:

**Types:**
- `TallyStatus` - ONAIR, PREVIEW, UNSELECTED, ERROR, UNKNOWN
- `RolandConfig` - Switch IP, port, channel, poll interval, model
- `TallyQueryResult` - Connection status, tally state, raw response

**Methods:**
- `begin(config)` - Initialize client with switcher settings
- `queryTallyStatus(result)` - Query current tally state
- `getModelName()` - Get switcher model identifier
- `isInitialized()` - Check if client is configured

**Implementations:**
- `V60HDClient` - Roland V-60HD video switcher
- `V160HDClient` - Roland V-160HD video switcher (HDMI/SDI banks)

**Extension Points:**
- Add new Roland model by inheriting from `IRolandClient`
- Implement model-specific query format and response parsing
- Add to `RolandClientFactory::createFromString()` with model name
- Update web configuration dropdown in `WebConfigServer`

---

<a name="glyphmanager-glyph-storage-and-rotation"></a>
#### **GlyphManager** - Glyph Storage and Rotation
**Location:** `include/Hardware/Display/GlyphManager.h`

Template class managing glyph data with automatic rotation based on device orientation. Key methods:

- `updateOrientation(orientation)` - Rotate all glyphs to new orientation
- `getGlyph(index)` - Get rotated glyph data (0-31)
- `getDigitGlyph(digit)` - Get rotated digit glyph (0-9)
- `getCurrentOrientation()` - Get current rotation setting

**Specialized Types:**
- `GlyphManager5x5` - For 5×5 displays
- `GlyphManager8x8` - For 8×8 displays

**Extension Points:**
- Glyph data defined in board configs: `Glyphs_5x5.h` or `Glyphs_8x8.h`
- Add custom glyphs by extending glyph arrays in board config
- Rotation lookup tables (`RotationLUT_*`) can be customized

---

<a name="state-management"></a>
### State Management

<a name="tallystatemanager-tally-state-tracking"></a>
#### **TallyStateManager** - Tally State Tracking
**Location:** `include/State/TallyStateManager.h`

Manages current tally state with change notifications. Key methods:

- `setState(newState)` - Transition to new tally state
- `getCurrentState()` - Get current TallyState (ONAIR, PREVIEW, etc.)
- `getPreviousState()` - Get previous state
- `getTimeSinceChange()` - Milliseconds since last state change
- `getStateColor()` - Get display color for current state
- `getStateString()` - Get human-readable state name

**Extension Points:**
- State change callbacks allow coordinated updates across subsystems
- Add custom state transitions or validation logic
- State colors defined in `include/Hardware/Display/Colors.h`

---

<a name="operatingmodemanager-system-mode-management"></a>
#### **OperatingModeManager** - System Mode Management
**Location:** `include/State/OperatingModeManager.h`

Manages system operating mode (NORMAL, PERIPHERAL, PROVISIONING). Key methods:

- `setMode(newMode)` - Change operating mode
- `getCurrentMode()` - Get current OperatingMode
- `getPreviousMode()` - Get previous mode
- `getModeString()` - Get human-readable mode name

**Extension Points:**
- Add new operating modes in `include/Config/Types.h`
- Create corresponding handler methods in `STACApp`
- Add mode transitions in button handlers or startup logic

---

<a name="systemstate-centralized-state-container"></a>
#### **SystemState** - Centralized State Container
**Location:** `include/State/SystemState.h`

Aggregates all state managers into a single coordinated system. Key components:

- `TallyStateManager& getTallyState()` - Access tally state
- `OperatingModeManager& getOperatingMode()` - Access operating mode

**Extension Points:**
- Add new state managers to SystemState as needed
- Use as central coordination point for state-dependent logic
- All state managers support change callbacks for reactive updates

---

<a name="network-layer"></a>
### Network Layer

<a name="wifimanager-wifi-connection-management"></a>
#### **WiFiManager** - WiFi Connection Management
**Location:** `include/Network/WiFiManager.h`

Handles WiFi station and access point modes. Key methods:

- `connect(ssid, password, timeout)` - Connect to WiFi network
- `startAP(ssid, password)` - Start access point mode
- `disconnect()` / `stopAP()` - Disconnect from network or stop AP
- `isConnected()` / `isAPMode()` - Check connection status
- `getLocalIP()` / `getMacAddress()` / `getRSSI()` - Network info
- `setHostname(hostname)` - Set mDNS hostname (e.g., "stac.local")

**Extension Points:**
- State callbacks notify on connection changes
- Customize AP settings (SSID format, password requirements)
- Add WiFi scanning or multi-network support

---

<a name="webconfigserver-web-based-configuration-portal"></a>
#### **WebConfigServer** - Web-Based Configuration Portal
**Location:** `include/Network/WebConfigServer.h`

Unified web portal for provisioning and OTA updates with OS-level auto-detection support. Key methods:

**Server Control:**
- `begin()` - Start web server, DNS server, and mDNS
- `end()` - Stop all services
- `handleClient()` - Process web requests (call in loop)

**Auto-Detection:**
- OS-level detection may trigger browser popup
- DNS server redirects all requests to portal
- Platform-specific detection endpoints

**Results:**
- `waitForCompletion()` - Blocking wait for user action
- `PortalResult` - CONFIG_RECEIVED, OTA_SUCCESS, OTA_FAILED, or FACTORY_RESET
- `ProvisioningData` - WiFi, switch settings, and device configuration

**Extension Points:**
- Add new configuration fields to HTML form
- Customize visual appearance (CSS in HTML template)
- Add new endpoints for additional functionality
- Modify OTA handling for custom update logic

---

<a name="factory-classes"></a>
### Factory Classes

Factory classes provide compile-time hardware selection without runtime overhead:

<a name="displayfactory"></a>
#### **DisplayFactory**
**Location:** `include/Hardware/Display/DisplayFactory.h`

- `create()` - Returns appropriate display implementation based on board config
- `getDisplayType()` - Returns display type as string

<a name="imufactory"></a>
#### **IMUFactory**
**Location:** `include/Hardware/Sensors/IMUFactory.h`

- `create()` - Returns appropriate IMU implementation based on board config
- `getIMUType()` - Returns IMU type as string

<a name="rolandclientfactory"></a>
#### **RolandClientFactory**
**Location:** `include/Network/Protocol/RolandClientFactory.h`

- `createFromString(model)` - Creates Roland client for specified model ("V-60HD", "V-160HD", etc.)
- `create(protocol)` - Creates Roland client from ProtocolType enum

**Extension Pattern:**
All factories follow the same pattern:
1. Add new implementation class
2. Include header with `#elif defined(...)`
3. Add to factory's create method
4. Update board config or runtime selection

---

<a name="utility-classes"></a>
### Utility Classes

<a name="startupconfig-interactive-configuration-at-boot"></a>
#### **StartupConfig** - Interactive Configuration at Boot
**Location:** `include/Application/StartupConfig.h`

Provides interactive configuration during peripheral mode startup. Key methods:

- `run(display, config, currentPMode, currentBrightness)` - Interactive config loop
- Allows user to select channel, brightness, and camera/talent mode
- Visual feedback with number glyphs and brightness preview
- Timeout returns to normal operation

**Extension Points:**
- Add new configuration parameters to startup sequence
- Customize visual feedback or timeout behavior
- Share configuration UI elements with normal mode settings

---

<a name="configuration-files"></a>
### Configuration Files

<a name="device_configh-user-hardware-selection"></a>
#### **Device_Config.h** - User Hardware Selection
**Location:** `include/Device_Config.h`

Single file to select target board. Include one board config:
```cpp
#include "BoardConfigs/AtomMatrix_Config.h"
// or
#include "BoardConfigs/M5StickCPlus_Config.h"
// etc.
```

<a name="board-configs-hardware-specific-settings"></a>
#### **Board Configs** - Hardware-Specific Settings
**Location:** `include/BoardConfigs/*.h`

Each board config defines:
- **Pin assignments** - GPIO pins for display, buttons, I2C, etc.
- **Display configuration** - Type (LED/TFT), size, color order
- **IMU settings** - Sensor type, I2C address
- **Power management** - Battery, charging, power button
- **Brightness maps** - Adjustment curves for different hardware
- **Glyph definitions** - Display-specific glyph arrays

**Common Board Configs:**
- `AtomMatrix_Config.h` - M5Stack ATOM Matrix (5×5 LED)
- `WaveshareS3_Config.h` - Waveshare ESP32-S3 (8×8 LED)
- `M5StickCPlus_Config.h` - M5StickC Plus (TFT LCD)
- `LilygoTDisplay_Config.h` - Lilygo T-Display (TFT LCD)
- `LilygoTQT_Config.h` - T-QT ESP32-S3 (TFT LCD)
- `AIPI_Lite_Config.h` - AIPI-Lite (TFT LCD)

---

<a name="common-extension-scenarios"></a>
### Common Extension Scenarios

<a name="adding-a-new-board"></a>
#### Adding a New Board
1. Create new board config in `BoardConfigs/`
2. Define all pins, display type, IMU type
3. Include in `Device_Config.h`
4. Test all features (display, buttons, WiFi, IMU)
5. See [HARDWARE_CONFIG.md](HARDWARE_CONFIG.md) for details

<a name="adding-a-new-roland-switcher-model"></a>
#### Adding a New Roland Switcher Model
1. Create client class inheriting from `IRolandClient`
2. Implement protocol-specific query and response parsing
3. Add to `RolandClientFactory`
4. Update web configuration form
5. See detailed example in [Adding Features](#adding-features) section

<a name="adding-a-custom-operating-mode"></a>
#### Adding a Custom Operating Mode
1. Add enum value to `OperatingMode` in `Types.h`
2. Create handler method in `STACApp` (e.g., `handleCustomMode()`)
3. Add case to `STACApp::loop()` switch statement
4. Trigger mode change from button handler or other event

<a name="customizing-tally-display-behavior"></a>
#### Customizing Tally Display Behavior
1. Modify `handleNormalMode()` in `STACApp.cpp`
2. Use `tallyStateManager->setState()` to update state
3. Display automatically updates via state color mapping
4. Add custom glyphs or effects in display update logic

<a name="adding-persistent-configuration-parameters"></a>
#### Adding Persistent Configuration Parameters
1. Add save/load methods to `ConfigManager` following existing pattern
2. Use NVS namespace for organization
3. Add fields to web configuration form
4. Update `ProvisioningData` struct if needed

---

<a name="architecture-overview"></a>
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

<a name="key-principles"></a>
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

<a name="project-structure"></a>
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

<a name="file-organization-rules"></a>
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

<a name="design-patterns"></a>
## Design Patterns

<a name="1-interface-pattern-abstract-base-classes"></a>
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

<a name="2-factory-pattern"></a>
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

<a name="3-singleton-pattern-careful-use"></a>
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

<a name="4-observer-pattern-callbacks"></a>
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

<a name="5-strategy-pattern-mode-handlers"></a>
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

<a name="adding-features"></a>
## Adding Features

<a name="adding-a-new-hardware-platform"></a>
### Adding a New Hardware Platform

See [HARDWARE_CONFIG.md](HARDWARE_CONFIG.md) for detailed steps.

**Quick summary:**

1. Create `BoardConfigs/YourBoard_Config.h`
2. Define all pins and settings
3. Add to `Device_Config.h`
4. Test thoroughly
5. Submit PR with documentation

<a name="adding-support-for-a-new-roland-video-switcher"></a>
### Adding Support for a New Roland Video Switcher

**Example: Adding Roland V-600UHD support**

STAC uses a factory pattern to support multiple Roland video switcher models. Each model has its own client implementation that handles model-specific protocol details.

**Step 1:** Create the client class

```cpp
// include/Network/Protocol/V600UHDClient.h
#ifndef STAC_V600UHD_CLIENT_H
#define STAC_V600UHD_CLIENT_H

#include "RolandClient.h"

namespace Net {

class V600UHDClient : public RolandClient {
public:
    V600UHDClient() = default;
    ~V600UHDClient() override = default;

    bool begin(const RolandConfig& config) override;
    void queryTallyStatus(TallyQueryResult& result) override;
    
    const char* getModelName() const override { return "V-600UHD"; }

private:
    // Model-specific helper methods
    bool connectToSwitch();
    String buildTallyQuery();
    TallyStatus parseTallyResponse(const String& response);
};

} // namespace Net

#endif
```

**Step 2:** Implement the protocol

```cpp
// src/Network/Protocol/V600UHDClient.cpp
#include "Network/Protocol/V600UHDClient.h"
#include <Arduino.h>

namespace Net {

bool V600UHDClient::begin(const RolandConfig& config) {
    this->config = config;
    initialized = true;
    
    log_i("V600UHDClient initialized for %s:%d, channel %d",
          config.switchIP.toString().c_str(),
          config.switchPort,
          config.tallyChannel);
    
    return true;
}

void V600UHDClient::queryTallyStatus(TallyQueryResult& result) {
    // Reset result
    result.connected = false;
    result.gotReply = false;
    result.timedOut = false;
    result.status = TallyStatus::UNKNOWN;
    result.rawResponse = "";

    // Connect to switch
    if (!connectToSwitch()) {
        result.timedOut = true;
        return;
    }
    
    result.connected = true;

    // Build and send query (model-specific format)
    String query = buildTallyQuery();
    client.print(query);
    
    // Wait for response with timeout
    unsigned long startTime = millis();
    while (!client.available() && (millis() - startTime) < 1000) {
        yield();
    }
    
    if (!client.available()) {
        result.timedOut = true;
        client.stop();
        return;
    }
    
    // Read response
    String response = client.readStringUntil('\n');
    result.rawResponse = response;
    result.gotReply = true;
    
    // Parse tally status (model-specific parsing)
    result.status = parseTallyResponse(response);
    
    client.stop();
}

bool V600UHDClient::connectToSwitch() {
    if (client.connected()) {
        return true;
    }
    
    return client.connect(config.switchIP, config.switchPort);
}

String V600UHDClient::buildTallyQuery() {
    // V-600UHD specific query format
    // Example: "QST:pgm ch1\r\n" for program bus query
    char query[64];
    snprintf(query, sizeof(query), "QST:pgm ch%d\r\n", config.tallyChannel);
    return String(query);
}

TallyStatus V600UHDClient::parseTallyResponse(const String& response) {
    // V-600UHD specific response parsing
    // Example responses:
    // "ON\r\n" = channel is on program
    // "OFF\r\n" = channel is not on program
    
    String trimmed = response;
    trimmed.trim();
    
    if (trimmed == "ON") {
        return TallyStatus::ONAIR;
    } else if (trimmed == "OFF") {
        // Need to check preview bus too
        // ... additional queries for preview ...
        return TallyStatus::UNSELECTED;
    }
    
    return TallyStatus::UNKNOWN;
}

} // namespace Net
```

**Step 3:** Add to factory

```cpp
// include/Network/Protocol/RolandClientFactory.h
static std::unique_ptr<RolandClient> createFromString(const String& model) {
    if (model == "V-60HD") {
        return std::make_unique<V60HDClient>();
    } else if (model == "V-160HD") {
        return std::make_unique<V160HDClient>();
    } else if (model == "V-600UHD") {  // NEW
        return std::make_unique<V600UHDClient>();
    }
    
    log_e("Unknown Roland model: %s", model.c_str());
    return nullptr;
}
```

**Step 4:** Update web configuration

```cpp
// src/Network/Web/WebConfigServer.cpp
// Add to switch model dropdown options
const char HTML_SWITCH_OPTIONS[] PROGMEM = R"rawliteral(
<option value="V-60HD">Roland V-60HD</option>
<option value="V-160HD">Roland V-160HD</option>
<option value="V-600UHD">Roland V-600UHD</option>
)rawliteral";
```

**Step 5:** Test with real hardware

```cpp
// Test checklist:
// [ ] Connection to switch succeeds
// [ ] Tally queries return correct status
// [ ] Program state shows red
// [ ] Preview state shows green  
// [ ] Unselected state correct (camera/talent modes)
// [ ] Error handling (disconnect, junk replies)
// [ ] Poll interval respected
// [ ] Fast re-poll on errors
```

**Protocol Documentation Tips:**

- Consult switcher's technical manual for protocol details
- Use terminal emulator (like CoolTerm) to test queries manually
- Log all raw responses during development
- Test edge cases (disconnection, invalid channels, etc.)
- Consider multiple query model (program + preview + aux buses)

<a name="adding-a-new-display-type"></a>
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

<a name="adding-a-new-imu"></a>
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

<a name="adding-a-state-machine-state"></a>
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

<a name="testing"></a>
## Testing

<a name="unit-testing-future"></a>
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

<a name="hardware-in-the-loop-testing"></a>
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

<a name="integration-testing"></a>
### Integration Testing

**Two-device test:**

1. Device A in Normal mode
2. Device B in Peripheral mode
3. Connect GROVE ports
4. Button press on A should update display on B

**Expected behavior:**

- `NO_TALLY → PREVIEW → PROGRAM → UNSELECTED → NO_TALLY`
- B mirrors A's display
- <2ms latency

---

<a name="code-style"></a>
## Code Style

<a name="naming-conventions"></a>
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

<a name="file-organization"></a>
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

<a name="comments"></a>
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

<a name="formatting"></a>
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

<a name="debugging"></a>
## Debugging

<a name="serial-logging"></a>
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

<a name="common-issues"></a>
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

**Problem:** Slow startup or mode switching (Waveshare S3)

- **Check:** Unnecessary polling loops in sensor wrappers
- **Compare:** Reference implementation (e.g., MPU6886 vs QMI8658)
- **Solution:** Use direct sensor library calls without added delays

**Problem:** Display wrong colors

- **Fix:** Toggle `DISPLAY_COLOR_ORDER_RGB` ↔ `DISPLAY_COLOR_ORDER_GRB`
- **Test:** Use `display->fill(StandardColors::RED, true)` to verify

<a name="using-esp32_exception_decoder"></a>
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

<a name="hardware-debugging-tools"></a>
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

<a name="common-tasks"></a>
## Common Tasks

<a name="using-infoprinter-for-serial-output"></a>
### Using InfoPrinter for Serial Output

STAC v3.0 includes `InfoPrinter` utility class for consistent, formatted serial output:

```cpp
#include "Utils/InfoPrinter.h"

// Print startup header (called in setup)
Utils::InfoPrinter::printHeader(stacID);

// Print WiFi connected status
Utils::InfoPrinter::printWiFiConnected();

// Print configuration summary
Utils::InfoPrinter::printFooter(ops, switchIP, switchPort, ssid);

// Print peripheral mode status
Utils::InfoPrinter::printPeripheral(cameraMode, brightnessLevel);

// Print OTA mode notification
Utils::InfoPrinter::printOTA();

// Print OTA result
Utils::InfoPrinter::printOTAResult(success, filename, bytesWritten, message);

// Print factory reset notification
Utils::InfoPrinter::printReset();

// Print configuration complete
Utils::InfoPrinter::printConfigDone();
```

**Benefits:**
- Consistent formatting across all serial output
- ASCII art headers for visual separation
- Easy to read status information
- Centralized changes (update once, applies everywhere)

<a name="changing-default-settings"></a>
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

<a name="adding-a-new-color"></a>
### Adding a New Color

```cpp
// include/Hardware/Display/Colors.h
namespace StandardColors {
    // ... existing colors ...
    constexpr color_t PINK = makeRGB(255, 192, 203);  // NEW
}
```

<a name="changing-tally-color-scheme"></a>
### Changing Tally Color Scheme

```cpp
// include/Hardware/Display/Colors.h
namespace STACColors {
    constexpr color_t PROGRAM = StandardColors::YELLOW;  // Was RED
    constexpr color_t PREVIEW = StandardColors::BLUE;    // Was GREEN
    // ...
}
```

<a name="adding-a-configuration-parameter"></a>
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

<a name="profiling-performance"></a>
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

<a name="contributing"></a>
## Contributing

<a name="workflow"></a>
### Workflow

1. **Fork** the repository
2. **Create branch** from `dev`: `feature/my-new-feature`
3. **Make changes** and test thoroughly
4. **Commit** with clear messages
5. **Push** to your fork
6. **Create PR** to `dev` branch

<a name="commit-messages"></a>
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

<a name="pull-request-checklist"></a>
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

<a name="resources"></a>
## Resources

<a name="esp32-documentation"></a>
### ESP32 Documentation
- [ESP32 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [PlatformIO ESP32](https://docs.platformio.org/en/latest/platforms/espressif32.html)

<a name="libraries"></a>
### Libraries
- [LiteLED Documentation](https://github.com/Xylopyrographer/LiteLED)
- [XP_Button Documentation](https://github.com/Xylopyrographer/XP_Button)

<a name="tools"></a>
### Tools
- [PlatformIO IDE](https://platformio.org/platformio-ide)
- [Fritzing](https://fritzing.org/) - Circuit diagrams
- [Doxygen](https://www.doxygen.nl/) - Documentation generator

---

**Happy coding!** 

**Last Updated:** 2026-01-31  
**Version:** 3.0.0-RC.25


<!-- EOF -->
