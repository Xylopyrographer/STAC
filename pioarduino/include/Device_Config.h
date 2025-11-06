// Device_Config.h
// STAC Hardware Configuration
// Edit this file to match your hardware

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

// ============================================================================
// SELECT YOUR BOARD (uncomment ONE if not using PlatformIO build environments)
// ============================================================================

// If building with PlatformIO, the board is set via platformio.ini
// If building with Arduino IDE, uncomment ONE of these:

#ifndef BOARD_M5STACK_ATOM_MATRIX
#ifndef BOARD_WAVESHARE_ESP32_S3_MATRIX
#ifndef BOARD_CUSTOM_5X5
#ifndef BOARD_CUSTOM_8X8
    #define BOARD_M5STACK_ATOM_MATRIX
    // #define BOARD_WAVESHARE_ESP32_S3_MATRIX
    // #define BOARD_CUSTOM_5X5
    // #define BOARD_CUSTOM_8X8
#endif
#endif
#endif
#endif

// ============================================================================
// BOARD-SPECIFIC INCLUDES (automatically selected based on board choice above)
// ============================================================================

#if defined(BOARD_M5STACK_ATOM_MATRIX)
    #include "BoardConfigs/AtomMatrix_Config.h"

#elif defined(BOARD_WAVESHARE_ESP32_S3_MATRIX)
    #include "BoardConfigs/WaveshareS3_Config.h"

#elif defined(BOARD_CUSTOM_5X5)
    #include "BoardConfigs/Custom5x5_Config.h"

#elif defined(BOARD_CUSTOM_8X8)
    #include "BoardConfigs/Custom8x8_Config.h"

#else
    #error "No board selected in Device_Config.h! Please uncomment one BOARD_* define."
#endif

#endif // DEVICE_CONFIG_H


//  --- EOF --- //
