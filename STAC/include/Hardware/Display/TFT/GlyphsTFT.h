/**
 * @file GlyphsTFT.h
 * @brief Glyph definitions for TFT displays
 *
 * For TFT displays, glyphs are drawn using graphics primitives rather than
 * bitmap data. This file provides compatible index definitions to match the
 * LED matrix glyph system, but the actual rendering is done using vector graphics.
 *
 * The DisplayTFT class interprets glyph indices and draws appropriate graphics
 * using LovyanGFX primitives (arcs, lines, fills, text, etc.)
 */

#pragma once

#include <cstdint>

namespace Display {

    // ========================================================================
    // TFT Glyph System Compatibility Layer
    // ========================================================================
    // TFT displays use the same glyph index constants as LED matrices, but
    // render them using graphics primitives instead of bitmaps.
    //
    // The GlyphManager interface remains the same - it returns pointers to
    // "glyph data" which for TFT are stub arrays. The DisplayTFT class
    // intercepts drawGlyph() calls and renders using primitives based on
    // the glyph pointer address (which encodes the glyph index).

    // ========================================================================
    // Display Size Constants (for template compatibility)
    // ========================================================================

    // TFT uses a nominal "1x1" glyph size for GlyphManager template
    // Actual rendering is resolution-independent using primitives
    constexpr uint8_t GLYPH_WIDTH = 1;
    constexpr uint8_t GLYPH_HEIGHT = 1;
    constexpr uint8_t GLYPH_SIZE = 1;

    // ========================================================================
    // Glyph Index Constants (compatible with Glyphs5x5.h)
    // ========================================================================
    // These must match the indices used in the LED matrix glyph files
    // so STACApp code works unchanged

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
    constexpr uint8_t GLF_RA  = 15;  ///< UNUSED - reserved
    constexpr uint8_t GLF_LA  = 16;  ///< UNUSED - reserved
    constexpr uint8_t GLF_HF  = 17;  ///< UNUSED - reserved
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
    constexpr uint8_t GLF_CORNERS = 33;  ///< Four corner pixels
    constexpr uint8_t GLF_FR  = 34;  ///< Factory reset icon (circular arrow)
    constexpr uint8_t GLF_P_CANCEL = 35; ///< Letter P with cancel slash (PMode cancel)
    constexpr uint8_t GLF_N   = 36;  ///< Letter N

    // ========================================================================
    // Stub Glyph Data (for GlyphManager compatibility)
    // ========================================================================
    // Each "glyph" is a 1-byte array containing its index value.
    // GlyphManager returns pointers to these, and DisplayTFT extracts
    // the index to determine which primitive to draw.

    constexpr uint8_t BASE_GLYPHS[][ GLYPH_SIZE ] = {
        {GLF_0}, {GLF_1}, {GLF_2}, {GLF_3}, {GLF_4},
        {GLF_5}, {GLF_6}, {GLF_7}, {GLF_8}, {GLF_9},
        {GLF_X}, {GLF_WIFI}, {GLF_ST}, {GLF_C}, {GLF_T},
        {GLF_RA}, {GLF_LA}, {GLF_HF}, {GLF_BX}, {GLF_FM},
        {GLF_DF}, {GLF_QM}, {GLF_CBD}, {GLF_CK}, {GLF_EN},
        {GLF_EM}, {GLF_DOT}, {GLF_CFG}, {GLF_A}, {GLF_S},
        {GLF_P}, {GLF_UD}, {GLF_PO}, {GLF_CORNERS}, {GLF_FR},
        {GLF_P_CANCEL}, {GLF_N}
    };

    constexpr uint8_t GLYPH_COUNT = sizeof( BASE_GLYPHS ) / sizeof( BASE_GLYPHS[ 0 ] );

    // ========================================================================
    // Rotation Lookup Tables (no-op for TFT)
    // ========================================================================
    // TFT handles rotation in hardware or via display orientation setting.
    // Provide identity LUTs for GlyphManager compatibility.

    namespace Rotation {
        // All orientations use identity mapping for 1x1 "glyphs"
        // TFT handles rotation in hardware, so these are no-op identity LUTs
        constexpr uint8_t LUT_ROTATE_0[ GLYPH_SIZE ] = {0};
        constexpr uint8_t LUT_ROTATE_90[ GLYPH_SIZE ] = {0};
        constexpr uint8_t LUT_ROTATE_180[ GLYPH_SIZE ] = {0};
        constexpr uint8_t LUT_ROTATE_270[ GLYPH_SIZE ] = {0};
    }

} // namespace Display
