/**
 * @file AXP192.cpp
 * @brief AXP192 Power Management IC driver implementation
 */

#include "Hardware/Power/AXP192.h"
#include <Arduino.h>

namespace Hardware {

    AXP192::AXP192(uint8_t i2cAddr, TwoWire& wire)
        : _wire(wire)
        , _i2cAddr(i2cAddr)
        , _currentBrightness(0)
    {
    }

    bool AXP192::begin() {
        // Initialize I2C bus first (M5StickC Plus uses GPIO 21/22)
        _wire.begin(21, 22, 100000);
        
        // Check if AXP192 is present
        if (!isPresent()) {
            log_e("AXP192 not found at address 0x%02X", _i2cAddr);
            return false;
        }
        
        log_i("AXP192 found, initializing for M5StickC Plus...");
        
        // ====================================================================
        // DEVELOPMENT MODE: Disable battery power output
        // This ensures a USB power cycle will reset the device
        // Comment out for production use!
        // ====================================================================
        #define AXP192_DEV_MODE_NO_BATTERY 1
        
        #if AXP192_DEV_MODE_NO_BATTERY
        log_w("DEV MODE: Battery power output DISABLED - USB power only!");
        
        // Disable battery discharge to IPSOUT
        // Register 0x30: VBUS-IPSOUT path control
        // Bit 7: 0 = VBUS limited, 1 = VBUS not limited  
        // Bit 1: Battery->IPSOUT path enable (0=disable)
        // Set to 0x80 to disable battery output but allow VBUS pass-through
        writeRegister(AXP192Reg::VBUS_IPSOUT, 0x80);
        
        // Also configure for no battery backup if USB is lost
        // Register 0x32 bit 3: Enable battery output to IPSOUT when VBUS removed
        // Clear this bit to prevent battery from powering device when USB removed
        uint8_t shutdownReg = readRegister(AXP192Reg::SHUTDOWN_CTRL);
        writeRegister(AXP192Reg::SHUTDOWN_CTRL, shutdownReg & ~0x08);
        #endif
        // ====================================================================
        
        // Initialize for M5StickC Plus (based on M5GFX initialization)
        // Register 0x12: Enable power outputs
        // Bit 0: DCDC1 (ESP32 power)
        // Bit 2: LDO2 (backlight)
        // Bit 3: LDO3 (LCD power)
        // Bit 6: EXTEN
        writeRegister(AXP192Reg::POWER_OUTPUT_CTRL, 0x4D);  // DCDC1, LDO2, LDO3, EXTEN
        
        // VBUS-IPSOUT pass-through management
        // Bit 7: VBUS can be used regardless of N_VBUSEN
        writeRegister(AXP192Reg::VBUS_IPSOUT, 0x80);
        
        // Disable DCDC3 (not used on StickC Plus)
        setDCDC3(0);
        
        // Set LDO3 to 3.0V for LCD power
        setLDO3(3000);
        
        // ADC settings
        writeRegister(AXP192Reg::ADC_SAMPLE_RATE, 0xF2);  // 200Hz sample rate
        writeRegister(AXP192Reg::ADC_ENABLE1, 0xFF);      // Enable all ADC channels
        
        // Charge settings
        writeRegister(AXP192Reg::CHARGE_CTRL1, 0xC0);     // Charge 4.2V, 100mA
        
        // Power key settings
        writeRegister(AXP192Reg::PEK_PARAMS, 0x0C);       // 128ms power on, 4s power off
        
        // Enable RTC backup battery charging
        writeRegister(AXP192Reg::BACKUP_BATT, 0xA2);
        
        // VOFF shutdown at 3.0V
        writeRegister(AXP192Reg::VOFF_SHUTDOWN, 0x04);
        
        // Enable battery monitoring
        writeRegister(AXP192Reg::SHUTDOWN_CTRL, 0x42);
        
        // Disable all IRQs initially
        writeRegister(AXP192Reg::IRQ_ENABLE1, 0x00);
        writeRegister(AXP192Reg::IRQ_ENABLE2, 0x00);
        writeRegister(AXP192Reg::IRQ_ENABLE3, 0x03);  // Power key IRQ
        writeRegister(AXP192Reg::IRQ_ENABLE4, 0x00);
        writeRegister(AXP192Reg::IRQ_ENABLE5, 0x00);
        
        // Temperature protection
        writeRegister(0x39, 0xFC);
        
        log_i("AXP192 initialization complete");
        return true;
    }

    bool AXP192::isPresent() {
        _wire.beginTransmission(_i2cAddr);
        return (_wire.endTransmission() == 0);
    }

    // ========================================================================
    // Power Rail Control
    // ========================================================================

    void AXP192::setDCDC1(uint16_t voltage_mV) {
        if (voltage_mV < 700 || voltage_mV > 3500) {
            clearBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::DCDC1_EN);
            return;
        }
        
        setBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::DCDC1_EN);
        uint8_t val = (voltage_mV - 700) / 25;
        writeRegister(AXP192Reg::DCDC1_VOLTAGE, val & 0x7F);
    }

    void AXP192::setDCDC3(uint16_t voltage_mV) {
        if (voltage_mV < 700 || voltage_mV > 3500) {
            clearBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::DCDC3_EN);
            return;
        }
        
        setBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::DCDC3_EN);
        uint8_t val = (voltage_mV - 700) / 25;
        writeRegister(AXP192Reg::DCDC3_VOLTAGE, val & 0x7F);
    }

    void AXP192::setLDO2(uint16_t voltage_mV) {
        if (voltage_mV < 1800 || voltage_mV > 3300) {
            clearBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::LDO2_EN);
            return;
        }
        
        setBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::LDO2_EN);
        
        // LDO2 is upper nibble (bits 7:4) of register 0x28
        uint8_t currentVal = readRegister(AXP192Reg::LDO2_LDO3_VOLTAGE);
        uint8_t ldo2Val = ((voltage_mV - 1800) / 100) & 0x0F;
        writeRegister(AXP192Reg::LDO2_LDO3_VOLTAGE, (currentVal & 0x0F) | (ldo2Val << 4));
    }

    void AXP192::setLDO3(uint16_t voltage_mV) {
        if (voltage_mV < 1800 || voltage_mV > 3300) {
            clearBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::LDO3_EN);
            return;
        }
        
        setBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::LDO3_EN);
        
        // LDO3 is lower nibble (bits 3:0) of register 0x28
        uint8_t currentVal = readRegister(AXP192Reg::LDO2_LDO3_VOLTAGE);
        uint8_t ldo3Val = ((voltage_mV - 1800) / 100) & 0x0F;
        writeRegister(AXP192Reg::LDO2_LDO3_VOLTAGE, (currentVal & 0xF0) | ldo3Val);
    }

    void AXP192::setEXTEN(bool enable) {
        if (enable) {
            setBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::EXTEN_EN);
        } else {
            clearBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::EXTEN_EN);
        }
    }

    // ========================================================================
    // Backlight Control
    // ========================================================================

    void AXP192::setBacklight(uint8_t brightness) {
        _currentBrightness = brightness;
        
        if (brightness == 0) {
            // Turn off LDO2
            clearBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::LDO2_EN);
        } else {
            // Enable LDO2
            setBits(AXP192Reg::POWER_OUTPUT_CTRL, AXP192Power::LDO2_EN);
            
            // Convert brightness (0-255) to LDO2 voltage setting
            // M5GFX formula: brightness = (((brightness >> 1) + 8) / 13) + 5
            // This maps 0-255 to approximately 5-14 (voltage settings for 2.3V-3.2V range)
            // We'll use a similar approach
            uint8_t ldo2Setting = (((brightness >> 1) + 8) / 13) + 5;
            if (ldo2Setting > 15) ldo2Setting = 15;  // Clamp to max
            
            // Write to upper nibble of register 0x28
            uint8_t currentVal = readRegister(AXP192Reg::LDO2_LDO3_VOLTAGE);
            writeRegister(AXP192Reg::LDO2_LDO3_VOLTAGE, (currentVal & 0x0F) | (ldo2Setting << 4));
        }
    }

    // ========================================================================
    // Battery Monitoring
    // ========================================================================

    float AXP192::getBatteryVoltage() {
        uint16_t raw = (readRegister(AXP192Reg::BATTERY_VOLTAGE_H) << 4) |
                       readRegister(AXP192Reg::BATTERY_VOLTAGE_L);
        return raw * 1.1f;  // mV
    }

    float AXP192::getBatteryDischargeCurrent() {
        uint16_t raw = (readRegister(AXP192Reg::BATTERY_DIS_CUR_H) << 5) |
                       readRegister(AXP192Reg::BATTERY_DIS_CUR_L);
        return raw * 0.5f;  // mA
    }

    float AXP192::getBatteryChargeCurrent() {
        uint16_t raw = (readRegister(AXP192Reg::BATTERY_CHG_CUR_H) << 5) |
                       readRegister(AXP192Reg::BATTERY_CHG_CUR_L);
        return raw * 0.5f;  // mA
    }

    float AXP192::getVbusVoltage() {
        uint16_t raw = (readRegister(AXP192Reg::VBUS_VOLTAGE_H) << 4) |
                       readRegister(AXP192Reg::VBUS_VOLTAGE_L);
        return raw * 1.7f;  // mV
    }

    float AXP192::getVbusCurrent() {
        uint16_t raw = (readRegister(AXP192Reg::VBUS_CURRENT_H) << 4) |
                       readRegister(AXP192Reg::VBUS_CURRENT_L);
        return raw * 0.375f;  // mA
    }

    float AXP192::getInternalTemperature() {
        uint16_t raw = (readRegister(AXP192Reg::INTERNAL_TEMP_H) << 4) |
                       readRegister(AXP192Reg::INTERNAL_TEMP_L);
        return -144.7f + raw * 0.1f;  // °C
    }

    int8_t AXP192::getBatteryLevel() {
        float voltage = getBatteryVoltage();
        
        // Simple linear approximation
        // 3300mV = 0%, 4150mV = 100%
        int level = (int)((voltage - 3300.0f) * 100.0f / (4150.0f - 3300.0f));
        
        if (level < 0) level = 0;
        if (level > 100) level = 100;
        
        return (int8_t)level;
    }

    bool AXP192::isCharging() {
        // Read charge current - if > 0, we're charging
        return getBatteryChargeCurrent() > getBatteryDischargeCurrent();
    }

    // ========================================================================
    // Power Control
    // ========================================================================

    void AXP192::powerOff() {
        setBits(AXP192Reg::SHUTDOWN_CTRL, 0x80);  // Set shutdown bit
    }

    uint8_t AXP192::getPowerKeyStatus() {
        uint8_t status = readRegister(AXP192Reg::PEK_KEY_STATUS);
        // Clear the status by writing back
        writeRegister(AXP192Reg::PEK_KEY_STATUS, 0x03);
        return status;
    }

    // ========================================================================
    // Private I2C Methods
    // ========================================================================

    uint8_t AXP192::readRegister(uint8_t reg) {
        _wire.beginTransmission(_i2cAddr);
        _wire.write(reg);
        _wire.endTransmission();
        
        _wire.requestFrom(_i2cAddr, (uint8_t)1);
        return _wire.read();
    }

    void AXP192::writeRegister(uint8_t reg, uint8_t value) {
        _wire.beginTransmission(_i2cAddr);
        _wire.write(reg);
        _wire.write(value);
        _wire.endTransmission();
    }

    void AXP192::setBits(uint8_t reg, uint8_t bits) {
        uint8_t val = readRegister(reg);
        writeRegister(reg, val | bits);
    }

    void AXP192::clearBits(uint8_t reg, uint8_t bits) {
        uint8_t val = readRegister(reg);
        writeRegister(reg, val & ~bits);
    }

    void AXP192::writeRegisterMasked(uint8_t reg, uint8_t value, uint8_t mask) {
        uint8_t current = readRegister(reg);
        uint8_t newVal = (current & ~mask) | (value & mask);
        writeRegister(reg, newVal);
    }

} // namespace Hardware
