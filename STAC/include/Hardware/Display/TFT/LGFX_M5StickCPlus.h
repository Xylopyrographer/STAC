/**
 * @file LGFX_M5StickCPlus.h
 * @brief LovyanGFX configuration for M5StickC Plus display
 * 
 * This file configures the LovyanGFX library for the M5StickC Plus
 * which uses an ST7789V2 TFT display (135x240 pixels).
 */

#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

namespace Display {

    /**
     * @brief LovyanGFX display configuration for M5StickC Plus
     * 
     * ST7789V2 TFT Display specifications:
     * - Resolution: 135 x 240 pixels
     * - Interface: SPI
     * - Color depth: 16-bit (RGB565)
     */
    class LGFX_M5StickCPlus : public lgfx::LGFX_Device {
    public:
        // Panel and bus instances
        lgfx::Panel_ST7789 _panel_instance;
        lgfx::Bus_SPI _bus_instance;

        LGFX_M5StickCPlus() {
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
                
                // Pin configuration
                cfg.pin_sclk = 13;            // SPI SCLK pin
                cfg.pin_mosi = 15;            // SPI MOSI pin
                cfg.pin_miso = -1;            // SPI MISO pin (-1 = not used)
                cfg.pin_dc = 23;              // SPI D/C pin (data/command)

                _bus_instance.config(cfg);
                _panel_instance.setBus(&_bus_instance);
            }

            // Configure display panel
            {
                auto cfg = _panel_instance.config();

                cfg.pin_cs = 5;               // Chip select pin
                cfg.pin_rst = 18;             // Reset pin
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

            setPanel(&_panel_instance);
        }
    };

} // namespace Display
