#ifndef STAC_CONSTANTS_H
#define STAC_CONSTANTS_H

#include <cstdint>
#include "../Device_Config.h"


    namespace Config {

        // ============================================================================
        // HARDWARE PINS (from Device_Config.h)
        // ============================================================================

        namespace Pins {
            constexpr uint8_t DISPLAY_DATA = PIN_DISPLAY_DATA;
            constexpr uint8_t BUTTON = PIN_BUTTON;

            #ifdef PIN_IMU_SCL
                constexpr uint8_t IMU_SCL = PIN_IMU_SCL;
                constexpr uint8_t IMU_SDA = PIN_IMU_SDA;
            #endif

            constexpr uint8_t PM_CHECK_OUT = PIN_PM_CHECK_OUT;
            constexpr uint8_t PM_CHECK_IN = PIN_PM_CHECK_IN;
            constexpr uint8_t TALLY_STATUS_0 = PIN_TALLY_STATUS_0;
            constexpr uint8_t TALLY_STATUS_1 = PIN_TALLY_STATUS_1;
        }

        // ============================================================================
        // DISPLAY CONFIGURATION (from Device_Config.h)
        // ============================================================================

        namespace Display {
            constexpr uint8_t MATRIX_WIDTH = DISPLAY_MATRIX_WIDTH;
            constexpr uint8_t MATRIX_HEIGHT = DISPLAY_MATRIX_HEIGHT;
            constexpr uint8_t MATRIX_SIZE = MATRIX_WIDTH * MATRIX_HEIGHT;  // Calculated from dimensions
            
            // Board-specific brightness map (from board config)
            constexpr uint8_t BRIGHTNESS_MAP[] = BOARD_BRIGHTNESS_MAP;
            constexpr uint8_t BRIGHTNESS_LEVELS = (sizeof(BRIGHTNESS_MAP) / sizeof(uint8_t)) - 1;
            
            // Compile-time validation of brightness map
            static_assert(sizeof(BRIGHTNESS_MAP) >= 2, "Brightness map must have at least 2 entries");
            static_assert(sizeof(BRIGHTNESS_MAP) <= 10, "Brightness map must have no more than 10 entries");
            static_assert(BRIGHTNESS_MAP[0] == 0, "First brightness entry must be 0");

            // Display update control
            constexpr bool SHOW = true;
            constexpr bool NO_SHOW = false;
        }

        // ============================================================================
        // TIMING CONSTANTS (from Device_Config.h)
        // ============================================================================

        namespace Timing {
            constexpr unsigned long AUTOSTART_PULSE_MS = TIMING_AUTOSTART_PULSE_MS;
            constexpr unsigned long AUTOSTART_TIMEOUT_MS = TIMING_AUTOSTART_TIMEOUT_MS;
            constexpr unsigned long GUI_PAUSE_MS = TIMING_GUI_PAUSE_MS;
            constexpr unsigned long GUI_PAUSE_SHORT_MS = TIMING_GUI_PAUSE_SHORT_MS;
            constexpr unsigned long BUTTON_SELECT_MS = TIMING_BUTTON_SELECT_MS;
            constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = TIMING_WIFI_CONNECT_TIMEOUT_MS;
            constexpr unsigned long ERROR_REPOLL_MS = TIMING_ERROR_REPOLL_MS;
            constexpr unsigned long PM_POLL_INTERVAL_MS = TIMING_PM_POLL_INTERVAL_MS;
            constexpr unsigned long OP_MODE_TIMEOUT_MS = TIMING_OP_MODE_TIMEOUT_MS;
        }

        // ============================================================================
        // BUTTON CONFIGURATION (from Device_Config.h)
        // ============================================================================

        namespace Button {
            constexpr unsigned long DEBOUNCE_MS = BUTTON_DEBOUNCE_MS;
            constexpr bool ACTIVE_LOW = BUTTON_ACTIVE_LOW;
        }

        // ============================================================================
        // NETWORK CONFIGURATION (from Device_Config.h)
        // ============================================================================

        namespace Net {
            constexpr uint8_t MAX_POLL_ERRORS = NETWORK_MAX_POLL_ERRORS;
            constexpr uint16_t DEFAULT_PORT = 80;
            constexpr uint32_t CONNECT_TIMEOUT_MS = 1000;
        }

        // ============================================================================
        // PERIPHERAL MODE (from Device_Config.h)
        // ============================================================================

        namespace Peripheral {
            constexpr uint8_t PM_CHECK_COUNT = PM_CHECK_TOGGLE_COUNT;
            constexpr uint8_t INVALID_STATE = 0xFF;
        }

        // ============================================================================
        // NVS SCHEMA VERSIONS
        // ============================================================================
        /**
         * @brief NVS schema version numbers
         * 
         * These version numbers track the structure of data stored in NVS.
         * Increment when making schema changes that are incompatible with previous versions.
         * 
         * NOM_PREFS_VERSION: Normal Operating Mode schema (wifi, switch, v60hd, v160hd, identity namespaces)
         *   - Stored in 'wifi' namespace only
         *   - Checked on boot, mismatch triggers warning and requires factory reset
         * 
         * PM_PREFS_VERSION: Peripheral Mode schema (peripheral namespace)
         *   - Stored in 'peripheral' namespace
         *   - Independent versioning from NOM
         */
        namespace NVS {
            constexpr uint8_t NOM_PREFS_VERSION = 4;  ///< Normal operating mode schema version
            constexpr uint8_t PM_PREFS_VERSION = 2;   ///< Peripheral mode schema version
        };

        // ============================================================================
        // STORAGE ACCESS MODES
        // ============================================================================

        namespace Storage {
            constexpr bool READ_ONLY = true;
            constexpr bool READ_WRITE = false;
        }

        // ============================================================================
        // STRING CONSTANTS
        // ============================================================================

        namespace Strings {
            constexpr const char *ID_PREFIX = "STAC";  ///< Global prefix for all STAC instances
            constexpr const char *BOARD_NAME = STAC_BOARD_NAME;
            constexpr const char *SOFTWARE_VERSION = STAC_SOFTWARE_VERSION;
        }

    } // namespace Config


#endif // STAC_CONSTANTS_H


//  --- EOF --- //
