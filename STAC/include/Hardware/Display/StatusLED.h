#pragma once

#include <Arduino.h>
#include "Device_Config.h"

// Status LED control - turns LED on during hardware init, off when complete
// Used by boards with HAS_STATUS_LED defined

#if HAS_STATUS_LED && defined(PIN_STATUS_LED)
#if defined(STATUS_LED_TYPE_GPIO)
// Simple GPIO LED
#if defined(STATUS_LED_ACTIVE_LOW) && STATUS_LED_ACTIVE_LOW
    #define STATUS_LED_ON  LOW
    #define STATUS_LED_OFF HIGH
#else
    #define STATUS_LED_ON  HIGH
    #define STATUS_LED_OFF LOW
#endif

inline void statusLedOn() {
    pinMode( PIN_STATUS_LED, OUTPUT );
    digitalWrite( PIN_STATUS_LED, STATUS_LED_ON );
}

inline void statusLedOff() {
    digitalWrite( PIN_STATUS_LED, STATUS_LED_OFF );
}

#elif defined(STATUS_LED_TYPE_ADDRESSABLE)
// Addressable RGB LED (WS2812, etc.)
#include <LiteLED.h>

static LiteLED statusLed( STATUS_LED_STRIP_TYPE, STATUS_LED_IS_RGBW );
static bool statusLedInitialized = false;

inline void statusLedOn() {
    if ( !statusLedInitialized ) {
        statusLed.begin( PIN_STATUS_LED, 1 ); // 1 LED
        statusLed.brightness( 32 ); // Low brightness for status
        statusLedInitialized = true;
    }
    statusLed.setPixel( 0, 0x00FF00, true ); // Green
}

inline void statusLedOff() {
    if ( statusLedInitialized ) {
        statusLed.setPixel( 0, 0, true ); // Off
    }
}

#else
inline void statusLedOn() {}
inline void statusLedOff() {}
#endif

#else
// No status LED configured
inline void statusLedOn() {}
inline void statusLedOff() {}
#endif
