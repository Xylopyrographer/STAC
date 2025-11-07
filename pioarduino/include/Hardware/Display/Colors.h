#ifndef STAC_COLORS_H
#define STAC_COLORS_H


#include <cstdint>
#include "Device_Config.h"  // Add this line to get DISPLAY_COLOR_ORDER_*

namespace STAC {
    namespace Display {

        // RGB color type (32-bit: 0x00RRGGBB)
        using rgb_t = uint32_t;

        // Helper function to create RGB color
        constexpr rgb_t makeRGB( uint8_t r, uint8_t g, uint8_t b ) {
            return ( ( uint32_t )r << 16 ) | ( ( uint32_t )g << 8 ) | b;
        }

        // Helper function to create GRB color (for GRB LED strips)
        constexpr rgb_t makeGRB( uint8_t r, uint8_t g, uint8_t b ) {
            return ( ( uint32_t )g << 16 ) | ( ( uint32_t )r << 8 ) | b;
        }

        // Standard colors (RGB order)
        namespace StandardColors {
            constexpr rgb_t BLACK       = makeRGB( 0, 0, 0 );
            constexpr rgb_t WHITE       = makeRGB( 255, 255, 255 );
            constexpr rgb_t RED         = makeRGB( 255, 0, 0 );
            constexpr rgb_t GREEN       = makeRGB( 0, 255, 0 );
            constexpr rgb_t BLUE        = makeRGB( 0, 0, 255 );
            constexpr rgb_t PURPLE      = makeRGB( 128, 0, 128 );
            constexpr rgb_t ORANGE      = makeRGB( 255, 165, 0 );
            constexpr rgb_t YELLOW      = makeRGB( 255, 255, 0 );
            constexpr rgb_t DARK_BLUE   = makeRGB( 0, 0, 127 );
            constexpr rgb_t DARK_GREEN  = makeRGB( 7, 13, 64 );
            constexpr rgb_t DARK_ORANGE = makeRGB( 0, 43, 33 );
            constexpr rgb_t DARK_PURPLE = makeRGB( 112, 56, 0 );
            constexpr rgb_t TEAL        = makeRGB( 98, 224, 230 );
            constexpr rgb_t DARK_TEAL   = makeRGB( 33, 58, 58 );
        }

        // STAC-specific semantic colors
        namespace STACColors {
            constexpr rgb_t PROGRAM         = StandardColors::RED;      // On-air
            constexpr rgb_t PREVIEW         = StandardColors::GREEN;    // Preview
            constexpr rgb_t UNSELECTED      = StandardColors::PURPLE;   // Not selected
            constexpr rgb_t WARNING         = StandardColors::ORANGE;   // Warning state
            constexpr rgb_t ALERT           = StandardColors::RED;      // Alert/error
            constexpr rgb_t GTG             = StandardColors::GREEN;    // Good-to-go
            constexpr rgb_t POWER_ON        = StandardColors::BLUE;     // Power indicator
            constexpr rgb_t HDMI_VALUE      = StandardColors::BLUE;     // HDMI channel
            constexpr rgb_t SDI_VALUE       = StandardColors::ORANGE;   // SDI channel
            constexpr rgb_t AUTOSTART_PULSE = StandardColors::BLUE;     // Auto-start pulse
        }


        /**
        * @brief Convert RGB color to board-specific color order
        * @param r Red component (0-255)
        * @param g Green component (0-255)
        * @param b Blue component (0-255)
        * @return Color value in correct order for current board
        */
        inline rgb_t boardColor( uint8_t r, uint8_t g, uint8_t b ) {
#if defined(DISPLAY_COLOR_ORDER_RGB)
            return makeRGB( r, g, b );
#elif defined(DISPLAY_COLOR_ORDER_GRB)
            return makeGRB( r, g, b );
#else
#error "No color order defined! Check Device_Config.h"
            return 0;  // This line won't be reached but satisfies compiler
#endif
        }

        /**
         * @brief Convert standard RGB color to board-specific order
         * @param rgbColor Color in RGB format
         * @return Color in board-specific format
         */
        inline rgb_t boardColor( rgb_t rgbColor ) {
            uint8_t r = ( rgbColor >> 16 ) & 0xFF;
            uint8_t g = ( rgbColor >> 8 ) & 0xFF;
            uint8_t b = rgbColor & 0xFF;
            return boardColor( r, g, b );
        }

    } // namespace Display
} // namespace STAC

#endif // STAC_COLORS_H


//  --- EOF --- //