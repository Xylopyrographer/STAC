#include "State/OperatingModeManager.h"
#include <Arduino.h>

namespace STAC {
    namespace State {

        OperatingModeManager::OperatingModeManager()
            : currentMode( OperatingMode::NORMAL )
            , previousMode( OperatingMode::NORMAL )
            , lastChangeTime( 0 )
            , callback( nullptr ) {
        }

        bool OperatingModeManager::setMode( OperatingMode newMode ) {
            if ( newMode == currentMode ) {
                return false;  // No change
            }

            // Store previous mode
            previousMode = currentMode;
            currentMode = newMode;
            lastChangeTime = millis();

            // Log mode change
            log_i( "Operating mode: %s -> %s",
                   modeToString( previousMode ),
                   modeToString( currentMode ) );

            // Notify callback if registered
            if ( callback ) {
                callback( previousMode, currentMode );
            }

            return true;
        }

        unsigned long OperatingModeManager::getTimeSinceChange() const {
            return millis() - lastChangeTime;
        }

        const char *OperatingModeManager::getModeString() const {
            return modeToString( currentMode );
        }

        void OperatingModeManager::setModeChangeCallback( ModeChangeCallback cb ) {
            callback = cb;
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
} // namespace STAC


//  --- EOF --- //
