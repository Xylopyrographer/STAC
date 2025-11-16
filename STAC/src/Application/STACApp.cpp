#include "Application/STACApp.h"
#include "Hardware/Display/DisplayFactory.h"
#include "Hardware/Display/GlyphManager.h"
#include "Hardware/Display/Glyphs5x5.h"
#include "Hardware/Display/Glyphs8x8.h"
#include "Hardware/Sensors/IMUFactory.h"
#include "Hardware/Input/ButtonFactory.h"
#include "Hardware/Interface/InterfaceFactory.h"
#include "Network/Protocol/RolandClientFactory.h"
#include "Utils/TestConfig.h"
#include <Arduino.h>

// Add these using declarations
using STAC::Display::DisplayFactory;
using STAC::Hardware::IMUFactory;
using STAC::Hardware::ButtonFactory;
using STAC::Hardware::InterfaceFactory;

namespace STAC {
    namespace Application {

        STACApp::STACApp()
            : initialized( false )
            , stacID( "" )
            , lastOrientation( Orientation::UNKNOWN )
            , glyphTestMode( false )
            , currentGlyphIndex( 0 )
            , lastGlyphChange( 0 )
            , autoAdvanceGlyphs( true )
            , lastRolandPoll( 0 )
            , rolandPollInterval( 300 )
            , rolandClientInitialized( false ) {
            // unique_ptr members default to nullptr
        }

        bool STACApp::setup() {
            Serial.println();
            Serial.println( "╔════════════════════════════════════════════╗" );
            Serial.println( "║          STAC Application Starting         ║" );
            Serial.println( "╚════════════════════════════════════════════╝" );
            Serial.printf( "\nBoard: %s\n", Config::Strings::BOARD_NAME );
            Serial.printf( "Software: v2.3.0\n" );

            // Initialize hardware
            if ( !initializeHardware() ) {
                log_e( "Hardware initialization failed" );
                showError( 1 );
                return false;
            }

            // Show startup animation
            showStartupAnimation();

            // Initialize network and storage
            if ( !initializeNetworkAndStorage() ) {
                log_e( "Network/Storage initialization failed" );
                showError( 2 );
                return false;
            }

            // Initialize system state
            systemState = std::make_unique<State::SystemState>();
            if ( !systemState->begin() ) {
                log_e( "System state initialization failed" );
                showError( 3 );
                return false;
            }

            // Determine operating mode
            OperatingMode mode = determineOperatingMode();
            systemState->getOperatingMode().setMode( mode );

            log_i( "Operating Mode: %s", systemState->getOperatingMode().getModeString() );
            log_i( "STAC ID: %s", stacID.c_str() );

            // Configure GROVE port based on mode
            bool isOutput = !systemState->getOperatingMode().isPeripheralMode();
            // grovePort = Hardware::InterfaceFactory::createGrovePort( isOutput );
            grovePort = InterfaceFactory::createGrovePort( isOutput );

            // Set up state change callback
            systemState->getTallyState().setStateChangeCallback(
            [ this ]( TallyState oldState, TallyState newState ) {
                log_i( "Tally: %s -> %s",
                       State::TallyStateManager::stateToString( oldState ),
                       State::TallyStateManager::stateToString( newState ) );
                updateDisplay();

                // Update GROVE output if in normal mode
                if ( systemState->getOperatingMode().isNormalMode() ) {
                    grovePort->setTallyState( newState );
                }
            }
            );

        initialized = true;

        Serial.println();
        Serial.println( "╔════════════════════════════════════════════╗" );
        Serial.println( "║               STAC Ready!                  ║" );
        Serial.println( "╚════════════════════════════════════════════╝\n" );

        // Handle provisioning mode if needed (blocking call)
        if ( systemState->getOperatingMode().getCurrentMode() == OperatingMode::PROVISIONING ) {
            handleProvisioningMode();
            // After provisioning completes, device will restart
        }

        // Initial display update
        updateDisplay();

        return true;
    }        void STACApp::loop() {
            if ( !initialized ) {
                return;
            }

            // Update hardware
            button->update();

            // Handle button input
            handleButton();

            // Handle IMU orientation
            if ( imu->isAvailable() ) {
                handleOrientation();
            }

            // Update managers
            wifiManager->update();
            systemState->update();

            // Check for glyph test mode
            if ( glyphTestMode ) {
                handleGlyphTestMode();
                return;
            }

            // Mode-specific handling
            switch ( systemState->getOperatingMode().getCurrentMode() ) {
                case OperatingMode::NORMAL:
                    handleNormalMode();
                    break;

                case OperatingMode::PERIPHERAL:
                    handlePeripheralMode();
                    break;

                case OperatingMode::PROVISIONING:
                    // Provisioning is handled once in setup(), not in loop
                    break;
            }
        }

        bool STACApp::initializeHardware() {
            Serial.println( "\n--- Hardware Initialization ---" );

            // Display
            display = DisplayFactory::create();
            if ( !display->begin() ) {
                log_e( "Display initialization failed" );
                return false;
            }
            log_i( "✓ Display (%s)", DisplayFactory::getDisplayType() );

            // IMU
            // imu = Hardware::IMUFactory::create();
            imu = IMUFactory::create();
            if ( imu->begin() ) {
                log_i( "✓ IMU (%s)", imu->getType() );
                lastOrientation = imu->getOrientation();
                const char *orientationNames[] = { "UP", "DOWN", "LEFT", "RIGHT", "FLAT", "UNKNOWN" };
                log_i( "  Initial orientation: %s", orientationNames[ static_cast<int>( lastOrientation ) ] );
            }
            else {
                log_w( "⚠ IMU unavailable" );
            }

            // Button
            // button = Hardware::ButtonFactory::create();
            button = ButtonFactory::create();
            if ( !button->begin() ) {
                log_e( "Button initialization failed" );
                return false;
            }
            log_i( "✓ Button" );

            // Peripheral mode detector
            // peripheralDetector = Hardware::InterfaceFactory::createPeripheralDetector();
            peripheralDetector = InterfaceFactory::createPeripheralDetector();
            log_i( "✓ Peripheral detector" );

            return true;
        }

        bool STACApp::initializeNetworkAndStorage() {
            Serial.println( "\n--- Network & Storage ---" );

            // Config Manager
            configManager = std::make_unique<Storage::ConfigManager>();
            if ( !configManager->begin() ) {
                log_e( "Config manager initialization failed" );
                return false;
            }
            log_i( "✓ Config Manager" );

            // Load or generate STAC ID
            if ( !configManager->loadStacID( stacID ) ) {
                stacID = configManager->generateAndSaveStacID();
                log_i( "  Generated STAC ID: %s", stacID.c_str() );
            }
            else {
                log_i( "  STAC ID: %s", stacID.c_str() );
            }

#ifdef ENABLE_TEST_CONFIG
            // TEST MODE: Apply hardcoded test configuration
            log_w( "═══════════════════════════════════════════" );
            log_w( "   TEST MODE: Using hardcoded config" );
            log_w( "═══════════════════════════════════════════" );
            
            // Save test WiFi credentials
            configManager->saveWiFiCredentials( 
                Test::TEST_WIFI_SSID, 
                Test::TEST_WIFI_PASSWORD 
            );
            log_i( "  WiFi: %s", Test::TEST_WIFI_SSID );

            // Save test switch configuration
            IPAddress testSwitchIP;
            testSwitchIP.fromString( Test::TEST_SWITCH_IP );
            configManager->saveSwitchConfig(
                Test::TEST_SWITCH_MODEL,
                testSwitchIP,
                Test::TEST_SWITCH_PORT,
                Test::TEST_SWITCH_USERNAME,
                Test::TEST_SWITCH_PASSWORD
            );
            log_i( "  Switch: %s @ %s:%d", 
                   Test::TEST_SWITCH_MODEL, 
                   Test::TEST_SWITCH_IP, 
                   Test::TEST_SWITCH_PORT );

            // Save test operations
            StacOperations testOps;
            testOps.switchModel = Test::TEST_SWITCH_MODEL;
            testOps.tallyChannel = Test::TEST_TALLY_CHANNEL;
            testOps.channelBank = Test::TEST_CHANNEL_BANK;
            testOps.statusPollInterval = Test::TEST_POLL_INTERVAL_MS;
            testOps.cameraOperatorMode = Test::TEST_CAMERA_OPERATOR_MODE;
            testOps.displayBrightnessLevel = Test::TEST_BRIGHTNESS_LEVEL;
            configManager->saveOperations( testOps );
            log_i( "  Channel: %d, Poll: %dms", 
                   Test::TEST_TALLY_CHANNEL, 
                   Test::TEST_POLL_INTERVAL_MS );
            
            log_w( "═══════════════════════════════════════════" );
#endif

            // WiFi Manager
            wifiManager = std::make_unique<Network::WiFiManager>();
            if ( !wifiManager->begin() ) {
                log_e( "WiFi manager initialization failed" );
                return false;
            }
            wifiManager->setHostname( stacID );
            log_i( "✓ WiFi Manager" );

            return true;
        }

        OperatingMode STACApp::determineOperatingMode() {
            // Check for peripheral mode jumper
            if ( peripheralDetector->detect() ) {
                log_i( "Peripheral mode jumper detected" );
                return OperatingMode::PERIPHERAL;
            }

            // Check for button hold at boot (provisioning/factory reset/OTA)
            OperatingMode bootMode = checkBootButtonSequence();
            if (bootMode == OperatingMode::PROVISIONING) {
                log_i("Boot button: Forced provisioning mode");
                return OperatingMode::PROVISIONING;
            }
            // Note: Factory reset and OTA modes restart the device, so we never return from them

            // Check if configured
            if (!configManager->hasWiFiCredentials()) {
                log_i("No WiFi configuration found, entering provisioning mode");
                return OperatingMode::PROVISIONING;
            }

            // Check if switch is configured
            String switchModel;
            IPAddress switchIP;
            uint16_t switchPort;
            String username, password;  // Not used for check, but required by loadSwitchConfig
            if (!configManager->loadSwitchConfig(switchModel, switchIP, switchPort, username, password)) {
                log_i("No switch configuration found, entering provisioning mode");
                return OperatingMode::PROVISIONING;
            }

            log_i("Configuration found, starting in NORMAL mode");
            return OperatingMode::NORMAL;
        }

        void STACApp::handleButton() {
            // Long press: Toggle between tally mode and glyph test mode
            static bool longPressHandled = false;

            if ( button->isLongPress() ) {
                if ( !longPressHandled ) {
                    longPressHandled = true;
                    glyphTestMode = !glyphTestMode;

                    if ( glyphTestMode ) {
                        log_i( "Entering GLYPH TEST mode" );
                        currentGlyphIndex = 0;
                        autoAdvanceGlyphs = true;
                        lastGlyphChange = millis();
                        advanceToNextGlyph();
                    }
                    else {
                        log_i( "Returning to TALLY mode" );
                        updateDisplay();
                    }
                }
                return;  // Don't process short press while long press is active
            }

            // Reset long press flag when button is released
            if ( !button->isPressed() && longPressHandled ) {
                longPressHandled = false;
            }

        // Short press behavior depends on mode
        if ( button->wasClicked() ) {
            if ( glyphTestMode ) {
                // In glyph test mode: advance to next glyph
                advanceToNextGlyph();
                autoAdvanceGlyphs = false;  // Stop auto-advance when user manually advances
            }
            // Note: Tally state is controlled by Roland switch polling, not button presses
        }
    }        void STACApp::handleOrientation() {
            Orientation currentOrientation = imu->getOrientation();

            if ( currentOrientation != lastOrientation &&
                    currentOrientation != Orientation::UNKNOWN ) {

                lastOrientation = currentOrientation;
                // Orientation tracking active (logging disabled for normal operation)
            }
        }

        void STACApp::updateDisplay() {
            Display::color_t color = systemState->getTallyState().getStateColor();
            display->fill( color, true );
        }

        // void STACApp::handleNormalMode() {
        //     // TODO: Implement Roland switch polling
        //     // For now, tally state is controlled by button

        //     // Ensure WiFi is connected
        //     if ( !wifiManager->isConnected() && configManager->hasWiFiCredentials() ) {
        //         String ssid, password;
        //         if ( configManager->loadWiFiCredentials( ssid, password ) ) {
        //             static unsigned long lastConnectAttempt = 0;
        //             if ( millis() - lastConnectAttempt > 30000 ) { // Try every 30 seconds
        //                 lastConnectAttempt = millis();
        //                 log_i( "Attempting to connect to WiFi: %s", ssid.c_str() );
        //                 wifiManager->connect( ssid, password );
        //             }
        //         }
        //     }
        // }

void STACApp::handleNormalMode() {
    // Ensure WiFi is connected
    static bool wifiAttempted = false;

    if ( !wifiAttempted && !wifiManager->isConnected() && configManager->hasWiFiCredentials() ) {
        wifiAttempted = true;

        String ssid, password;
        if ( configManager->loadWiFiCredentials( ssid, password ) ) {
            log_i( "Attempting to connect to WiFi: %s", ssid.c_str() );
            wifiManager->connect( ssid, password );
        }
    }

    // Initialize Roland client if WiFi connected and not yet initialized
    if ( wifiManager->isConnected() && !rolandClientInitialized ) {
        if ( initializeRolandClient() ) {
            rolandClientInitialized = true;
            log_i( "Roland client initialized" );
        }
    }

    // Poll Roland switch if initialized
    if ( rolandClientInitialized ) {
        pollRolandSwitch();
    }
}
        void STACApp::handlePeripheralMode() {
            // Read tally state from GROVE port
            static unsigned long lastRead = 0;
            if ( millis() - lastRead > Config::Timing::PM_POLL_INTERVAL_MS ) {
                lastRead = millis();

                TallyState receivedState = grovePort->readTallyState();

                if ( receivedState != systemState->getTallyState().getCurrentState() ) {
                    systemState->getTallyState().setState( receivedState );
                }
            }
        }

        void STACApp::handleProvisioningMode() {
            log_i("Entering provisioning mode");
            
            // Create and start web configuration server
            Network::WebConfigServer configServer(stacID);
            
            // Set up pulsing teal display callback
            bool pulseState = false;
            configServer.setDisplayUpdateCallback([this, &pulseState]() {
                pulseState = !pulseState;
                if (pulseState) {
                    display->fill(Display::StandardColors::TEAL, true);
                } else {
                    display->fill(Display::StandardColors::DARK_TEAL, true); // Dimmer teal
                }
            });
            
            if (!configServer.begin()) {
                log_e("Failed to start configuration server");
                showError(2); // Show error code 2
                return;
            }
            
            // Initial teal display
            display->fill(Display::StandardColors::TEAL, true);
            
            // Wait for configuration
            ProvisioningData provData = configServer.waitForConfiguration();
            
            // Show green to confirm receipt
            display->fill(Display::StandardColors::GREEN, true);
            delay(1000);
            
            // Stop the web server
            configServer.end();
            
            // Save configuration to NVS
            log_i("Saving configuration to NVS");
            
            // Save WiFi credentials
            if (!configManager->saveWiFiCredentials(provData.wifiSSID, provData.wifiPassword)) {
                log_e("Failed to save WiFi credentials");
                showError(3);
                return;
            }
            
            // Convert IP string to IPAddress
            IPAddress switchIP;
            if (!switchIP.fromString(provData.switchIPString)) {
                log_e("Invalid IP address: %s", provData.switchIPString.c_str());
                showError(4);
                return;
            }
            
            // Save switch configuration
            String username = provData.lanUserID;
            String password = provData.lanPassword;
            
            if (!configManager->saveSwitchConfig(
                provData.switchModel,
                switchIP,
                provData.switchPort,
                username,
                password)) {
                log_e("Failed to save switch configuration");
                showError(5);
                return;
            }
            
            // Create and save operations configuration
            StacOperations ops;
            ops.switchModel = provData.switchModel;
            ops.tallyChannel = 1; // Default to channel 1
            ops.statusPollInterval = provData.pollInterval;
            ops.displayBrightnessLevel = 1; // Default brightness
            ops.cameraOperatorMode = true; // Camera operator mode
            ops.autoStartEnabled = false;
            
            // Set model-specific parameters
            if (provData.switchModel == "V-60HD") {
                ops.maxChannelCount = provData.maxChannel;
                ops.maxHDMIChannel = 0;
                ops.maxSDIChannel = 0;
                ops.channelBank = "";
            } else { // V-160HD
                ops.maxChannelCount = 0;
                ops.maxHDMIChannel = provData.maxHDMIChannel;
                ops.maxSDIChannel = provData.maxSDIChannel;
                ops.channelBank = "hdmi_"; // Default to HDMI bank
            }
            
            if (!configManager->saveOperations(ops)) {
                log_e("Failed to save operations configuration");
                showError(6);
                return;
            }
            
            log_i("Configuration saved successfully");
            
            // Show success animation
            display->fill(Display::StandardColors::GREEN, true);
            delay(2000);
            
            // Restart to apply new configuration
            log_i("Restarting to apply configuration");
            ESP.restart();
        }

        void STACApp::showStartupAnimation() {
            // Quick color sweep
            Display::color_t colors[] = {
                Display::StandardColors::RED,
                Display::StandardColors::GREEN,
                Display::StandardColors::BLUE
            };

            for ( auto color : colors ) {
                display->fill( color, true );
                delay( 250 );
            }

            display->clear( true );
            delay( 250 );
        }

        void STACApp::showError( uint8_t errorCode ) {
            // Flash red with error code
            for ( uint8_t i = 0; i < errorCode; i++ ) {
                display->fill( Display::StandardColors::RED, true );
                delay( 300 );
                display->clear( true );
                delay( 300 );
            }
        }

        void STACApp::handleGlyphTestMode() {
            // Auto-advance glyphs every 500ms if enabled
            if ( autoAdvanceGlyphs && ( millis() - lastGlyphChange >= 500 ) ) {
                advanceToNextGlyph();
            }
        }

        void STACApp::advanceToNextGlyph() {
            // Get the appropriate glyph manager for this display size
            uint8_t displaySize = display->getWidth();
            uint8_t maxGlyphs = ( displaySize == 5 ) ? 32 : 28;

            // Advance to next glyph
            currentGlyphIndex++;
            if ( currentGlyphIndex >= maxGlyphs ) {
                currentGlyphIndex = 0;
            }

            lastGlyphChange = millis();

            // Get the glyph data and draw it
            const uint8_t *glyphData = nullptr;

            if ( displaySize == 5 ) {
                Display::GlyphManager5x5 glyphMgr;
                glyphMgr.updateOrientation( lastOrientation );
                glyphData = glyphMgr.getGlyph( currentGlyphIndex );
            }
            else {
                Display::GlyphManager8x8 glyphMgr;
                glyphMgr.updateOrientation( lastOrientation );
                glyphData = glyphMgr.getGlyph( currentGlyphIndex );
            }

            if ( glyphData != nullptr ) {
                display->drawGlyph( glyphData,
                                    Display::StandardColors::GREEN,
                                    Display::StandardColors::BLACK,
                                    true );
                log_d( "Displaying glyph %d", currentGlyphIndex );
            }
        }

        bool STACApp::initializeRolandClient() {
            // Load switch configuration from storage
            String model;
            IPAddress switchIP;
            uint16_t switchPort;
            String username, password;

            if ( !configManager->loadSwitchConfig( model, switchIP, switchPort, username, password ) ) {
                log_w( "No switch configuration found" );
                return false;
            }

            // Load operations (for tally channel and bank)
            StacOperations ops;
            if ( !configManager->loadOperations( ops ) ) {
                log_w( "No operations configuration found" );
                return false;
            }

            // Cache poll interval to avoid repeated NVS reads
            rolandPollInterval = ops.statusPollInterval;

            // Create Roland client based on switch model
            rolandClient = Network::RolandClientFactory::createFromString( model );
            if ( !rolandClient ) {
                log_e( "Failed to create Roland client for model: %s", model.c_str() );
                return false;
            }

            // Build configuration
            Network::RolandConfig rolandConfig;
            rolandConfig.switchIP = switchIP;
            rolandConfig.switchPort = switchPort;
            rolandConfig.tallyChannel = ops.tallyChannel;
            rolandConfig.username = username;
            rolandConfig.password = password;
            rolandConfig.channelBank = ops.channelBank;
            rolandConfig.stacID = stacID;

            // Initialize the client
            if ( !rolandClient->begin( rolandConfig ) ) {
                log_e( "Failed to initialize Roland client" );
                rolandClient.reset();
                return false;
            }

            log_i( "Roland client ready: %s @ %s:%d (ch %d)",
                   model.c_str(), switchIP.toString().c_str(), switchPort, ops.tallyChannel );

            return true;
        }

        void STACApp::pollRolandSwitch() {
            // Check if it's time to poll
            unsigned long now = millis();

            if ( now - lastRolandPoll < rolandPollInterval ) {
                return;
            }

            lastRolandPoll = now;

            // Query tally status
            Network::TallyQueryResult result;
            if ( !rolandClient->queryTallyStatus( result ) ) {
                log_w( "Roland query failed: %s", Network::tallyStatusToString( result.status ).c_str() );
                return;
            }

            // Map Roland status to TallyState
            TallyState newState = TallyState::UNSELECTED;
            switch ( result.status ) {
                case Network::TallyStatus::ONAIR:
                    newState = TallyState::PROGRAM;
                    break;
                case Network::TallyStatus::SELECTED:
                    newState = TallyState::PREVIEW;
                    break;
                case Network::TallyStatus::UNSELECTED:
                    newState = TallyState::UNSELECTED;
                    break;
                default:
                    log_w( "Unexpected tally status: %s", result.rawResponse.c_str() );
                    return;
            }

            // Update tally state if changed
            if ( newState != systemState->getTallyState().getCurrentState() ) {
                systemState->getTallyState().setState( newState );
                log_i( "Tally updated from Roland: %s",
                       State::TallyStateManager::stateToString( newState ) );
            }
        }

        void STACApp::handleOTAUpdateMode() {
            log_i("Entering OTA update mode");
            
            // Show blue pulsing to indicate OTA mode
            display->fill(Display::StandardColors::BLUE, true);
            delay(500);
            
            // Create and start OTA update server
            Network::OTAUpdateServer otaServer(stacID);
            
            if (!otaServer.begin()) {
                log_e("Failed to start OTA update server");
                showError(8); // Show error code 8
                ESP.restart(); // Restart on error
                return;
            }
            
            // Wait for firmware upload and update
            // This will either restart the ESP32 (success) or return (failure)
            Network::OTAUpdateResult result = otaServer.waitForUpdate();
            
            if (!result.success) {
                log_e("OTA update failed: %s", result.statusMessage.c_str());
                showError(9); // Show error code 9
                delay(3000);
            }
            
            // Restart either way
            ESP.restart();
        }

        void STACApp::handleFactoryReset() {
            log_i("Performing factory reset");
            
            // Show red flashing to indicate factory reset
            for (int i = 0; i < 5; i++) {
                display->fill(Display::StandardColors::RED, true);
                delay(200);
                display->clear(true);
                delay(200);
            }
            
            // Clear all NVS data
            log_i("Clearing all NVS configuration data");
            
            // Clear each namespace
            Preferences prefs;
            
            prefs.begin("stac", false);
            prefs.clear();
            prefs.end();
            
            prefs.begin("wifi", false);
            prefs.clear();
            prefs.end();
            
            prefs.begin("switch", false);
            prefs.clear();
            prefs.end();
            
            prefs.begin("operations", false);
            prefs.clear();
            prefs.end();
            
            log_i("Factory reset complete, restarting...");
            
            // Show green to confirm
            display->fill(Display::StandardColors::GREEN, true);
            delay(2000);
            
            // Restart
            ESP.restart();
        }

        OperatingMode STACApp::checkBootButtonSequence() {
            // If button not pressed at boot, return NORMAL (will check config later)
            if (!button->isPressed()) {
                return OperatingMode::NORMAL;
            }
            
            log_i("Button held at boot - entering button sequence state machine");
            
            // Button state machine timing (in milliseconds)
            static constexpr unsigned long STATE_HOLD_TIME = 2000;  // 2 seconds per state
            
            enum class BootButtonState {
                PROVISIONING_PENDING,  // Short hold -> provisioning
                FACTORY_RESET_PENDING, // Medium hold -> factory reset
                OTA_UPDATE_PENDING     // Long hold -> OTA update
            };
            
            BootButtonState state = BootButtonState::PROVISIONING_PENDING;
            unsigned long stateArmTime = millis() + STATE_HOLD_TIME;
            bool sequenceExit = false;
            OperatingMode resultMode = OperatingMode::NORMAL;
            
            // Show initial state - yellow for provisioning
            display->fill(Display::StandardColors::YELLOW, true);
            delay(250);
            
            // Flash to confirm we're in button sequence mode
            for (int i = 0; i < 4; i++) {
                display->clear(true);
                delay(125);
                display->fill(Display::StandardColors::YELLOW, true);
                delay(125);
            }
            
            // Button state machine loop
            while (!sequenceExit) {
                button->update();
                
                switch (state) {
                    case BootButtonState::PROVISIONING_PENDING:
                        if (!button->isPressed()) {
                            // Released - enter provisioning mode
                            log_i("Boot button sequence: PROVISIONING selected");
                            resultMode = OperatingMode::PROVISIONING;
                            sequenceExit = true;
                        } else if (millis() >= stateArmTime) {
                            // Held long enough - advance to factory reset state
                            log_v("Advancing to FACTORY_RESET_PENDING state");
                            display->fill(Display::StandardColors::RED, true);
                            delay(250);
                            
                            // Flash to show state change
                            for (int i = 0; i < 4; i++) {
                                display->clear(true);
                                delay(125);
                                display->fill(Display::StandardColors::RED, true);
                                delay(125);
                            }
                            
                            state = BootButtonState::FACTORY_RESET_PENDING;
                            stateArmTime = millis() + STATE_HOLD_TIME;
                        }
                        break;
                        
                    case BootButtonState::FACTORY_RESET_PENDING:
                        if (!button->isPressed()) {
                            // Released - perform factory reset
                            log_i("Boot button sequence: FACTORY RESET selected");
                            handleFactoryReset();
                            // Never returns - ESP32 restarts
                        } else if (millis() >= stateArmTime) {
                            // Held long enough - advance to OTA update state
                            log_v("Advancing to OTA_UPDATE_PENDING state");
                            display->fill(Display::StandardColors::BLUE, true);
                            delay(250);
                            
                            // Flash to show state change
                            for (int i = 0; i < 4; i++) {
                                display->clear(true);
                                delay(125);
                                display->fill(Display::StandardColors::BLUE, true);
                                delay(125);
                            }
                            
                            state = BootButtonState::OTA_UPDATE_PENDING;
                            // No timeout for this state - wait for release
                        }
                        break;
                        
                    case BootButtonState::OTA_UPDATE_PENDING:
                        if (!button->isPressed()) {
                            // Released - enter OTA update mode
                            log_i("Boot button sequence: OTA UPDATE selected");
                            handleOTAUpdateMode();
                            // Never returns - ESP32 restarts after OTA
                        }
                        break;
                }
                
                yield();
            }
            
            return resultMode;
        }

    } // namespace Application
} // namespace STAC


//  --- EOF --- //
