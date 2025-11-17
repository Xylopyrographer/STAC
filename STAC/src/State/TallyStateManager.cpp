#include "State/TallyStateManager.h"
#include "Hardware/Display/Colors.h"
#include <Arduino.h>

namespace STAC {
    namespace State {

        TallyStateManager::TallyStateManager()
            : currentState( TallyState::NO_TALLY )
            , previousState( TallyState::NO_TALLY )
            , lastChangeTime( 0 )
            , callback( nullptr ) {
        }

        bool TallyStateManager::setState( TallyState newState ) {
            if ( newState == currentState ) {
                return false;  // No change
            }

            // Store previous state
            previousState = currentState;
            currentState = newState;
            lastChangeTime = millis();

            // Log state change
            log_i( "Tally state: %s -> %s",
                   stateToString( previousState ),
                   stateToString( currentState ) );

            // Notify callback if registered
            if ( callback ) {
                callback( previousState, currentState );
            }

            return true;
        }

        unsigned long TallyStateManager::getTimeSinceChange() const {
            return millis() - lastChangeTime;
        }

        Display::color_t TallyStateManager::getStateColor() const {
            return stateToColor( currentState );
        }

        const char *TallyStateManager::getStateString() const {
            return stateToString( currentState );
        }

        void TallyStateManager::setStateChangeCallback( StateChangeCallback cb ) {
            callback = cb;
        }

        void TallyStateManager::reset() {
            setState( TallyState::NO_TALLY );
        }

        const char *TallyStateManager::stateToString( TallyState state ) {
            switch ( state ) {
                case TallyState::PROGRAM:
                    return "PROGRAM";
                case TallyState::PREVIEW:
                    return "PREVIEW";
                case TallyState::UNSELECTED:
                    return "UNSELECTED";
                case TallyState::NO_TALLY:
                    return "NO_TALLY";
                case TallyState::ERROR:
                    return "ERROR";
                default:
                    return "UNKNOWN";
            }
        }

        Display::color_t TallyStateManager::stateToColor( TallyState state ) {
            using namespace Display;

            switch ( state ) {
                case TallyState::PROGRAM:
                    return STACColors::PROGRAM;      // Red
                case TallyState::PREVIEW:
                    return STACColors::PREVIEW;      // Green
                case TallyState::UNSELECTED:
                    return StandardColors::PURPLE;   // Purple (matches baseline)
                case TallyState::NO_TALLY:
                    return StandardColors::PURPLE;   // Purple
                case TallyState::ERROR:
                    return STACColors::ALERT;        // Red (alert)
                default:
                    return StandardColors::BLACK;
            }
        }

    } // namespace State
} // namespace STAC

//  --- EOF --- //
