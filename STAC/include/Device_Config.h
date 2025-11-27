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

    /**
     *  QUESTION(Claude): Should we have a better way to select boards?
     *  The user should put the board name/type in the  {board}_Config.h file and we adpot it from there?
     * 
     */ 


    #ifndef BOARD_M5STACK_ATOM_MATRIX
    #ifndef BOARD_WAVESHARE_ESP32_S3_MATRIX
    #ifndef BOARD_M5STICKC_PLUS
    #ifndef BOARD_CUSTOM_5X5
    #ifndef BOARD_CUSTOM_8X8
    #define BOARD_M5STACK_ATOM_MATRIX
    // #define BOARD_WAVESHARE_ESP32_S3_MATRIX
    // #define BOARD_M5STICKC_PLUS
    // #define BOARD_CUSTOM_5X5
    // #define BOARD_CUSTOM_8X8
    #endif
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

    #elif defined(BOARD_M5STICKC_PLUS)
        #include "BoardConfigs/M5StickCPlus_Config.h"

    #elif defined(BOARD_CUSTOM_5X5)
        #include "BoardConfigs/Custom5x5_Config.h"

    #elif defined(BOARD_CUSTOM_8X8)
        #include "BoardConfigs/Custom8x8_Config.h"

    #else
        #error "No board selected in Device_Config.h! Please uncomment one BOARD_* define."
    #endif

    // ============================================================================
    // SOFTWARE VERSION
    // ============================================================================

    // FIXME(Claude): This is better in STACApp or ??
    #define STAC_SOFTWARE_VERSION "3.0.0-RC.9"  // Software version string

#endif // DEVICE_CONFIG_H


//  --- EOF --- //
