#include <Arduino.h>
#include "Device_Config.h"
#include "Config/Constants.h"
#include "Config/Types.h"
#include "Hardware/Display/DisplayFactory.h"
#include "Hardware/Display/Colors.h"
#include "Hardware/Sensors/IMUFactory.h"
#include "Hardware/Input/ButtonFactory.h"
#include "Hardware/Interface/InterfaceFactory.h"

using namespace STAC;
using namespace STAC::Display;
using namespace STAC::Hardware;

// Create hardware objects
std::unique_ptr<IDisplay> display;
std::unique_ptr<IIMU> imu;
std::unique_ptr<IButton> button;
std::unique_ptr<GrovePort> grovePort;
std::unique_ptr<PeripheralMode> peripheralDetector;

// State tracking
TallyState currentTallyState = TallyState::NO_TALLY;
bool isPeripheralMode = false;
int tallyStateIndex = 0;

void setup() {
    Serial.begin( 115200 );
    delay( 1000 );

    Serial.println( "\n=== STAC Phase 4: Hardware Interfaces Test ===" );
    Serial.printf( "Board: %s\n", Config::Strings::BOARD_NAME );

    // Initialize display
    Serial.printf( "\nDisplay Type: %s\n", DisplayFactory::getDisplayType() );
    display = DisplayFactory::create();

    if ( !display->begin() ) {
        Serial.println( "ERROR: Failed to initialize display!" );
        while ( true ) {
            delay( 1000 );
        }
    }
    Serial.println( "✓ Display initialized" );

    // Initialize IMU
    Serial.printf( "\nIMU Type: %s\n", IMUFactory::getIMUType() );
    imu = IMUFactory::create();

    if ( !imu->begin() ) {
        Serial.println( "⚠ IMU not initialized" );
    }
    else {
        Serial.println( "✓ IMU initialized" );
    }

    // Detect peripheral mode
    Serial.println( "\nChecking for peripheral mode..." );
    peripheralDetector = InterfaceFactory::createPeripheralDetector();
    isPeripheralMode = peripheralDetector->detect();

    if ( isPeripheralMode ) {
        Serial.println( "✓ PERIPHERAL MODE DETECTED" );
        Serial.println( "  GROVE port will be used as INPUT" );
    }
    else {
        Serial.println( "✓ NORMAL MODE" );
        Serial.println( "  GROVE port will be used as OUTPUT" );
    }

    // Initialize GROVE port
    Serial.println( "\nInitializing GROVE port..." );
    grovePort = InterfaceFactory::createGrovePort( !isPeripheralMode );
    Serial.println( "✓ GROVE port initialized" );

    // Initialize button
    Serial.println( "\nInitializing button..." );
    button = ButtonFactory::create();

    if ( !button->begin() ) {
        Serial.println( "ERROR: Failed to initialize button!" );
        while ( true ) {
            delay( 1000 );
        }
    }
    Serial.println( "✓ Button initialized" );

    Serial.println( "\n=== Phase 4 Test Instructions ===" );
    if ( isPeripheralMode ) {
        Serial.println( "PERIPHERAL MODE:" );
        Serial.println( "  - Reading tally state from GROVE port" );
        Serial.println( "  - Display shows received tally state" );
    }
    else {
        Serial.println( "NORMAL MODE:" );
        Serial.println( "  - Button press cycles tally states:" );
        Serial.println( "    1. NO_TALLY (Purple)" );
        Serial.println( "    2. PREVIEW (Green)" );
        Serial.println( "    3. PROGRAM (Red)" );
        Serial.println( "    4. UNSELECTED (Blue)" );
        Serial.println( "  - States sent to GROVE port pins" );
    }

    // Show ready state
    display->clear( false );
    display->setPixel( Config::Display::POWER_LED_PIXEL,
                       STACColors::POWER_ON, true );

    Serial.println( "\n=== Setup Complete! ===\n" );
}

void loop() {
    button->update();

    if ( isPeripheralMode ) {
        // PERIPHERAL MODE: Read tally state from GROVE port
        static unsigned long lastRead = 0;
        if ( millis() - lastRead > 100 ) { // Read every 100ms
            lastRead = millis();

            TallyState readState = grovePort->readTallyState();

            if ( readState != currentTallyState ) {
                currentTallyState = readState;

                const char *stateStr = "UNKNOWN";
                color_t stateColor = StandardColors::PURPLE;

                switch ( currentTallyState ) {
                    case TallyState::PROGRAM:
                        stateStr = "PROGRAM";
                        stateColor = STACColors::PROGRAM;
                        break;
                    case TallyState::PREVIEW:
                        stateStr = "PREVIEW";
                        stateColor = STACColors::PREVIEW;
                        break;
                    case TallyState::UNSELECTED:
                        stateStr = "UNSELECTED";
                        stateColor = STACColors::UNSELECTED;
                        break;
                    case TallyState::NO_TALLY:
                        stateStr = "NO_TALLY";
                        stateColor = StandardColors::PURPLE;
                        break;
                    default:
                        break;
                }

                Serial.printf( "Received tally state: %s\n", stateStr );
                display->fill( stateColor, true );
            }
        }

    }
    else {
        // NORMAL MODE: Button cycles tally states
        if ( button->wasClicked() ) {
            // Cycle through tally states
            tallyStateIndex = ( tallyStateIndex + 1 ) % 4;

            TallyState states[] = {
                TallyState::NO_TALLY,
                TallyState::PREVIEW,
                TallyState::PROGRAM,
                TallyState::UNSELECTED
            };

            currentTallyState = states[ tallyStateIndex ];

            const char *stateStr = "UNKNOWN";
            color_t stateColor = StandardColors::PURPLE;

            switch ( currentTallyState ) {
                case TallyState::PROGRAM:
                    stateStr = "PROGRAM";
                    stateColor = STACColors::PROGRAM;
                    break;
                case TallyState::PREVIEW:
                    stateStr = "PREVIEW";
                    stateColor = STACColors::PREVIEW;
                    break;
                case TallyState::UNSELECTED:
                    stateStr = "UNSELECTED";
                    stateColor = STACColors::UNSELECTED;
                    break;
                case TallyState::NO_TALLY:
                    stateStr = "NO_TALLY";
                    stateColor = StandardColors::PURPLE;
                    break;
                default:
                    break;
            }

            Serial.printf( "Tally state: %s\n", stateStr );

            // Send to GROVE port
            grovePort->setTallyState( currentTallyState );

            // Show on display
            display->fill( stateColor, true );

            delay( 300 ); // Visual feedback
        }

        // Show button state
        if ( button->isPressed() && !button->isLongPress() ) {
            display->setPixel( Config::Display::POWER_LED_PIXEL,
                               StandardColors::WHITE, true );
        }
    }

    delay( 10 );
}


//  --- EOF --- //
