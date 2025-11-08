#include <Arduino.h>
#include "Device_Config.h"
#include "Config/Constants.h"
#include "Config/Types.h"
#include "Hardware/Display/DisplayFactory.h"
#include "Hardware/Display/Colors.h"
#include "Hardware/Sensors/IMUFactory.h"

using namespace STAC;
using namespace STAC::Display;
using namespace STAC::Hardware;

// Create hardware objects
std::unique_ptr<IDisplay> display;
std::unique_ptr<IIMU> imu;

void setup() {
    Serial.begin( 115200 );
    delay( 1000 ); // Give time for USB CDC

    Serial.println( "\n=== STAC Phase 3: IMU Test ===" );
    Serial.printf( "Board: %s\n", Config::Strings::BOARD_NAME );

    // Initialize display
    Serial.printf( "Display Type: %s\n", DisplayFactory::getDisplayType() );
    display = DisplayFactory::create();

    if ( !display->begin() ) {
        Serial.println( "ERROR: Failed to initialize display! - Halting program." );
        while ( true ) {
            delay( 1000 );
        }
    }
    Serial.println( "Display initialized successfully!" );

    // Initialize IMU
    Serial.printf( "IMU Type: %s\n", IMUFactory::getIMUType() );
    Serial.printf( "Has IMU: %s\n", IMUFactory::hasIMU() ? "Yes" : "No" );

    imu = IMUFactory::create();

    if ( !imu->begin() ) {
        Serial.println( "WARNING: Failed to initialize IMU (may not be present)" );
    }
    else {
        Serial.println( "IMU initialized successfully!" );
    }

    // Test orientation detection
    if ( imu->isAvailable() ) {
        Serial.println( "\nTesting orientation detection..." );
        Serial.println( "Rotate the device to see orientation changes:" );

        for ( int i = 0; i < 10; i++ ) {
            Orientation orient = imu->getOrientation();

            const char *orientStr = "UNKNOWN";
            switch ( orient ) {
                case Orientation::UP:
                    orientStr = "UP";
                    break;
                case Orientation::DOWN:
                    orientStr = "DOWN";
                    break;
                case Orientation::LEFT:
                    orientStr = "LEFT";
                    break;
                case Orientation::RIGHT:
                    orientStr = "RIGHT";
                    break;
                case Orientation::FLAT:
                    orientStr = "FLAT";
                    break;
                default:
                    break;
            }

            Serial.printf( "  Orientation %d/10: %s\n", i + 1, orientStr );

            // Show orientation on display
            display->clear( false );

            // Draw a simple indicator based on orientation
            uint8_t centerPixel = Config::Display::POWER_LED_PIXEL;
            display->setPixel( centerPixel, STACColors::POWER_ON, false );

            // Add directional indicator
            switch ( orient ) {
                case Orientation::UP:
                    // Light up top pixel
                    display->setPixel( centerPixel - display->getWidth(),
                                       StandardColors::GREEN, false );
                    break;
                case Orientation::DOWN:
                    // Light up bottom pixel
                    display->setPixel( centerPixel + display->getWidth(),
                                       StandardColors::GREEN, false );
                    break;
                case Orientation::LEFT:
                    // Light up left pixel
                    display->setPixel( centerPixel - 1,
                                       StandardColors::GREEN, false );
                    break;
                case Orientation::RIGHT:
                    // Light up right pixel
                    display->setPixel( centerPixel + 1,
                                       StandardColors::GREEN, false );
                    break;
                case Orientation::FLAT:
                    // Light up all around center
                    display->fill( StandardColors::DARK_BLUE, false );
                    display->setPixel( centerPixel, STACColors::POWER_ON, false );
                    break;
                default:
                    // Unknown - pulse red
                    display->setPixel( centerPixel, StandardColors::RED, false );
                    break;
            }

            display->show();
            delay( 1000 );
        }
    }
    else {
        Serial.println( "\nIMU not available - skipping orientation tests" );
    }

    Serial.println( "\n=== Phase 3 IMU Tests Complete! ===" );

    // Leave display with power indicator
    display->clear( false );
    display->setPixel( Config::Display::POWER_LED_PIXEL, STACColors::POWER_ON, true );
}

void loop() {
    // Show current orientation continuously
    if ( imu->isAvailable() ) {
        static unsigned long lastUpdate = 0;
        static Orientation lastOrient = Orientation::UNKNOWN;

        if ( millis() - lastUpdate > 500 ) {
            lastUpdate = millis();
            Orientation currentOrient = imu->getOrientation();

            // Only update if orientation changed
            if ( currentOrient != lastOrient ) {
                lastOrient = currentOrient;

                const char *orientStr = "UNKNOWN";
                switch ( currentOrient ) {
                    case Orientation::UP:
                        orientStr = "UP";
                        break;
                    case Orientation::DOWN:
                        orientStr = "DOWN";
                        break;
                    case Orientation::LEFT:
                        orientStr = "LEFT";
                        break;
                    case Orientation::RIGHT:
                        orientStr = "RIGHT";
                        break;
                    case Orientation::FLAT:
                        orientStr = "FLAT";
                        break;
                    default:
                        break;
                }

                Serial.printf( "Orientation changed: %s\n", orientStr );
            }
        }
    }
}


//  --- EOF --- //
