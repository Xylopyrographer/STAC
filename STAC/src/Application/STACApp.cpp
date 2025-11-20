#include <Arduino.h>
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
#include "Utils/InfoPrinter.h"

// Add these using declarations
using Display::DisplayFactory;
using Hardware::IMUFactory;
using Hardware::ButtonFactory;
using Hardware::InterfaceFactory;

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
            // Initialize hardware
            if ( !initializeHardware() ) {
                log_e( "Hardware initialization failed" );
                showError( 1 );
                return false;
            }

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

        // Handle provisioning mode if needed (blocking call)
        if ( systemState->getOperatingMode().getCurrentMode() == OperatingMode::PROVISIONING ) {
            handleProvisioningMode();
            // After provisioning completes, device will restart
        }

        // Initialize GROVE port GPIO pins (only in normal mode - peripheral mode is already handled)
        // This prevents floating pins from causing artifacts on peripheral mode devices
        if ( systemState->getOperatingMode().isNormalMode() ) {
            // GROVE port is already configured above, but we need to set initial state
            grovePort->setTallyState( TallyState::ERROR );  // Set to ERROR (both pins LOW)
            log_i( "GROVE port initialized to UNKNOWN state" );
        }

        // Create startup config handler
#ifdef GLYPH_SIZE_5X5
        startupConfig = std::make_unique<StartupConfig5x5>(
#else
        startupConfig = std::make_unique<StartupConfig8x8>(
#endif
            button,
            display.get(),
            glyphManager.get(),
            configManager.get()
        );

        // Note: Don't call updateDisplay() here - display shows only power pixel until WiFi connects

        return true;
    }        void STACApp::loop() {
            if ( !initialized ) {
                return;
            }

            // Update hardware - read button state
            button->read();

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
            // Display
            display = DisplayFactory::create();
            if ( !display->begin() ) {
                log_e( "Display initialization failed" );
                return false;
            }
            log_i( "✓ Display (%s)", DisplayFactory::getDisplayType() );

            // Clear display buffer and set initial brightness to remove power-up artifacts
            display->clear( false );  // Clear buffer without showing
            #ifdef GLYPH_SIZE_5X5
            display->setBrightness( Config::Display::BRIGHTNESS_MAP_5X5[1], false );  // Set to level 1, no show
            #else
            display->setBrightness( Config::Display::BRIGHTNESS_MAP_8X8[1], false );  // Set to level 1, no show
            #endif

            // Show power pixel immediately using BASE_GLYPHS (before orientation detection)
            #ifdef GLYPH_SIZE_5X5
            const uint8_t* earlyPowerGlyph = Display::Glyphs5x5::BASE_GLYPHS[static_cast<uint8_t>(Display::Glyphs5x5::GlyphId::CENTER_PIXEL)];
            #else
            const uint8_t* earlyPowerGlyph = Display::Glyphs8x8::BASE_GLYPHS[static_cast<uint8_t>(Display::Glyphs8x8::GlyphId::CENTER_DOT)];
            #endif
            display->drawGlyphOverlay(earlyPowerGlyph, Display::StandardColors::ORANGE, true);

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

            // Button - Create XP_Button directly
            button = new Button(
                Config::Pins::BUTTON,
                Config::Button::DEBOUNCE_MS,
                Config::Button::ACTIVE_LOW,    // puEnable: true for active low (needs pullup)
                Config::Button::ACTIVE_LOW     // invert: true for active low
            );
            button->begin();
            // Wait for button to stabilize
            do {
                button->read();
            } while (!button->isStable());
            log_i( "✓ Button" );

            // GlyphManager - initialize with current orientation from IMU
            Orientation initialOrientation = Orientation::UP;  // Default if IMU unavailable
            if ( imu ) {
                Orientation detectedOrientation = imu->getOrientation();
                if ( detectedOrientation != Orientation::UNKNOWN ) {
                    initialOrientation = detectedOrientation;
                }
            }
#ifdef GLYPH_SIZE_5X5
            glyphManager = std::make_unique<Display::GlyphManager5x5>( initialOrientation );
#else
            glyphManager = std::make_unique<Display::GlyphManager8x8>( initialOrientation );
#endif
            lastOrientation = initialOrientation;  // Track for future updates
            log_i( "✓ GlyphManager" );

            // Peripheral mode detector
            // peripheralDetector = Hardware::InterfaceFactory::createPeripheralDetector();
            peripheralDetector = InterfaceFactory::createPeripheralDetector();
            log_i( "✓ Peripheral detector" );

            return true;
        }

        bool STACApp::initializeNetworkAndStorage() {
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
                // Validate loaded ID format (should contain hyphen)
                if ( stacID.indexOf('-') == -1 ) {
                    log_w( "  Invalid STAC ID format detected: %s", stacID.c_str() );
                    log_w( "  Regenerating STAC ID..." );
                    stacID = configManager->generateAndSaveStacID();
                    log_i( "  Generated STAC ID: %s", stacID.c_str() );
                } else {
                    log_i( "  STAC ID: %s", stacID.c_str() );
                }
            }

            // Print startup header to serial
            Utils::InfoPrinter::printHeader(stacID);

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
            wifiManager = std::make_unique<Net::WiFiManager>();
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
            // Long press during normal operation: Adjust brightness
            static bool longPressHandled = false;

            if ( button->pressedFor(Config::Timing::BUTTON_SELECT_MS) ) {
                if ( !longPressHandled ) {
                    longPressHandled = true;
                    
                    // Set GROVE port to UNKNOWN state while adjusting settings
                    // This prevents peripheral devices from showing false tally information
                    grovePort->setTallyState( TallyState::ERROR );
                    
                    // Call brightness adjustment
                    StacOperations ops = systemState->getOperations();
                    uint8_t oldBrightness = ops.displayBrightnessLevel;
                    
                    // Run brightness change (will show checkerboard and allow selection)
                    if (startupConfig) {
                        // Use a temporary flag to indicate we're in runtime brightness adjust
                        startupConfig->changeBrightness(ops);
                        
                        // Save if changed
                        if (ops.displayBrightnessLevel != oldBrightness) {
                            systemState->setOperations(ops);
                            if (!configManager->saveOperations(ops)) {
                                log_e("Failed to save brightness level");
                            }
                        }
                        
                        // Restore the display
                        updateDisplay();
                    }
                }
                return;  // Don't process short press while long press is active
            }

            // Reset long press flag when button is released
            if ( button->isReleased() && longPressHandled ) {
                longPressHandled = false;
            }

        // Short press behavior depends on mode
        if ( button->wasReleased() ) {
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
                
                // Update glyph manager orientation
                if ( glyphManager ) {
                    glyphManager->updateOrientation( currentOrientation );
                }
                
                // Orientation tracking active (logging disabled for normal operation)
            }
        }

        void STACApp::updateDisplay() {
            TallyState currentState = systemState->getTallyState().getCurrentState();
            StacOperations ops = systemState->getOperations();
            
            // Draw the tally state (without showing yet)
            if ( currentState == TallyState::UNSELECTED ) {
                if ( ops.cameraOperatorMode ) {
                    // Camera Operator mode: Show dotted frame glyph
#ifdef GLYPH_SIZE_5X5
                    using namespace Display::Glyphs5x5::GlyphIndex;
#else
                    using namespace Display::Glyphs8x8::GlyphIndex;
#endif
                    const uint8_t* glyphData = glyphManager->getGlyph( GLF_DF );
                    display->drawGlyph( glyphData, 
                                       Display::StandardColors::PURPLE, 
                                       Display::StandardColors::BLACK, 
                                       false );  // Don't show yet
                } else {
                    // Talent mode: Show solid green
                    display->fill( Display::StandardColors::GREEN, false );  // Don't show yet
                }
            } else {
                // All other states: Fill with state color
                Display::color_t color = systemState->getTallyState().getStateColor();
                display->fill( color, false );  // Don't show yet
            }
            
            // Overlay power-on indicator (center pixel for 5x5, center 4 pixels for 8x8)
            // After orientation is determined, use rotated glyphs from GlyphManager
#ifdef GLYPH_SIZE_5X5
            using namespace Display::Glyphs5x5::GlyphIndex;
            const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_PO );  // Power-on center pixel
#else
            using namespace Display::Glyphs8x8::GlyphIndex;
            const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_MID );  // Center 4 pixels for 8x8
#endif
            display->drawGlyphOverlay( powerGlyph, Display::StandardColors::ORANGE, false );
            
            // Now show the complete display
            display->show();
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

void STACApp::displayWiFiStatus( Net::WiFiState state ) {
    using namespace Config::Timing;
    using namespace Display;

#ifdef GLYPH_SIZE_5X5
    using namespace Glyphs5x5::GlyphIndex;
    const uint8_t* wifiGlyph = glyphManager->getGlyph( GLF_WIFI );
#else
    using namespace Glyphs8x8::GlyphIndex;
    const uint8_t* wifiGlyph = glyphManager->getGlyph( GLF_WIFI );
#endif

    switch ( state ) {
        case Net::WiFiState::CONNECTING: {
            // Show orange WiFi glyph while attempting connection
            display->drawGlyph( wifiGlyph, StandardColors::ORANGE, StandardColors::BLACK, true );
            log_i( "WiFi: Attempting connection (orange glyph displayed)" );
            break;
        }

        case Net::WiFiState::CONNECTED: {
            // Show green WiFi glyph on successful connection
            display->drawGlyph( wifiGlyph, StandardColors::GREEN, StandardColors::BLACK, true );
            log_i( "WiFi: Connected (green glyph displayed)" );
            
            // Print WiFi connected status to serial
            Utils::InfoPrinter::printWiFiConnected();
            
            delay( GUI_PAUSE_MS );  // Pause to show success

            // Clear display and show power pixel
            // After orientation is determined, use rotated glyphs from GlyphManager
            display->fill( StandardColors::BLACK, false );
#ifdef GLYPH_SIZE_5X5
            using namespace Display::Glyphs5x5::GlyphIndex;
            const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_PO );  // Center pixel for 5x5
#else
            using namespace Display::Glyphs8x8::GlyphIndex;
            const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_MID );  // Center 4 pixels for 8x8
#endif
            display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, false );
            display->show();
            break;
        }

        case Net::WiFiState::FAILED: {
            // Flash red WiFi glyph on timeout
            display->drawGlyph( wifiGlyph, StandardColors::RED, StandardColors::BLACK, true );
            log_e( "WiFi: Connection timeout (flashing red glyph)" );
            display->flash( 8, 300, display->getBrightness() );  // Flash 8 times at 300ms intervals
            delay( GUI_PAUSE_MS );

            // Show orange glyph again for retry attempt
            display->drawGlyph( wifiGlyph, StandardColors::ORANGE, StandardColors::BLACK, true );
            break;
        }

        default:
            break;
    }
}

void STACApp::handleNormalMode() {
    // Ensure WiFi is connected
    static bool wifiAttempted = false;
    static bool startupConfigDone = false;

    // Run startup configuration sequence (once)
    if ( !startupConfigDone ) {
        startupConfigDone = true;

        // Load operations from NVS
        StacOperations ops;
        if ( !configManager->loadOperations( ops ) ) {
            log_w( "Failed to load operations from NVS, using defaults" );
            ops = systemState->getOperations();
        }
        
        // Sync switchModel from switch config if it's not set
        if (ops.switchModel == "NO_MODEL") {
            String switchModel;
            IPAddress switchIP;
            uint16_t switchPort;
            String username, password;
            if (configManager->loadSwitchConfig(switchModel, switchIP, switchPort, username, password)) {
                ops.switchModel = switchModel;
                // Set appropriate maxChannelCount based on model
                if (switchModel == "V-60HD") {
                    if (ops.maxChannelCount == 0 || ops.maxChannelCount > 8) {
                        ops.maxChannelCount = 8;
                    }
                } else if (switchModel == "V-160HD") {
                    ops.maxChannelCount = 0; // V-160HD uses maxHDMIChannel/maxSDIChannel
                }
                configManager->saveOperations(ops); // Save the synced model
                log_i("Synced switchModel to operations: %s", switchModel.c_str());
            }
        }
        
        // Update system state with loaded operations
        systemState->setOperations( ops );
        
        // Apply the stored brightness level to the display hardware
        {
            uint8_t absoluteBrightness;
            #ifdef GLYPH_SIZE_5X5
            absoluteBrightness = Config::Display::BRIGHTNESS_MAP_5X5[ops.displayBrightnessLevel];
            #else
            absoluteBrightness = Config::Display::BRIGHTNESS_MAP_8X8[ops.displayBrightnessLevel];
            #endif
            display->setBrightness(absoluteBrightness, true);
            log_i("Applied brightness level %d", ops.displayBrightnessLevel);
        }
        
        // Check if autostart is enabled
        bool autoStartBypass = false;
        
        if ( ops.autoStartEnabled ) {
            // Autostart mode: Show channel with blinking corners, wait for timeout or button press
            log_i( "Autostart mode active - waiting for timeout or button press" );
            
            using namespace Config::Timing;
            using namespace Display;
            
            #ifdef GLYPH_SIZE_5X5
            using namespace Glyphs5x5::GlyphIndex;
            #else
            using namespace Glyphs8x8::GlyphIndex;
            #endif
            
            // Show tally channel
            const uint8_t* channelGlyph = glyphManager->getGlyph( ops.tallyChannel );
            display->drawGlyph( channelGlyph, StandardColors::BLUE, StandardColors::BLACK, false );
            
            // Turn on corner pixels (green)
            #ifdef GLYPH_SIZE_5X5
            display->setPixel( 0, StandardColors::GREEN, false );
            display->setPixel( 4, StandardColors::GREEN, false );
            display->setPixel( 20, StandardColors::GREEN, false );
            display->setPixel( 24, StandardColors::GREEN, false );
            #else
            display->setPixel( 0, StandardColors::GREEN, false );
            display->setPixel( 7, StandardColors::GREEN, false );
            display->setPixel( 56, StandardColors::GREEN, false );
            display->setPixel( 63, StandardColors::GREEN, false );
            #endif
            display->show();
            
            unsigned long autostartTimeout = millis() + AUTOSTART_TIMEOUT_MS;
            unsigned long nextFlash = millis() + AUTOSTART_PULSE_MS;
            bool cornersOn = true;
            
            while ( millis() < autostartTimeout ) {
                button->read();
                
                // Button pressed: Cancel autostart
                if ( button->isPressed() ) {
                    log_i( "Button pressed - cancelling autostart" );
                    autoStartBypass = false;
                    break;
                }
                
                // Flash corner pixels
                if ( millis() >= nextFlash ) {
                    cornersOn = !cornersOn;
                    nextFlash = millis() + AUTOSTART_PULSE_MS;
                    
                    display->drawGlyph( channelGlyph, StandardColors::BLUE, StandardColors::BLACK, false );
                    if ( cornersOn ) {
                        #ifdef GLYPH_SIZE_5X5
                        display->setPixel( 0, StandardColors::GREEN, false );
                        display->setPixel( 4, StandardColors::GREEN, false );
                        display->setPixel( 20, StandardColors::GREEN, false );
                        display->setPixel( 24, StandardColors::GREEN, false );
                        #else
                        display->setPixel( 0, StandardColors::GREEN, false );
                        display->setPixel( 7, StandardColors::GREEN, false );
                        display->setPixel( 56, StandardColors::GREEN, false );
                        display->setPixel( 63, StandardColors::GREEN, false );
                        #endif
                    }
                    display->show();
                }
                
                yield();
            }
            
            // If we timed out (no button press), bypass startup config
            if ( millis() >= autostartTimeout ) {
                autoStartBypass = true;
                log_i( "Autostart timeout - bypassing startup config" );
            }
        }
        
        // Run startup configuration (unless autostart bypassed)
        if ( startupConfig->runStartupSequence( ops, autoStartBypass ) ) {
            // Update operations in system state
            systemState->setOperations( ops );
            
            // Save operations to NVS
            if ( !configManager->saveOperations( ops ) ) {
                log_e( "Failed to save operations configuration after startup" );
            }
        }

        // Print configuration summary to serial
        String ssid, password;
        IPAddress switchIP;
        uint16_t switchPort;
        String username, passwordSwitch;
        if (configManager->loadWiFiCredentials(ssid, password) &&
            configManager->loadSwitchConfig(ops.switchModel, switchIP, switchPort, username, passwordSwitch)) {
            Utils::InfoPrinter::printFooter(ops, switchIP, switchPort, ssid);
        }
    }

    if ( !wifiAttempted && !wifiManager->isConnected() && configManager->hasWiFiCredentials() ) {
        wifiAttempted = true;

        String ssid, password;
        if ( configManager->loadWiFiCredentials( ssid, password ) ) {
            log_i( "Attempting to connect to WiFi: %s", ssid.c_str() );
            
            // Set callback for visual feedback
            wifiManager->setStateCallback( 
                [this]( Net::WiFiState state ) {
                    displayWiFiStatus( state );
                }
            );
            
            wifiManager->connect( ssid, password );
        }
    }

    // Initialize Roland client if WiFi connected and not yet initialized
    if ( wifiManager->isConnected() && !rolandClientInitialized ) {
        if ( initializeRolandClient() ) {
            rolandClientInitialized = true;
            log_i( "Roland client initialized" );
            // Show initial tally state after WiFi/Roland setup complete
            updateDisplay();
        }
    }

    // Poll Roland switch if initialized
    if ( rolandClientInitialized ) {
        pollRolandSwitch();
    }
}

        void STACApp::handlePeripheralMode() {
            using namespace Display;
            using namespace Config::Timing;

            // Get glyph indices based on display size
            #ifdef GLYPH_SIZE_5X5
            using namespace Glyphs5x5::GlyphIndex;
            #else
            using namespace Glyphs8x8::GlyphIndex;
            #endif

            log_i( "Entering Peripheral Mode" );

            // Configure GROVE GPIO pins as inputs for reading tally state
            grovePort->configurePinsAsInputs();
            log_i( "GROVE port configured as inputs for peripheral mode" );

            // ===== Load or initialize peripheral mode settings =====
            bool cameraMode = false;  // Default: talent mode
            uint8_t brightnessLevel = 1;  // Default: lowest brightness

            if ( !configManager->loadPeripheralSettings( cameraMode, brightnessLevel ) ) {
                // First time in peripheral mode - save defaults
                log_i( "First time in peripheral mode - using defaults" );
                configManager->savePeripheralSettings( cameraMode, brightnessLevel );
            }

            // Apply brightness
            uint8_t absoluteBrightness;
            #ifdef GLYPH_SIZE_5X5
            absoluteBrightness = Config::Display::BRIGHTNESS_MAP_5X5[brightnessLevel];
            #else
            absoluteBrightness = Config::Display::BRIGHTNESS_MAP_8X8[brightnessLevel];
            #endif
            display->setBrightness( absoluteBrightness, false );

            // Print peripheral mode status to serial
            Utils::InfoPrinter::printPeripheral(cameraMode, brightnessLevel);

            // ===== Startup animation =====
            // Show "P" glyph in green (perifmodecolor)
            const uint8_t* pGlyph = glyphManager->getGlyph( GLF_P );
            display->drawGlyph( pGlyph, StandardColors::GREEN, StandardColors::BLACK, true );

            // Flash display 4 times
            for ( int i = 0; i < 4; i++ ) {
                delay( 250 );
                display->clear( true );
                delay( 250 );
                display->drawGlyph( pGlyph, StandardColors::GREEN, StandardColors::BLACK, true );
            }

            delay( GUI_PAUSE_MS );

            // Show checkmark confirmation
            const uint8_t* checkGlyph = glyphManager->getGlyph( GLF_CK );
            display->drawGlyph( checkGlyph, StandardColors::GREEN, StandardColors::BLACK, true );
            delay( GUI_PAUSE_MS );

            // Clear and show power-on glyph
            display->clear( false );
            #ifdef GLYPH_SIZE_5X5
            const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_PO );
            #else
            const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_MID );
            #endif
            display->drawGlyph( powerGlyph, StandardColors::ORANGE, StandardColors::BLACK, true );

            // Wait for button release
            while ( button->read() );

            log_i( "Peripheral mode initialized: camera=%s, brightness=%d",
                   cameraMode ? "true" : "false", brightnessLevel );

            // ===== Main peripheral mode loop =====
            uint8_t lastTallyState = 0xff;  // Invalid state to force initial update
            unsigned long nextCheck = 0;

            while ( true ) {
                // Read tally state from Grove port
                if ( millis() >= nextCheck ) {
                    nextCheck = millis() + PM_POLL_INTERVAL_MS;

                    TallyState receivedState = grovePort->readTallyState();

                    // Convert to numeric state for comparison
                    uint8_t currentState;
                    switch ( receivedState ) {
                        case TallyState::PROGRAM:    currentState = 3; break;
                        case TallyState::PREVIEW:    currentState = 2; break;
                        case TallyState::UNSELECTED: currentState = 1; break;
                        default:                     currentState = 0; break;
                    }

                    // Update display if state changed
                    if ( currentState != lastTallyState ) {
                        lastTallyState = currentState;

                        switch ( receivedState ) {
                            case TallyState::PROGRAM: {
                                // Red (program)
                                display->fill( StandardColors::RED, false );
                                #ifdef GLYPH_SIZE_5X5
                                const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_PO );
                                #else
                                const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_MID );
                                #endif
                                display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, true );
                                break;
                            }

                            case TallyState::PREVIEW: {
                                // Green (preview)
                                display->fill( StandardColors::GREEN, false );
                                #ifdef GLYPH_SIZE_5X5
                                const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_PO );
                                #else
                                const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_MID );
                                #endif
                                display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, true );
                                break;
                            }

                            case TallyState::UNSELECTED: {
                                if ( cameraMode ) {
                                    // Camera mode: Show dark frame glyph in purple
                                    const uint8_t* dfGlyph = glyphManager->getGlyph( GLF_DF );
                                    display->drawGlyph( dfGlyph, StandardColors::PURPLE, StandardColors::BLACK, false );
                                } else {
                                    // Talent mode: Show green
                                    display->fill( StandardColors::GREEN, false );
                                }
                                #ifdef GLYPH_SIZE_5X5
                                const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_PO );
                                #else
                                const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_MID );
                                #endif
                                display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, true );
                                break;
                            }

                            default: {
                                // Error/unknown state
                                if ( cameraMode ) {
                                    // Camera mode: Show orange X
                                    const uint8_t* xGlyph = glyphManager->getGlyph( GLF_BX );
                                    display->drawGlyph( xGlyph, StandardColors::ORANGE, StandardColors::BLACK, false );
                                    #ifdef GLYPH_SIZE_5X5
                                    const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_PO );
                                    #else
                                    const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_MID );
                                    #endif
                                    display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, true );
                                } else {
                                    // Talent mode: Show green with power glyph
                                    display->fill( StandardColors::GREEN, false );
                                    #ifdef GLYPH_SIZE_5X5
                                    const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_PO );
                                    #else
                                    const uint8_t* powerGlyph = glyphManager->getGlyph( GLF_MID );
                                    #endif
                                    display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, true );
                                }
                                break;
                            }
                        }
                    }
                }

                // Handle button for settings adjustment
                button->read();

                if ( button->pressedFor( BUTTON_SELECT_MS ) ) {
                    // User wants to change peripheral mode settings
                    
                    // Show brightness selection screen
                    display->fill( StandardColors::WHITE, false );
                    
                    // Blank center columns
                    #ifdef GLYPH_SIZE_5X5
                    const uint8_t* centerBlank = glyphManager->getGlyph( GLF_EN );
                    #else
                    const uint8_t* centerBlank = glyphManager->getGlyph( GLF_EN );
                    #endif
                    display->drawGlyphOverlay( centerBlank, StandardColors::BLACK, false );

                    // Show current brightness level
                    const uint8_t* levelGlyph = glyphManager->getDigitGlyph( brightnessLevel );
                    display->drawGlyphOverlay( levelGlyph, StandardColors::ORANGE, true );

                    // State machine: brightness adjustment or mode change
                    // Wait for release (brightness) or keep holding (mode change)
                    bool exitSettings = false;
                    unsigned long modeChangeTimeout = millis() + BUTTON_SELECT_MS;

                    do {
                        button->read();

                        // Released before timeout: Enter brightness adjustment
                        if ( button->isReleased() && ( modeChangeTimeout >= millis() ) ) {
                            // Use shared changeBrightness with peripheral save callback
                            #ifdef GLYPH_SIZE_5X5
                            brightnessLevel = startupConfig->changeBrightness( brightnessLevel,
                                [this, &cameraMode]( uint8_t newBrightness ) {
                                    configManager->savePeripheralSettings( cameraMode, newBrightness );
                                } );
                            #else
                            brightnessLevel = startupConfig->changeBrightness( brightnessLevel,
                                [this, &cameraMode]( uint8_t newBrightness ) {
                                    configManager->savePeripheralSettings( cameraMode, newBrightness );
                                } );
                            #endif
                            exitSettings = true;
                        }
                        // Still pressed after timeout: Enter mode change
                        else if ( button->isPressed() && ( modeChangeTimeout < millis() ) ) {
                            // Use shared changeCameraTalentMode with peripheral save callback
                            #ifdef GLYPH_SIZE_5X5
                            cameraMode = startupConfig->changeCameraTalentMode( cameraMode,
                                [this, &brightnessLevel]( bool newMode ) {
                                    configManager->savePeripheralSettings( newMode, brightnessLevel );
                                } );
                            #else
                            cameraMode = startupConfig->changeCameraTalentMode( cameraMode,
                                [this, &brightnessLevel]( bool newMode ) {
                                    configManager->savePeripheralSettings( newMode, brightnessLevel );
                                } );
                            #endif
                            exitSettings = true;
                        }

                    } while ( !exitSettings );

                    // Force immediate tally state refresh by setting impossible value
                    lastTallyState = 0xff;  // Set to impossible value
                    nextCheck = 0;          // Force immediate check on next loop iteration
                }

                yield();  // Let other tasks run
            }
        }

        void STACApp::handleProvisioningMode() {
            log_i("Entering provisioning mode");
            
            // Create and start web configuration server immediately
            Net::WebConfigServer configServer(stacID);
            
            if (!configServer.begin()) {
                log_e("Failed to start configuration server");
                showError(2); // Show error code 2
                return;
            }
            
            // Get glyph indices
            #ifdef GLYPH_SIZE_5X5
            using namespace Display::Glyphs5x5::GlyphIndex;
            #else
            using namespace Display::Glyphs8x8::GlyphIndex;
            #endif
            
            // Show GLF_CFG in red (matching baseline alertcolor for provisioning)
            const uint8_t* cfgGlyph = glyphManager->getGlyph(GLF_CFG);
            const uint8_t normalBrightness = display->getBrightness();
            const uint8_t dimBrightness = normalBrightness / 2;
            
            display->drawGlyph(cfgGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, true);
            
            // Flash 4 times at 500ms (matching baseline)
            for (int i = 0; i < 4; i++) {
                delay(500);
                display->clear(true);
                delay(500);
                display->drawGlyph(cfgGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, true);
            }
            
            // Set up pulsing config glyph display callback using brightness modulation
            bool pulseState = false;
            configServer.setDisplayUpdateCallback([this, cfgGlyph, normalBrightness, dimBrightness, &pulseState]() {
                display->pulseDisplay(cfgGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK,
                                     pulseState, normalBrightness, dimBrightness);
            });
            
            // Initial config glyph display at normal brightness
            display->setBrightness(normalBrightness, false);
            display->drawGlyph(cfgGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, true);
            
            // Wait for configuration
            ProvisioningData provData = configServer.waitForConfiguration();
            
            // Show green checkmark to confirm receipt (matching baseline)
            const uint8_t* checkmarkGlyph = glyphManager->getGlyph(GLF_CK);
            display->drawGlyph(checkmarkGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, true);
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
            
            // Print configuration complete message to serial
            Utils::InfoPrinter::printConfigDone();
            
            // Checkmark already shown above - just delay before restart
            delay(1000);
            
            // Restart to apply new configuration
            log_i("Restarting to apply configuration");
            ESP.restart();
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
            rolandClient = Net::RolandClientFactory::createFromString( model );
            if ( !rolandClient ) {
                log_e( "Failed to create Roland client for model: %s", model.c_str() );
                return false;
            }

            // Build configuration
            Net::RolandConfig rolandConfig;
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
            using namespace Display;
            using namespace Config::Timing;
            using namespace Config::Net;

            // Get glyph indices based on display size
            #ifdef GLYPH_SIZE_5X5
            using namespace Glyphs5x5::GlyphIndex;
            #else
            using namespace Glyphs8x8::GlyphIndex;
            #endif

            // Check if it's time to poll
            unsigned long now = millis();
            if ( now - lastRolandPoll < rolandPollInterval ) {
                return;
            }

            lastRolandPoll = now;

            // Get references to state
            SwitchState& switchState = systemState->getSwitchState();
            StacOperations& ops = systemState->getOperations();

            // Query tally status
            Net::TallyQueryResult result;
            rolandClient->queryTallyStatus( result );

            // Update switch state from query result
            switchState.connected = result.connected;
            switchState.timeout = result.timedOut;
            switchState.noReply = !result.gotReply;
            switchState.currentTallyState = result.rawResponse;

            // ===== NORMAL OPERATION: Valid tally response =====
            if ( result.connected && result.gotReply ) {
                // Check if response is a valid tally state
                bool validResponse = false;
                TallyState newState = TallyState::UNSELECTED;

                switch ( result.status ) {
                    case Net::TallyStatus::ONAIR:
                        newState = TallyState::PROGRAM;
                        validResponse = true;
                        break;
                    case Net::TallyStatus::SELECTED:
                        newState = TallyState::PREVIEW;
                        validResponse = true;
                        break;
                    case Net::TallyStatus::UNSELECTED:
                        newState = TallyState::UNSELECTED;
                        validResponse = true;
                        break;
                    default:
                        // Junk/invalid reply
                        validResponse = false;
                        break;
                }

                if ( validResponse ) {
                    // ===== Valid response - update tally state =====
                    rolandPollInterval = ops.statusPollInterval;  // Use normal poll interval
                    switchState.junkReply = false;
                    switchState.junkReplyCount = 0;  // Clear error counters
                    switchState.noReplyCount = 0;
                    switchState.lastTallyState = switchState.currentTallyState;

                    // Update tally state if changed
                    if ( newState != systemState->getTallyState().getCurrentState() ) {
                        systemState->getTallyState().setState( newState );
                        log_i( "Tally: %s", State::TallyStateManager::stateToString( newState ) );
                    }
                    else {
                        // State unchanged, but we need to update display and GROVE in case we're recovering from error
                        updateDisplay();
                        if ( systemState->getOperatingMode().isNormalMode() && grovePort ) {
                            grovePort->setTallyState( newState );
                        }
                    }

                    // Grove port will be updated by tally state change callback
                }
                else {
                    // ===== Junk reply received =====
                    rolandPollInterval = ERROR_REPOLL_MS;  // Use faster error polling
                    switchState.junkReply = true;
                    switchState.junkReplyCount++;
                    switchState.lastTallyState = "JUNK";
                    switchState.currentTallyState = "NO_TALLY";

                    if ( switchState.junkReplyCount >= MAX_POLL_ERRORS ) {
                        // Hit error threshold - display error
                        switchState.junkReplyCount = 0;  // Reset counter
                        
                        // Set Grove to unknown state
                        if ( grovePort ) {
                            grovePort->setTallyState( TallyState::ERROR );
                        }

                        if ( ops.cameraOperatorMode ) {
                            // Camera operator mode: Show purple question mark
                            const uint8_t* qmGlyph = glyphManager->getGlyph( GLF_QM );
                            display->drawGlyph( qmGlyph, StandardColors::PURPLE, StandardColors::BLACK, true );
                            log_e( "Junk reply error - showing purple '?'" );
                        }
                        else {
                            // Talent mode: Show preview with power pixel
                            systemState->getTallyState().setState( TallyState::PREVIEW );
                        }
                    }
                }
            }
            // ===== ERROR CONDITIONS =====
            else {
                switchState.currentTallyState = "NO_INIT";
                switchState.lastTallyState = "NO_TALLY";
                switchState.junkReplyCount = 0;  // Clear junk counter (not a junk reply error)
                rolandPollInterval = ERROR_REPOLL_MS;  // Use faster error polling

                if ( !result.connected && result.timedOut ) {
                    // ===== Connection failed and timed out =====
                    switchState.noReplyCount = 0;  // Clear no-reply counter
                    
                    // Set Grove to error state (immediate - connection timeout)
                    if ( grovePort ) {
                        grovePort->setTallyState( TallyState::ERROR );
                    }
                    
                    if ( ops.cameraOperatorMode ) {
                        // Camera operator mode: Show orange X
                        const uint8_t* xGlyph = glyphManager->getGlyph( GLF_BX );
                        display->drawGlyph( xGlyph, StandardColors::ORANGE, StandardColors::BLACK, true );
                        log_e( "Connection timeout - showing orange 'X'" );
                    }
                    else {
                        // Talent mode: Show preview with power pixel
                        systemState->getTallyState().setState( TallyState::PREVIEW );
                    }
                }
                else if ( result.connected && ( result.timedOut || !result.gotReply ) ) {
                    // ===== Connected but no reply or timed out =====
                    switchState.noReplyCount++;

                    if ( switchState.noReplyCount >= MAX_POLL_ERRORS ) {
                        // Hit error threshold
                        switchState.noReplyCount = 0;  // Reset counter
                        
                        // Set Grove to error state (threshold reached)
                        if ( grovePort ) {
                            grovePort->setTallyState( TallyState::ERROR );
                        }
                        
                        if ( ops.cameraOperatorMode ) {
                            // Camera operator mode: Show purple X (big purple X)
                            const uint8_t* xGlyph = glyphManager->getGlyph( GLF_BX );
                            display->drawGlyph( xGlyph, StandardColors::PURPLE, StandardColors::BLACK, true );
                            log_e( "No reply error - showing purple 'X'" );
                        }
                        else {
                            // Talent mode: Show preview with power pixel
                            systemState->getTallyState().setState( TallyState::PREVIEW );
                        }
                    }
                }
                else {
                    // ===== Some other error condition =====
                    switchState.noReplyCount = 0;  // Clear counter
                    
                    // Set Grove to error state (unknown error)
                    if ( grovePort ) {
                        grovePort->setTallyState( TallyState::ERROR );
                    }
                    
                    if ( ops.cameraOperatorMode ) {
                        // Camera operator mode: Show red X
                        const uint8_t* xGlyph = glyphManager->getGlyph( GLF_BX );
                        display->drawGlyph( xGlyph, StandardColors::RED, StandardColors::BLACK, true );
                        log_e( "Unknown error - showing red 'X'" );
                    }
                    else {
                        // Talent mode: Show preview with power pixel
                        systemState->getTallyState().setState( TallyState::PREVIEW );
                    }
                }
            }
        }

        void STACApp::handleOTAUpdateMode() {
            log_i("Entering OTA update mode");
            
            // Print OTA mode notification to serial
            Utils::InfoPrinter::printOTA();
            
            // OTA glyph is already showing from boot button sequence - leave it
            // (matching baseline behavior which doesn't change display on OTA entry)
            
            // Get glyph for pulsing display
            #ifdef GLYPH_SIZE_5X5
            using namespace Display::Glyphs5x5::GlyphIndex;
            #else
            using namespace Display::Glyphs8x8::GlyphIndex;
            #endif
            
            const uint8_t* udGlyph = glyphManager->getGlyph(GLF_UD);
            const uint8_t normalBrightness = display->getBrightness();
            const uint8_t dimBrightness = normalBrightness / 2;
            
            // Create and start OTA update server
            Net::OTAUpdateServer otaServer(stacID);
            
            if (!otaServer.begin()) {
                log_e("Failed to start OTA update server");
                showError(8); // Show error code 8
                ESP.restart(); // Restart on error
                return;
            }
            
            // Set up pulsing OTA glyph display callback using brightness modulation
            bool pulseState = false;
            otaServer.setDisplayUpdateCallback([this, udGlyph, normalBrightness, dimBrightness, &pulseState]() {
                display->pulseDisplay(udGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK,
                                     pulseState, normalBrightness, dimBrightness);
            });
            
            // Wait for firmware upload and update
            // This will either restart the ESP32 (success) or return (failure)
            Net::OTAUpdateResult result = otaServer.waitForUpdate();
            
            // Print OTA update result to serial
            Utils::InfoPrinter::printOTAResult(result.success, result.filename, 
                                              result.bytesWritten, result.statusMessage);
            
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
            
            // Print factory reset notification to serial
            Utils::InfoPrinter::printReset();
            
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
            
            // Get glyph indices
            #ifdef GLYPH_SIZE_5X5
            using namespace Display::Glyphs5x5::GlyphIndex;
            #else
            using namespace Display::Glyphs8x8::GlyphIndex;
            #endif
            
            // Get all glyphs we'll need
            const uint8_t* cfgGlyph = glyphManager->getGlyph(GLF_CFG);
            const uint8_t* udGlyph = glyphManager->getGlyph(GLF_UD);
            
            // Show initial state - GLF_CFG in orange for provisioning
            display->drawGlyph(cfgGlyph, Display::StandardColors::ORANGE, Display::StandardColors::BLACK, true);
            delay(250);
            
            // Flash to confirm we're in button sequence mode
            for (int i = 0; i < 4; i++) {
                display->clear(true);
                delay(125);
                display->drawGlyph(cfgGlyph, Display::StandardColors::ORANGE, Display::StandardColors::BLACK, true);
                delay(125);
            }
            
            // Button state machine loop
            while (!sequenceExit) {
                button->read();
                
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
                            
                            // GLF_FM (solid frame) in red with GLF_CK (checkmark) in green overlay
                            const uint8_t* fmGlyph = glyphManager->getGlyph(GLF_FM);
                            const uint8_t* ckGlyph = glyphManager->getGlyph(GLF_CK);
                            display->drawGlyph(fmGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, false);
                            display->drawGlyphOverlay(ckGlyph, Display::StandardColors::GREEN, true);
                            delay(250);
                            
                            // Flash to show state change
                            for (int i = 0; i < 4; i++) {
                                display->clear(true);
                                delay(125);
                                display->drawGlyph(fmGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, false);
                                display->drawGlyphOverlay(ckGlyph, Display::StandardColors::GREEN, true);
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
                            // Held long enough - advance to OTA update state and start server immediately
                            log_v("Advancing to OTA_UPDATE_PENDING state");
                            
                            // GLF_UD (firmware update icon) in red
                            display->drawGlyph(udGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, true);
                            delay(250);
                            
                            // Flash to show state change
                            for (int i = 0; i < 4; i++) {
                                display->clear(true);
                                delay(125);
                                display->drawGlyph(udGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, true);
                                delay(125);
                            }
                            
                            // Show static OTA glyph after flash sequence
                            display->drawGlyph(udGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, true);
                            
                            // Start OTA server immediately (don't wait for button release)
                            log_i("Boot button sequence: OTA UPDATE selected - starting server");
                            handleOTAUpdateMode();
                            // Never returns - ESP32 restarts after OTA
                        }
                        break;
                        
                    case BootButtonState::OTA_UPDATE_PENDING:
                        // This state is no longer reachable - OTA mode starts immediately above
                        break;
                }
                
                yield();
            }
            
            return resultMode;
        }

} // namespace Application


//  --- EOF --- //
