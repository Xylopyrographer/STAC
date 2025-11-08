#ifndef STAC_IDISPLAY_H
#define STAC_IDISPLAY_H

#include <cstdint>
#include "Colors.h"

namespace STAC {
    namespace Display {

        /**
         * @brief Abstract interface for STAC display devices
         *
         * Defines the contract that all display implementations must follow.
         * This allows for easy testing with mock displays and supports
         * different display types (5x5, 8x8, future TFT).
         */
        class IDisplay {
          public:
            virtual ~IDisplay() = default;

            /**
             * @brief Initialize the display hardware
             * @return true if initialization succeeded
             */
            virtual bool begin() = 0;

            /**
             * @brief Clear the entire display
             * @param show If true, immediately update the physical display
             */
            virtual void clear( bool show = true ) = 0;

            /**
             * @brief Set a single pixel by position
             * @param position Pixel position (0 to getPixelCount()-1)
             * @param color RGB color value
             * @param show If true, immediately update the physical display
             */
            virtual void setPixel( uint8_t position, color_t color, bool show = true ) = 0;

            /**
             * @brief Set a single pixel by X,Y coordinates
             * @param x X coordinate (0 to getWidth()-1)
             * @param y Y coordinate (0 to getHeight()-1)
             * @param color RGB color value
             * @param show If true, immediately update the physical display
             */
            virtual void setPixelXY( uint8_t x, uint8_t y, color_t color, bool show = true ) = 0;

            /**
             * @brief Fill entire display with a color
             * @param color RGB color value
             * @param show If true, immediately update the physical display
             */
            virtual void fill( color_t color, bool show = true ) = 0;

            /**
             * @brief Draw a glyph on the display
             * @param glyph Pointer to glyph data (format depends on implementation)
             * @param foreground Foreground color (for '1' bits)
             * @param background Background color (for '0' bits)
             * @param show If true, immediately update the physical display
             */
            virtual void drawGlyph( const uint8_t* glyph, color_t foreground, color_t background, bool show = true ) = 0;

            /**
             * @brief Set display brightness
             * @param brightness Brightness level (0-255)
             * @param show If true, immediately update the physical display
             */
            virtual void setBrightness( uint8_t brightness, bool show = true ) = 0;

            /**
             * @brief Get current brightness
             * @return Current brightness level (0-255)
             */
            virtual uint8_t getBrightness() const = 0;

            /**
             * @brief Force update of physical display from buffer
             */
            virtual void show() = 0;

            /**
             * @brief Get display width
             * @return Width in pixels
             */
            virtual uint8_t getWidth() const = 0;

            /**
             * @brief Get display height
             * @return Height in pixels
             */
            virtual uint8_t getHeight() const = 0;

            /**
             * @brief Get total pixel count
             * @return Total number of pixels
             */
            virtual uint8_t getPixelCount() const = 0;
        };

    } // namespace Display
} // namespace STAC

#endif // STAC_IDISPLAY_H


//  --- EOF --- //
