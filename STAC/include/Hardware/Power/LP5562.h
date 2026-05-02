/**
 * @file LP5562.h
 * @brief LP5562 RGBW LED driver for M5Stack ATOM S3R
 *
 * The LP5562 is a 4-channel (R/G/B/W) I2C LED driver used on the ATOM S3R.
 * Channel assignments on the ATOM S3R:
 *   - Channel 0 (R): Red status LED
 *   - Channel 1 (G): Green status LED
 *   - Channel 2 (B): Blue status LED
 *   - Channel 3 (W): Display backlight
 *
 * All four channels operate in direct I2C PWM mode (no program engine needed).
 * Engines are left in HOLD state so the PWM registers drive LEDs directly.
 *
 * I2C address: 0x60 (fixed, cannot be changed)
 * Recommended PWM frequency: 500 Hz (set via internal 32 kHz oscillator)
 *
 * References:
 *   - LP5562 datasheet (Texas Instruments SNVS765)
 *   - https://github.com/tedlanghorst/LP5562
 */

#pragma once

#include <cstdint>
#include <Wire.h>

namespace Hardware {

    /**
     * @brief LP5562 register addresses
     */
    namespace LP5562Reg {
        constexpr uint8_t ENABLE   = 0x00;  // Chip enable + engine modes
        constexpr uint8_t OP_MODE  = 0x01;  // Engine operation mode
        constexpr uint8_t B_PWM    = 0x02;  // Blue channel PWM (0-255)
        constexpr uint8_t G_PWM    = 0x03;  // Green channel PWM (0-255)
        constexpr uint8_t R_PWM    = 0x04;  // Red channel PWM (0-255)
        constexpr uint8_t CONFIG   = 0x08;  // Clock and misc configuration
        constexpr uint8_t RESET    = 0x0D;  // Software reset (write 0xFF)
        constexpr uint8_t W_PWM    = 0x0E;  // White channel PWM (0-255)
        constexpr uint8_t LED_MAP  = 0x70;  // Per-channel program engine assignment
    }

    // ENABLE register bits
    //   bit 6    : CHIP_EN  — 1 = chip on
    //   bits 5:4 : ENG1 mode  — 00 = HOLD (direct PWM), 10 = RUN (program)
    //   bits 3:2 : ENG2 mode
    //   bits 1:0 : ENG3 mode
    constexpr uint8_t LP5562_CHIP_EN   = 0x40;  // CHIP_EN=1, all engines HOLD

    // CONFIG register: use internal 32 kHz oscillator, no PWM power-save
    constexpr uint8_t LP5562_INTERNAL_CLK = 0x01;

    /**
     * @brief LP5562 RGBW LED driver
     *
     * Provides direct PWM control of all four channels via I2C.
     * All channels use HOLD mode (no engine programs required).
     */
    class LP5562 {
      public:
        /**
         * @brief Construct LP5562 driver
         * @param addr  7-bit I2C address (default 0x60)
         * @param wire  TwoWire instance to use (default Wire)
         * @param sda   I2C SDA pin (-1 = do not call Wire.begin)
         * @param scl   I2C SCL pin (-1 = do not call Wire.begin)
         * @param clock I2C clock frequency (Hz)
         *
         * When sda/scl are supplied (≥ 0), LP5562::begin() will call
         * Wire.begin(sda, scl, clock) before probing.  Pass -1/-1 (default)
         * when the I2C bus is already initialised by another driver (e.g.
         * BMI270_IMU) on the same bus.
         */
        LP5562( uint8_t addr = 0x60, TwoWire& wire = Wire,
                int8_t sda = -1, int8_t scl = -1, uint32_t clock = 100000 );

        /**
         * @brief Initialise the LP5562
         *
         * Resets the chip, enables it, and sets all channels to 0 (off).
         * Does NOT call Wire.begin() — the caller is responsible for
         * initialising the I2C bus with the correct SDA/SCL pins first.
         *
         * @return true on success, false if the chip does not ACK
         */
        bool begin();

        /**
         * @brief Set White channel PWM (display backlight)
         * @param pwm  Brightness 0 (off) – 255 (full)
         */
        void setW( uint8_t pwm );

        /**
         * @brief Set Red channel PWM
         * @param pwm  Brightness 0–255
         */
        void setR( uint8_t pwm );

        /**
         * @brief Set Green channel PWM
         * @param pwm  Brightness 0–255
         */
        void setG( uint8_t pwm );

        /**
         * @brief Set Blue channel PWM
         * @param pwm  Brightness 0–255
         */
        void setB( uint8_t pwm );

        /**
         * @brief Set all four channels at once
         * @param r  Red PWM 0–255
         * @param g  Green PWM 0–255
         * @param b  Blue PWM 0–255
         * @param w  White (backlight) PWM 0–255
         */
        void setRGBW( uint8_t r, uint8_t g, uint8_t b, uint8_t w );

        /**
         * @brief Turn all channels off
         */
        void off();

      private:
        TwoWire &_wire;
        uint8_t  _addr;
        int8_t   _sda;
        int8_t   _scl;
        uint32_t _clock;

        /**
         * @brief Write a single byte to a register
         * @param reg   Register address
         * @param value Value to write
         * @return true on success
         */
        bool writeReg( uint8_t reg, uint8_t value );

        /**
         * @brief Read a single byte from a register
         * @param reg   Register address
         * @param out   Reference to store the result
         * @return true on success
         */
        bool readReg( uint8_t reg, uint8_t &out );
    };

} // namespace Hardware

//  --- EOF --- //
