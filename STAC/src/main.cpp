#include <Arduino.h>
#include "Application/STACApp.h"

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
    // Immediately blink LED to show we're alive
    #if defined(DEBUG_LED_PIN)
        pinMode(DEBUG_LED_PIN, OUTPUT);
        // Rapid blink 3 times to show boot started
        for (int i = 0; i < 3; i++) {
            digitalWrite(DEBUG_LED_PIN, DEBUG_LED_ON);
            delay(100);
            digitalWrite(DEBUG_LED_PIN, DEBUG_LED_OFF);
            delay(100);
        }
    #endif
    
    Serial.begin( 115200 );
    
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    // ESP32-S3 with USB CDC - wait for serial connection
    while (!Serial && millis() < 3000) {
        delay(10);
    }
    #endif
    
    Serial.println("\n\n=== STAC M5StickC Plus Boot ===");
    
    delay( 1000 );

    // Create and initialize application
    Serial.println("Creating STACApp...");
    #if defined(DEBUG_LED_PIN)
        digitalWrite(DEBUG_LED_PIN, DEBUG_LED_ON);  // LED on during init
    #endif
    
    app = std::make_unique<STACApp>();

    Serial.println("Calling app->setup()...");
    if ( !app->setup() ) {
        Serial.println( "\n❌ STAC initialization failed!" );
        Serial.println( "System halted." );
        // Error blink - rapid continuous
        while ( true ) {
            #if defined(DEBUG_LED_PIN)
                digitalWrite(DEBUG_LED_PIN, DEBUG_LED_ON);
                delay(50);
                digitalWrite(DEBUG_LED_PIN, DEBUG_LED_OFF);
                delay(50);
            #else
                delay( 1000 );
            #endif
        }
    }
    
    // Success - LED off (will be managed by app)
    #if defined(DEBUG_LED_PIN)
        digitalWrite(DEBUG_LED_PIN, DEBUG_LED_OFF);
    #endif
    Serial.println("Setup complete!");
}

void loop() {
    app->loop();
}


//  --- EOF --- //
