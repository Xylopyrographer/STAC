/**
 * @file LGFX_STAC.h
 * @brief Unified LovyanGFX configuration for all STAC TFT displays
 * 
 * This file configures the LovyanGFX library using defines from the
 * board configuration file. All board-specific parameters are set
 * via preprocessor defines, allowing a single configuration class
 * to support multiple TFT display boards.
 * 
 * Required defines from board config:
 * 
 * SPI Pins:
 *   TFT_SCLK  - SPI clock pin
 *   TFT_MOSI  - SPI data pin
 *   TFT_CS    - Chip select pin
 *   TFT_DC    - Data/Command pin
 *   TFT_RST   - Reset pin (-1 if not connected)
 * 
 * Panel Configuration:
 *   TFT_PANEL_ST7789 or TFT_PANEL_ILI9341 etc. - Panel driver type
 *   DISPLAY_WIDTH    - Panel width in pixels
 *   DISPLAY_HEIGHT   - Panel height in pixels
 *   TFT_OFFSET_X     - X offset for centering (default 0)
 *   TFT_OFFSET_Y     - Y offset for centering (default 0)
 *   TFT_INVERT       - Color inversion (default true for ST7789)
 *   TFT_RGB_ORDER    - true for RGB, false for BGR (default false)
 * 
 * Backlight (optional):
 *   DISPLAY_BACKLIGHT_PWM - Define if using PWM backlight
 *   TFT_BL           - Backlight GPIO pin
 *   TFT_BL_INVERT    - true if low = on (default false)
 */

#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "Device_Config.h"

// ============================================================================
// Default values for optional parameters
// ============================================================================

#ifndef TFT_OFFSET_X
    #define TFT_OFFSET_X 0
#endif

#ifndef TFT_OFFSET_Y
    #define TFT_OFFSET_Y 0
#endif

#ifndef TFT_INVERT
    #if defined(TFT_PANEL_ST7789)
        #define TFT_INVERT true    // ST7789 typically needs inversion
    #else
        #define TFT_INVERT false
    #endif
#endif

#ifndef TFT_RGB_ORDER
    #define TFT_RGB_ORDER false    // BGR is more common
#endif

#ifndef TFT_BL_INVERT
    #define TFT_BL_INVERT false
#endif

#ifndef TFT_SPI_FREQ_WRITE
    #define TFT_SPI_FREQ_WRITE 40000000
#endif

#ifndef TFT_SPI_FREQ_READ
    #define TFT_SPI_FREQ_READ 16000000
#endif

namespace Display {

    /**
     * @brief Unified LovyanGFX display configuration for STAC
     * 
     * Supports multiple TFT panels via board configuration defines.
     * All pin assignments and panel parameters come from the board
     * config file, making this class fully portable.
     */
    class LGFX_STAC : public lgfx::LGFX_Device {
    public:
        // ====================================================================
        // Panel type selection via preprocessor (TFT panels only)
        // ====================================================================
        // Small/Medium TFT panels
        #if defined(TFT_PANEL_ST7789)
            lgfx::Panel_ST7789 _panel_instance;
        #elif defined(TFT_PANEL_ST7735)
            lgfx::Panel_ST7735 _panel_instance;
        #elif defined(TFT_PANEL_ILI9341)
            lgfx::Panel_ILI9341 _panel_instance;
        #elif defined(TFT_PANEL_ILI9342)
            lgfx::Panel_ILI9342 _panel_instance;
        #elif defined(TFT_PANEL_ILI9163)
            lgfx::Panel_ILI9163 _panel_instance;
        #elif defined(TFT_PANEL_GC9A01)
            lgfx::Panel_GC9A01 _panel_instance;
        // Larger TFT panels
        #elif defined(TFT_PANEL_ST7796)
            lgfx::Panel_ST7796 _panel_instance;
        #elif defined(TFT_PANEL_ILI9481)
            lgfx::Panel_ILI9481 _panel_instance;
        #elif defined(TFT_PANEL_ILI9486)
            lgfx::Panel_ILI9486 _panel_instance;
        #elif defined(TFT_PANEL_ILI9488)
            lgfx::Panel_ILI9488 _panel_instance;
        #elif defined(TFT_PANEL_R61529)
            lgfx::Panel_R61529 _panel_instance;
        #elif defined(TFT_PANEL_HX8357D)
            lgfx::Panel_HX8357D _panel_instance;
        #else
            #error "No TFT panel type defined! Define one of: TFT_PANEL_ST7789, TFT_PANEL_ILI9341, etc."
        #endif

        lgfx::Bus_SPI _bus_instance;

        // PWM backlight control (optional)
        #if defined(DISPLAY_BACKLIGHT_PWM) && defined(TFT_BL)
            lgfx::Light_PWM _light_instance;
        #endif

        LGFX_STAC() {
            // ================================================================
            // Configure SPI bus
            // ================================================================
            {
                auto cfg = _bus_instance.config();

                // SPI host selection (ESP32 has VSPI and HSPI)
                #if defined(TFT_SPI_HOST)
                    cfg.spi_host = TFT_SPI_HOST;
                #else
                    cfg.spi_host = VSPI_HOST;
                #endif

                cfg.spi_mode = 0;
                cfg.freq_write = TFT_SPI_FREQ_WRITE;
                cfg.freq_read = TFT_SPI_FREQ_READ;
                cfg.spi_3wire = true;
                cfg.use_lock = true;
                cfg.dma_channel = SPI_DMA_CH_AUTO;

                // Pin configuration from board defines
                cfg.pin_sclk = TFT_SCLK;
                cfg.pin_mosi = TFT_MOSI;
                cfg.pin_miso = -1;  // MISO not used for display
                cfg.pin_dc = TFT_DC;

                _bus_instance.config(cfg);
                _panel_instance.setBus(&_bus_instance);
            }

            // ================================================================
            // Configure display panel
            // ================================================================
            {
                auto cfg = _panel_instance.config();

                cfg.pin_cs = TFT_CS;
                cfg.pin_rst = TFT_RST;
                cfg.pin_busy = -1;

                cfg.panel_width = DISPLAY_WIDTH;
                cfg.panel_height = DISPLAY_HEIGHT;
                cfg.offset_x = TFT_OFFSET_X;
                cfg.offset_y = TFT_OFFSET_Y;
                cfg.offset_rotation = 0;
                cfg.dummy_read_pixel = 8;
                cfg.dummy_read_bits = 1;
                cfg.readable = true;
                cfg.invert = TFT_INVERT;
                cfg.rgb_order = TFT_RGB_ORDER;
                cfg.dlen_16bit = false;
                cfg.bus_shared = true;

                _panel_instance.config(cfg);
            }

            // ================================================================
            // Configure PWM backlight (if enabled)
            // ================================================================
            #if defined(DISPLAY_BACKLIGHT_PWM) && defined(TFT_BL)
            {
                auto cfg = _light_instance.config();

                cfg.pin_bl = TFT_BL;
                cfg.invert = TFT_BL_INVERT;
                cfg.freq = 44100;
                cfg.pwm_channel = 7;

                _light_instance.config(cfg);
                _panel_instance.setLight(&_light_instance);
            }
            #endif

            setPanel(&_panel_instance);
        }
    };

} // namespace Display
