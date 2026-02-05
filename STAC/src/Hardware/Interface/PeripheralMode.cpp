#include "Hardware/Interface/PeripheralMode.h"
#include <Arduino.h>


namespace Hardware {

    PeripheralMode::PeripheralMode( uint8_t outPin, uint8_t inPin, uint8_t toggleCount )
        : outPin( outPin )
        , inPin( inPin )
        , toggleCount( toggleCount )
        , peripheralModeDetected( false ) {
    }

    bool PeripheralMode::begin() {
        // Configure pins
        pinMode( outPin, OUTPUT );
        pinMode( inPin, INPUT );

        // Set output to known state
        digitalWrite( outPin, LOW );

        log_i( "Peripheral mode detector initialized (OUT=%d, IN=%d)", outPin, inPin );
        return true;
    }

    bool PeripheralMode::detect() {
        log_d( "Testing for peripheral mode jumper..." );

        // Perform multiple toggle tests to confirm connection
        uint8_t successCount = 0;

        for ( uint8_t i = 0; i < toggleCount; i++ ) {
            // Test HIGH state
            if ( testConnection( true ) ) {
                successCount++;
            }
            delayMicroseconds( 10 );

            // Test LOW state
            if ( testConnection( false ) ) {
                successCount++;
            }
            delayMicroseconds( 10 );
        }

        // Consider it connected if most tests passed
        // (toggleCount * 2 total tests, require at least 80% success)
        uint8_t requiredSuccess = ( toggleCount * 2 * 4 ) / 5; // 80%
        peripheralModeDetected = ( successCount >= requiredSuccess );

        if ( peripheralModeDetected ) {
            log_i( "Peripheral mode DETECTED (%d/%d tests passed)",
                   successCount, toggleCount * 2 );
        }
        else {
            log_i( "Peripheral mode NOT detected (%d/%d tests passed)",
                   successCount, toggleCount * 2 );
        }

        // Leave output in safe state
        digitalWrite( outPin, LOW );

        return peripheralModeDetected;
    }

    bool PeripheralMode::testConnection( bool outputState ) {
        // Set output state
        digitalWrite( outPin, outputState ? HIGH : LOW );

        // Small delay for signal to stabilize
        delayMicroseconds( 5 );

        // Read input and check if it matches
        bool inputState = digitalRead( inPin );

        return ( inputState == outputState );
    }

} // namespace Hardware


//  --- EOF --- //
