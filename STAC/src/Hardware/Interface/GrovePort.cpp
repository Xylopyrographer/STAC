#include "Hardware/Interface/GrovePort.h"
#include <Arduino.h>


    namespace Hardware {

        GrovePort::GrovePort( uint8_t pin0, uint8_t pin1 )
            : pin0( pin0 )
            , pin1( pin1 )
            , isOutputMode( true ) {
        }

        bool GrovePort::begin( bool asOutput ) {
            isOutputMode = asOutput;

            if ( isOutputMode ) {
                pinMode( pin0, OUTPUT );
                pinMode( pin1, OUTPUT );

                // Initialize to NO_TALLY state
                setTallyState( TallyState::NO_TALLY );

                log_i( "GROVE port initialized as OUTPUT (TS_0=%d, TS_1=%d)", pin0, pin1 );
            }
            else {
                // Use INPUT_PULLDOWN for peripheral mode to prevent floating inputs
                // Floating inputs would read random values; pulldown ensures 00 (NO_TALLY)
                // when no external device is connected, displaying the orange X
                pinMode( pin0, INPUT_PULLDOWN );
                pinMode( pin1, INPUT_PULLDOWN );

                log_i( "GROVE port initialized as INPUT_PULLDOWN (TS_0=%d, TS_1=%d)", pin0, pin1 );
            }

            return true;
        }

        void GrovePort::setTallyState( TallyState state ) {
            if ( !isOutputMode ) {
                log_w( "GROVE port in INPUT mode, cannot set state" );
                return;
            }

            // Encoding:
            // PROGRAM:     TS_1=HIGH, TS_0=HIGH
            // PREVIEW:     TS_1=HIGH, TS_0=LOW
            // UNSELECTED:  TS_1=LOW,  TS_0=HIGH
            // NO_TALLY:    TS_1=LOW,  TS_0=LOW

            switch ( state ) {
                case TallyState::PROGRAM:
                    digitalWrite( pin1, HIGH );
                    digitalWrite( pin0, HIGH );
                    break;

                case TallyState::PREVIEW:
                    digitalWrite( pin1, HIGH );
                    digitalWrite( pin0, LOW );
                    break;

                case TallyState::UNSELECTED:
                    digitalWrite( pin1, LOW );
                    digitalWrite( pin0, HIGH );
                    break;

                case TallyState::NO_TALLY:
                case TallyState::ERROR:
                default:
                    digitalWrite( pin1, LOW );
                    digitalWrite( pin0, LOW );
                    break;
            }
        }

        TallyState GrovePort::readTallyState() {
            if ( isOutputMode ) {
                log_w( "GROVE port in OUTPUT mode, cannot read state" );
                return TallyState::ERROR;
            }

            bool state0 = digitalRead( pin0 );
            bool state1 = digitalRead( pin1 );

            // Decode based on pin states
            if ( state1 && state0 ) {
                return TallyState::PROGRAM;
            }
            else if ( state1 && !state0 ) {
                return TallyState::PREVIEW;
            }
            else if ( !state1 && state0 ) {
                return TallyState::UNSELECTED;
            }
            else {
                return TallyState::NO_TALLY;
            }
        }

        void GrovePort::setPins( bool pin0State, bool pin1State ) {
            if ( !isOutputMode ) {
                log_w( "GROVE port in INPUT mode, cannot set pins" );
                return;
            }

            digitalWrite( pin0, pin0State ? HIGH : LOW );
            digitalWrite( pin1, pin1State ? HIGH : LOW );
        }

        void GrovePort::readPins( bool& pin0State, bool& pin1State ) {
            pin0State = digitalRead( pin0 );
            pin1State = digitalRead( pin1 );
        }

        void GrovePort::configurePinsAsInputs() {
            // Configure pins as inputs with pulldowns for peripheral mode
            // This matches baseline behavior in STACPeripheral.h lines 54-55
            pinMode( pin0, INPUT_PULLDOWN );
            pinMode( pin1, INPUT_PULLDOWN );
            isOutputMode = false;
            log_i( "GROVE port configured as INPUT_PULLDOWN (TS_0=%d, TS_1=%d)", pin0, pin1 );
        }

    } // namespace Hardware



//  --- EOF --- //
