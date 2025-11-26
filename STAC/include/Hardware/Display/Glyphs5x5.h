#ifndef STAC_GLYPHS_5X5_H
#define STAC_GLYPHS_5X5_H

#include <cstdint>

namespace Display {

    /**
     * @brief 5×5 glyph definitions for ATOM Matrix display
     *
     * Glyphs are stored as 25-byte arrays in row-major order.
     * Each byte is either 0 (background) or 1 (foreground).
     */

    constexpr uint8_t GLYPH_WIDTH = 5;
    constexpr uint8_t GLYPH_HEIGHT = 5;
    constexpr uint8_t GLYPH_SIZE = 25;

    /**
     * @brief Mnemonic constants for glyph indices (baseline compatibility)
     * 
     * These match the baseline GLF_* defines for easier code reading.
     * IMPORTANT: Digits 0-9 MUST remain at indices 0-9 for numeric display.
     */
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
    constexpr uint8_t GLF_P       = 30;  ///< Letter P
    constexpr uint8_t GLF_UD      = 31;  ///< Firmware update icon
    constexpr uint8_t GLF_PO      = 32;  ///< Power-on pixel (center)
    constexpr uint8_t GLF_CORNERS = 33;  ///< Four corner pixels

    /**
     * @brief Base glyph data (unrotated)
     *
     * Each glyph is 25 bytes representing a 5×5 matrix in row-major order.
     * 0 = background pixel, 1 = foreground pixel
     */
    constexpr uint8_t BASE_GLYPHS[][GLYPH_SIZE] = {
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
        {0,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0},
        // CORNERS (pixels 0, 4, 20, 24 - for autostart/pulsing)
        {1,0,0,0,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 1,0,0,0,1}
    };

    // Derive glyph count from array size at compile time
    constexpr uint8_t GLYPH_COUNT = sizeof(BASE_GLYPHS) / sizeof(BASE_GLYPHS[0]);

    // ========================================================================
    // ROTATION LOOKUP TABLES
    // ========================================================================

    /**
     * @brief 5×5 rotation lookup tables
     *
     * These arrays map source pixel positions to destination positions
     * for each 90° rotation. Format: destPixel = sourceLUT[sourcePixel]
     */
    namespace Rotation {
        // No rotation (UP orientation)
        constexpr uint8_t LUT_UP[25] = {
            0,1,2,3,4,
            5,6,7,8,9,
            10,11,12,13,14,
            15,16,17,18,19,
            20,21,22,23,24
        };

        // 90° clockwise (RIGHT orientation)
        constexpr uint8_t LUT_RIGHT[25] = {
            20,15,10,5,0,
            21,16,11,6,1,
            22,17,12,7,2,
            23,18,13,8,3,
            24,19,14,9,4
        };

        // 180° (DOWN orientation)
        constexpr uint8_t LUT_DOWN[25] = {
            24,23,22,21,20,
            19,18,17,16,15,
            14,13,12,11,10,
            9,8,7,6,5,
            4,3,2,1,0
        };

        // 270° clockwise / 90° counter-clockwise (LEFT orientation)
        constexpr uint8_t LUT_LEFT[25] = {
            4,9,14,19,24,
            3,8,13,18,23,
            2,7,12,17,22,
            1,6,11,16,21,
            0,5,10,15,20
        };
    }

} // namespace Display

// Forward declare template classes (defined in GlyphManager.h)
namespace Display {
    template<uint8_t SIZE>
    class GlyphManager;
}

// Forward declare template classes (defined in StartupConfig.h)
namespace Application {
    template<uint8_t GLYPH_SIZE>
    class StartupConfig;
}

// ========================================================================
// TYPE ALIASES - Automatically select correct template based on display size
// ========================================================================
namespace Display {
    // Type aliases for dimension-agnostic code
    using GlyphManagerType = GlyphManager<GLYPH_WIDTH>;
}

namespace Application {
    using StartupConfigType = StartupConfig<Display::GLYPH_WIDTH>;
}

#endif // STAC_GLYPHS_5X5_H


//  --- EOF --- //
