/**
 * @file ArduinoGFX_STAC.h
 * @brief Arduino_GFX configuration for STAC TFT displays
 *
 * Provides display configuration for Arduino_GFX library
 * Board-specific settings are pulled from board config headers
 */

#pragma once

#include <Arduino_GFX_Library.h>
#include "Device_Config.h"

namespace Display {

    /**
     * Create Arduino_GFX bus and display instances for TFT displays
     * Returns configured display ready for initialization
     */
    inline Arduino_GFX *createArduinoGFXDisplay( uint8_t rotation = 0 ) {

        #if defined(TFT_PANEL_ST7789) || defined(TFT_PANEL_ST7735S) || defined(TFT_PANEL_GC9A01)

        // Create SPI bus
        Arduino_DataBus *bus = new Arduino_ESP32SPI(
            TFT_DC,     // DC pin
            TFT_CS,     // CS pin
            TFT_SCLK,   // SCK pin
            TFT_MOSI,   // MOSI pin
            -1,         // MISO (not used)
            HSPI        // SPI peripheral (M5StickC Plus uses HSPI)
        );

        // Get rotation-specific offsets from board config
        #ifdef TFT_ROTATION_OFFSETS
        static const int16_t offsets[][ 2 ] = TFT_ROTATION_OFFSETS;
        // Driver uses mixed offsets per rotation:
        // Rot0: COL1, ROW1 | Rot1: ROW1, COL2 | Rot2: COL2, ROW2 | Rot3: ROW2, COL1
        // Playground testing confirmed correct offsets: 52, 40, 53, 40
        int16_t col_offset1 = 52;  // Confirmed working in playground
        int16_t row_offset1 = 40;  // Confirmed working in playground
        int16_t col_offset2 = 53;  // Confirmed working in playground
        int16_t row_offset2 = 40;  // Confirmed working in playground
        #else
        int16_t col_offset1 = TFT_OFFSET_X;
        int16_t row_offset1 = TFT_OFFSET_Y;
        int16_t col_offset2 = TFT_OFFSET_X;
        int16_t row_offset2 = TFT_OFFSET_Y;
        #endif

        // IMPORTANT: Always pass physical panel dimensions (135x240 for M5StickC Plus)
        // Arduino_ST7789 handles rotation internally and swaps dimensions as needed
        // Do NOT pre-swap width/height based on rotation!
        int16_t width = DISPLAY_WIDTH;   // 135 (physical width in default orientation)
        int16_t height = DISPLAY_HEIGHT; // 240 (physical height in default orientation)

        // Create display based on panel type
        Arduino_GFX *gfx;

        #ifdef TFT_PANEL_ST7789
        // Arduino_ST7789 constructor: (bus, rst, rotation, ips, width, height, col_offset1, row_offset1, col_offset2, row_offset2)
        // Driver mixes offsets per rotation - see Arduino_TFT::setRotation()
        gfx = new Arduino_ST7789(
            bus,              // Bus instance
            TFT_RST,          // Reset pin
            rotation,         // Rotation (0-3) - driver handles dimension swapping
            true,             // IPS display = true for M5StickC Plus
            width,            // Physical panel width (135)
            height,           // Physical panel height (240)
            ( uint8_t )col_offset1, // 52
            ( uint8_t )row_offset1, // 40
            ( uint8_t )col_offset2, // 53
            ( uint8_t )row_offset2 // 40
        );
        #elif defined(TFT_PANEL_ST7735S)
        // Arduino_ST7735 for AIPI-Lite 128x128 display
        gfx = new Arduino_ST7735(
            bus,              // Bus instance
            TFT_RST,          // Reset pin
            rotation,         // Rotation (0-3)
            false,            // IPS display = false for ST7735
            width,            // Physical panel width (128)
            height,           // Physical panel height (128)
            ( uint8_t )col_offset1,
            ( uint8_t )row_offset1,
            ( uint8_t )col_offset2,
            ( uint8_t )row_offset2
        );
        #elif defined(TFT_PANEL_GC9A01)
        // Arduino_GC9A01 for LilyGo T-QT 128x128 display
        // The controller has 240x240 RAM but physical display is 128x128
        // Constructor params: (bus, rst, rotation, ips, width, height, col_offset1, row_offset1, col_offset2, row_offset2)
        // For rotation 2: uses col_offset2 and row_offset2
        // We want NO offset - the display is natively 128x128, not windowed
        int16_t gc9a01_width = 128;
        int16_t gc9a01_height = 128;

        gfx = new Arduino_GC9A01(
            bus,                     // Bus instance
            TFT_RST,                 // Reset pin
            rotation,                // Rotation (0-3)
            true,                    // IPS display = true for GC9A01
            gc9a01_width,            // Logical width (128)
            gc9a01_height,           // Logical height (128)
            0,                       // col_offset1 (rotation 0)
            0,                       // row_offset1 (rotation 0)
            0,                       // col_offset2 (rotation 2) - no offset needed
            0                        // row_offset2 (rotation 2) - no offset needed
        );
        #endif

        return gfx;

        #else
#error "TFT panel type not defined - check board config"
        return nullptr;
        #endif
    }

} // namespace Display
