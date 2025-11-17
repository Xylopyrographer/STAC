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
            constexpr uint8_t MATRIX_SIZE = DISPLAY_TOTAL_LEDS;
            constexpr uint8_t POWER_LED_PIXEL = DISPLAY_POWER_LED_PIXEL;
            constexpr uint8_t BRIGHTNESS_MIN = DISPLAY_BRIGHTNESS_MIN;
            constexpr uint8_t BRIGHTNESS_MAX = DISPLAY_BRIGHTNESS_MAX;
            constexpr uint8_t BRIGHTNESS_DEFAULT = DISPLAY_BRIGHTNESS_DEFAULT;
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
            constexpr unsigned long NEXT_STATE_MS = TIMING_NEXT_STATE_MS;
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
        }

        // ============================================================================
        // NVS (from Device_Config.h)
        // ============================================================================

        namespace NVS {
            constexpr uint8_t NOM_PREFS_VERSION = NVS_NOM_PREFS_VERSION;
            constexpr uint8_t PM_PREFS_VERSION = NVS_PM_PREFS_VERSION;
        }

        // ============================================================================
        // STRING CONSTANTS
        // ============================================================================

        namespace Strings {
            constexpr const char *BOARD_NAME = STAC_BOARD_NAME;
            constexpr const char *ID_PREFIX = STAC_ID_PREFIX;
        }

    } // namespace Config


#endif // STAC_CONSTANTS_H


//  --- EOF --- //
