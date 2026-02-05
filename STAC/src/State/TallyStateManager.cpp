#include "State/TallyStateManager.h"
#include "Hardware/Display/Colors.h"
#include <Arduino.h>


namespace State {

    TallyStateManager::TallyStateManager()
        : StateManagerBase<TallyState>( TallyState::NO_TALLY ) {
    }

    bool TallyStateManager::setState( TallyState newState ) {
        return StateManagerBase<TallyState>::setState( newState, stateToString );
    }

    Display::color_t TallyStateManager::getStateColor() const {
        return stateToColor( currentState );
    }

    const char *TallyStateManager::getStateString() const {
        return stateToString( currentState );
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
                return StandardColors::BLACK;    // Black - no tally info available
            case TallyState::ERROR:
                return STACColors::ALERT;        // Red (alert)
            default:
                return StandardColors::BLACK;
        }
    }

} // namespace State


//  --- EOF --- //
