#include <Arduino.h>
#include "Application/STACApp.h"

using namespace Application;

// The application
std::unique_ptr<STACApp> app;

void setup() {
    Serial.begin( 115200 );
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
