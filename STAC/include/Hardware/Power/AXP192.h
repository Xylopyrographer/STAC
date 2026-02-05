/**
 * @file AXP192.h
 * @brief AXP192 Power Management IC driver for M5StickC Plus
 *
 * The AXP192 is an I2C-controlled PMU that handles:
 * - Power rails (DCDC1/2/3, LDO2/3)
 * - TFT backlight control (via LDO2 voltage)
 * - LCD power (via LDO3)
 * - Battery charging and monitoring
 * - Power button handling
 *
 * M5StickC Plus power rail assignments:
 * - DCDC1: 3.35V - ESP32 VDD
 * - DCDC3: 0V (disabled on StickC)
 * - LDO2: 2.8V - TFT backlight
 * - LDO3: 3.0V - TFT display IC power
 *
 * References:
 * - AXP192 datasheet
 * - M5GFX library (Light_M5StickC)
 * - I2C_AXP192 library by tanakamasayuki
 */

#pragma once

#include <cstdint>
#include <Wire.h>

namespace Hardware {

    /**
     * @brief AXP192 Register Map (relevant registers)
     */
    namespace AXP192Reg {
        // Power control registers
        constexpr uint8_t POWER_OUTPUT_CTRL = 0x12;  // Power output enable control
        constexpr uint8_t DCDC1_VOLTAGE     = 0x26;  // DCDC1 output voltage setting
        constexpr uint8_t DCDC2_VOLTAGE     = 0x23;  // DCDC2 output voltage setting
        constexpr uint8_t DCDC3_VOLTAGE     = 0x27;  // DCDC3 output voltage setting
        constexpr uint8_t LDO2_LDO3_VOLTAGE = 0x28;  // LDO2 (7:4) and LDO3 (3:0) voltage
        constexpr uint8_t VBUS_IPSOUT       = 0x30;  // VBUS-IPSOUT path control
        constexpr uint8_t VOFF_SHUTDOWN     = 0x31;  // VOFF shutdown voltage
        constexpr uint8_t SHUTDOWN_CTRL     = 0x32;  // Shutdown/battery/CHGLED control
        constexpr uint8_t CHARGE_CTRL1      = 0x33;  // Charge control 1
        constexpr uint8_t BACKUP_BATT       = 0x35;  // Backup battery charge control
        constexpr uint8_t PEK_PARAMS        = 0x36;  // PEK (power key) parameters

        // IRQ registers
        constexpr uint8_t IRQ_ENABLE1       = 0x40;
        constexpr uint8_t IRQ_ENABLE2       = 0x41;
        constexpr uint8_t IRQ_ENABLE3       = 0x42;
        constexpr uint8_t IRQ_ENABLE4       = 0x43;
        constexpr uint8_t IRQ_ENABLE5       = 0x44;
        constexpr uint8_t IRQ_STATUS1       = 0x48;
        constexpr uint8_t IRQ_STATUS2       = 0x49;
        constexpr uint8_t IRQ_STATUS3       = 0x4A;
        constexpr uint8_t IRQ_STATUS4       = 0x4B;
        constexpr uint8_t IRQ_STATUS5       = 0x4C;

        // ADC registers
        constexpr uint8_t ADC_ENABLE1       = 0x82;  // ADC enable 1
        constexpr uint8_t ADC_ENABLE2       = 0x83;  // ADC enable 2
        constexpr uint8_t ADC_SAMPLE_RATE   = 0x84;  // ADC sample rate

        // GPIO registers
        constexpr uint8_t GPIO0_CTRL        = 0x90;  // GPIO0 (LDOio0) control
        constexpr uint8_t GPIO0_VOLTAGE     = 0x91;  // GPIO0 LDO output voltage
        constexpr uint8_t GPIO1_CTRL        = 0x92;  // GPIO1 control
        constexpr uint8_t GPIO2_CTRL        = 0x93;  // GPIO2 control
        constexpr uint8_t GPIO_STATE        = 0x94;  // GPIO 0-2 signal status
        constexpr uint8_t GPIO34_CTRL       = 0x95;  // GPIO 3/4 control
        constexpr uint8_t GPIO34_STATE      = 0x96;  // GPIO 3/4 state

        // PWM registers
        constexpr uint8_t PWM1_DUTY_RATIO_X = 0x98;  // PWM1 X value
        constexpr uint8_t PWM1_DUTY_RATIO_Y1 = 0x99; // PWM1 Y1 value
        constexpr uint8_t PWM1_DUTY_RATIO_Y2 = 0x9A; // PWM1 Y2 value

        // Battery/power measurement registers
        constexpr uint8_t ACIN_VOLTAGE_H    = 0x56;
        constexpr uint8_t ACIN_VOLTAGE_L    = 0x57;
        constexpr uint8_t ACIN_CURRENT_H    = 0x58;
        constexpr uint8_t ACIN_CURRENT_L    = 0x59;
        constexpr uint8_t VBUS_VOLTAGE_H    = 0x5A;
        constexpr uint8_t VBUS_VOLTAGE_L    = 0x5B;
        constexpr uint8_t VBUS_CURRENT_H    = 0x5C;
        constexpr uint8_t VBUS_CURRENT_L    = 0x5D;
        constexpr uint8_t INTERNAL_TEMP_H   = 0x5E;
        constexpr uint8_t INTERNAL_TEMP_L   = 0x5F;
        constexpr uint8_t BATTERY_POWER_H   = 0x70;
        constexpr uint8_t BATTERY_POWER_M   = 0x71;
        constexpr uint8_t BATTERY_POWER_L   = 0x72;
        constexpr uint8_t BATTERY_VOLTAGE_H = 0x78;
        constexpr uint8_t BATTERY_VOLTAGE_L = 0x79;
        constexpr uint8_t BATTERY_CHG_CUR_H = 0x7A;
        constexpr uint8_t BATTERY_CHG_CUR_L = 0x7B;
        constexpr uint8_t BATTERY_DIS_CUR_H = 0x7C;
        constexpr uint8_t BATTERY_DIS_CUR_L = 0x7D;
        constexpr uint8_t APS_VOLTAGE_H     = 0x7E;
        constexpr uint8_t APS_VOLTAGE_L     = 0x7F;

        // Power key status
        constexpr uint8_t PEK_KEY_STATUS    = 0x46;
    }

    /**
     * @brief Power output control bits (register 0x12)
     */
    namespace AXP192Power {
        constexpr uint8_t DCDC1_EN  = ( 1 << 0 ); // DCDC1 enable
        constexpr uint8_t DCDC3_EN  = ( 1 << 1 ); // DCDC3 enable
        constexpr uint8_t LDO2_EN   = ( 1 << 2 ); // LDO2 enable
        constexpr uint8_t LDO3_EN   = ( 1 << 3 ); // LDO3 enable
        constexpr uint8_t DCDC2_EN  = ( 1 << 4 ); // DCDC2 enable
        constexpr uint8_t EXTEN_EN  = ( 1 << 6 ); // EXTEN enable
    }

    /**
     * @brief AXP192 PMU driver for M5StickC Plus
     */
    class AXP192 {
      public:
        /**
         * @brief Default I2C address for AXP192
         */
        static constexpr uint8_t DEFAULT_I2C_ADDR = 0x34;

        /**
         * @brief Construct AXP192 driver
         * @param i2cAddr I2C address (default 0x34)
         * @param wire TwoWire instance (default Wire)
         */
        explicit AXP192( uint8_t i2cAddr = DEFAULT_I2C_ADDR, TwoWire& wire = Wire );

        /**
         * @brief Initialize the AXP192 for M5StickC Plus
         * @return true if initialization successful
         */
        bool begin();

        /**
         * @brief Check if AXP192 is present on I2C bus
         * @return true if AXP192 responds
         */
        bool isPresent();

        // ====================================================================
        // Power Rail Control
        // ====================================================================

        /**
         * @brief Set DCDC1 voltage (ESP32 power)
         * @param voltage_mV Voltage in mV (700-3500, 25mV steps)
         */
        void setDCDC1( uint16_t voltage_mV );

        /**
         * @brief Set DCDC3 voltage
         * @param voltage_mV Voltage in mV (700-3500, 25mV steps), 0 to disable
         */
        void setDCDC3( uint16_t voltage_mV );

        /**
         * @brief Set LDO2 voltage (TFT backlight)
         * @param voltage_mV Voltage in mV (1800-3300, 100mV steps), 0 to disable
         */
        void setLDO2( uint16_t voltage_mV );

        /**
         * @brief Set LDO3 voltage (TFT display power)
         * @param voltage_mV Voltage in mV (1800-3300, 100mV steps), 0 to disable
         */
        void setLDO3( uint16_t voltage_mV );

        /**
         * @brief Enable/disable EXTEN output
         * @param enable true to enable
         */
        void setEXTEN( bool enable );

        // ====================================================================
        // Backlight Control (for TFT displays)
        // ====================================================================

        /**
         * @brief Set backlight brightness (0-255)
         *
         * Maps brightness 0-255 to LDO2 voltage range.
         * Uses the same formula as M5GFX for compatibility.
         *
         * @param brightness Brightness level (0=off, 255=max)
         */
        void setBacklight( uint8_t brightness );

        /**
         * @brief Get current backlight brightness
         * @return Brightness level (0-255)
         */
        uint8_t getBacklight() const {
            return _currentBrightness;
        }

        // ====================================================================
        // Battery Monitoring
        // ====================================================================

        /**
         * @brief Get battery voltage
         * @return Battery voltage in mV
         */
        float getBatteryVoltage();

        /**
         * @brief Get battery discharge current
         * @return Discharge current in mA
         */
        float getBatteryDischargeCurrent();

        /**
         * @brief Get battery charge current
         * @return Charge current in mA
         */
        float getBatteryChargeCurrent();

        /**
         * @brief Get VBUS voltage (USB input)
         * @return VBUS voltage in mV
         */
        float getVbusVoltage();

        /**
         * @brief Get VBUS current
         * @return VBUS current in mA
         */
        float getVbusCurrent();

        /**
         * @brief Get internal PMU temperature
         * @return Temperature in °C
         */
        float getInternalTemperature();

        /**
         * @brief Get estimated battery level
         * @return Battery percentage (0-100)
         */
        int8_t getBatteryLevel();

        /**
         * @brief Check if battery is charging
         * @return true if charging
         */
        bool isCharging();

        // ====================================================================
        // Power Control
        // ====================================================================

        /**
         * @brief Power off the device
         */
        void powerOff();

        /**
         * @brief Get power button status
         * @return Button status (see datasheet)
         */
        uint8_t getPowerKeyStatus();

      private:
        TwoWire &_wire;
        uint8_t _i2cAddr;
        uint8_t _currentBrightness;

        /**
         * @brief Read single byte from register
         */
        uint8_t readRegister( uint8_t reg );

        /**
         * @brief Write single byte to register
         */
        void writeRegister( uint8_t reg, uint8_t value );

        /**
         * @brief Set bits in register
         */
        void setBits( uint8_t reg, uint8_t bits );

        /**
         * @brief Clear bits in register
         */
        void clearBits( uint8_t reg, uint8_t bits );

        /**
         * @brief Write register with mask (preserves other bits)
         * @param reg Register address
         * @param value Value to write
         * @param mask Bits to modify (1 = modify, 0 = preserve)
         */
        void writeRegisterMasked( uint8_t reg, uint8_t value, uint8_t mask );
    };

} // namespace Hardware
