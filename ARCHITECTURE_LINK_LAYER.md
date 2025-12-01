# Link Layer Abstraction Architecture

**Branch:** `architecture/link-layer-abstraction`  
**Based on:** v3.0.0-RC.9 (`v3_RC` branch)  
**Status:** Future design - not for current release  
**Created:** November 21, 2025  
**Reviewed:** December 1, 2025

---

## Review Summary (December 2025)

### Recommendation: Staged Implementation

This architecture is **sound and forward-thinking**, but should be implemented in phases rather than all at once. The v3.0 release should proceed without these changes.

**For v3.0 Release (Current):**
- Do NOT implement the full link layer abstraction
- v3.0 is feature-complete with working TFT display support (AIPI-Lite, LilyGo T-Display)
- Adding abstraction layer risks destabilizing tested code
- User testing needed before more architectural changes

**For v3.1/v3.2 (Near-term incremental improvements):**
1. Dependency-inject WiFiClient into protocols (instead of internal creation)
2. Create `PeripheralClient : IRolandClient` to unify peripheral mode
3. Peripheral mode becomes just another "switch type"

**For v4.0 (Full implementation):**
- Implement complete `ILinkDriver` abstraction
- Add non-WiFi transports (Bluetooth, Ethernet) if needed
- Add non-Roland protocols (ATEM, VMix) if needed

### Key Refinements Suggested

1. **Simplify ILinkDriver interface** - proposed 15+ methods may be excessive:
   ```cpp
   class ILinkDriver {
   public:
       virtual bool connect() = 0;
       virtual void disconnect() = 0;
       virtual bool isConnected() const = 0;
       virtual int send(const String& data) = 0;
       virtual String receive(uint32_t timeoutMs) = 0;
   };
   ```

2. **Consider Arduino Stream class** - already a link abstraction that WiFiClient, Serial, etc. inherit from

3. **Simplify GPIO Peripheral Mode** - may not need full link driver, just direct GPIO reads

4. **Memory considerations** - profile virtual table and unique_ptr overhead on ESP32

---

## Purpose

This document captures the architectural vision for proper separation of concerns between:
1. **Link Layer** - Transport mechanisms (WiFi, Bluetooth, Ethernet, I2C, GPIO)
2. **Protocol Layer** - Tally query protocols (V-60HD, V-160HD, ATEM, BMD, etc.)
3. **Display Layer** - Output devices (LED Matrix, LCD, OLED, headless)

This separation will enable:
- Protocol reuse across different link types
- Easy addition of new protocols without changing link code
- Easy addition of new displays without changing protocol code
- Better testability with mock implementations
- Support for future hardware (LCD screens, different network types)

---

## Current v3.0 Architecture (RC.9)

### What Works Well ✅

**Display Layer:**
```
IDisplay (interface)
  ├─ Display5x5 (M5Stack ATOM Matrix - 5×5 LED)
  ├─ Display8x8 (Waveshare ESP32-S3 - 8×8 LED)
  ├─ DisplayTFT (TFT LCD displays via LovyanGFX)
  │     ├─ M5StickC Plus (135×240 ST7789)
  │     ├─ LilyGo T-Display (135×240 ST7789)
  │     └─ AIPI-Lite (128×128 ST7735S) ✅ Tested Nov 2025
  └─ DisplayFactory (creation)
```
- Clean hardware abstraction
- Easy to add new display types (proven with AIPI-Lite)
- Factory pattern for instantiation
- Board-specific configuration via `*_Config.h` files

**Protocol Layer:**
```
IRolandClient (interface)
  ├─ RolandClientBase (shared implementation)
  ├─ V60HDClient (WiFiClient-based, raw TCP)
  ├─ V160HDClient (HTTPClient-based, HTTP + auth)
  └─ RolandClientFactory (creation)
```
- Protocol-agnostic interface
- Standard `TallyQueryResult` return type
- Factory pattern for instantiation

**IMU Layer:**
```
IIMU (interface)
  ├─ MPU6886_IMU (M5Stack devices)
  ├─ QMI8658_IMU (Waveshare ESP32-S3)
  ├─ NullIMU (boards without IMU)
  └─ IMUFactory (creation)
```
- Clean hardware abstraction
- NullIMU pattern for IMU-less boards

### What Needs Improvement ⚠️

**Link Layer - Currently Entangled:**
```
WiFiManager (WiFi-specific)
  └─ Used directly by application layer
  └─ Protocols create their own WiFiClient/HTTPClient
  └─ No abstraction for other link types
```

**Application Layer - Knows Too Much:**
```cpp
// STACApp.cpp - WiFi-specific checks
if ( !wifiManager->isConnected() ) {
    return;  // Application knows about WiFi
}
```

**Protocol Layer - Link Coupled:**
```cpp
// V60HDClient.cpp - Creates own WiFiClient
WiFiClient client;  // Protocol knows about WiFi

// V160HDClient.cpp - Creates own HTTPClient
HTTPClient httpClient;  // Protocol knows about HTTP/WiFi
```

---

## Proposed Architecture

### Layer 1: Link Driver Interface

**Purpose:** Abstract transport mechanism from protocols

```cpp
namespace Net {

    enum class LinkType {
        WIFI,
        BLUETOOTH,
        ETHERNET,
        I2C,
        GPIO,
        SERIAL
    };

    enum class LinkState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        ERROR,
        TIMEOUT
    };

    struct LinkConfig {
        // Base config - specific implementations add fields
        String name;
        uint32_t timeout_ms;
    };

    struct LinkStatus {
        LinkState state;
        bool connected;
        uint32_t uptime_ms;
        uint32_t lastError;
        String errorMessage;
    };

    class ILinkDriver {
    public:
        virtual ~ILinkDriver() = default;

        // Connection management
        virtual bool begin(const LinkConfig& config) = 0;
        virtual bool connect() = 0;
        virtual void disconnect() = 0;
        virtual void end() = 0;

        // State queries
        virtual bool isConnected() const = 0;
        virtual LinkState getState() const = 0;
        virtual LinkStatus getStatus() const = 0;
        virtual LinkType getType() const = 0;

        // Data transfer (protocol-agnostic)
        virtual int send(const uint8_t* data, size_t length) = 0;
        virtual int send(const String& data) = 0;
        virtual int receive(uint8_t* buffer, size_t maxLength) = 0;
        virtual String receiveString(size_t maxLength = 256) = 0;

        // Stream-like operations
        virtual bool available() const = 0;
        virtual int read() = 0;
        virtual void flush() = 0;

        // Configuration
        virtual void setTimeout(uint32_t timeout_ms) = 0;
    };

} // namespace Net
```

### Layer 2: Link Driver Implementations

**WiFi Link Driver:**
```cpp
namespace Net {

    struct WiFiLinkConfig : public LinkConfig {
        String ssid;
        String password;
        String hostname;
        IPAddress targetIP;      // For client connections
        uint16_t targetPort;     // For client connections
        bool useHTTP;            // true for HTTPClient, false for WiFiClient
        String httpBasicAuthUser;
        String httpBasicAuthPass;
    };

    class WiFiLinkDriver : public ILinkDriver {
    private:
        WiFiLinkConfig config;
        WiFiClient* rawClient;      // For V-60HD raw TCP
        HTTPClient* httpClient;     // For V-160HD HTTP
        LinkState currentState;
        bool connected;

    public:
        WiFiLinkDriver();
        ~WiFiLinkDriver() override;

        // ILinkDriver interface
        bool begin(const LinkConfig& cfg) override;
        bool connect() override;
        void disconnect() override;
        void end() override;

        bool isConnected() const override;
        LinkState getState() const override;
        LinkStatus getStatus() const override;
        LinkType getType() const override { return LinkType::WIFI; }

        int send(const uint8_t* data, size_t length) override;
        int send(const String& data) override;
        int receive(uint8_t* buffer, size_t maxLength) override;
        String receiveString(size_t maxLength = 256) override;

        bool available() const override;
        int read() override;
        void flush() override;
        void setTimeout(uint32_t timeout_ms) override;

        // WiFi-specific methods
        void setCredentials(const String& ssid, const String& password);
        void setHostname(const String& hostname);
        IPAddress getLocalIP() const;
        int getRSSI() const;
    };

} // namespace Net
```

**GPIO Link Driver (for Peripheral Mode):**
```cpp
namespace Net {

    struct GPIOLinkConfig : public LinkConfig {
        uint8_t pinProgram;   // Pin for PROGRAM bit
        uint8_t pinPreview;   // Pin for PREVIEW bit
        bool pullupEnabled;
    };

    class GPIOLinkDriver : public ILinkDriver {
    private:
        GPIOLinkConfig config;
        
    public:
        // ILinkDriver interface implementation
        // Reads 2-bit GPIO state, interprets as tally status
        
        // GPIO-specific
        uint8_t readRawState();  // Returns 2-bit value
    };

} // namespace Net
```

### Layer 3: Updated Protocol Layer

**Protocol Interface:**
```cpp
namespace Net {

    class ITallyProtocol {
    protected:
        ILinkDriver* linkDriver;  // Injected, not owned
        
    public:
        virtual ~ITallyProtocol() = default;

        // Initialize with link driver
        virtual bool begin(ILinkDriver* link) = 0;
        virtual void end() = 0;

        // Query tally status (uses injected link)
        virtual bool queryTallyStatus(TallyQueryResult& result) = 0;

        // Protocol info
        virtual String getProtocolName() const = 0;
        virtual bool isInitialized() const = 0;
    };

} // namespace Net
```

**V-60HD Protocol (refactored):**
```cpp
namespace Net {

    class V60HDProtocol : public ITallyProtocol {
    private:
        ILinkDriver* link;
        RolandConfig config;
        bool initialized;

    public:
        V60HDProtocol();
        ~V60HDProtocol() override;

        bool begin(ILinkDriver* linkDriver) override {
            this->link = linkDriver;
            initialized = (link != nullptr);
            return initialized;
        }

        bool queryTallyStatus(TallyQueryResult& result) override {
            if (!link || !link->isConnected()) {
                result.status = TallyStatus::NO_CONNECTION;
                return false;
            }

            // Send request via link driver (not WiFiClient directly)
            String request = "GET /tally/" + String(config.tallyChannel) + "/status\r\n\r\n";
            link->send(request);

            // Read response via link driver
            String response = link->receiveString();
            
            // Parse response (protocol-specific logic)
            result.status = parseResponse(response);
            return true;
        }

        String getProtocolName() const override { return "V-60HD"; }
        
    private:
        TallyStatus parseResponse(const String& response);
    };

} // namespace Net
```

**Peripheral Protocol (new):**
```cpp
namespace Net {

    class PeripheralProtocol : public ITallyProtocol {
    private:
        ILinkDriver* link;  // GPIOLinkDriver expected
        
    public:
        bool queryTallyStatus(TallyQueryResult& result) override {
            // Read 2-bit GPIO state via link driver
            GPIOLinkDriver* gpioLink = static_cast<GPIOLinkDriver*>(link);
            uint8_t state = gpioLink->readRawState();
            
            // Decode 2-bit tally state
            // 00 = UNSELECTED, 01 = PREVIEW, 10 = PROGRAM, 11 = ERROR
            switch (state) {
                case 0b00: result.status = TallyStatus::UNSELECTED; break;
                case 0b01: result.status = TallyStatus::SELECTED; break;
                case 0b10: result.status = TallyStatus::ONAIR; break;
                case 0b11: result.status = TallyStatus::ERROR; break;
            }
            
            result.connected = true;
            result.gotReply = true;
            return true;
        }

        String getProtocolName() const override { return "Peripheral-2bit"; }
    };

} // namespace Net
```

### Layer 4: Updated Application Layer

**STACApp with Link Abstraction:**
```cpp
namespace Application {

    class STACApp {
    private:
        // Hardware
        std::unique_ptr<Display::IDisplay> display;
        
        // Network/Link (abstracted)
        std::unique_ptr<Net::ILinkDriver> linkDriver;
        std::unique_ptr<Net::ITallyProtocol> tallyProtocol;
        
        // State
        std::unique_ptr<State::SystemState> systemState;

    public:
        void handleNormalMode() {
            // Generic link check (no WiFi knowledge)
            if (!linkDriver->isConnected()) {
                // Try to reconnect
                linkDriver->connect();
                return;
            }

            // Poll tally status (protocol-agnostic)
            Net::TallyQueryResult result;
            if (tallyProtocol->queryTallyStatus(result)) {
                updateTallyState(result);
            }
        }

        bool initializeLinkAndProtocol() {
            // Load configuration
            String switchModel;
            // ... load config ...

            // Create appropriate link driver
            if (switchModel == "V-60HD" || switchModel == "V-160HD") {
                auto wifiLink = std::make_unique<Net::WiFiLinkDriver>();
                Net::WiFiLinkConfig cfg;
                cfg.ssid = ssid;
                cfg.password = password;
                cfg.targetIP = switchIP;
                cfg.targetPort = switchPort;
                cfg.useHTTP = (switchModel == "V-160HD");
                
                wifiLink->begin(cfg);
                linkDriver = std::move(wifiLink);
            }
            else if (systemState->getOperatingMode().isPeripheralMode()) {
                auto gpioLink = std::make_unique<Net::GPIOLinkDriver>();
                // ... configure GPIO pins ...
                linkDriver = std::move(gpioLink);
            }

            // Create appropriate protocol
            if (switchModel == "V-60HD") {
                auto protocol = std::make_unique<Net::V60HDProtocol>();
                protocol->begin(linkDriver.get());
                tallyProtocol = std::move(protocol);
            }
            else if (switchModel == "V-160HD") {
                auto protocol = std::make_unique<Net::V160HDProtocol>();
                protocol->begin(linkDriver.get());
                tallyProtocol = std::move(protocol);
            }
            else if (systemState->getOperatingMode().isPeripheralMode()) {
                auto protocol = std::make_unique<Net::PeripheralProtocol>();
                protocol->begin(linkDriver.get());
                tallyProtocol = std::move(protocol);
            }

            return true;
        }
    };

} // namespace Application
```

---

## Benefits of This Architecture

### 1. Protocol Reuse Across Links
Same V-60HD protocol could work over:
- WiFi (current)
- Ethernet (future)
- Bluetooth (future)
- Even serial/RS-232 for legacy equipment

### 2. Easy Addition of New Protocols
Want to add ATEM support?
```cpp
class ATEMProtocol : public ITallyProtocol {
    // Just implement the protocol
    // Link driver handles transport
};
```

### 3. Peripheral Mode Becomes Normal
No more special case! Peripheral mode is just:
- `GPIOLinkDriver` (transport)
- `PeripheralProtocol` (2-bit decode)

### 4. Display Independence
Already have this in v3.0! Add LCD easily:
```cpp
class LCDDisplayDriver : public IDisplay {
    void showTallyState(TallyState state) {
        lcd.setCursor(0, 0);
        lcd.print("Tally: ");
        lcd.print(stateToString(state));
    }
};
```

### 5. Testability
Mock implementations for testing:
```cpp
class MockLinkDriver : public ILinkDriver {
    // Simulate network conditions
    // Inject errors, timeouts, etc.
};
```

### 6. Configuration Flexibility
Different configs for different links:
- WiFi: SSID, password, IP, port
- Bluetooth: device MAC, pairing
- Ethernet: static IP, DHCP
- GPIO: pin numbers, pull-ups

---

## Migration Path from v3.0

### Phase 1: Create Link Driver Interface
- Define `ILinkDriver` interface
- Implement `WiFiLinkDriver` wrapping current WiFi code
- No functional changes, just reorganization

### Phase 2: Inject Link into Protocols
- Update `RolandClient` base to accept `ILinkDriver*`
- Refactor `V60HDClient` and `V160HDClient` to use injected link
- Remove direct `WiFiClient`/`HTTPClient` creation from protocols

### Phase 3: Update Application Layer
- Replace `WiFiManager` with `ILinkDriver`
- Update `STACApp` to create link driver and inject into protocol
- Remove WiFi-specific checks

### Phase 4: Add New Link Types
- Implement `GPIOLinkDriver` for peripheral mode
- Implement `PeripheralProtocol`
- Unify peripheral mode with normal mode architecture

### Phase 5: Prove Extensibility
- Add `LCDDisplayDriver` as proof-of-concept
- Maybe add `BluetoothLinkDriver` for future wireless option

---

## File Organization

Proposed structure:
```
include/Network/
├── Link/
│   ├── ILinkDriver.h          (interface)
│   ├── WiFiLinkDriver.h       (WiFi implementation)
│   ├── GPIOLinkDriver.h       (GPIO implementation)
│   ├── BluetoothLinkDriver.h  (future)
│   └── LinkFactory.h          (factory)
├── Protocol/
│   ├── ITallyProtocol.h       (interface)
│   ├── V60HDProtocol.h        (refactored)
│   ├── V160HDProtocol.h       (refactored)
│   ├── PeripheralProtocol.h   (new)
│   ├── ATEMProtocol.h         (future)
│   └── ProtocolFactory.h      (factory)
└── Web/
    ├── WebConfigServer.h      (unchanged)
    └── OTAUpdateServer.h      (unchanged)

src/Network/
├── Link/
│   ├── WiFiLinkDriver.cpp
│   ├── GPIOLinkDriver.cpp
│   └── LinkFactory.cpp
├── Protocol/
│   ├── V60HDProtocol.cpp
│   ├── V160HDProtocol.cpp
│   ├── PeripheralProtocol.cpp
│   └── ProtocolFactory.cpp
└── Web/
    ├── WebConfigServer.cpp
    └── OTAUpdateServer.cpp
```

---

## Compatibility Notes

### Breaking Changes
- None for end users (same functionality)
- API changes for developers extending the codebase
- Configuration file format unchanged

### Backward Compatibility
- All v3.0 configurations continue to work
- Existing board configs unchanged
- Display layer unchanged

---

## Future Possibilities

With this architecture, we could easily add:

**New Link Types:**
- Ethernet shield support
- Bluetooth LE for wireless tally
- USB serial for wired connection
- LoRa for long-range wireless

**New Protocols:**
- ATEM switchers (Blackmagic Design)
- VMix
- OBS Studio (via WebSocket)
- Grass Valley
- Sony switchers
- Generic HTTP/REST APIs

**New Displays:**
- Character LCD (16x2, 20x4)
- OLED displays (SSD1306, etc.)
- E-ink displays (ultra low power)
- WS2812 LED strips
- Headless/logging mode (no display)

**Advanced Features:**
- Multi-protocol support (poll multiple switches)
- Link redundancy (WiFi + Ethernet failover)
- Remote configuration via Bluetooth
- Mesh networking between STAC devices

---

## Testing Strategy

### Unit Tests
- Mock link drivers for protocol testing
- Mock protocols for application testing
- Independent component testing

### Integration Tests
- Link driver + protocol combinations
- Different error conditions
- State transitions

### Hardware Tests
- Real WiFi connections
- GPIO peripheral mode
- Multiple display types

---

## Implementation Priority

**Must Have (v4.0):**
1. `ILinkDriver` interface
2. `WiFiLinkDriver` implementation
3. Protocol refactoring to use injected link
4. Application layer cleanup

**Should Have (v4.x):**
1. `GPIOLinkDriver` for peripheral mode
2. `PeripheralProtocol` implementation
3. Peripheral mode unification

**Nice to Have (v5.0+):**
1. `LCDDisplayDriver`
2. Additional protocol support (ATEM, etc.)
3. Additional link types (Bluetooth, Ethernet)

---

## Questions to Resolve

1. **Memory footprint**: Does abstraction layer add significant overhead?
2. **Performance**: Any latency added by indirection?
3. **Configuration UI**: How to expose link/protocol choices in web UI?
4. **Factory complexity**: How to manage link+protocol combinations?
5. **Error propagation**: How do link errors bubble up through protocol layer?

---

## References

**Current v3.0 Implementation:**
- `src/Network/WiFiManager.cpp` - WiFi-specific manager
- `src/Network/Protocol/V60HDClient.cpp` - WiFi-coupled protocol
- `src/Network/Protocol/V160HDClient.cpp` - HTTP-coupled protocol
- `src/Application/STACApp.cpp` - WiFi-aware application

**Design Patterns:**
- Strategy Pattern (link drivers, protocols, displays)
- Dependency Injection (link injected into protocol)
- Factory Pattern (creation of concrete implementations)
- Interface Segregation (small, focused interfaces)

**Similar Architectures:**
- OSI Network Model (layered abstraction)
- UART/SPI/I2C abstraction in Arduino core
- Network stacks (lwIP, etc.)

---

## Success Criteria

This architecture will be considered successful when:

1. ✅ New protocol can be added without modifying link code
2. ✅ New link type can be added without modifying protocol code
3. ✅ New display can be added without modifying application logic
4. ✅ Peripheral mode is just another link+protocol combination
5. ✅ Test coverage increases due to mockable components
6. ✅ LCD display driver implemented as proof-of-concept
7. ✅ No performance regression vs v3.0
8. ✅ No increase in flash/RAM usage > 5%

---

## Next Steps

### Immediate (v3.0)
1. ✅ **Park this branch** for future work
2. **Complete v3.0 release** on `v3_RC` branch
3. **Gather user feedback** on v3.0 stability and features

### Near-term (v3.1/v3.2)
4. **Incremental refactoring:**
   - Dependency-inject `WiFiClient` into `V60HDClient` constructor
   - Create `PeripheralClient : IRolandClient` to unify peripheral mode
   - Remove WiFi-specific checks from application layer where possible
5. **Validate approach** with real-world usage

### Future (v4.0)
6. **Revisit full link layer design** based on lessons learned
7. **Prototype `ILinkDriver`** interface (simplified version)
8. **Measure overhead** of abstraction layer on ESP32
9. **Implement only if needed** for new transport types (Bluetooth, Ethernet)

### Triggers for Full Implementation
Consider implementing full abstraction when:
- Adding non-WiFi transport (Bluetooth LE, Ethernet shield)
- Adding non-Roland protocols (ATEM, VMix, OBS)
- Major version bump (v4.0) planned anyway
- User requests for features requiring link abstraction

---

**Last Updated:** December 1, 2025  
**Status:** Design document - staged implementation recommended  
**Target Version:** v4.0.0 (full), v3.1 (incremental)  
**Branch:** `architecture/link-layer-abstraction`

<!-- End of Architecture Document -->
