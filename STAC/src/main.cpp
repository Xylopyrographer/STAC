#include <Arduino.h>
#include "Application/STACApp.h"
#include "Device_Config.h"  // For STAC_BOARD_NAME

#if defined(DISPLAY_TYPE_LED_MATRIX)
    #include <LiteLED.h>
#endif

using namespace Application;

// The application
std::unique_ptr<STACApp> app;

// Debug LED for M5StickC Plus (GPIO 10, active LOW)
#if defined(BOARD_M5STICKC_PLUS)
    #define DEBUG_LED_PIN 10
    #define DEBUG_LED_ON  LOW
    #define DEBUG_LED_OFF HIGH
#endif

void setup() {
    // IMMEDIATELY turn off TFT backlight to prevent showing stale LCD content
    // This must happen before ANYTHING else, as LCD memory persists through soft reset
    #if defined(TFT_BL)
    pinMode( TFT_BL, OUTPUT );
    digitalWrite( TFT_BL, LOW ); // Backlight OFF
    #endif

    // Note: LED matrix clearing is handled by DisplayBase::begin()
    // Early clearing causes RMT channel conflicts

    // Immediately blink LED to show we're alive
    #if defined(DEBUG_LED_PIN)
    pinMode( DEBUG_LED_PIN, OUTPUT );
    // Rapid blink 3 times to show boot started
    for ( int i = 0; i < 3; i++ ) {
        digitalWrite( DEBUG_LED_PIN, DEBUG_LED_ON );
        delay( 100 );
        digitalWrite( DEBUG_LED_PIN, DEBUG_LED_OFF );
        delay( 100 );
    }
    #endif

    Serial.begin( 115200 );

    #ifdef CONFIG_IDF_TARGET_ESP32S3
    // ESP32-S3 with USB CDC - wait for serial connection
    while ( !Serial && millis() < 3000 ) {
        delay( 10 );
    }
    #endif

    log_i( "\n\n=== STAC " STAC_BOARD_NAME " Boot ===" );

    // Create and initialize application
    log_i( "Creating STACApp..." );
    #if defined(DEBUG_LED_PIN)
    digitalWrite( DEBUG_LED_PIN, DEBUG_LED_ON ); // LED on during init
    #endif

    app = std::make_unique<STACApp>();

    log_i( "Calling app->setup()..." );
    if ( !app->setup() ) {
        Serial.println( "\n❌ STAC initialization failed!" );
        Serial.println( "System halted." );

        // Show red power glyph on LED matrix to indicate failure
        #if defined(DISPLAY_TYPE_LED_MATRIX) && defined(PIN_DISPLAY_DATA)
        // Create minimal display to show error (app may not be fully initialized)
        LiteLED errorDisplay( static_cast<led_strip_type_t>( DISPLAY_LED_TYPE ), 0 );
        #if defined(DISPLAY_MATRIX_WIDTH) && defined(DISPLAY_MATRIX_HEIGHT)
        uint8_t numLeds = DISPLAY_MATRIX_WIDTH * DISPLAY_MATRIX_HEIGHT;
        #else
        uint8_t numLeds = 25;
        #endif
        if ( errorDisplay.begin( PIN_DISPLAY_DATA, numLeds ) == 0 ) {
            // Show red pixel in center (position 0 for power glyph)
            errorDisplay.setPixel( 0, 0xFF0000, true );  // Red
        }
        #endif

        // Error blink - rapid continuous
        while ( true ) {
            #if defined(DEBUG_LED_PIN)
            digitalWrite( DEBUG_LED_PIN, DEBUG_LED_ON );
            delay( 50 );
            digitalWrite( DEBUG_LED_PIN, DEBUG_LED_OFF );
            delay( 50 );
            #else
            delay( 1000 );
            #endif
        }
    }

    // Success - LED off (will be managed by app)
    #if defined(DEBUG_LED_PIN)
    digitalWrite( DEBUG_LED_PIN, DEBUG_LED_OFF );
    #endif

    log_i( "Setup complete!" );
}

void loop() {
    app->loop();
}


//  --- EOF --- //
