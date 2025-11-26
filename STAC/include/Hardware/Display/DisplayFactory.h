#ifndef STAC_DISPLAY_FACTORY_H
#define STAC_DISPLAY_FACTORY_H

#include "IDisplay.h"
#include "../../Device_Config.h"
#include "../../Config/Constants.h"
#include <memory>

// Include the appropriate display implementation based on glyph width define
#if defined(GLYPH_WIDTH_5)
    #include "Matrix5x5/Display5x5.h"
#elif defined(GLYPH_WIDTH_8)
    #include "Matrix8x8/Display8x8.h"
#else
    #error "No GLYPH_WIDTH_* defined in board configuration!"
#endif


    namespace Display {

        /**
         * @brief Factory for creating display instances
         *
         * Automatically selects the correct display implementation
         * based on GLYPH_WIDTH_* define from board config.
         */
        class DisplayFactory {
          public:
            /**
             * @brief Create a display instance for the configured board
             * @return Unique pointer to IDisplay implementation
             */
            static std::unique_ptr<IDisplay> create() {
#if defined(GLYPH_WIDTH_5)
                return std::make_unique<Display5x5>(
                           Config::Pins::DISPLAY_DATA,
                           Config::Display::MATRIX_SIZE,
                           DISPLAY_LED_TYPE
                       );
#elif defined(GLYPH_WIDTH_8)
                return std::make_unique<Display8x8>(
                           Config::Pins::DISPLAY_DATA,
                           Config::Display::MATRIX_SIZE,
                           DISPLAY_LED_TYPE
                       );
#else
#error "Unsupported display configuration"
#endif
            }

            /**
             * @brief Get display type name as string
             * @return Display type description
             */
            static const char *getDisplayType() {
#if defined(GLYPH_SIZE_5X5)
                return "5x5 LED Matrix";
#elif defined(GLYPH_SIZE_8X8)
                return "8x8 LED Matrix";
#else
                return "Unknown";
#endif
            }
        };

    } // namespace Display


#endif // STAC_DISPLAY_FACTORY_H


//  --- EOF --- //
