#include <Arduino.h>
#include "Device_Config.h"
#include "Config/Constants.h"
#include "Config/Types.h"
#include "Hardware/Display/DisplayFactory.h"
#include "Hardware/Display/Colors.h"

using namespace STAC;
using namespace STAC::Display;

// Create display object
std::unique_ptr<IDisplay> display;

void setup() {
    Serial.begin( 115200 );
    delay( 1000 ); // Give time for USB CDC

    Serial.println( "\n=== STAC Phase 2: Display Test ===" );
    Serial.printf( "Board: %s\n", Config::Strings::BOARD_NAME );
    Serial.printf( "Display Type: %s\n", DisplayFactory::getDisplayType() );

    // Create display using factory
    display = DisplayFactory::create();

    if ( !display->begin() ) {
        Serial.println( "ERROR: Failed to initialize display!" );
        while ( true ) {
            delay( 1000 );
        }
    }

    Serial.println( "Display initialized successfully!" );
    Serial.printf( "Display size: %dx%d (%d pixels)\n",
                   display->getWidth(),
                   display->getHeight(),
                   display->getPixelCount() );

    // Test 1: Fill with different colors
    Serial.println( "\nTest 1: Color fills" );

    Serial.println( "  RED..." );
    display->fill( STACColors::ALERT, true );
    delay( 1000 );

    Serial.println( "  GREEN..." );
    display->fill( STACColors::GTG, true );
    delay( 1000 );

    Serial.println( "  BLUE..." );
    display->fill( STACColors::POWER_ON, true );
    delay( 1000 );

    Serial.println( "  PURPLE..." );
    display->fill( STACColors::UNSELECTED, true );
    delay( 1000 );

    // Test 2: Individual pixels
    Serial.println( "\nTest 2: Individual pixels" );
    display->clear( true );
    delay( 500 );

    // Light up corner pixels
    display->setPixel( 0, StandardColors::RED, false ); // Top-left
    display->setPixel( display->getWidth() - 1, StandardColors::GREEN, false ); // Top-right
    display->setPixel( display->getPixelCount() - display->getWidth(), StandardColors::BLUE, false ); // Bottom-left
    display->setPixel( display->getPixelCount() - 1, StandardColors::YELLOW, false ); // Bottom-right
    display->show();
    delay( 2000 );

    // Test 3: Center pixel (power indicator position)
    Serial.println( "\nTest 3: Power indicator pixel" );
    display->clear( false );
    display->setPixel( Config::Display::POWER_LED_PIXEL, STACColors::POWER_ON, true );
    delay( 2000 );

    // Test 4: Brightness
    Serial.println( "\nTest 4: Brightness levels" );
    display->fill( StandardColors::WHITE, true );

    for ( uint8_t brightness = 0; brightness <= 40; brightness += 10 ) {
        Serial.printf( "  Brightness: %d\n", brightness );
        display->setBrightness( brightness, true );
        delay( 500 );
    }

    // Test 5: XY coordinates
    Serial.println( "\nTest 5: XY coordinate test" );
    display->clear( true );
    delay( 500 );

    // Draw a diagonal line
    for ( uint8_t i = 0; i < display->getWidth() && i < display->getHeight(); i++ ) {
        display->setPixelXY( i, i, StandardColors::TEAL, false );
    }
    display->show();
    delay( 2000 );

    Serial.println( "\n=== All Display Tests Passed! ===" );
    Serial.println( "Phase 2 display abstraction is working!" );

    // Leave display with power indicator on
    display->clear( false );
    display->setPixel( Config::Display::POWER_LED_PIXEL, STACColors::POWER_ON, true );
}

void loop() {
    // Pulse the power indicator
    static unsigned long lastPulse = 0;
    static bool pulseState = true;

    if ( millis() - lastPulse > 2000 ) {
        lastPulse = millis();
        pulseState = !pulseState;

        if ( pulseState ) {
            display->setPixel( Config::Display::POWER_LED_PIXEL, STACColors::POWER_ON, true );
        }
        else {
            display->setPixel( Config::Display::POWER_LED_PIXEL, StandardColors::DARK_BLUE, true );
        }
    }
}


//  --- EOF --- //
