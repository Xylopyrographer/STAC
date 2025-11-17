#ifndef STAC_GLYPH_MANAGER_H
#define STAC_GLYPH_MANAGER_H

#include <cstdint>
#include <array>
#include "Config/Types.h"
#include "Device_Config.h"
#include "Hardware/Display/Glyphs5x5.h"
#include "Hardware/Display/Glyphs8x8.h"

namespace STAC {
    namespace Display {

        /**
         * @brief Manages glyph storage and rotation based on device orientation
         *
         * This class handles:
         * - Glyph data storage (compile-time selected for 5×5 or 8×8)
         * - Rotation of glyphs based on IMU orientation
         * - Access to rotated glyph data for display rendering
         *
         * Template based on display size for compile-time optimization.
         */
        template<uint8_t SIZE>
        class GlyphManager {
          public:
            static constexpr uint8_t GLYPH_SIZE = SIZE * SIZE;
            // Get GLYPH_COUNT from the appropriate glyph header
            static constexpr uint8_t GLYPH_COUNT = ( SIZE == 5 ) ? Glyphs5x5::GLYPH_COUNT : Glyphs8x8::GLYPH_COUNT;

            /**
             * @brief Construct GlyphManager with initial orientation
             * @param orientation Initial device orientation
             */
            explicit GlyphManager( Orientation orientation = Orientation::UP );

            ~GlyphManager() = default;

            /**
             * @brief Update glyph rotation based on new orientation
             * @param orientation New device orientation
             */
            void updateOrientation( Orientation orientation );

            /**
             * @brief Get rotated glyph data by index
             * @param glyphIndex Index of the glyph (0-31)
             * @return Pointer to rotated glyph data (SIZE×SIZE bytes)
             */
            const uint8_t* getGlyph( uint8_t glyphIndex ) const;

            /**
             * @brief Get rotated glyph data for a digit (0-9)
             * @param digit Digit value (0-9)
             * @return Pointer to rotated glyph data, or nullptr if digit invalid
             */
            const uint8_t* getDigitGlyph( uint8_t digit ) const;

            /**
             * @brief Get current orientation
             * @return Current orientation setting
             */
            Orientation getCurrentOrientation() const {
                return currentOrientation;
            }

            /**
             * @brief Get glyph size (width = height)
             * @return Glyph size in pixels
             */
            static constexpr uint8_t getGlyphSize() {
                return SIZE;
            }

          private:
            Orientation currentOrientation;

            // Storage for rotated glyphs (populated on orientation change)
            std::array<std::array<uint8_t, GLYPH_SIZE>, GLYPH_COUNT> rotatedGlyphs;

            /**
             * @brief Rotate all glyphs based on orientation
             */
            void rotateAllGlyphs();

            /**
             * @brief Get rotation lookup table for current orientation
             * @return Pointer to rotation LUT array
             */
            const uint8_t* getRotationLUT() const;

            /**
             * @brief Get base (unrotated) glyph data
             * @return Pointer to base glyph array
             */
            const uint8_t* getBaseGlyphs() const;
        };

        // Type aliases for specific display sizes
        using GlyphManager5x5 = GlyphManager<5>;
        using GlyphManager8x8 = GlyphManager<8>;

    } // namespace Display
} // namespace STAC

#endif // STAC_GLYPH_MANAGER_H


//  --- EOF --- //
