#include "Hardware/Input/ButtonHandler.h"
#include <Arduino.h>

namespace STAC {
    namespace Hardware {

        ButtonHandler::ButtonHandler( uint8_t pin, bool activeLow,
                                      unsigned long debounceMs,
                                      unsigned long longPressMs )
            : pin( pin )
            , activeLow( activeLow )
            , debounceMs( debounceMs )
            , longPressMs( longPressMs )
            , currentState( false )
            , lastState( false )
            , clickFlag( false )
            , longPressFlag( false )
            , pressStartTime( 0 )
            , lastChangeTime( 0 )
            , eventCallback( nullptr ) {
        }

        bool ButtonHandler::begin() {
            // Configure pin
            // GPIO39 on ESP32 is input-only and cannot use internal pull-up
            #if defined(BUTTON_NEEDS_EXTERNAL_PULLUP) && BUTTON_NEEDS_EXTERNAL_PULLUP
                pinMode( pin, INPUT ); // Use INPUT without pullup
                log_i( "Button initialized on pin %d (active %s, EXTERNAL PULLUP required)",
                    pin, activeLow ? "LOW" : "HIGH" );
            #else
                pinMode( pin, activeLow ? INPUT_PULLUP : INPUT );
                log_i( "Button initialized on pin %d (active %s)",
                    pin, activeLow ? "LOW" : "HIGH" );
            #endif

            // Read initial state
            currentState = readRawState();
            lastState = currentState;
            lastChangeTime = millis();

            return true;
        }
        void ButtonHandler::update() {
            bool rawState = readRawState();
            unsigned long now = millis();

            // Debouncing: only update if state has been stable
            if ( rawState != lastState ) {
                lastChangeTime = now;
                lastState = rawState;
            }

            // Check if debounce period has elapsed
            if ( ( now - lastChangeTime ) > debounceMs ) {
                bool newState = rawState;

                // State changed after debounce
                if ( newState != currentState ) {
                    currentState = newState;

                    if ( currentState ) {
                        // Button pressed
                        pressStartTime = now;
                        longPressFlag = false;

                        if ( eventCallback ) {
                            eventCallback( ButtonEvent::PRESSED );
                        }
                    }
                    else {
                        // Button released
                        unsigned long pressDuration = now - pressStartTime;

                        // Check for long press
                        if ( pressDuration >= longPressMs && !longPressFlag ) {
                            longPressFlag = true;
                            if ( eventCallback ) {
                                eventCallback( ButtonEvent::LONG_PRESS );
                            }
                        }
                        // Regular click
                        else if ( pressDuration < longPressMs ) {
                            clickFlag = true;
                            if ( eventCallback ) {
                                eventCallback( ButtonEvent::CLICK );
                            }
                        }

                        if ( eventCallback ) {
                            eventCallback( ButtonEvent::RELEASED );
                        }

                        pressStartTime = 0;
                    }
                }
                // Button held - check for long press
                else if ( currentState && !longPressFlag ) {
                    unsigned long pressDuration = now - pressStartTime;
                    if ( pressDuration >= longPressMs ) {
                        longPressFlag = true;
                        if ( eventCallback ) {
                            eventCallback( ButtonEvent::LONG_PRESS );
                        }
                    }
                }
            }
        }

        bool ButtonHandler::isPressed() const {
            return currentState;
        }

        bool ButtonHandler::wasClicked() {
            bool result = clickFlag;
            clickFlag = false;  // Clear flag on read
            return result;
        }

        bool ButtonHandler::isLongPress() const {
            if ( !currentState ) {
                return false;
            }

            unsigned long pressDuration = millis() - pressStartTime;
            return pressDuration >= longPressMs;
        }

        unsigned long ButtonHandler::getPressedDuration() const {
            if ( !currentState || pressStartTime == 0 ) {
                return 0;
            }

            return millis() - pressStartTime;
        }

        void ButtonHandler::setEventCallback( std::function<void( ButtonEvent )> callback ) {
            eventCallback = callback;
        }

        bool ButtonHandler::readRawState() const {
            bool state = digitalRead( pin );

            // Invert if active low
            if ( activeLow ) {
                state = !state;
            }

            return state;
        }

    } // namespace Hardware
} // namespace STAC


//  --- EOF --- //
