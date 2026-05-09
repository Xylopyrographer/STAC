/**
 * @file LP5562.cpp
 * @brief LP5562 RGBW LED driver implementation
 */

#include "Hardware/Power/LP5562.h"
#include <Arduino.h>

namespace Hardware {

    LP5562::LP5562( uint8_t addr, TwoWire& wire, int8_t sda, int8_t scl, uint32_t clock )
        : _wire( wire )
        , _addr( addr )
        , _sda( sda )
        , _scl( scl )
        , _clock( clock ) {
    }

    bool LP5562::begin() {
        // If I2C pins were supplied, initialise the bus now
        if ( _sda >= 0 && _scl >= 0 ) {
            _wire.begin( _sda, _scl, _clock );
        }

        // Verify presence: ping the device address
        _wire.beginTransmission( _addr );
        if ( _wire.endTransmission() != 0 ) {
            log_e( "LP5562 not found at I2C address 0x%02X", _addr );
            return false;
        }

        // Software reset — register 0x0D, value 0xFF
        // (LP5562 uses 0x0D as the RESET register; writing 0xFF triggers reset)
        writeReg( LP5562Reg::RESET, 0xFF );
        delay( 1 );  // Allow chip to complete reset (~0.5 ms typical)

        // Enable chip; leave all engines in HOLD (direct PWM) mode
        writeReg( LP5562Reg::ENABLE, LP5562_CHIP_EN );

        // Use internal 32 kHz oscillator (~500 Hz PWM frequency)
        writeReg( LP5562Reg::CONFIG, LP5562_INTERNAL_CLK );

        // Ensure all channels start off
        writeReg( LP5562Reg::R_PWM, 0x00 );
        writeReg( LP5562Reg::G_PWM, 0x00 );
        writeReg( LP5562Reg::B_PWM, 0x00 );
        writeReg( LP5562Reg::W_PWM, 0x00 );

        log_i( "LP5562 initialised at address 0x%02X", _addr );
        return true;
    }

    void LP5562::setW( uint8_t pwm ) {
        writeReg( LP5562Reg::W_PWM, pwm );
    }

    void LP5562::setR( uint8_t pwm ) {
        writeReg( LP5562Reg::R_PWM, pwm );
    }

    void LP5562::setG( uint8_t pwm ) {
        writeReg( LP5562Reg::G_PWM, pwm );
    }

    void LP5562::setB( uint8_t pwm ) {
        writeReg( LP5562Reg::B_PWM, pwm );
    }

    void LP5562::setRGBW( uint8_t r, uint8_t g, uint8_t b, uint8_t w ) {
        writeReg( LP5562Reg::R_PWM, r );
        writeReg( LP5562Reg::G_PWM, g );
        writeReg( LP5562Reg::B_PWM, b );
        writeReg( LP5562Reg::W_PWM, w );
    }

    void LP5562::off() {
        setRGBW( 0, 0, 0, 0 );
    }

    // -------------------------------------------------------------------------
    // Private helpers
    // -------------------------------------------------------------------------

    bool LP5562::writeReg( uint8_t reg, uint8_t value ) {
        _wire.beginTransmission( _addr );
        _wire.write( reg );
        _wire.write( value );
        uint8_t err = _wire.endTransmission();
        if ( err != 0 ) {
            log_w( "LP5562 writeReg(0x%02X, 0x%02X) failed, err=%d", reg, value, err );
            return false;
        }
        return true;
    }

    bool LP5562::readReg( uint8_t reg, uint8_t &out ) {
        // Use full stop (endTransmission(true)) — not repeated-start.
        // The ESP32 I2C driver can produce timeouts with repeated-start
        // on certain hardware (notably ESP32-S3 with OPI PSRAM).
        _wire.beginTransmission( _addr );
        _wire.write( reg );
        if ( _wire.endTransmission( true ) != 0 ) {  // full stop
            return false;
        }
        if ( _wire.requestFrom( _addr, ( uint8_t )1 ) != 1 ) {
            return false;
        }
        out = _wire.read();
        return true;
    }

} // namespace Hardware

//  --- EOF --- //
