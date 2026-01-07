#include <Arduino.h>
#include "Application/STACApp.h"
#include "Hardware/Display/DisplayFactory.h"
#include "Hardware/Display/GlyphManager.h"
// Glyph definitions included via Device_Config.h -> board config
#include "Hardware/Sensors/IMUFactory.h"
#include "Hardware/Input/ButtonFactory.h"
#include "Hardware/Interface/InterfaceFactory.h"
#include "Network/Protocol/RolandClientFactory.h"
#include "Network/WebPortalServer.h"
#include "Utils/InfoPrinter.h"

// Add these 'using' declarations
using Display::DisplayFactory;
using Hardware::IMUFactory;
using Hardware::ButtonFactory;
using Hardware::InterfaceFactory;

namespace Application {

    STACApp::STACApp()
        : initialized( false )
        , stacID( "" )
        , lastRolandPoll( 0 )
        , rolandPollInterval( 300 )
        , rolandClientInitialized( false ) {
        // unique_ptr members default to nullptr
    }

    bool STACApp::setup() {
        // Initialize hardware
        if ( !initializeHardware() ) {
            log_e( "Hardware initialization failed" );
            return false;
        }

        // Initialize network and storage
        if ( !initializeNetworkAndStorage() ) {
            log_e( "Network/Storage initialization failed" );
            return false;
        }

        // Initialize system state
        systemState = std::make_unique<State::SystemState>();
        if ( !systemState->begin() ) {
            log_e( "System state initialization failed" );
            return false;
        }

        // Determine operating mode
        OperatingMode mode = determineOperatingMode();
        systemState->getOperatingMode().setMode( mode );

        log_i( "Operating Mode: %s", systemState->getOperatingMode().getModeString() );
        log_i( "STAC ID: %s", stacID.c_str() );

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Configure GROVE port based on mode
        bool isOutput = !systemState->getOperatingMode().isPeripheralMode();
        // grovePort = Hardware::InterfaceFactory::createGrovePort( isOutput );
        grovePort = InterfaceFactory::createGrovePort( isOutput );
        #endif

        // Set up state change callback
        systemState->getTallyState().setStateChangeCallback(
        [ this ]( TallyState oldState, TallyState newState ) {
            log_i( "Tally: %s -> %s",
                   State::TallyStateManager::stateToString( oldState ),
                   State::TallyStateManager::stateToString( newState ) );
            updateDisplay();

            #if HAS_PERIPHERAL_MODE_CAPABILITY
            // Update GROVE output if in normal mode
            if ( systemState->getOperatingMode().isNormalMode() ) {
                grovePort->setTallyState( newState );
            }
            #endif
        }
        );

        initialized = true;

        // Handle provisioning mode if needed (blocking call)
        if ( systemState->getOperatingMode().getCurrentMode() == OperatingMode::PROVISIONING ) {
            handleProvisioningMode();
            // After provisioning completes, device will restart
        }

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Initialize GROVE port GPIO pins (only in normal mode - peripheral mode is already handled)
        // This prevents floating pins from causing artifacts on peripheral mode devices
        if ( systemState->getOperatingMode().isNormalMode() ) {
            // GROVE port is already configured above, but we need to set initial state
            grovePort->setTallyState( TallyState::ERROR );  // Set to ERROR (both pins LOW)
            log_i( "GROVE port initialized to UNKNOWN state" );
        }
        #endif

        // Create startup config handler (dimension-agnostic using type alias)
        #if defined(BUTTON_B_PIN)
            startupConfig = std::make_unique<Application::StartupConfigType>(
                                button,
                                display.get(),
                                glyphManager.get(),
                                configManager.get(),
                                buttonB
                            );
        #else
            startupConfig = std::make_unique<Application::StartupConfigType>(
                                button,
                                display.get(),
                                glyphManager.get(),
                                configManager.get()
                            );
        #endif

        // Note: Don't call updateDisplay() here - display shows only power pixel until WiFi connects

        return true;
    }

    void STACApp::loop() {
        if ( !initialized ) {
            return;
        }

        // Update hardware - read button state(s)
        button->read();
        #if defined(BUTTON_B_PIN)
            buttonB->read();
            handleButtonB();
        #endif

        // Handle button input
        handleButton();

        // Update managers
        wifiManager->update();
        systemState->update();

        // Mode-specific handling
        switch ( systemState->getOperatingMode().getCurrentMode() ) {
            case OperatingMode::NORMAL:
                handleNormalMode();
                break;

            #if HAS_PERIPHERAL_MODE_CAPABILITY
            case OperatingMode::PERIPHERAL:
                handlePeripheralMode();
                break;
            #endif

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

        // Clear display buffer (backlight is still OFF from begin())
        display->clear( Config::Display::NO_SHOW );
        
        // Draw power glyph to buffer BEFORE turning on backlight
        const uint8_t *earlyPowerGlyph = Display::BASE_GLYPHS[ Display::GLF_PO ];
        display->drawGlyphOverlay( earlyPowerGlyph, Display::StandardColors::ORANGE, Config::Display::SHOW );
        
        // NOW turn on backlight - display already shows the power glyph
        display->setBrightness( Config::Display::BRIGHTNESS_MAP[ 1 ], Config::Display::NO_SHOW );

        // IMU - only read orientation once at startup for glyph rotation
        imu = IMUFactory::create();
        if ( imu->begin() ) {
            log_i( "✓ IMU (%s)", imu->getType() );
            Orientation detectedOrientation = imu->getOrientation();
            const char *orientationNames[] = { "UP", "DOWN", "LEFT", "RIGHT", "FLAT", "UNKNOWN" };
            log_i( "  Initial orientation: %s", orientationNames[ static_cast<int>( detectedOrientation ) ] );
        }
        else {
            log_w( "⚠ IMU unavailable" );
        }

        // Button - Create XP_Button directly
        // puEnable must be false for input-only GPIOs that can't use internal pullup
        bool enableInternalPullup = Config::Button::ACTIVE_LOW && !Config::Button::NEEDS_EXTERNAL_PULLUP;
        button = new Button(
            Config::Pins::BUTTON,
            Config::Button::DEBOUNCE_MS,
            enableInternalPullup,          // puEnable: only if active low AND has internal pullup capability
            Config::Button::ACTIVE_LOW     // invert: true for active low
        );
        button->begin();
        // Wait for button to stabilize
        do {
            button->read();
        } while ( !button->isStable() );
        log_i( "✓ Button" );

        // Button B (secondary) - for reset on M5StickC Plus
        #if defined(BUTTON_B_PIN)
            buttonB = new Button(
                BUTTON_B_PIN,
                Config::Button::DEBOUNCE_MS,
                BUTTON_B_ACTIVE_LOW,    // puEnable: true for active low (needs pullup)
                BUTTON_B_ACTIVE_LOW     // invert: true for active low
            );
            buttonB->begin();
            // Wait for button to stabilize
            do {
                buttonB->read();
            } while ( !buttonB->isStable() );
            log_i( "✓ Button B (reset)" );
        #endif

        // GlyphManager - initialize with current orientation from IMU
        Orientation initialOrientation = Orientation::UP;  // Default if IMU unavailable
        if ( imu ) {
            Orientation detectedOrientation = imu->getOrientation();
            if ( detectedOrientation != Orientation::UNKNOWN ) {
                initialOrientation = detectedOrientation;
            }
        }

        // Set display rotation based on detected orientation (TFT displays rotate, LED matrix uses rotated glyphs)
        display->setOrientationRotation( initialOrientation );

        // GlyphManager - initialize with current orientation from IMU (dimension-agnostic using type alias)
        glyphManager = std::make_unique<Display::GlyphManagerType>( initialOrientation );
        log_i( "✓ GlyphManager" );

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Peripheral mode detector
        // peripheralDetector = Hardware::InterfaceFactory::createPeripheralDetector();
        peripheralDetector = InterfaceFactory::createPeripheralDetector();
        log_i( "✓ Peripheral detector" );
        #endif

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
            if ( stacID.indexOf( '-' ) == -1 ) {
                log_w( "  Invalid STAC ID format detected: %s", stacID.c_str() );
                log_w( "  Regenerating STAC ID..." );
                stacID = configManager->generateAndSaveStacID();
                log_i( "  Generated STAC ID: %s", stacID.c_str() );
            }
            else {
                log_i( "  STAC ID: %s", stacID.c_str() );
            }
        }

        // Print startup header to serial
        Utils::InfoPrinter::printHeader( stacID );

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
        // Check for button hold at boot (PMode toggle/provisioning/factory reset/OTA)
        // This must be checked BEFORE PMode NVS check since button sequence can toggle PMode
        OperatingMode bootMode = checkBootButtonSequence();
        if ( bootMode == OperatingMode::PROVISIONING ) {
            log_i( "Boot button: Forced provisioning mode" );
            return OperatingMode::PROVISIONING;
        }
        if ( bootMode == OperatingMode::PERIPHERAL ) {
            log_i( "Boot button: Peripheral mode selected" );
            return OperatingMode::PERIPHERAL;
        }
        // Note: Factory reset and OTA modes restart the device, so we never return from them

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Check NVS for peripheral mode setting (replaces hardware jumper detection)
        if ( configManager->loadPModeEnabled() ) {
            log_i( "Peripheral mode enabled in NVS" );
            return OperatingMode::PERIPHERAL;
        }
        #endif

        // Check if device is provisioned (has WiFi credentials AND switch configuration)
        if ( !configManager->isProvisioned() ) {
            log_i( "Device not provisioned, entering provisioning mode" );
            Serial.println( "      ***** STAC not configured *****" );
            return OperatingMode::PROVISIONING;
        }

        log_i( "Configuration found, starting in NORMAL mode" );
        return OperatingMode::NORMAL;
    }

    void STACApp::handleButton() {
        // Long press during normal operation: Adjust brightness
        static bool longPressHandled = false;

        if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
            if ( !longPressHandled ) {
                longPressHandled = true;

                #if HAS_PERIPHERAL_MODE_CAPABILITY
                // Set GROVE port to UNKNOWN state while adjusting settings
                // This prevents peripheral devices from showing false tally information
                grovePort->setTallyState( TallyState::ERROR );
                #endif

                // Call brightness adjustment
                StacOperations ops = systemState->getOperations();
                uint8_t oldBrightness = ops.displayBrightnessLevel;

                // Run brightness change (will show checkerboard and allow selection)
                if ( startupConfig ) {
                    // Use a temporary flag to indicate we're in runtime brightness adjust
                    startupConfig->changeBrightness( ops );

                    // Save if changed
                    if ( ops.displayBrightnessLevel != oldBrightness ) {
                        systemState->setOperations( ops );
                        // Save to protocol-specific namespace
                        bool saved = false;
                        if ( ops.isV60HD() ) {
                            saved = configManager->saveV60HDConfig( ops );
                        }
                        else if ( ops.isV160HD() ) {
                            saved = configManager->saveV160HDConfig( ops );
                        }
                        if ( !saved ) {
                            log_e( "Failed to save brightness level" );
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
        // Note: Short press not used in normal operation

    }

    /**
     * @brief Handle Button B events - reset on press (M5StickC Plus only)
     * Note: buttonB->read() must be called before this in the loop
     */
    void STACApp::handleButtonB() {
        #if defined(BUTTON_B_PIN)
            // Button B press triggers immediate restart (after debounce)
            if ( buttonB->wasPressed() ) {
                log_i( "Button B pressed - Restarting..." );
                Serial.println( "\n*** Button B pressed - Restarting... ***" );
                display->setBrightness( 0 );  // Turn off backlight before restart
                delay( 100 );  // Allow serial to flush
                ESP.restart();
            }
        #endif
    }

    /**
     * @brief Update display based on current tally state
     */
    void STACApp::updateDisplay() {
        TallyState currentState = systemState->getTallyState().getCurrentState();
        StacOperations ops = systemState->getOperations();

        // Draw the tally state (without showing yet)
        if ( currentState == TallyState::UNSELECTED ) {
            if ( ops.cameraOperatorMode ) {
                // Camera Operator mode: Show dotted frame glyph
                using namespace Display;
                const uint8_t *glyphData = glyphManager->getGlyph( Display::GLF_DF );
                display->drawGlyph( glyphData,
                                    Display::StandardColors::PURPLE,
                                    Display::StandardColors::BLACK,
                                    false );  // Don't show yet
            }
            else {
                // Talent mode: Show solid green
                display->fill( Display::StandardColors::GREEN, Config::Display::NO_SHOW );  // Don't show yet
            }
        }
        else {
            // All other states: Fill with state color
            Display::color_t color = systemState->getTallyState().getStateColor();
            display->fill( color, Config::Display::NO_SHOW );  // Don't show yet
        }

        // Overlay power-on indicator glyph
        // After orientation is determined, use rotated glyphs from GlyphManager
        const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
        display->drawGlyphOverlay( powerGlyph, Display::StandardColors::ORANGE, Config::Display::NO_SHOW );

        // Now show the complete display
        display->show();
    }

    void STACApp::displayWiFiStatus( Net::WiFiState state ) {
        using namespace Config::Timing;
        using namespace Display;

        const uint8_t *wifiGlyph = glyphManager->getGlyph( Display::GLF_WIFI );

        switch ( state ) {
            case Net::WiFiState::CONNECTING: {
                // Show orange WiFi glyph while attempting connection
                display->drawGlyph( wifiGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::SHOW );
                log_i( "WiFi: Attempting connection (orange glyph displayed)" );
                break;
            }

            case Net::WiFiState::CONNECTED: {
                // Show green WiFi glyph on successful connection
                display->drawGlyph( wifiGlyph, StandardColors::GREEN, StandardColors::BLACK, Config::Display::SHOW );
                log_i( "WiFi: Connected (green glyph displayed)" );

                // Print WiFi connected status to serial
                Utils::InfoPrinter::printWiFiConnected();

                delay( GUI_PAUSE_MS );  // Pause to show success

                // Clear display and show power pixel
                // After orientation is determined, use rotated glyphs from GlyphManager
                display->fill( StandardColors::BLACK, Config::Display::NO_SHOW );

                const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::NO_SHOW );
                display->show();
                break;
            }

            case Net::WiFiState::FAILED: {
                // Flash red WiFi glyph on timeout
                display->drawGlyph( wifiGlyph, StandardColors::RED, StandardColors::BLACK, Config::Display::SHOW );
                log_e( "WiFi: Connection timeout (flashing red glyph)" );
                display->flash( 8, 300, display->getBrightness() );  // Flash 8 times at 300ms intervals
                delay( GUI_PAUSE_MS );

                // Show orange glyph again for retry attempt
                display->drawGlyph( wifiGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::SHOW );
                break;
            }

            default:
                break;
        }
    }

    void STACApp::handleNormalMode() {
        // Ensure WiFi is connected
        static bool wifiAttempted = false;
        static bool interactiveConfigDone = false;
        
        // Switch configuration - loaded once and reused for Roland client initialization
        static IPAddress switchIP;
        static uint16_t switchPort = 0;
        static String username;
        static String passwordSwitch;
        static bool switchConfigLoaded = false;

        // Run startup configuration sequence (once)
        if ( !interactiveConfigDone ) {
            interactiveConfigDone = true;

            // Load protocol-specific operations from NVS
            StacOperations ops;
            String protocol = configManager->getActiveProtocol();
            bool opsLoaded = false;

            // @Claude: another place where we can simplify if we define an enum for the switch models
            if ( protocol == "V-60HD" ) {
                opsLoaded = configManager->loadV60HDConfig( ops );
            }
            else if ( protocol == "V-160HD" ) {
                opsLoaded = configManager->loadV160HDConfig( ops );
            }

            // @Claude: Again, should only be one place where we check if we're provisioned or not. Defaults should be loaded int ops parameters based on the configured switch at startup
            if ( !opsLoaded ) {
                log_w( "Failed to load protocol configuration from NVS, using defaults" );
                ops = systemState->getOperations();
                // Set switchModel from active protocol if available
                if ( !protocol.isEmpty() ) {
                    ops.switchModel = protocol;
                }
            }

            // Update system state with loaded operations
            systemState->setOperations( ops );

            // Apply the stored brightness level to the display hardware
            {
                uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ ops.displayBrightnessLevel ];
                display->setBrightness( absoluteBrightness, Config::Display::SHOW );
                log_i( "Applied brightness level %d", ops.displayBrightnessLevel );
            }

            // Load switch configuration for printing and Roland client initialization
            // @Claude: I guess using an enum for the switch models can be problematic if we want to print the model name. Maybe we can have a function that converts from enum to string for printing purposes?
            String ssid, password;
            switchConfigLoaded = configManager->loadSwitchConfig( ops.switchModel, switchIP, switchPort, username, passwordSwitch );
            
            // Print configuration summary to serial (always, before autostart check)
            if ( switchConfigLoaded && configManager->loadWiFiCredentials( ssid, password ) ) {
                Utils::InfoPrinter::printFooter( ops, switchIP, switchPort, ssid );
            }

            // Display the active tally channel (always, regardless of autostart setting)
            // For V-160HD SDI channels (9-20), display the channel within bank (1-8)
            uint8_t displayChannel = ops.tallyChannel;
            if ( ops.switchModel != "V-60HD" && ops.tallyChannel > 8 ) {
                displayChannel = ops.tallyChannel - 8;  // SDI 9→1, 10→2, etc.
            }
            const uint8_t *channelGlyph = glyphManager->getDigitGlyph( displayChannel );

            // Channel and autostart colors depend on switch model and channel bank
            Display::color_t channelColor;
            Display::color_t autostartColor;

            if ( ops.switchModel != "V-60HD" && ops.tallyChannel > 8 ) {
                // V-160HD second bank (SDI channels 9-20)
                channelColor = Display::StandardColors::LIGHT_GREEN;
                autostartColor = Display::StandardColors::BLUE;
            }
            else {
                // V-60HD or V-160HD first bank (HDMI channels 1-8)
                channelColor = Display::StandardColors::BLUE;
                autostartColor = Display::StandardColors::BRIGHT_GREEN;
            }

            // Clear any previous display state before showing channel glyph
            display->clear( Config::Display::NO_SHOW );
            display->drawGlyph( channelGlyph, channelColor, Display::StandardColors::BLACK, Config::Display::SHOW );

            // Wait for button release before proceeding
            while ( button->read() ) {
                // Button read is non-blocking, no yield needed
            }            // Check if autostart is enabled
            bool autoStartBypass = false;

            if ( ops.autoStartEnabled ) {
                // Autostart mode: Add blinking corners and wait for timeout or button press
                log_i( "Autostart mode active - waiting for timeout or button press" );

                using namespace Config::Timing;
                using namespace Display;

                // Turn on corner pixels (color based on channel bank) - channel already displayed above
                using namespace Display;
                const uint8_t *cornersGlyph = glyphManager->getGlyph( Display::GLF_CORNERS );
                display->pulseCorners( cornersGlyph, true, autostartColor );

                unsigned long autostartTimeout = millis() + AUTOSTART_TIMEOUT_MS;
                unsigned long nextFlash = millis() + AUTOSTART_PULSE_MS;
                bool cornersOn = true;

                while ( millis() < autostartTimeout ) {
                    button->read();
                    #if defined(BUTTON_B_PIN)
                        buttonB->read();
                    #endif
                    handleButtonB();  // Check for Button B reset

                    // Button pressed: Cancel autostart
                    if ( button->isPressed() ) {
                        log_i( "Button pressed - cancelling autostart" );
                        autoStartBypass = false;
                        break;
                    }

                    // Flash corner pixels
                    if ( millis() >= nextFlash ) {
                        nextFlash = millis() + AUTOSTART_PULSE_MS;
                        cornersOn = !cornersOn;

                        // Only pulse corners - channel glyph already drawn
                        display->pulseCorners( cornersGlyph, cornersOn, autostartColor );
                    }
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

                // Save to protocol-specific namespace
                bool saved = false;
                if ( ops.isV60HD() ) {
                    saved = configManager->saveV60HDConfig( ops );
                }
                else if ( ops.isV160HD() ) {
                    saved = configManager->saveV160HDConfig( ops );
                }
                if ( !saved ) {
                    log_e( "Failed to save protocol configuration after startup" );
                }
            }
        }

        if ( !wifiAttempted && !wifiManager->isConnected() && configManager->hasWiFiCredentials() ) {
            wifiAttempted = true;

            String ssid, password;
            if ( configManager->loadWiFiCredentials( ssid, password ) ) {
                log_i( "Attempting to connect to WiFi: %s", ssid.c_str() );

                // Set callback for visual feedback
                wifiManager->setStateCallback(
                [ this ]( Net::WiFiState state ) {
                    displayWiFiStatus( state );
                }
                );

                wifiManager->connect( ssid, password );
            }
        }

        // Initialize Roland client if WiFi connected and not yet initialized
        if ( wifiManager->isConnected() && !rolandClientInitialized && switchConfigLoaded ) {
            if ( initializeRolandClient( switchIP, switchPort, username, passwordSwitch ) ) {
                rolandClientInitialized = true;
                log_i( "Roland client initialized" );
                // Don't update display yet - wait for first valid tally response
                // Display will remain black (with power pixel) until tally state is known
            }
        }

        // Poll Roland switch if initialized
        if ( rolandClientInitialized ) {
            pollRolandSwitch();
        }
    }

#if HAS_PERIPHERAL_MODE_CAPABILITY
    void STACApp::handlePeripheralMode() {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph indices based on display size
        using namespace Display;

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
        uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ brightnessLevel ];
        display->setBrightness( absoluteBrightness, Config::Display::NO_SHOW );

        // Print peripheral mode status to serial
        Utils::InfoPrinter::printPeripheral( cameraMode, brightnessLevel );

        // ===== Startup animation =====
        // Show "P" glyph in green (perifmodecolor)
        const uint8_t *pGlyph = glyphManager->getGlyph( Display::GLF_P );
        display->drawGlyph( pGlyph, StandardColors::GREEN, StandardColors::BLACK, Config::Display::SHOW );

        // Flash display 4 times
        for ( int i = 0; i < 4; i++ ) {
            delay( 250 );
            display->clear( Config::Display::SHOW );
            delay( 250 );
            display->drawGlyph( pGlyph, StandardColors::GREEN, StandardColors::BLACK, Config::Display::SHOW );
        }

        delay( GUI_PAUSE_MS );

        // Show checkmark confirmation
        const uint8_t *checkGlyph = glyphManager->getGlyph( Display::GLF_CK );
        display->drawGlyph( checkGlyph, StandardColors::GREEN, StandardColors::BLACK, Config::Display::SHOW );
        delay( GUI_PAUSE_MS );

        // Clear and show power-on glyph
        display->clear( Config::Display::NO_SHOW );

        const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
        display->drawGlyph( powerGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::SHOW );

        // Wait for button release
        while ( button->read() );

        log_i( "Peripheral mode initialized: camera=%s, brightness=%d",
               cameraMode ? "true" : "false", brightnessLevel );

        // ===== Main peripheral mode loop =====
        uint8_t lastTallyState = Config::Peripheral::INVALID_STATE;
        unsigned long nextCheck = 0;

        while ( true ) {
            // Read tally state from Grove port
            if ( millis() >= nextCheck ) {
                nextCheck = millis() + PM_POLL_INTERVAL_MS;

                TallyState receivedState = grovePort->readTallyState();

                // Convert to numeric state for comparison
                uint8_t currentState;
                switch ( receivedState ) {
                    case TallyState::PROGRAM:
                        currentState = 3;
                        break;
                    case TallyState::PREVIEW:
                        currentState = 2;
                        break;
                    case TallyState::UNSELECTED:
                        currentState = 1;
                        break;
                    default:
                        currentState = 0;
                        break;
                }

                // Update display if state changed
                if ( currentState != lastTallyState ) {
                    lastTallyState = currentState;

                    switch ( receivedState ) {
                        case TallyState::PROGRAM: {
                            // Red (program)
                            display->fill( StandardColors::RED, Config::Display::NO_SHOW );

                            const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                            display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            break;
                        }

                        case TallyState::PREVIEW: {
                            // Green (preview)
                            display->fill( StandardColors::GREEN, Config::Display::NO_SHOW );

                            const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                            display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            break;
                        }

                        case TallyState::UNSELECTED: {
                            if ( cameraMode ) {
                                // Camera mode: Show dark frame glyph in purple
                                const uint8_t *dfGlyph = glyphManager->getGlyph( Display::GLF_DF );
                                display->drawGlyph( dfGlyph, StandardColors::PURPLE, StandardColors::BLACK, Config::Display::NO_SHOW );
                            }
                            else {
                                // Talent mode: Show green
                                display->fill( StandardColors::GREEN, Config::Display::NO_SHOW );
                            }
                            const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                            display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            break;
                        }

                        default: {
                            // Error/unknown state
                            if ( cameraMode ) {
                                // Camera mode: Show orange X
                                const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_BX );
                                display->drawGlyph( xGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::NO_SHOW );

                                const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                                display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            }
                            else {
                                // Talent mode: Show green with power glyph
                                display->fill( StandardColors::GREEN, Config::Display::NO_SHOW );

                                const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                                display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            }
                            break;
                        }
                    }
                }
            }

            // Handle button for settings adjustment
            button->read();
            #if defined(BUTTON_B_PIN)
                buttonB->read();
            #endif
            handleButtonB();  // Check for Button B reset

            if ( button->pressedFor( BUTTON_SELECT_MS ) ) {
                // User wants to change peripheral mode settings

                // Show brightness selection screen
                display->fill( StandardColors::WHITE, Config::Display::NO_SHOW );

                // Blank center columns
                const uint8_t *centerBlank = glyphManager->getGlyph( Display::GLF_EN );
                display->drawGlyphOverlay( centerBlank, StandardColors::BLACK, Config::Display::NO_SHOW );

                // Show current brightness level
                const uint8_t *levelGlyph = glyphManager->getDigitGlyph( brightnessLevel );
                display->drawGlyphOverlay( levelGlyph, StandardColors::ORANGE, Config::Display::SHOW );

                // State machine: brightness adjustment or mode change
                // Wait for release (brightness) or keep holding (mode change)
                bool exitSettings = false;
                unsigned long modeChangeTimeout = millis() + BUTTON_SELECT_MS;

                do {
                    button->read();
                    #if defined(BUTTON_B_PIN)
                        buttonB->read();
                    #endif
                    handleButtonB();  // Check for Button B reset

                    // Released before timeout: Enter brightness adjustment
                    if ( button->isReleased() && ( modeChangeTimeout >= millis() ) ) {
                        // Use shared changeBrightness with peripheral save callback

                        brightnessLevel = startupConfig->changeBrightness( brightnessLevel,
                        [ this, &cameraMode ]( uint8_t newBrightness ) {
                            configManager->savePeripheralSettings( cameraMode, newBrightness );
                        } );
                        exitSettings = true;
                    }
                    // Still pressed after timeout: Enter mode change
                    else if ( button->isPressed() && ( modeChangeTimeout < millis() ) ) {
                        // Use shared changeCameraTalentMode with peripheral save callback
                        cameraMode = startupConfig->changeCameraTalentMode( cameraMode,
                        [ this, &brightnessLevel ]( bool newMode ) {
                            configManager->savePeripheralSettings( newMode, brightnessLevel );
                        } );
                        exitSettings = true;
                    }

                } while ( !exitSettings );

                // Force immediate tally state refresh by setting impossible value
                lastTallyState = Config::Peripheral::INVALID_STATE;
                nextCheck = 0;          // Force immediate check on next loop iteration
            }
        }
    }
#endif // HAS_PERIPHERAL_MODE_CAPABILITY

    void STACApp::handleProvisioningMode() {
        log_i( "Entering unified portal mode (provisioning/OTA)" );

        // Check if device was already provisioned - affects display color
        // Provisioned: ORANGE (warning - proceeding will modify existing config)
        // Not provisioned: RED (alert - must provision to use device)
        bool wasProvisioned = configManager->isProvisioned();
        Display::color_t provisionColor = wasProvisioned 
            ? Display::StandardColors::ORANGE 
            : Display::StandardColors::RED;
        log_i( "Provisioning color: %s", wasProvisioned ? "ORANGE (already provisioned)" : "RED (not provisioned)" );

        // Create and start unified web portal server immediately
        Net::WebPortalServer portalServer( stacID );

        if ( !portalServer.begin() ) {
            log_e( "Failed to start portal server" );
            return;
        }

        // Get glyph indices
        using namespace Display;

        // Show GLF_CFG in appropriate color based on provisioned state
        const uint8_t *cfgGlyph = glyphManager->getGlyph( Display::GLF_CFG );
        const uint8_t normalBrightness = display->getBrightness();
        
        // Calculate dim brightness using adjacent brightness levels from the map
        // Find current level index in brightness map
        uint8_t currentLevel = 1;
        for ( uint8_t i = 1; i <= Config::Display::BRIGHTNESS_LEVELS; i++ ) {
            if ( Config::Display::BRIGHTNESS_MAP[ i ] == normalBrightness ) {
                currentLevel = i;
                break;
            }
        }
        // Pulse to adjacent level: down one level, or up one if already at minimum
        const uint8_t dimBrightness = ( currentLevel > 1 ) 
            ? Config::Display::BRIGHTNESS_MAP[ currentLevel - 1 ]
            : Config::Display::BRIGHTNESS_MAP[ 2 ];

        display->drawGlyph( cfgGlyph, provisionColor, Display::StandardColors::BLACK, Config::Display::SHOW );

        // Flash 4 times at 500ms (matching baseline)
        for ( int i = 0; i < 4; i++ ) {
            delay( 500 );
            display->clear( Config::Display::SHOW );
            delay( 500 );
            display->drawGlyph( cfgGlyph, provisionColor, Display::StandardColors::BLACK, Config::Display::SHOW );
        }

        // Set up pulsing config glyph display callback using brightness modulation
        bool pulseState = false;
        portalServer.setDisplayUpdateCallback( [ this, cfgGlyph, provisionColor, normalBrightness, dimBrightness, &pulseState ]() {
            display->pulseDisplay( cfgGlyph, provisionColor, Display::StandardColors::BLACK,
                                   pulseState, normalBrightness, dimBrightness );
        } );

        // Set up Button B reset check callback (for M5StickC Plus)
        #if defined(BUTTON_B_PIN)
            portalServer.setResetCheckCallback( [ this ]() {
                buttonB->read();
                return buttonB->wasReleased();
            } );

            // Set up pre-restart callback to turn off backlight on TFT displays
            portalServer.setPreRestartCallback( [ this ]() {
                display->setBrightness( 0 );
            } );
        #endif

        // Initial config glyph display at normal brightness
        display->setBrightness( normalBrightness, Config::Display::NO_SHOW );
        display->drawGlyph( cfgGlyph, provisionColor, Display::StandardColors::BLACK, Config::Display::SHOW );

        // Wait for either configuration or OTA to complete
        Net::WebPortalServer::PortalResult result = portalServer.waitForCompletion();

        // Handle result based on type
        if ( result.type == Net::WebPortalServer::PortalResultType::OTA_SUCCESS ) {
            // OTA succeeded - server will restart automatically
            log_i( "OTA update successful - restarting..." );
            const uint8_t *checkmarkGlyph = glyphManager->getGlyph( Display::GLF_CK );
            display->drawGlyph( checkmarkGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
            delay( 1000 );
            portalServer.end();
            display->setBrightness( 0 );
            ESP.restart();
            // Never returns
        }
        else if ( result.type == Net::WebPortalServer::PortalResultType::OTA_FAILED ) {
            // OTA failed - show error and restart
            log_e( "OTA update failed: %s", result.otaResult.statusMessage.c_str() );
            const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_X );
            display->drawGlyph( xGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, Config::Display::SHOW );
            delay( 3000 );
            portalServer.end();
            display->setBrightness( 0 );
            ESP.restart();
            // Never returns
        }
        else if ( result.type == Net::WebPortalServer::PortalResultType::FACTORY_RESET ) {
            // Factory reset requested from web portal
            log_i( "Factory reset requested from web portal" );
            portalServer.end();
            
            // Print factory reset notification to serial
            Utils::InfoPrinter::printReset();
            
            // Clear all NVS data using ConfigManager
            if ( !configManager->clearAll() ) {
                log_e( "Factory reset failed - NVS clear unsuccessful" );
            }
            else {
                log_i( "Factory reset complete - restarting" );
            }
            
            // Restart immediately (web-based reset doesn't wait for button)
            delay( 1000 );
            ESP.restart();
            // Never returns
        }
        // else CONFIG_RECEIVED - continue with provisioning flow
        
        ProvisioningData provData = result.configData;

        // Show green checkmark to confirm receipt (matching baseline)
        const uint8_t *checkmarkGlyph = glyphManager->getGlyph( Display::GLF_CK );
        display->drawGlyph( checkmarkGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
        delay( 1000 );

        // Stop the web server
        portalServer.end();

        // Save configuration to NVS
        log_i( "Saving configuration to NVS" );

        // Save WiFi credentials
        if ( !configManager->saveWiFiCredentials( provData.wifiSSID, provData.wifiPassword ) ) {
            log_e( "Failed to save WiFi credentials" );
            return;
        }

        // Convert IP string to IPAddress
        IPAddress switchIP;
        if ( !switchIP.fromString( provData.switchIPString ) ) {
            log_e( "Invalid IP address: %s", provData.switchIPString.c_str() );
            return;
        }

        // Save switch configuration
        String username = provData.lanUserID;
        String password = provData.lanPassword;

        if ( !configManager->saveSwitchConfig(
                    provData.switchModel,
                    switchIP,
                    provData.switchPort,
                    username,
                    password ) ) {
            log_e( "Failed to save switch configuration" );
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
        if ( provData.switchModel == "V-60HD" ) {
            ops.maxChannelCount = provData.maxChannel;
            ops.maxHDMIChannel = 0;
            ops.maxSDIChannel = 0;
            ops.channelBank = "";
        }
        else {   // V-160HD
            ops.maxChannelCount = 0;
            ops.maxHDMIChannel = provData.maxHDMIChannel;
            ops.maxSDIChannel = provData.maxSDIChannel;
            ops.channelBank = "hdmi_"; // Default to HDMI bank
        }

        // Save to protocol-specific namespace
        bool saved = false;
        if ( ops.isV60HD() ) {
            saved = configManager->saveV60HDConfig( ops );
        }
        else if ( ops.isV160HD() ) {
            saved = configManager->saveV160HDConfig( ops );
        }
        if ( !saved ) {
            log_e( "Failed to save protocol configuration" );
            return;
        }

        log_i( "Configuration saved successfully" );

        // Print configuration complete message to serial
        Utils::InfoPrinter::printConfigDone();

        // Checkmark already shown above - just delay before restart
        delay( 1000 );

        // Restart to apply new configuration
        log_i( "Restarting to apply configuration" );
        display->setBrightness( 0 );  // Turn off backlight before restart
        ESP.restart();
    }

    bool STACApp::initializeRolandClient( const IPAddress& switchIP, uint16_t switchPort,
                                          const String& username, const String& password ) {
        // Get operations from system state (already loaded during startup)
        StacOperations ops = systemState->getOperations();

        // Cache poll interval to avoid repeated NVS reads
        rolandPollInterval = ops.statusPollInterval;

        // Create Roland client based on switch model from operations
        rolandClient = Net::RolandClientFactory::createFromString( ops.switchModel );
        if ( !rolandClient ) {
            log_e( "Failed to create Roland client for model: %s", ops.switchModel.c_str() );
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
               ops.switchModel.c_str(), switchIP.toString().c_str(), switchPort, ops.tallyChannel );

        return true;
    }

    void STACApp::pollRolandSwitch() {
        using namespace Display;
        using namespace Config::Timing;
        using namespace Config::Net;

        // Get glyph indices based on display size
        using namespace Display;

        // Check if it's time to poll
        unsigned long now = millis();
        if ( now - lastRolandPoll < rolandPollInterval ) {
            return;
        }

        // Don't poll if WiFi is not connected
        if ( !wifiManager->isConnected() ) {
            return;
        }

        // Get references to state
        SwitchState& switchState = systemState->getSwitchState();
        StacOperations& ops = systemState->getOperations();

        // Query tally status
        Net::TallyQueryResult result;
        rolandClient->queryTallyStatus( result );

        // Update last poll time AFTER query completes
        // This ensures we don't count the blocking HTTP request time as part of the interval
        lastRolandPoll = millis();

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
                    #if HAS_PERIPHERAL_MODE_CAPABILITY
                    if ( systemState->getOperatingMode().isNormalMode() && grovePort ) {
                        grovePort->setTallyState( newState );
                    }
                    #endif
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

                    #if HAS_PERIPHERAL_MODE_CAPABILITY
                    // Set Grove to unknown state
                    if ( grovePort ) {
                        grovePort->setTallyState( TallyState::ERROR );
                    }
                    #endif

                    if ( ops.cameraOperatorMode ) {
                        // Camera operator mode: Show purple question mark
                        const uint8_t *qmGlyph = glyphManager->getGlyph( Display::GLF_QM );
                        display->drawGlyph( qmGlyph, StandardColors::PURPLE, StandardColors::BLACK, Config::Display::SHOW );
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

                #if HAS_PERIPHERAL_MODE_CAPABILITY
                // Set Grove to error state (immediate - connection timeout)
                if ( grovePort ) {
                    grovePort->setTallyState( TallyState::ERROR );
                }
                #endif

                if ( ops.cameraOperatorMode ) {
                    // Camera operator mode: Show orange X
                    const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_BX );
                    display->drawGlyph( xGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::SHOW );
                    log_e( "Connection timeout - showing orange 'X'" );
                }
                else {
                    // Talent mode: Show preview with power pixel
                    systemState->getTallyState().setState( TallyState::PREVIEW );
                }
            }
            else if ( result.connected && ( result.timedOut || !result.gotReply ) ) {
                // ===== Connected but no reply or timed out =====
                rolandPollInterval = ERROR_REPOLL_MS;  // Use faster error polling
                switchState.noReplyCount++;

                // Don't update display or tally state until threshold is reached
                // Keep showing last valid state (or blank on first connection)

                if ( switchState.noReplyCount >= MAX_POLL_ERRORS ) {
                    // Hit error threshold
                    switchState.noReplyCount = 0;  // Reset counter
                    switchState.currentTallyState = "NO_INIT";
                    switchState.lastTallyState = "NO_TALLY";

                    #if HAS_PERIPHERAL_MODE_CAPABILITY
                    // Set Grove to error state (threshold reached)
                    if ( grovePort ) {
                        grovePort->setTallyState( TallyState::ERROR );
                    }
                    #endif

                    if ( ops.cameraOperatorMode ) {
                        // Camera operator mode: Show purple X (big purple X)
                        const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_BX );
                        display->drawGlyph( xGlyph, StandardColors::PURPLE, StandardColors::BLACK, Config::Display::SHOW );
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

                #if HAS_PERIPHERAL_MODE_CAPABILITY
                // Set Grove to error state (unknown error)
                if ( grovePort ) {
                    grovePort->setTallyState( TallyState::ERROR );
                }
                #endif

                if ( ops.cameraOperatorMode ) {
                    // Camera operator mode: Show red X
                    const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_BX );
                    display->drawGlyph( xGlyph, StandardColors::RED, StandardColors::BLACK, Config::Display::SHOW );
                    log_e( "Unknown error - showing red 'X'" );
                }
                else {
                    // Talent mode: Show preview with power pixel
                    systemState->getTallyState().setState( TallyState::PREVIEW );
                }
            }
        }
    }

    void STACApp::handleFactoryReset() {
        log_i( "Performing factory reset" );

        // Print factory reset notification to serial
        Utils::InfoPrinter::printReset();

        // Clear all NVS data using ConfigManager
        if ( !configManager->clearAll() ) {
            log_e( "Factory reset failed - NVS clear unsuccessful" );
            return;
        }

        log_i( "Factory reset complete" );

        // Brief pause
        delay( Config::Timing::GUI_PAUSE_MS );

        // Flash display once to confirm (baseline behavior)
        // Use brightness level 1 for the flash
        uint8_t brightness = Config::Display::BRIGHTNESS_MAP[ 1 ];
        display->flash( 1, 500, brightness );

        // Park here forever showing the factory reset glyph (baseline behavior)
        // User must power cycle or press reset button to restart
        while ( true ) {
            #if defined(BUTTON_B_PIN)
                buttonB->read();
                if ( buttonB->wasReleased() ) {
                    log_i( "Button B pressed after factory reset - restarting" );
                    display->setBrightness( 0 );  // Turn off backlight before restart
                    ESP.restart();
                }
            #endif
            yield();
        }
    }

    OperatingMode STACApp::checkBootButtonSequence() {
        // If button not pressed at boot, return NORMAL (will check config later)
        if ( !button->isPressed() ) {
            return OperatingMode::NORMAL;
        }

        log_i( "Button held at boot - entering button sequence state machine" );

        // Check if device is provisioned - affects state machine behavior
        // If not provisioned: skip factory reset (device already in factory default state)
        //                     show RED glyph (user cannot proceed without provisioning)
        // If provisioned: show ORANGE glyph (warning that proceeding will modify config)
        //                 include factory reset option
        bool isProvisioned = configManager->isProvisioned();
        log_i( "Device provisioned: %s", isProvisioned ? "YES" : "NO" );

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Check current PMode setting to determine glyph/color for PMODE_PENDING state
        bool pmodeCurrentlyEnabled = configManager->loadPModeEnabled();
        log_i( "PMode currently enabled: %s", pmodeCurrentlyEnabled ? "YES" : "NO" );
        #endif

        // Button state machine timing (in milliseconds)
        static constexpr unsigned long STATE_HOLD_TIME = 2000;  // 2 seconds per state

        enum class BootButtonState {
            #if HAS_PERIPHERAL_MODE_CAPABILITY
            PMODE_PENDING,         // First state: Toggle peripheral mode (0-2 sec)
            #endif
            PROVISIONING_PENDING,  // Short hold -> unified portal (2-4 sec)
            FACTORY_RESET_PENDING  // Medium hold -> factory reset (4-6 sec)
        };

        // Determine starting state based on PMode capability
        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // PMode capable: Always start at PMODE_PENDING
        BootButtonState state = BootButtonState::PMODE_PENDING;
        #else
        // No PMode: Start at PROVISIONING_PENDING
        BootButtonState state = BootButtonState::PROVISIONING_PENDING;
        #endif

        unsigned long stateArmTime = millis() + STATE_HOLD_TIME;
        bool sequenceExit = false;
        OperatingMode resultMode = OperatingMode::NORMAL;

        // Get glyph indices
        using namespace Display;

        // Get all glyphs we'll need
        const uint8_t *cfgGlyph = glyphManager->getGlyph( Display::GLF_CFG );
        const uint8_t *udGlyph = glyphManager->getGlyph( Display::GLF_UD );
        #if HAS_PERIPHERAL_MODE_CAPABILITY
        const uint8_t *pGlyph = glyphManager->getGlyph( Display::GLF_P );
        const uint8_t *nGlyph = glyphManager->getGlyph( Display::GLF_N );
        #endif

        // Show initial glyph based on starting state
        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // PMode capable: Show P or N glyph
        // - If PMode disabled: Show [P] in GREEN (action: enable PMode)
        // - If PMode enabled: Show [N] in GREEN (action: disable PMode → Normal mode)
        if ( pmodeCurrentlyEnabled ) {
            display->drawGlyph( nGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
            delay( 250 );
            for ( int i = 0; i < 4; i++ ) {
                display->clear( Config::Display::SHOW );
                delay( 125 );
                display->drawGlyph( nGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
                delay( 125 );
            }
        }
        else {
            display->drawGlyph( pGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
            delay( 250 );
            for ( int i = 0; i < 4; i++ ) {
                display->clear( Config::Display::SHOW );
                delay( 125 );
                display->drawGlyph( pGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
                delay( 125 );
            }
        }
        #else
        // No PMode capability: Show provisioning glyph based on provisioned state
        auto color = isProvisioned ? Display::StandardColors::ORANGE : Display::StandardColors::RED;
        display->drawGlyph( cfgGlyph, color, Display::StandardColors::BLACK, Config::Display::SHOW );
        delay( 250 );
        for ( int i = 0; i < 4; i++ ) {
            display->clear( Config::Display::SHOW );
            delay( 125 );
            display->drawGlyph( cfgGlyph, color, Display::StandardColors::BLACK, Config::Display::SHOW );
            delay( 125 );
        }
        #endif

        // Button state machine loop
        while ( !sequenceExit ) {
            button->read();

            switch ( state ) {
                #if HAS_PERIPHERAL_MODE_CAPABILITY
                case BootButtonState::PMODE_PENDING:
                    if ( !button->isPressed() ) {
                        // Released - toggle PMode setting
                        bool newPModeState = !pmodeCurrentlyEnabled;
                        configManager->savePModeEnabled( newPModeState );
                        log_i( "Boot button sequence: PMODE toggled to %s", newPModeState ? "ENABLED" : "DISABLED" );
                        
                        if ( newPModeState ) {
                            // PMode now enabled - return PERIPHERAL mode
                            resultMode = OperatingMode::PERIPHERAL;
                        }
                        else {
                            // PMode now disabled - show checkmark confirmation then return to NORMAL
                            delay( Config::Timing::GUI_PAUSE_MS );
                            const uint8_t *checkGlyph = glyphManager->getGlyph( Display::GLF_CK );
                            display->drawGlyph( checkGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
                            delay( Config::Timing::GUI_PAUSE_MS );
                            display->clear( Config::Display::SHOW );
                            resultMode = OperatingMode::NORMAL;
                        }
                        sequenceExit = true;
                    }
                    else if ( millis() >= stateArmTime ) {
                        // Held long enough - advance to provisioning state
                        log_v( "Advancing from PMODE_PENDING to PROVISIONING_PENDING state" );

                        // Show provisioning glyph (gear icon)
                        // Use ORANGE if provisioned, RED if not (different from old behavior)
                        auto color = isProvisioned ? Display::StandardColors::ORANGE : Display::StandardColors::RED;
                        display->drawGlyph( cfgGlyph, color, Display::StandardColors::BLACK, Config::Display::SHOW );
                        delay( 250 );
                        for ( int i = 0; i < 4; i++ ) {
                            display->clear( Config::Display::SHOW );
                            delay( 125 );
                            display->drawGlyph( cfgGlyph, color, Display::StandardColors::BLACK, Config::Display::SHOW );
                            delay( 125 );
                        }
                        state = BootButtonState::PROVISIONING_PENDING;
                        stateArmTime = millis() + STATE_HOLD_TIME;
                    }
                    break;
                #endif

                case BootButtonState::PROVISIONING_PENDING:
                    if ( !button->isPressed() ) {
                        // Released - enter unified portal mode
                        log_i( "Boot button sequence: UNIFIED PORTAL selected (provisioning/OTA)" );
                        resultMode = OperatingMode::PROVISIONING;
                        sequenceExit = true;
                    }
                    else if ( millis() >= stateArmTime ) {
                        // Held long enough - advance to factory reset state
                        log_v( "Advancing to FACTORY_RESET_PENDING state" );

                        // GLF_FR (factory reset icon) in red on black background
                        const uint8_t *frGlyph = glyphManager->getGlyph( Display::GLF_FR );
                        display->drawGlyph( frGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, Config::Display::SHOW );
                        delay( 250 );

                        // Flash to show state change
                        for ( int i = 0; i < 4; i++ ) {
                            display->clear( Config::Display::SHOW );
                            delay( 125 );
                            display->drawGlyph( frGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, Config::Display::SHOW );
                            delay( 125 );
                        }

                        state = BootButtonState::FACTORY_RESET_PENDING;
                        stateArmTime = millis() + STATE_HOLD_TIME;
                    }
                    break;

                case BootButtonState::FACTORY_RESET_PENDING:
                    if ( !button->isPressed() ) {
                        // Released - perform factory reset
                        log_i( "Boot button sequence: FACTORY RESET selected" );
                        handleFactoryReset();
                        // Never returns - ESP32 restarts
                    }
                    // If button stays pressed beyond this state, just wait for release
                    // (no more states after factory reset)
                    break;
            }
        }

        return resultMode;
    }

} // namespace Application


//  --- EOF --- //
