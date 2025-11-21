#include "State/OperatingModeManager.h"
#include <Arduino.h>


    namespace State {

        OperatingModeManager::OperatingModeManager()
            : StateManagerBase<OperatingMode>( OperatingMode::NORMAL ) {
        }

        bool OperatingModeManager::setMode( OperatingMode newMode ) {
            return StateManagerBase<OperatingMode>::setState( newMode, modeToString );
        }

        const char *OperatingModeManager::getModeString() const {
            return modeToString( currentState );
        }

        const char *OperatingModeManager::modeToString( OperatingMode mode ) {
            switch ( mode ) {
                case OperatingMode::NORMAL:
                    return "NORMAL";
                case OperatingMode::PERIPHERAL:
                    return "PERIPHERAL";
                case OperatingMode::PROVISIONING:
                    return "PROVISIONING";
                default:
                    return "UNKNOWN";
            }
        }

    } // namespace State



//  --- EOF --- //
