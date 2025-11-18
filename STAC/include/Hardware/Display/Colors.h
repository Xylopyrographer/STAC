#ifndef STAC_COLORS_H
#define STAC_COLORS_H

#include <cstdint>
#include "Device_Config.h"


    namespace Display {

        // Color type - 32-bit packed RGB (0x00RRGGBB)
        // Renamed from rgb_t to avoid conflict with LiteLED's rgb_t struct
        using color_t = uint32_t;

        // Helper function to create RGB color
        constexpr color_t makeRGB( uint8_t r, uint8_t g, uint8_t b ) {
            return ( ( uint32_t )r << 16 ) | ( ( uint32_t )g << 8 ) | b;
        }

        // Helper function to create GRB color (for GRB LED strips)
        constexpr color_t makeGRB( uint8_t r, uint8_t g, uint8_t b ) {
            return ( ( uint32_t )g << 16 ) | ( ( uint32_t )r << 8 ) | b;
        }

        // Standard colors (RGB order)
        namespace StandardColors {
            constexpr color_t BLACK       = makeRGB( 0, 0, 0 );
            constexpr color_t WHITE       = makeRGB( 255, 255, 255 );
            constexpr color_t RED         = makeRGB( 255, 0, 0 );
            constexpr color_t GREEN       = makeRGB( 0, 255, 0 );
            constexpr color_t BLUE        = makeRGB( 0, 0, 255 );
            constexpr color_t PURPLE      = makeRGB( 128, 0, 128 );
            constexpr color_t ORANGE      = makeRGB( 255, 165, 0 );
            constexpr color_t YELLOW      = makeRGB( 255, 255, 0 );
            constexpr color_t DARK_BLUE   = makeRGB( 0, 0, 127 );
            constexpr color_t DARK_GREEN  = makeRGB( 7, 13, 64 );
            constexpr color_t DARK_ORANGE = makeRGB( 0, 43, 33 );
            constexpr color_t DARK_PURPLE = makeRGB( 112, 56, 0 );
            constexpr color_t TEAL        = makeRGB( 98, 224, 230 );
            constexpr color_t DARK_TEAL   = makeRGB( 33, 58, 58 );
            constexpr color_t PINK        = makeRGB( 255, 20, 147 );  // For peripheral mode
        }

        // STAC-specific semantic colors
        namespace STACColors {
            constexpr color_t PROGRAM         = StandardColors::RED;      // On-air
            constexpr color_t PREVIEW         = StandardColors::GREEN;    // Preview
            constexpr color_t UNSELECTED      = StandardColors::BLUE;     // Changed from PURPLE
            // constexpr color_t WARNING         = StandardColors::ORANGE;   // Warning state
            constexpr color_t WARNING         = StandardColors::YELLOW;   // Warning state
            // constexpr color_t ALERT           = StandardColors::RED;      // Alert/error
            constexpr color_t ALERT           = StandardColors::ORANGE;      // Alert/error
            constexpr color_t GTG             = StandardColors::GREEN;    // Good-to-go
            constexpr color_t POWER_ON        = StandardColors::BLUE;     // Power indicator
            constexpr color_t HDMI_VALUE      = StandardColors::BLUE;     // HDMI channel
            constexpr color_t SDI_VALUE       = StandardColors::ORANGE;   // SDI channel
            constexpr color_t AUTOSTART_PULSE = StandardColors::BLUE;     // Auto-start pulse
        }

        /**
         * @brief Convert RGB color to board-specific color order
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @return Color value in correct order for current board
         */
        inline color_t boardColor( uint8_t r, uint8_t g, uint8_t b ) {
#if defined(DISPLAY_COLOR_ORDER_RGB)
            return makeRGB( r, g, b );
#elif defined(DISPLAY_COLOR_ORDER_GRB)
            return makeGRB( r, g, b );
#else
#error "No color order defined! Check Device_Config.h"
            return 0;
#endif
        }

        /**
         * @brief Convert standard RGB color to board-specific order
         * @param rgbColor Color in RGB format
         * @return Color in board-specific format
         */
        inline color_t boardColor( color_t rgbColor ) {
            uint8_t r = ( rgbColor >> 16 ) & 0xFF;
            uint8_t g = ( rgbColor >> 8 ) & 0xFF;
            uint8_t b = rgbColor & 0xFF;
            return boardColor( r, g, b );
        }

    } // namespace Display


#endif // STAC_COLORS_H


//  --- EOF --- //
