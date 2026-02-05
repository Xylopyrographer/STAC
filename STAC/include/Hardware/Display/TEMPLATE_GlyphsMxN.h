/**
 * @file TEMPLATE_GlyphsMxN.h
 * @brief Template for creating glyph definitions for a new display size
 *
 * INSTRUCTIONS:
 * 1. Copy this file to GlyphsWxH.h (e.g., Glyphs7x7.h, Glyphs16x16.h)
 * 2. Update the header guard (replace TEMPLATE_GLYPHS_MXN)
 * 3. Set GLYPH_WIDTH and GLYPH_HEIGHT to your display dimensions
 * 4. Create glyph data arrays for all required glyphs (see GLF_* indices)
 * 5. Generate rotation lookup tables for your display size
 * 6. Update type aliases at the bottom
 *
 * GLYPH DATA FORMAT:
 * - Each glyph is a 1D array of (WIDTH × HEIGHT) bytes
 * - Row-major order: pixels are stored row by row, left to right, top to bottom
 * - Each byte is either 0 (background/off) or 1 (foreground/on)
 * - Display driver applies colors at render time
 *
 * REQUIRED GLYPHS (indices 0-33):
 * - Digits 0-9 MUST be at indices 0-9 (used by getDigitGlyph)
 * - Other glyphs can be in any order but standard indices are recommended
 * - See existing Glyphs5x5.h or Glyphs8x8.h for complete glyph set
 *
 * ROTATION LOOKUP TABLES:
 * - LUT_UP: No rotation (identity mapping)
 * - LUT_RIGHT: 90° clockwise rotation
 * - LUT_DOWN: 180° rotation
 * - LUT_LEFT: 270° clockwise (90° counter-clockwise)
 * - Each LUT maps source pixel position to destination position
 * - Formula: destPixel = sourceLUT[sourcePixel]
 */

#ifndef TEMPLATE_GLYPHS_MXN_H
#define TEMPLATE_GLYPHS_MXN_H

#include <cstdint>

namespace Display {

    /**
     * @brief MxN glyph definitions for [Your Display Name]
     *
     * Glyphs are stored as (M×N)-byte arrays in row-major order.
     * Each byte is either 0 (background) or 1 (foreground).
     */

    // ============================================================================
    // DISPLAY DIMENSIONS
    // ============================================================================

    constexpr uint8_t GLYPH_WIDTH = <M>;   // <REQUIRED: Your display width, e.g., 5, 7, 8, 16>
    constexpr uint8_t GLYPH_HEIGHT = <N>;  // <REQUIRED: Your display height, e.g., 5, 7, 8, 16>
    constexpr uint8_t GLYPH_SIZE = GLYPH_WIDTH * GLYPH_HEIGHT;

    // ============================================================================
    // GLYPH INDEX CONSTANTS
    // ============================================================================

    /**
     * @brief Mnemonic constants for glyph indices
     *
     * CRITICAL: Digits 0-9 MUST remain at indices 0-9 for getDigitGlyph() to work.
     * Other glyphs can be at any index, but maintaining consistency with 5×5/8×8
     * makes the codebase easier to understand.
     */

    // Digits (MUST be indices 0-9)
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

    // Standard glyphs (indices 10+, order matches 5×5/8×8 where possible)
    constexpr uint8_t GLF_A       = 10;  ///< Letter A (for autostart)
    constexpr uint8_t GLF_C       = 11;  ///< Letter C (for camera mode)
    constexpr uint8_t GLF_P       = 12;  ///< Letter P (for peripheral mode)
    constexpr uint8_t GLF_S       = 13;  ///< Letter S (for startup)
    constexpr uint8_t GLF_T       = 14;  ///< Letter T (for talent mode)
    constexpr uint8_t GLF_X       = 15;  ///< Letter X
    constexpr uint8_t GLF_QM      = 16;  ///< Question mark (junk reply error)
    constexpr uint8_t GLF_WIFI    = 17;  ///< WiFi icon
    constexpr uint8_t GLF_CFG     = 18;  ///< Configuration required icon
    constexpr uint8_t GLF_BX      = 19;  ///< Big X (error indicators)
    constexpr uint8_t GLF_CK      = 20;  ///< Checkmark (confirmations)
    constexpr uint8_t GLF_CBD     = 21;  ///< Checkerboard (brightness adjustment)
    constexpr uint8_t GLF_FM      = 22;  ///< Frame solid (factory reset)
    constexpr uint8_t GLF_DF      = 23;  ///< Dotted frame (camera operator unselected)
    constexpr uint8_t GLF_EN      = 24;  ///< En space (blanks center columns)
    constexpr uint8_t GLF_PO      = 25;  ///< Power-on indicator (center pixel(s))
    constexpr uint8_t GLF_UD      = 26;  ///< Firmware update icon
    constexpr uint8_t GLF_CORNERS = 27;  ///< Corner pixels (autostart indicator)

    // Add more glyphs as needed for your display...

    // ============================================================================
    // BASE GLYPH DATA (Unrotated)
    // ============================================================================

    /**
     * @brief Base glyph data arrays
     *
     * Each glyph is GLYPH_SIZE bytes representing M×N matrix in row-major order.
     * 0 = background pixel (off), 1 = foreground pixel (on)
     *
     * EXAMPLE for 5×5 (25 bytes per glyph):
     * Row 0: bytes 0-4
     * Row 1: bytes 5-9
     * Row 2: bytes 10-14
     * Row 3: bytes 15-19
     * Row 4: bytes 20-24
     *
     * Visual layout for digit "0":
     *   . . # . .    (row 0)
     *   . # . # .    (row 1)
     *   . # . # .    (row 2)
     *   . # . # .    (row 3)
     *   . . # . .    (row 4)
     *
     * Becomes: {0,0,1,0,0, 0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,0,1,0,0}
     */

    constexpr uint8_t BASE_GLYPHS[][ GLYPH_SIZE ] = {
        // GLF_0 (index 0) - Digit 0
        {
            // <REQUIRED: Fill in your glyph data>
            // Example structure (replace with your design):
            0, 0, 1, 0, 0, // Row 0
            0, 1, 0, 1, 0, // Row 1
            0, 1, 0, 1, 0, // Row 2
            0, 1, 0, 1, 0, // Row 3
            0, 0, 1, 0, 0 // Row 4
        },

        // GLF_1 (index 1) - Digit 1
        {
            // <REQUIRED: Fill in your digit 1 design>
        },

        // GLF_2 (index 2) - Digit 2
        {
            // <REQUIRED: Fill in your digit 2 design>
        },

        // GLF_3 (index 3) - Digit 3
        {
            // <REQUIRED: Fill in your digit 3 design>
        },

        // GLF_4 (index 4) - Digit 4
        {
            // <REQUIRED: Fill in your digit 4 design>
        },

        // GLF_5 (index 5) - Digit 5
        {
            // <REQUIRED: Fill in your digit 5 design>
        },

        // GLF_6 (index 6) - Digit 6
        {
            // <REQUIRED: Fill in your digit 6 design>
        },

        // GLF_7 (index 7) - Digit 7
        {
            // <REQUIRED: Fill in your digit 7 design>
        },

        // GLF_8 (index 8) - Digit 8
        {
            // <REQUIRED: Fill in your digit 8 design>
        },

        // GLF_9 (index 9) - Digit 9
        {
            // <REQUIRED: Fill in your digit 9 design>
        },

        // GLF_A (index 10) - Letter A
        {
            // <REQUIRED: Fill in your letter A design>
        },

        // GLF_C (index 11) - Letter C
        {
            // <REQUIRED: Fill in your letter C design>
        },

        // GLF_P (index 12) - Letter P
        {
            // <REQUIRED: Fill in your letter P design>
        },

        // GLF_S (index 13) - Letter S
        {
            // <REQUIRED: Fill in your letter S design>
        },

        // GLF_T (index 14) - Letter T
        {
            // <REQUIRED: Fill in your letter T design>
        },

        // GLF_X (index 15) - Letter X
        {
            // <REQUIRED: Fill in your letter X design>
        },

        // GLF_QM (index 16) - Question mark
        {
            // <REQUIRED: Fill in your question mark design>
        },

        // GLF_WIFI (index 17) - WiFi icon
        {
            // <REQUIRED: Fill in your WiFi icon design>
        },

        // GLF_CFG (index 18) - Configuration required icon
        {
            // <REQUIRED: Fill in your config icon design>
        },

        // GLF_BX (index 19) - Big X (for error displays)
        {
            // <REQUIRED: Fill in your big X design>
        },

        // GLF_CK (index 20) - Checkmark
        {
            // <REQUIRED: Fill in your checkmark design>
        },

        // GLF_CBD (index 21) - Checkerboard (for brightness adjustment)
        {
            // <REQUIRED: Fill in checkerboard pattern>
            // Typical pattern: alternating 1s and 0s
        },

        // GLF_FM (index 22) - Solid frame
        {
            // <REQUIRED: Fill in solid frame (border pixels = 1, interior = 0)>
        },

        // GLF_DF (index 23) - Dotted frame
        {
            // <REQUIRED: Fill in dotted frame (border pixels alternating)>
        },

        // GLF_EN (index 24) - En space (blanks center columns for number overlay)
        {
            // <REQUIRED: Fill in en space pattern>
            // Typically: outer columns = 1, center columns = 0
        },

        // GLF_PO (index 25) - Power-on indicator
        {
            // <REQUIRED: Fill in power-on pixel(s)>
            // For 5×5: center pixel (position 12)
            // For 8×8: center 4 pixels (positions 27, 28, 35, 36)
            // All other positions = 0
        },

        // GLF_UD (index 26) - Firmware update icon
        {
            // <REQUIRED: Fill in firmware update icon>
        },

        // GLF_CORNERS (index 27) - Corner pixels (for autostart blinking)
        {
            // <REQUIRED: Fill in corner pixels>
            // For 5×5: pixels 0, 4, 20, 24 = 1, rest = 0
            // For 8×8: pixels 0, 7, 56, 63 = 1, rest = 0
        }

        // Add more glyphs as needed...
    };

    // Derive glyph count from array size at compile time
    constexpr uint8_t GLYPH_COUNT = sizeof( BASE_GLYPHS ) / sizeof( BASE_GLYPHS[ 0 ] );

    // ============================================================================
    // ROTATION LOOKUP TABLES
    // ============================================================================

    /**
     * @brief Rotation lookup tables for M×N display
     *
     * These arrays map source pixel positions to destination positions
     * for each 90° rotation. Format: destPixel = sourceLUT[sourcePixel]
     *
     * GENERATING ROTATION LUTs:
     *
     * 1. LUT_UP (identity - no rotation):
     *    Simple sequence: 0, 1, 2, 3, ..., (M×N - 1)
     *
     * 2. LUT_RIGHT (90° clockwise):
     *    For each pixel at position (row, col):
     *      source_index = row * WIDTH + col
     *      dest_row = col
     *      dest_col = HEIGHT - 1 - row
     *      dest_index = dest_row * WIDTH + dest_col
     *      LUT_RIGHT[source_index] = dest_index
     *
     * 3. LUT_DOWN (180°):
     *    For each pixel at position (row, col):
     *      source_index = row * WIDTH + col
     *      dest_row = HEIGHT - 1 - row
     *      dest_col = WIDTH - 1 - col
     *      dest_index = dest_row * WIDTH + dest_col
     *      LUT_DOWN[source_index] = dest_index
     *
     * 4. LUT_LEFT (270° clockwise / 90° counter-clockwise):
     *    For each pixel at position (row, col):
     *      source_index = row * WIDTH + col
     *      dest_row = WIDTH - 1 - col
     *      dest_col = row
     *      dest_index = dest_row * WIDTH + dest_col
     *      LUT_LEFT[source_index] = dest_index
     *
     * VERIFICATION:
     * - Apply LUT to a simple test pattern (e.g., arrow pointing right)
     * - Verify rotated output matches expected orientation
     * - Test with actual hardware to confirm display matches physical rotation
     */

    namespace Rotation {
        // No rotation (UP orientation) - Identity mapping
        constexpr uint8_t LUT_UP[ GLYPH_SIZE ] = {
            // <REQUIRED: Fill in identity mapping>
            // Example for 5×5: {0,1,2,3,4, 5,6,7,8,9, 10,11,12,13,14, 15,16,17,18,19, 20,21,22,23,24}
        };

        // 90° clockwise (RIGHT orientation)
        constexpr uint8_t LUT_RIGHT[ GLYPH_SIZE ] = {
            // <REQUIRED: Fill in 90° clockwise rotation mapping>
            // Use formula above or reference existing 5×5/8×8 patterns
        };

        // 180° (DOWN orientation)
        constexpr uint8_t LUT_DOWN[ GLYPH_SIZE ] = {
            // <REQUIRED: Fill in 180° rotation mapping>
        };

        // 270° clockwise / 90° counter-clockwise (LEFT orientation)
        constexpr uint8_t LUT_LEFT[ GLYPH_SIZE ] = {
            // <REQUIRED: Fill in 270° clockwise rotation mapping>
        };
    }

} // namespace Display

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

// Forward declare template classes (defined in their respective headers)
namespace Display {
    template<uint8_t SIZE>
    class GlyphManager;
}

namespace Application {
    template<uint8_t GLYPH_SIZE>
    class StartupConfig;
}

// ============================================================================
// TYPE ALIASES - Automatic template selection
// ============================================================================

/**
 * @brief Dimension-agnostic type aliases
 *
 * These aliases automatically select the correct template instantiation
 * based on GLYPH_WIDTH defined in this file. Application code uses these
 * types without needing to know the specific display dimensions.
 *
 * This eliminates all #ifdef conditionals in application code.
 */

namespace Display {
    // Type alias for this display size's GlyphManager
    using GlyphManagerType = GlyphManager<GLYPH_WIDTH>;
}

namespace Application {
    // Type alias for this display size's StartupConfig
    using StartupConfigType = StartupConfig<Display::GLYPH_WIDTH>;
}

#endif // TEMPLATE_GLYPHS_MXN_H

/**
 * TESTING CHECKLIST:
 *
 * Glyph Data Verification:
 * [ ] All 28+ glyphs defined with correct GLYPH_SIZE
 * [ ] Digits 0-9 are at indices 0-9
 * [ ] Each glyph array has exactly GLYPH_SIZE elements
 * [ ] Visual appearance verified (draw on paper/spreadsheet first)
 *
 * Rotation LUT Verification:
 * [ ] LUT_UP is identity mapping (0, 1, 2, ..., GLYPH_SIZE-1)
 * [ ] LUT_RIGHT rotates test pattern 90° clockwise correctly
 * [ ] LUT_DOWN rotates test pattern 180° correctly
 * [ ] LUT_LEFT rotates test pattern 270° clockwise correctly
 * [ ] All LUTs have exactly GLYPH_SIZE elements
 * [ ] All LUT values are in range [0, GLYPH_SIZE-1]
 *
 * Integration Testing:
 * [ ] Compiles without errors or warnings
 * [ ] GlyphManager creates successfully with this size
 * [ ] StartupConfig works with this display size
 * [ ] Numbers 0-9 display correctly on hardware
 * [ ] All glyphs render correctly at all orientations
 * [ ] IMU orientation changes rotate display correctly
 * [ ] Autostart corners blink at correct pixel positions
 * [ ] Power-on pixel appears in center of display
 *
 * Performance:
 * [ ] Glyph rotation executes without noticeable delay
 * [ ] Display updates smoothly during orientation changes
 * [ ] No memory issues (stack overflow, heap fragmentation)
 */

//  --- EOF --- //
