#include <Arduino.h>
#include "Application/STACApp.h"

using namespace Application;

// The application
std::unique_ptr<STACApp> app;

void setup() {
    Serial.begin( 115200 );
    
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    // ESP32-S3 with USB CDC - wait for serial connection
    while (!Serial && millis() < 3000) {
        delay(10);
    }
    #endif
    
    delay( 1000 );

    // Create and initialize application
    app = std::make_unique<STACApp>();

    if ( !app->setup() ) {
        Serial.println( "\n❌ STAC initialization failed!" );
        Serial.println( "System halted." );
        while ( true ) {
            delay( 1000 );
        }
    }
}

void loop() {
    app->loop();
}


//  --- EOF --- //
