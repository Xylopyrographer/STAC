#ifndef STAC_GLYPHS_5X5_H
#define STAC_GLYPHS_5X5_H

#include <cstdint>

namespace STAC {
    namespace Display {

        /**
         * @brief 5×5 glyph definitions for ATOM Matrix display
         *
         * Glyphs are stored as 25-byte arrays in row-major order.
         * Each byte is either 0 (background) or 1 (foreground).
         */
        namespace Glyphs5x5 {

            constexpr uint8_t GLYPH_WIDTH = 5;
            constexpr uint8_t GLYPH_HEIGHT = 5;
            constexpr uint8_t GLYPH_SIZE = 25;
            constexpr uint8_t GLYPH_COUNT = 33;

            /**
             * @brief Glyph identifier enum
             *
             * IMPORTANT: Digits 0-9 MUST remain at indices 0-9 for numeric display.
             */
            enum class GlyphId : uint8_t {
                DIGIT_0 = 0,        ///< Number 0
                DIGIT_1 = 1,        ///< Number 1
                DIGIT_2 = 2,        ///< Number 2
                DIGIT_3 = 3,        ///< Number 3
                DIGIT_4 = 4,        ///< Number 4
                DIGIT_5 = 5,        ///< Number 5
                DIGIT_6 = 6,        ///< Number 6
                DIGIT_7 = 7,        ///< Number 7
                DIGIT_8 = 8,        ///< Number 8
                DIGIT_9 = 9,        ///< Number 9
                LETTER_X = 10,      ///< Letter X
                WIFI = 11,          ///< WiFi icon
                SMART_TALLY = 12,   ///< Smart Tally (ST) icon
                LETTER_C = 13,      ///< Letter C
                LETTER_T = 14,      ///< Letter T
                ARROW_RIGHT = 15,   ///< Right arrow
                ARROW_LEFT = 16,    ///< Left arrow
                HAPPY_FACE = 17,    ///< Smiley face
                BIG_X = 18,         ///< Big X (error/cancel)
                FRAME = 19,         ///< Solid frame
                FRAME_DOTTED = 20,  ///< Dotted frame
                QUESTION_MARK = 21, ///< Question mark
                CHECKERBOARD = 22,  ///< Checkerboard pattern
                CHECKMARK = 23,     ///< Checkmark
                SPACE_EN = 24,      ///< En space (3 columns)
                SPACE_EM = 25,      ///< Em space (full display)
                DOT = 26,           ///< Dot/period
                WIFI_CONFIG = 27,   ///< WiFi configuration icon
                LETTER_A = 28,      ///< Letter A
                LETTER_S = 29,      ///< Letter S
                LETTER_P = 30,      ///< Letter P
                FIRMWARE_UPDATE = 31, ///< Firmware update icon
                CENTER_PIXEL = 32   ///< Center pixel only (for power indicator)
            };

            /**
             * @brief Mnemonic constants for glyph indices (baseline compatibility)
             * 
             * These match the baseline GLF_* defines for easier code reading.
             */
            namespace GlyphIndex {
                constexpr uint8_t GLF_0   = 0;   ///< Number 0
                constexpr uint8_t GLF_1   = 1;   ///< Number 1
                constexpr uint8_t GLF_2   = 2;   ///< Number 2
                constexpr uint8_t GLF_3   = 3;   ///< Number 3
                constexpr uint8_t GLF_4   = 4;   ///< Number 4
                constexpr uint8_t GLF_5   = 5;   ///< Number 5
                constexpr uint8_t GLF_6   = 6;   ///< Number 6
                constexpr uint8_t GLF_7   = 7;   ///< Number 7
                constexpr uint8_t GLF_8   = 8;   ///< Number 8
                constexpr uint8_t GLF_9   = 9;   ///< Number 9
                constexpr uint8_t GLF_X   = 10;  ///< Letter X
                constexpr uint8_t GLF_WIFI = 11; ///< WiFi icon
                constexpr uint8_t GLF_ST  = 12;  ///< Smart Tally icon
                constexpr uint8_t GLF_C   = 13;  ///< Letter C
                constexpr uint8_t GLF_T   = 14;  ///< Letter T
                constexpr uint8_t GLF_RA  = 15;  ///< Right arrow
                constexpr uint8_t GLF_LA  = 16;  ///< Left arrow
                constexpr uint8_t GLF_HF  = 17;  ///< Happy face
                constexpr uint8_t GLF_BX  = 18;  ///< Big X
                constexpr uint8_t GLF_FM  = 19;  ///< Frame (solid)
                constexpr uint8_t GLF_DF  = 20;  ///< Dotted frame
                constexpr uint8_t GLF_QM  = 21;  ///< Question mark
                constexpr uint8_t GLF_CBD = 22;  ///< Checkerboard
                constexpr uint8_t GLF_CK  = 23;  ///< Checkmark
                constexpr uint8_t GLF_EN  = 24;  ///< En space
                constexpr uint8_t GLF_EM  = 25;  ///< Em space
                constexpr uint8_t GLF_DOT = 26;  ///< Dot
                constexpr uint8_t GLF_CFG = 27;  ///< WiFi config icon
                constexpr uint8_t GLF_A   = 28;  ///< Letter A
                constexpr uint8_t GLF_S   = 29;  ///< Letter S
                constexpr uint8_t GLF_P   = 30;  ///< Letter P
                constexpr uint8_t GLF_UD  = 31;  ///< Firmware update icon
                constexpr uint8_t GLF_PO  = 32;  ///< Power-on pixel (center)
            }

            /**
             * @brief Base glyph data (unrotated)
             *
             * Each glyph is 25 bytes representing a 5×5 matrix in row-major order.
             * 0 = background pixel, 1 = foreground pixel
             */
            constexpr uint8_t BASE_GLYPHS[GLYPH_COUNT][GLYPH_SIZE] = {
                // DIGIT_0
                {0,0,1,0,0, 0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,0,1,0,0},
                // DIGIT_1
                {0,0,1,0,0, 0,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,1,1,1,0},
                // DIGIT_2
                {0,1,1,0,0, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 0,1,1,1,0},
                // DIGIT_3
                {0,1,1,0,0, 0,0,0,1,0, 0,0,1,0,0, 0,0,0,1,0, 0,1,1,0,0},
                // DIGIT_4
                {0,1,0,1,0, 0,1,0,1,0, 0,0,1,1,0, 0,0,0,1,0, 0,0,0,1,0},
                // DIGIT_5
                {0,1,1,1,0, 0,1,0,0,0, 0,0,1,1,0, 0,0,0,1,0, 0,1,1,1,0},
                // DIGIT_6
                {0,0,1,1,0, 0,1,0,0,0, 0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0},
                // DIGIT_7
                {0,1,1,1,0, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 0,1,0,0,0},
                // DIGIT_8
                {0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0},
                // DIGIT_9
                {0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0, 0,0,0,1,0, 0,1,1,0,0},
                // LETTER_X
                {0,1,0,1,0, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 0,1,0,1,0},
                // WIFI
                {0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1, 0,0,1,0,0, 0,1,0,1,0},
                // SMART_TALLY
                {1,1,0,0,1, 1,1,0,1,1, 1,1,1,1,1, 1,1,0,1,1, 1,1,0,0,1},
                // LETTER_C
                {0,0,1,1,0, 0,1,0,0,0, 0,1,0,0,0, 0,1,0,0,0, 0,0,1,1,0},
                // LETTER_T
                {0,1,1,1,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0},
                // ARROW_RIGHT
                {0,0,1,0,0, 0,0,0,1,0, 1,1,1,1,1, 0,0,0,1,0, 0,0,1,0,0},
                // ARROW_LEFT
                {0,0,1,0,0, 0,1,0,0,0, 1,1,1,1,1, 0,1,0,0,0, 0,0,1,0,0},
                // HAPPY_FACE
                {0,1,0,1,0, 0,0,0,0,0, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
                // BIG_X
                {1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1},
                // FRAME
                {1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1},
                // FRAME_DOTTED
                {1,0,1,0,1, 0,0,0,0,0, 1,0,0,0,1, 0,0,0,0,0, 1,0,1,0,1},
                // QUESTION_MARK
                {0,1,1,0,0, 0,0,0,1,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,1,0,0},
                // CHECKERBOARD
                {1,0,1,0,1, 0,1,0,1,0, 1,0,1,0,1, 0,1,0,1,0, 1,0,1,0,1},
                // CHECKMARK
                {0,0,0,0,1, 0,0,0,1,0, 1,0,1,0,0, 0,1,0,0,0, 0,0,0,0,0},
                // SPACE_EN (3 inner columns)
                {0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0},
                // SPACE_EM (full display)
                {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1},
                // DOT
                {0,0,0,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,1,1,0},
                // WIFI_CONFIG
                {0,0,1,1,1, 0,0,0,0,1, 1,0,1,0,1, 1,0,0,0,0, 1,1,1,0,0},
                // LETTER_A
                {0,0,1,0,0, 0,1,0,1,0, 0,1,1,1,0, 0,1,0,1,0, 0,1,0,1,0},
                // LETTER_S
                {0,0,1,1,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,1,0, 0,1,1,0,0},
                // LETTER_P
                {0,1,1,0,0, 0,1,0,1,0, 0,1,1,0,0, 0,1,0,0,0, 0,1,0,0,0},
                // FIRMWARE_UPDATE
                {1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,1,1,0, 0,1,1,1,0},
                // CENTER_PIXEL (pixel 12 only - for power indicator overlay)
                {0,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0}
            };

        } // namespace Glyphs5x5
    } // namespace Display
} // namespace STAC

#endif // STAC_GLYPHS_5X5_H


//  --- EOF --- //
