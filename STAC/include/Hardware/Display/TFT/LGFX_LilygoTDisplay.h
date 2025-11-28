/**
 * @file LGFX_LilygoTDisplay.h
 * @brief LovyanGFX configuration for LilyGo T-Display
 * 
 * This file configures the LovyanGFX library for the LilyGo T-Display
 * which uses an ST7789 TFT display (135x240 pixels).
 * 
 * Pin Reference (V1.1):
 * | Function   | GPIO |
 * | ---------- | ---- |
 * | TFT_MOSI   | 19   |
 * | TFT_SCLK   | 18   |
 * | TFT_CS     | 5    |
 * | TFT_DC     | 16   |
 * | TFT_RST    | 23   |
 * | TFT_BL     | 4    |
 */

#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

namespace Display {

    /**
     * @brief LovyanGFX display configuration for LilyGo T-Display
     * 
     * ST7789 TFT Display specifications:
     * - Resolution: 135 x 240 pixels
     * - Interface: SPI
     * - Color depth: 16-bit (RGB565)
     */
    class LGFX_LilygoTDisplay : public lgfx::LGFX_Device {
    public:
        // Panel and bus instances
        lgfx::Panel_ST7789 _panel_instance;
        lgfx::Bus_SPI _bus_instance;
        lgfx::Light_PWM _light_instance;  // PWM backlight control

        LGFX_LilygoTDisplay() {
            // Configure SPI bus
            {
                auto cfg = _bus_instance.config();

                cfg.spi_host = VSPI_HOST;     // Select SPI host (VSPI_HOST or HSPI_HOST)
                cfg.spi_mode = 0;             // SPI mode (0-3)
                cfg.freq_write = 40000000;    // SPI clock for write (max 80MHz, but 40MHz is more stable)
                cfg.freq_read = 16000000;     // SPI clock for read
                cfg.spi_3wire = true;         // Set true if using MOSI for both send and receive
                cfg.use_lock = true;          // Set true to use transaction lock
                cfg.dma_channel = SPI_DMA_CH_AUTO; // Set DMA channel (0=DMA not used, 1=ch1, 2=ch2, auto=automatic)
                
                // Pin configuration - LilyGo T-Display specific
                cfg.pin_sclk = 18;            // SPI SCLK pin
                cfg.pin_mosi = 19;            // SPI MOSI pin
                cfg.pin_miso = -1;            // SPI MISO pin (-1 = not used)
                cfg.pin_dc = 16;              // SPI D/C pin (data/command)

                _bus_instance.config(cfg);
                _panel_instance.setBus(&_bus_instance);
            }

            // Configure display panel
            {
                auto cfg = _panel_instance.config();

                cfg.pin_cs = 5;               // Chip select pin
                cfg.pin_rst = 23;             // Reset pin
                cfg.pin_busy = -1;            // Busy pin (-1 = not used)

                cfg.panel_width = 135;        // Actual displayable width
                cfg.panel_height = 240;       // Actual displayable height
                cfg.offset_x = 52;            // Panel X offset (for ST7789 centering)
                cfg.offset_y = 40;            // Panel Y offset (for ST7789 centering)
                cfg.offset_rotation = 0;      // Rotation offset (0-7)
                cfg.dummy_read_pixel = 8;     // Dummy read bits before pixel read
                cfg.dummy_read_bits = 1;      // Dummy read bits for non-pixel reads
                cfg.readable = true;          // True if data can be read back
                cfg.invert = true;            // True if panel color is inverted
                cfg.rgb_order = false;        // True for RGB, false for BGR
                cfg.dlen_16bit = false;       // True for 16-bit data length parallel panels
                cfg.bus_shared = true;        // True if bus is shared with other devices

                _panel_instance.config(cfg);
            }

            // Configure PWM backlight
            {
                auto cfg = _light_instance.config();

                cfg.pin_bl = 4;               // Backlight GPIO pin
                cfg.invert = false;           // True if backlight is inverted (low = on)
                cfg.freq = 44100;             // PWM frequency
                cfg.pwm_channel = 7;          // PWM channel (avoid conflict with other uses)

                _light_instance.config(cfg);
                _panel_instance.setLight(&_light_instance);
            }

            setPanel(&_panel_instance);
        }
    };

} // namespace Display
