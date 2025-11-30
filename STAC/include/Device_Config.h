// Device_Config.h
// STAC Hardware Configuration
// Board selection is done via platformio.ini build flags

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

// ============================================================================
// BOARD CONFIGURATION INCLUDE
// ============================================================================
// The board config file is specified via -DBOARD_CONFIG_FILE in platformio.ini
// For Arduino IDE users: uncomment ONE of the includes below instead
//
// Example platformio.ini entry:
//   build_flags = -DBOARD_CONFIG_FILE=\"BoardConfigs/AtomMatrix_Config.h\"

#ifdef BOARD_CONFIG_FILE
    // PlatformIO build - config file path passed via build flags
    #include BOARD_CONFIG_FILE
#else
    // Arduino IDE fallback - uncomment ONE of these:
    #include "BoardConfigs/AtomMatrix_Config.h"
    // #include "BoardConfigs/WaveshareS3_Config.h"
    // #include "BoardConfigs/M5StickCPlus_Config.h"
    // #include "BoardConfigs/LilygoTDisplay_Config.h"
    // #include "BoardConfigs/AIPI_Lite_Config.h"
    // #include "BoardConfigs/Custom5x5_Config.h"
    // #include "BoardConfigs/Custom8x8_Config.h"
#endif

// ============================================================================
// SOFTWARE VERSION
// ============================================================================

// FIXME(Claude): This is better in STACApp or ??
#define STAC_SOFTWARE_VERSION "3.0.0-RC.9"  // Software version string

#endif // DEVICE_CONFIG_H


//  --- EOF --- //
