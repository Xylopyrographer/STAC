#include "Hardware/Display/GlyphManager.h"
// Glyph data and rotation LUTs included via GlyphManager.h
#include <cstring>

    namespace Display {

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
            // LUTs are now defined in the glyph header files (Glyphs5x5.h or Glyphs8x8.h)\n            // and accessible via Display::Rotation namespace
            switch ( currentOrientation ) {
                case Orientation::DOWN:
                    return Display::Rotation::LUT_DOWN;
                case Orientation::LEFT:
                    return Display::Rotation::LUT_LEFT;
                case Orientation::RIGHT:
                    return Display::Rotation::LUT_RIGHT;
                case Orientation::UP:
                case Orientation::FLAT:
                case Orientation::UNKNOWN:
                default:
                    return Display::Rotation::LUT_UP;
            }
        }

        template<uint8_t SIZE>
        const uint8_t* GlyphManager<SIZE>::getBaseGlyphs() const {
            return &Display::BASE_GLYPHS[0][0];
        }

        // ========================================================================
        // EXPLICIT TEMPLATE INSTANTIATIONS
        // ========================================================================

        template class GlyphManager<5>;
        template class GlyphManager<8>;
        template class GlyphManager<1>;  // TFT stub glyphs (for compatibility layer)

    } // namespace Display



//  --- EOF --- //
