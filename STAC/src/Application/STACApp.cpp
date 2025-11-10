#include "Application/STACApp.h"
#include "Hardware/Display/DisplayFactory.h"
#include "Hardware/Sensors/IMUFactory.h"
#include "Hardware/Input/ButtonFactory.h"
#include "Hardware/Interface/InterfaceFactory.h"
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
            , lastOrientation( Orientation::UNKNOWN ) {
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

            // Initial display update
            updateDisplay();

            return true;
        }

        void STACApp::loop() {
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

            // Mode-specific handling
            switch ( systemState->getOperatingMode().getCurrentMode() ) {
                case OperatingMode::NORMAL:
                    handleNormalMode();
                    break;

                case OperatingMode::PERIPHERAL:
                    handlePeripheralMode();
                    break;

                case OperatingMode::PROVISIONING:
                    // Skip provisioning for now
                    // handleProvisioningMode();
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

            // For now, always start in NORMAL mode for testing
            // TODO: Check configuration and decide if provisioning is needed
            log_i( "Starting in NORMAL mode" );
            return OperatingMode::NORMAL;

            /* Original code - uncomment when ready for provisioning
            // Check if configured
            if (!configManager->hasWiFiCredentials()) {
                log_i("No configuration found, entering provisioning mode");
                return OperatingMode::PROVISIONING;
            }

            return OperatingMode::NORMAL;
            */
        }

        void STACApp::handleButton() {
            // For now, simple tally state cycling for demo
            if ( button->wasClicked() ) {
                // Cycle through states
                TallyState currentState = systemState->getTallyState().getCurrentState();
                TallyState newState;

                switch ( currentState ) {
                    case TallyState::NO_TALLY:
                        newState = TallyState::PREVIEW;
                        break;
                    case TallyState::PREVIEW:
                        newState = TallyState::PROGRAM;
                        break;
                    case TallyState::PROGRAM:
                        newState = TallyState::UNSELECTED;
                        break;
                    case TallyState::UNSELECTED:
                        newState = TallyState::NO_TALLY;
                        break;
                    default:
                        newState = TallyState::NO_TALLY;
                        break;
                }

                systemState->getTallyState().setState( newState );
            }

            // Disable provisioning mode for now
            // Long press - enter provisioning mode
            // if ( button->isLongPress() ) {
            //     static bool longPressHandled = false;

            //     if ( !longPressHandled ) {
            //         longPressHandled = true;
            //         log_i( "Long press detected - entering provisioning mode" );
            //         systemState->getOperatingMode().setMode( OperatingMode::PROVISIONING );

            //         // Start AP for configuration
            //         wifiManager->startAP( stacID, "" );

            //         // Visual feedback
            //         display->fill( Display::StandardColors::YELLOW, true );
            //         delay( 500 );
            //         updateDisplay();
            //     }

            //     if ( !button->isPressed() ) {
            //         longPressHandled = false;
            //     }
            // }
        }

        void STACApp::handleOrientation() {
            Orientation currentOrientation = imu->getOrientation();

            if ( currentOrientation != lastOrientation &&
                    currentOrientation != Orientation::UNKNOWN ) {

                lastOrientation = currentOrientation;

                // TODO: Rotate glyphs based on orientation
                // This will be implemented when we add glyph management

                log_d( "Orientation: %d", static_cast<int>( currentOrientation ) );
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
            
            // [RJL] Commented out this whole thing to prevent repeated connection attempts flooding the serial monitor
            
            // TODO: Implement Roland switch polling
            // For now, tally state is controlled by button

            // // Only try to connect if we have credentials and aren't already connected
            // static bool wifiAttempted = false;

            // if ( !wifiAttempted && !wifiManager->isConnected() && configManager->hasWiFiCredentials() ) {
            //     wifiAttempted = true;  // Only try once

            //     String ssid, password;
            //     if ( configManager->loadWiFiCredentials( ssid, password ) ) {
            //         log_i( "Attempting to connect to WiFi: %s", ssid.c_str() );
            //         wifiManager->connect( ssid, password );
            //     }
            // }
            // // If connection failed, WiFiManager will handle auto-reconnect
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
            // Ensure AP is running
            // [RJL] Following 3 line commented out to avoid starting AP repeatedly 
            // if ( !wifiManager->isAPMode() ) {
            //     wifiManager->startAP( stacID, "" );
            // }

            // TODO: Implement web server for configuration
            // For now, just pulse the display
            static unsigned long lastPulse = 0;
            static bool pulseState = false;

            if ( millis() - lastPulse > 1000 ) {
                lastPulse = millis();
                pulseState = !pulseState;

                if ( pulseState ) {
                    display->fill( Display::StandardColors::TEAL, true );
                }
                else {
                    display->clear( false );
                    display->setPixel( Config::Display::POWER_LED_PIXEL,
                                       Display::StandardColors::TEAL, true );
                }
            }
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

    } // namespace Application
} // namespace STAC


//  --- EOF --- //
