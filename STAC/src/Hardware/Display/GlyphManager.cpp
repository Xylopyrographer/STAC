#include "Hardware/Display/GlyphManager.h"
#include "Hardware/Display/Glyphs5x5.h"
#include "Hardware/Display/Glyphs8x8.h"
#include <cstring>

namespace STAC {
    namespace Display {

        // ========================================================================
        // ROTATION LOOKUP TABLES
        // ========================================================================

        /**
         * @brief 5×5 rotation lookup tables
         *
         * These arrays map source pixel positions to destination positions
         * for each 90° rotation. Format: destPixel = sourceLUT[sourcePixel]
         */
        namespace Rotation5x5 {
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

        /**
         * @brief 8×8 rotation lookup tables
         */
        namespace Rotation8x8 {
            // No rotation (UP orientation)
            constexpr uint8_t LUT_UP[64] = {
                0,1,2,3,4,5,6,7,
                8,9,10,11,12,13,14,15,
                16,17,18,19,20,21,22,23,
                24,25,26,27,28,29,30,31,
                32,33,34,35,36,37,38,39,
                40,41,42,43,44,45,46,47,
                48,49,50,51,52,53,54,55,
                56,57,58,59,60,61,62,63
            };

            // 90° clockwise (RIGHT orientation)
            constexpr uint8_t LUT_RIGHT[64] = {
                56,48,40,32,24,16,8,0,
                57,49,41,33,25,17,9,1,
                58,50,42,34,26,18,10,2,
                59,51,43,35,27,19,11,3,
                60,52,44,36,28,20,12,4,
                61,53,45,37,29,21,13,5,
                62,54,46,38,30,22,14,6,
                63,55,47,39,31,23,15,7
            };

            // 180° (DOWN orientation)
            constexpr uint8_t LUT_DOWN[64] = {
                63,62,61,60,59,58,57,56,
                55,54,53,52,51,50,49,48,
                47,46,45,44,43,42,41,40,
                39,38,37,36,35,34,33,32,
                31,30,29,28,27,26,25,24,
                23,22,21,20,19,18,17,16,
                15,14,13,12,11,10,9,8,
                7,6,5,4,3,2,1,0
            };

            // 270° clockwise / 90° counter-clockwise (LEFT orientation)
            constexpr uint8_t LUT_LEFT[64] = {
                7,15,23,31,39,47,55,63,
                6,14,22,30,38,46,54,62,
                5,13,21,29,37,45,53,61,
                4,12,20,28,36,44,52,60,
                3,11,19,27,35,43,51,59,
                2,10,18,26,34,42,50,58,
                1,9,17,25,33,41,49,57,
                0,8,16,24,32,40,48,56
            };
        }

        // ========================================================================
        // CONSTRUCTOR
        // ========================================================================

        template<uint8_t SIZE>
        GlyphManager<SIZE>::GlyphManager( Orientation orientation )
            : currentOrientation( orientation )
        {
            rotateAllGlyphs();
        }

        // ========================================================================
        // PUBLIC METHODS
        // ========================================================================

        template<uint8_t SIZE>
        void GlyphManager<SIZE>::updateOrientation( Orientation orientation ) {
            if ( orientation != currentOrientation ) {
                currentOrientation = orientation;
                rotateAllGlyphs();
            }
        }

        template<uint8_t SIZE>
        const uint8_t* GlyphManager<SIZE>::getGlyph( uint8_t glyphIndex ) const {
            if ( glyphIndex >= GLYPH_COUNT ) {
                return nullptr;
            }
            return rotatedGlyphs[glyphIndex].data();
        }

        template<uint8_t SIZE>
        const uint8_t* GlyphManager<SIZE>::getDigitGlyph( uint8_t digit ) const {
            if ( digit > 9 ) {
                return nullptr;
            }
            return getGlyph( digit );
        }

        // ========================================================================
        // PRIVATE METHODS
        // ========================================================================

        template<uint8_t SIZE>
        void GlyphManager<SIZE>::rotateAllGlyphs() {
            const uint8_t* baseGlyphs = getBaseGlyphs();
            const uint8_t* rotationLUT = getRotationLUT();

            // Rotate each glyph
            for ( uint8_t glyphIdx = 0; glyphIdx < GLYPH_COUNT; ++glyphIdx ) {
                const uint8_t* srcGlyph = baseGlyphs + ( glyphIdx * GLYPH_SIZE );

                for ( uint8_t pixelIdx = 0; pixelIdx < GLYPH_SIZE; ++pixelIdx ) {
                    rotatedGlyphs[glyphIdx][pixelIdx] = srcGlyph[rotationLUT[pixelIdx]];
                }
            }
        }

        template<uint8_t SIZE>
        const uint8_t* GlyphManager<SIZE>::getRotationLUT() const {
            if constexpr ( SIZE == 5 ) {
                switch ( currentOrientation ) {
                    case Orientation::DOWN:
                        return Rotation5x5::LUT_DOWN;
                    case Orientation::LEFT:
                        return Rotation5x5::LUT_LEFT;
                    case Orientation::RIGHT:
                        return Rotation5x5::LUT_RIGHT;
                    case Orientation::UP:
                    case Orientation::FLAT:
                    case Orientation::UNKNOWN:
                    default:
                        return Rotation5x5::LUT_UP;
                }
            } else if constexpr ( SIZE == 8 ) {
                switch ( currentOrientation ) {
                    case Orientation::DOWN:
                        return Rotation8x8::LUT_DOWN;
                    case Orientation::LEFT:
                        return Rotation8x8::LUT_LEFT;
                    case Orientation::RIGHT:
                        return Rotation8x8::LUT_RIGHT;
                    case Orientation::UP:
                    case Orientation::FLAT:
                    case Orientation::UNKNOWN:
                    default:
                        return Rotation8x8::LUT_UP;
                }
            }
            return nullptr;  // Should never reach here
        }

        template<uint8_t SIZE>
        const uint8_t* GlyphManager<SIZE>::getBaseGlyphs() const {
            if constexpr ( SIZE == 5 ) {
                return &Glyphs5x5::BASE_GLYPHS[0][0];
            } else if constexpr ( SIZE == 8 ) {
                return &Glyphs8x8::BASE_GLYPHS[0][0];
            }
            return nullptr;  // Should never reach here
        }

        // ========================================================================
        // EXPLICIT TEMPLATE INSTANTIATIONS
        // ========================================================================

        template class GlyphManager<5>;
        template class GlyphManager<8>;

    } // namespace Display
} // namespace STAC


//  --- EOF --- //
