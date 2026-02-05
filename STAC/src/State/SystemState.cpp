#include "State/SystemState.h"
#include <Arduino.h>


namespace State {

    SystemState::SystemState()
        : tallyState()
        , operatingMode()
        , operations()
        , wifiInfo()
        , switchState()
        , initialized( false ) {
    }

    bool SystemState::begin() {
        log_i( "System state manager initialized" );

        // Set up state change callbacks
        tallyState.setStateChangeCallback( [ this ]( TallyState oldState, TallyState newState ) {
            // Could trigger additional actions on state change
            log_d( "Tally state change callback: %s -> %s",
                   tallyState.stateToString( oldState ),
                   tallyState.stateToString( newState ) );
        } );

        operatingMode.setStateChangeCallback( [ this ]( OperatingMode oldMode, OperatingMode newMode ) {
            // Could trigger mode-specific setup on mode change
            log_d( "Operating mode change callback: %s -> %s",
                   operatingMode.modeToString( oldMode ),
                   operatingMode.modeToString( newMode ) );
        } );

        initialized = true;
        return true;
    }

    bool SystemState::isReady() const {
        if ( !initialized ) {
            return false;
        }

        // System is ready if:
        // 1. In normal mode with WiFi credentials, OR
        // 2. In peripheral mode

        if ( operatingMode.isPeripheralMode() ) {
            return true;
        }

        if ( operatingMode.isNormalMode() ) {
            return !wifiInfo.stacID.isEmpty() &&
                   strlen( wifiInfo.networkSSID ) > 0;
        }

        return false;
    }

    void SystemState::update() {
        // Could add periodic state checks here
        // For now, state is event-driven
    }

} // namespace State


//  --- EOF --- //
