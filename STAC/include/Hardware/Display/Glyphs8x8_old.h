#ifndef STAC_GLYPHS_8X8_H
#define STAC_GLYPHS_8X8_H

#include <cstdint>

namespace Display {

        /**
         * @brief 8×8 glyph definitions for Waveshare ESP32-S3-Matrix display
         *
         * Glyphs are stored as 64-byte arrays in row-major order.
         * Each byte is either 0 (background) or 1 (foreground).
         * Converted from packed bit format to unpacked format for consistency with 5×5.
         * Uses DistantTears font for standard letters and symbols.
         */
        namespace Glyphs8x8 {

            constexpr uint8_t GLYPH_WIDTH = 8;
            constexpr uint8_t GLYPH_HEIGHT = 8;
            constexpr uint8_t GLYPH_SIZE = 64;

            /**
             * @brief Mnemonic constants for glyph indices (baseline compatibility)
             * 
             * These match the baseline GLF_* defines for easier code reading.
             * IMPORTANT: Digits 0-9 MUST remain at indices 0-9 for numeric display.
             */
            namespace GlyphIndex {
                constexpr uint8_t GLF_0    = 0;   ///< Number 0
                constexpr uint8_t GLF_1    = 1;   ///< Number 1
                constexpr uint8_t GLF_2    = 2;   ///< Number 2
                constexpr uint8_t GLF_3    = 3;   ///< Number 3
                constexpr uint8_t GLF_4    = 4;   ///< Number 4
                constexpr uint8_t GLF_5    = 5;   ///< Number 5
                constexpr uint8_t GLF_6    = 6;   ///< Number 6
                constexpr uint8_t GLF_7    = 7;   ///< Number 7
                constexpr uint8_t GLF_8    = 8;   ///< Number 8
                constexpr uint8_t GLF_9    = 9;   ///< Number 9
                constexpr uint8_t GLF_A    = 10;  ///< Letter A
                constexpr uint8_t GLF_C    = 11;  ///< Letter C
                constexpr uint8_t GLF_P    = 12;  ///< Letter P
                constexpr uint8_t GLF_S    = 13;  ///< Letter S
                constexpr uint8_t GLF_T    = 14;  ///< Letter T
                constexpr uint8_t GLF_QM   = 15;  ///< Question mark
                constexpr uint8_t GLF_WIFI = 16;  ///< WiFi icon
                constexpr uint8_t GLF_CFG  = 17;  ///< Config required icon
                constexpr uint8_t GLF_BX   = 18;  ///< Big X
                constexpr uint8_t GLF_CK   = 19;  ///< Checkmark
                constexpr uint8_t GLF_CBD  = 20;  ///< Checkerboard
                constexpr uint8_t GLF_FM   = 21;  ///< Frame (solid)
                constexpr uint8_t GLF_DF   = 22;  ///< Dotted frame
                constexpr uint8_t GLF_SF      = 23;  ///< Smiley face
                constexpr uint8_t GLF_EN      = 24;  ///< En space
                constexpr uint8_t GLF_PO      = 25;  ///< Power-on indicator (center 4 pixels)
                constexpr uint8_t GLF_UD      = 26;  ///< Firmware update
                constexpr uint8_t GLF_IMUX    = 27;  ///< IMU error icon
                constexpr uint8_t GLF_CORNERS = 28;  ///< Four corner pixels
            }

            /**
             * @brief Base glyph data (unrotated) for 8×8 display
             *
             * Each glyph is 64 bytes representing an 8×8 matrix in row-major order.
             * Converted from packed bit format (original STAC code) to unpacked format.
             * MSB = leftmost pixel in each row.
             */
            constexpr uint8_t BASE_GLYPHS[][GLYPH_SIZE] = {
                // DIGIT_0 - 0x1c, 0x36, 0x67, 0x63, 0x73, 0x36, 0x1c, 0x00
                {0,0,0,1,1,1,0,0, 0,0,1,1,0,1,1,0, 0,1,1,0,0,1,1,1, 0,1,1,0,0,0,1,1,
                 0,1,1,1,0,0,1,1, 0,0,1,1,0,1,1,0, 0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_1 - 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00
                {0,0,0,1,1,0,0,0, 0,0,1,1,1,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0,
                 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_2 - 0x7c, 0x66, 0x06, 0x1c, 0x30, 0x60, 0x7e, 0x00
                {0,1,1,1,1,1,0,0, 0,1,1,0,0,1,1,0, 0,0,0,0,0,1,1,0, 0,0,0,1,1,1,0,0,
                 0,0,1,1,0,0,0,0, 0,1,1,0,0,0,0,0, 0,1,1,1,1,1,1,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_3 - 0x7e, 0x66, 0x06, 0x1c, 0x06, 0x06, 0x7c, 0x00
                {0,1,1,1,1,1,1,0, 0,1,1,0,0,1,1,0, 0,0,0,0,0,1,1,0, 0,0,0,1,1,1,0,0,
                 0,0,0,0,0,1,1,0, 0,0,0,0,0,1,1,0, 0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_4 - 0x1e, 0x36, 0x36, 0x66, 0x7e, 0x06, 0x06, 0x00
                {0,0,0,1,1,1,1,0, 0,0,1,1,0,1,1,0, 0,0,1,1,0,1,1,0, 0,1,1,0,0,1,1,0,
                 0,1,1,1,1,1,1,0, 0,0,0,0,0,1,1,0, 0,0,0,0,0,1,1,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_5 - 0x7e, 0x66, 0x60, 0x7c, 0x06, 0x06, 0x7c, 0x00
                {0,1,1,1,1,1,1,0, 0,1,1,0,0,1,1,0, 0,1,1,0,0,0,0,0, 0,1,1,1,1,1,0,0,
                 0,0,0,0,0,1,1,0, 0,0,0,0,0,1,1,0, 0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_6 - 0x1c, 0x30, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00
                {0,0,0,1,1,1,0,0, 0,0,1,1,0,0,0,0, 0,1,1,0,0,0,0,0, 0,1,1,1,1,1,0,0,
                 0,1,1,0,0,1,1,0, 0,1,1,0,0,1,1,0, 0,0,1,1,1,1,0,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_7 - 0x7e, 0x66, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x00
                {0,1,1,1,1,1,1,0, 0,1,1,0,0,1,1,0, 0,0,0,0,0,1,1,0, 0,0,0,0,1,1,0,0,
                 0,0,0,0,1,1,0,0, 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_8 - 0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00
                {0,0,1,1,1,1,0,0, 0,1,1,0,0,1,1,0, 0,1,1,0,0,1,1,0, 0,0,1,1,1,1,0,0,
                 0,1,1,0,0,1,1,0, 0,1,1,0,0,1,1,0, 0,0,1,1,1,1,0,0, 0,0,0,0,0,0,0,0},
                
                // DIGIT_9 - 0x3c, 0x66, 0x66, 0x3e, 0x06, 0x0c, 0x38, 0x00
                {0,0,1,1,1,1,0,0, 0,1,1,0,0,1,1,0, 0,1,1,0,0,1,1,0, 0,0,1,1,1,1,1,0,
                 0,0,0,0,0,1,1,0, 0,0,0,0,1,1,0,0, 0,0,1,1,1,0,0,0, 0,0,0,0,0,0,0,0},
                
                // LETTER_A - 0x1e, 0x36, 0x36, 0x66, 0x7e, 0x66, 0x66, 0x00
                {0,0,0,1,1,1,1,0, 0,0,1,1,0,1,1,0, 0,0,1,1,0,1,1,0, 0,1,1,0,0,1,1,0,
                 0,1,1,1,1,1,1,0, 0,1,1,0,0,1,1,0, 0,1,1,0,0,1,1,0, 0,0,0,0,0,0,0,0},
                
                // LETTER_C - 0x1e, 0x36, 0x60, 0x60, 0x60, 0x30, 0x1e, 0x00
                {0,0,0,1,1,1,1,0, 0,0,1,1,0,1,1,0, 0,1,1,0,0,0,0,0, 0,1,1,0,0,0,0,0,
                 0,1,1,0,0,0,0,0, 0,0,1,1,0,0,0,0, 0,0,0,1,1,1,1,0, 0,0,0,0,0,0,0,0},
                
                // LETTER_P - 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00
                {0,1,1,1,1,1,0,0, 0,1,1,0,0,1,1,0, 0,1,1,0,0,1,1,0, 0,1,1,1,1,1,0,0,
                 0,1,1,0,0,0,0,0, 0,1,1,0,0,0,0,0, 0,1,1,0,0,0,0,0, 0,0,0,0,0,0,0,0},
                
                // LETTER_S - 0x3e, 0x66, 0x60, 0x3c, 0x06, 0x06, 0x7c, 0x00
                {0,0,1,1,1,1,1,0, 0,1,1,0,0,1,1,0, 0,1,1,0,0,0,0,0, 0,0,1,1,1,1,0,0,
                 0,0,0,0,0,1,1,0, 0,0,0,0,0,1,1,0, 0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,0},
                
                // LETTER_T - 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00
                {0,1,1,1,1,1,1,0, 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0,
                 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,0,0,0,0,0},
                
                // QUESTION_MARK - 0x7c, 0x66, 0x06, 0x1c, 0x00, 0x18, 0x18, 0x00
                {0,1,1,1,1,1,0,0, 0,1,1,0,0,1,1,0, 0,0,0,0,0,1,1,0, 0,0,0,1,1,1,0,0,
                 0,0,0,0,0,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,1,1,0,0,0, 0,0,0,0,0,0,0,0},
                
                // WIFI - 0x18, 0x3c, 0x66, 0xc3, 0x18, 0x3c, 0x66, 0x00
                {0,0,0,1,1,0,0,0, 0,0,1,1,1,1,0,0, 0,1,1,0,0,1,1,0, 1,1,0,0,0,0,1,1,
                 0,0,0,1,1,0,0,0, 0,0,1,1,1,1,0,0, 0,1,1,0,0,1,1,0, 0,0,0,0,0,0,0,0},
                
                // CONFIG_REQ - 0x1f, 0x07, 0x0f, 0x9d, 0xb9, 0xf0, 0xe0, 0xf8
                {0,0,0,1,1,1,1,1, 0,0,0,0,0,1,1,1, 0,0,0,0,1,1,1,1, 1,0,0,1,1,1,0,1,
                 1,0,1,1,1,0,0,1, 1,1,1,1,0,0,0,0, 1,1,1,0,0,0,0,0, 1,1,1,1,1,0,0,0},
                
                // BIG_X - 0xc3, 0xe7, 0x66, 0x18, 0x18, 0x66, 0xe7, 0xc3
                {1,1,0,0,0,0,1,1, 1,1,1,0,0,1,1,1, 0,1,1,0,0,1,1,0, 0,0,0,1,1,0,0,0,
                 0,0,0,1,1,0,0,0, 0,1,1,0,0,1,1,0, 1,1,1,0,0,1,1,1, 1,1,0,0,0,0,1,1},
                
                // CHECKMARK - 0x00, 0x01, 0x03, 0x46, 0x6c, 0x38, 0x10, 0x00
                {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1, 0,0,0,0,0,0,1,1, 0,1,0,0,0,1,1,0,
                 0,1,1,0,1,1,0,0, 0,0,1,1,1,0,0,0, 0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0},
                
                // CHECKERBOARD - 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55
                {1,0,1,0,1,0,1,0, 0,1,0,1,0,1,0,1, 1,0,1,0,1,0,1,0, 0,1,0,1,0,1,0,1,
                 1,0,1,0,1,0,1,0, 0,1,0,1,0,1,0,1, 1,0,1,0,1,0,1,0, 0,1,0,1,0,1,0,1},
                
                // FRAME - 0xff, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff
                {1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,0,0,0,0,1,1, 1,1,0,0,0,0,1,1,
                 1,1,0,0,0,0,1,1, 1,1,0,0,0,0,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1},
                
                // FRAME_DOTTED - 0xdb, 0xdb, 0x00, 0xc3, 0xc3, 0x00, 0xdb, 0xdb
                {1,1,0,1,1,0,1,1, 1,1,0,1,1,0,1,1, 0,0,0,0,0,0,0,0, 1,1,0,0,0,0,1,1,
                 1,1,0,0,0,0,1,1, 0,0,0,0,0,0,0,0, 1,1,0,1,1,0,1,1, 1,1,0,1,1,0,1,1},
                
                // SMILEY_FACE - 0x3c, 0x7e, 0xdb, 0xff, 0xff, 0xdb, 0x66, 0x3c
                {0,0,1,1,1,1,0,0, 0,1,1,1,1,1,1,0, 1,1,0,1,1,0,1,1, 1,1,1,1,1,1,1,1,
                 1,1,1,1,1,1,1,1, 1,1,0,1,1,0,1,1, 0,1,1,0,0,1,1,0, 0,0,1,1,1,1,0,0},
                
                // SPACE_EN - 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c
                {0,0,1,1,1,1,0,0, 0,0,1,1,1,1,0,0, 0,0,1,1,1,1,0,0, 0,0,1,1,1,1,0,0,
                 0,0,1,1,1,1,0,0, 0,0,1,1,1,1,0,0, 0,0,1,1,1,1,0,0, 0,0,1,1,1,1,0,0},
                
                // CENTER_DOT - 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00
                {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,1,1,0,0,0,
                 0,0,0,1,1,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0},
                
                // FIRMWARE_UPDATE - 0x01, 0x0a, 0x0c, 0x0e, 0xf0, 0x90, 0x90, 0xf0
                {0,0,0,0,0,0,0,1, 0,0,0,0,1,0,1,0, 0,0,0,0,1,1,0,0, 0,0,0,0,1,1,1,0,
                 1,1,1,1,0,0,0,0, 1,0,0,1,0,0,0,0, 1,0,0,1,0,0,0,0, 1,1,1,1,0,0,0,0},
                
                // IMU_ERROR - 0xe7, 0x81, 0xa5, 0x18, 0x18, 0xa5, 0x81, 0xe7
                {1,1,1,0,0,1,1,1, 1,0,0,0,0,0,0,1, 1,0,1,0,0,1,0,1, 0,0,0,1,1,0,0,0,
                 0,0,0,1,1,0,0,0, 1,0,1,0,0,1,0,1, 1,0,0,0,0,0,0,1, 1,1,1,0,0,1,1,1},
                
                // CORNERS (pixels 0, 7, 56, 63 - for autostart/pulsing)
                {1,0,0,0,0,0,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,0,0,1}
            };

            // Derive glyph count from array size at compile time
            constexpr uint8_t GLYPH_COUNT = sizeof(BASE_GLYPHS) / sizeof(BASE_GLYPHS[0]);

    } // namespace Glyphs8x8
} // namespace Display

#endif // STAC_GLYPHS_8X8_H


//  --- EOF --- //
