// main_calibrate.cpp
// IMU Calibration Tool
//
// Interactive development tool to determine IMU mounting configuration.
// Outputs board config values for copy/paste into board config header.
//
// CRITICAL: Display Rotation LUT Inversion
// -----------------------------------------
// Display rotation LUTs (LUT_ROTATE_0/90/180/270) rotate pixel content in the
// SAME direction as their name (e.g., LUT_ROTATE_90 rotates pixels 90° CW).
// However, when the device physically rotates, we need to rotate the content
// in the OPPOSITE direction to keep it upright from the user's perspective.
//
// Example: If device rotates 90° CW, content must rotate 90° CCW (= 270° CW)
// Therefore: Physical 90° rotation → Use LUT_ROTATE_270
//
// The LUT calculation formula inverts the rotation direction:
//   LUT[rotation] = (360 - rotation + displayOffset + deviceHome) % 360
//
// Usage:
//   1. Build and upload: pio run -e <board>-calibrate -t upload
//   2. Open serial monitor: pio device monitor
//   3. Follow on-screen instructions
//   4. Copy output values into board config file

#include <Arduino.h>
#include "Device_Config.h"

// Include appropriate calibration display
#if defined(DISPLAY_TYPE_LED_MATRIX)
    #include "Calibration/CalibrationDisplayLED.h"
    using CalibrationDisplayType = Calibration::CalibrationDisplayLED;
#elif defined(DISPLAY_TYPE_TFT)
    #include "Calibration/CalibrationDisplayTFT.h"
    using CalibrationDisplayType = Calibration::CalibrationDisplayTFT;
#else
    #error "Unknown display type for calibration"
#endif

// Include appropriate IMU
#include "Hardware/Sensors/IMUFactory.h"
using Hardware::IIMU;

// Calibration state machine
enum CalibrationState {
    STATE_WELCOME,
    STATE_FLAT_UP,
    STATE_WAIT_INPUT_FLAT_UP,
    STATE_POSITION_0,
    STATE_WAIT_INPUT_0,
    STATE_POSITION_90,
    STATE_WAIT_INPUT_90,
    STATE_POSITION_180,
    STATE_WAIT_INPUT_180,
    STATE_POSITION_270,
    STATE_WAIT_INPUT_270,
    STATE_DISPLAY_ALIGN,
    STATE_WAIT_INPUT_DISPLAY_ALIGN,
    STATE_CORNER_SELECT,
    STATE_CALCULATE,
    STATE_COMPLETE
};

// Measurement data for each rotation
struct Measurement {
    float accX;
    float accY;
    float accZ;
};

// Global state
CalibrationDisplayType *display = nullptr;
std::unique_ptr<IIMU> imu;
CalibrationState currentState = STATE_WELCOME;
Measurement measurements[ 5 ]; // FLAT_UP, 0°, 90°, 180°, 270°
int currentRotation = 0;
int displayCorner = 0; // 0=TL, 1=TR, 2=BR, 3=BL

// Color constants
const uint32_t COLOR_GREEN = 0x00FF00;
const uint32_t COLOR_ORANGE = 0xFF8800;
const uint32_t COLOR_RED = 0xFF0000;
const uint32_t COLOR_BLUE = 0x0000FF;

// Remapping configuration structure
struct RemapConfig {
    const char *remapX;
    const char *remapY;
    const char *remapZ;
    float (*getX)(const Measurement&);
    float (*getY)(const Measurement&);
    float (*getZ)(const Measurement&);
};

// Function declarations
void printWelcome();
void printInstructions( int step );
void waitForEnter();
void takeMeasurement( int index );
void getCornerSelection();
void calculateConfiguration();
void printConfiguration( const char* remapX, const char* remapY, const char* remapZ, 
                        int deviceHome, int displayOffset, int orientationEnums[4] );

void setup() {
    Serial.begin( 115200 );
    delay( 1000 ); // Wait for serial connection

    Serial.println( "\n\n" );
    Serial.println( "╔════════════════════════════════════════════╗" );
    Serial.println( "║     STAC IMU Calibration Tool v1.0.0       ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println();

    // Initialize display
    display = new CalibrationDisplayType();
    if ( !display->begin() ) {
        Serial.println( "ERROR: Failed to initialize display" );
        display->showMessage( "DISPLAY FAIL", COLOR_RED );
        while ( true ) {
            delay( 1000 );
        }
    }
    Serial.println( "✓ Display initialized" );
#if defined(DISPLAY_TYPE_LED_MATRIX)
    display->clear(); // LED can't show text, just clear
#else
    display->showMessage( "CALIBRATE", COLOR_GREEN );
    delay( 1000 );
#endif

    // Initialize IMU
    imu = Hardware::IMUFactory::create();
    if ( !imu || !imu->begin() ) {
        Serial.println( "ERROR: Failed to initialize IMU" );
        display->showMessage( "IMU FAIL", COLOR_RED );
        while ( true ) {
            delay( 1000 );
        }
    }
    Serial.println( "✓ IMU initialized" );
    Serial.printf( "  Type: %s\n", imu->getType() );
    delay( 100 ); // Allow IMU to stabilize

    currentState = STATE_WELCOME;
}

void loop() {
    switch ( currentState ) {
        case STATE_WELCOME:
            printWelcome();
            waitForEnter();
            currentState = STATE_FLAT_UP;
            break;

        case STATE_FLAT_UP:
            printInstructions( 0 );  // Step 0 = FLAT_UP
            currentState = STATE_WAIT_INPUT_FLAT_UP;
            break;

        case STATE_WAIT_INPUT_FLAT_UP:
            takeMeasurement( 0 );
            currentRotation = 0;
            currentState = STATE_POSITION_0;
            break;

        case STATE_POSITION_0:
            printInstructions( 1 );  // Step 1 = 0°
            currentState = STATE_WAIT_INPUT_0;
            break;

        case STATE_WAIT_INPUT_0:
            takeMeasurement( 1 );
            currentRotation = 90;
            currentState = STATE_POSITION_90;
            break;

        case STATE_POSITION_90:
            printInstructions( 2 );  // Step 2 = 90°
            currentState = STATE_WAIT_INPUT_90;
            break;

        case STATE_WAIT_INPUT_90:
            takeMeasurement( 2 );
            currentRotation = 180;
            currentState = STATE_POSITION_180;
            break;

        case STATE_POSITION_180:
            printInstructions( 3 );  // Step 3 = 180°
            currentState = STATE_WAIT_INPUT_180;
            break;

        case STATE_WAIT_INPUT_180:
            takeMeasurement( 3 );
            currentRotation = 270;
            currentState = STATE_POSITION_270;
            break;

        case STATE_POSITION_270:
            printInstructions( 4 );  // Step 4 = 270°
            currentState = STATE_WAIT_INPUT_270;
            break;

        case STATE_WAIT_INPUT_270:
            takeMeasurement( 4 );
            currentState = STATE_DISPLAY_ALIGN;
            break;

        case STATE_DISPLAY_ALIGN:
            display->showPixelAtIndex( 0 );  // Light pixel 0
            printInstructions( 5 );  // Step 5 = DISPLAY_ALIGN
            currentState = STATE_WAIT_INPUT_DISPLAY_ALIGN;
            break;

        case STATE_WAIT_INPUT_DISPLAY_ALIGN:
            // User confirms device is at home position with pixel 0 lit
            waitForEnter();
            currentState = STATE_CORNER_SELECT;
            break;

        case STATE_CORNER_SELECT:
            getCornerSelection();
            currentState = STATE_CALCULATE;
            break;

        case STATE_CALCULATE:
            Serial.println( "\n════════════════════════════════════════════" );
            Serial.println( "Calculating configuration..." );
            display->showMessage( "CALC...", COLOR_BLUE );
            calculateConfiguration();
            currentState = STATE_COMPLETE;
            break;

        case STATE_COMPLETE:
            display->clear();  // Clear display when done
            Serial.println( "\n════════════════════════════════════════════" );
            Serial.println( "Calibration complete!" );
            Serial.println( "Copy the values above into your board config." );
            Serial.println( "\nReset device to run calibration again." );
            while ( true ) {
                delay( 1000 );    // Halt
            }
            break;
    }
}

void printWelcome() {
    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║       IMU CALIBRATION TOOL                 ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println( "\nThis tool will measure accelerometer readings as you" );
    Serial.println( "position the device in 6 orientations." );
    Serial.println( "\nSTEPS:" );
    Serial.println( "  1. Flat with display facing up (determines Z-axis)" );
    Serial.println( "  2-5. Four vertical rotations: 0°, 90°, 180°, 270°" );
    Serial.println( "  6. Display alignment (pixel 0 position)" );
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "\nPress ENTER to begin..." );
}

void printInstructions( int step ) {
    Serial.println( "\n════════════════════════════════════════════" );
    
    switch ( step ) {
        case 0:  // FLAT UP
            Serial.println( "STEP 1/6: Device FLAT, display facing UP" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n1. Lay device FLAT on table, display facing UP" );
            Serial.println( "   (This determines Z-axis direction)" );
            break;
            
        case 1:  // 0°
            Serial.println( "STEP 2/6: Device rotated 0° (HOME position)" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n2. Hold device VERTICAL with USB port DOWN" );
            Serial.println( "   (This is your HOME position - 0°)" );
            break;
            
        case 2:  // 90°
            Serial.println( "STEP 3/6: Device rotated 90° clockwise" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n3. Rotate device 90° CLOCKWISE (keep vertical!)" );
            Serial.println( "   (USB port now pointing LEFT)" );
            break;
            
        case 3:  // 180°
            Serial.println( "STEP 4/6: Device rotated 180° clockwise" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n4. Rotate device 180° CLOCKWISE (keep vertical!)" );
            Serial.println( "   (USB port now pointing UP)" );
            break;
            
        case 4:  // 270°
            Serial.println( "STEP 5/6: Device rotated 270° clockwise" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n5. Rotate device 270° CLOCKWISE (keep vertical!)" );
            Serial.println( "   (USB port now pointing RIGHT)" );
            break;
            
        case 5:  // DISPLAY ALIGN
            Serial.println( "STEP 6/6: Display Alignment" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n6. You should see ONE PIXEL lit" );
            Serial.println( "   Rotate device to HOME position (USB port DOWN)" );
            Serial.println( "   Then identify which corner the lit pixel is in" );
            Serial.println( "   We will ask you after you press ENTER" );
            break;
    }

    Serial.println( "\nPress ENTER when ready..." );
    waitForEnter();
}

void waitForEnter() {
    while ( !Serial.available() ) {
        delay( 10 );
    }
    // Clear the input buffer
    while ( Serial.available() ) {
        Serial.read();
    }
}

void takeMeasurement( int index ) {
    Serial.println( "\nReading IMU data..." );

    // Read raw accelerometer data from IMU
    if ( !imu->getRawAcceleration( measurements[ index ].accX,
                                   measurements[ index ].accY,
                                   measurements[ index ].accZ ) ) {
        Serial.println( "ERROR: Failed to read IMU data" );
        measurements[ index ].accX = 0.0f;
        measurements[ index ].accY = 0.0f;
        measurements[ index ].accZ = 0.0f;
    }

    Serial.println( "\n┌────────────────────────────────────────┐" );
    Serial.printf(  "│ Raw Sensor Values (no remapping):     │\n" );
    Serial.println( "├────────────────────────────────────────┤" );
    Serial.printf(  "│  Sensor X: %7.3f g                   │\n", measurements[ index ].accX );
    Serial.printf(  "│  Sensor Y: %7.3f g                   │\n", measurements[ index ].accY );
    Serial.printf(  "│  Sensor Z: %7.3f g                   │\n", measurements[ index ].accZ );
    Serial.println( "└────────────────────────────────────────┘" );
}

void getCornerSelection() {
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "Which corner is the lit pixel in?" );
    Serial.println( "════════════════════════════════════════════\n" );
    Serial.println( "Enter the number corresponding to the corner:" );
    Serial.println( "  1 = Top-Left (TL)" );
    Serial.println( "  2 = Top-Right (TR)" );
    Serial.println( "  3 = Bottom-Right (BR)" );
    Serial.println( "  4 = Bottom-Left (BL)" );
    Serial.println( "\nEnter 1, 2, 3, or 4: " );
    
    while ( true ) {
        while ( !Serial.available() ) {
            delay( 10 );
        }
        
        char input = Serial.read();
        // Clear rest of buffer
        while ( Serial.available() ) {
            Serial.read();
        }
        
        if ( input >= '1' && input <= '4' ) {
            displayCorner = input - '1';  // Convert to 0-3
            const char* cornerNames[] = { "Top-Left", "Top-Right", "Bottom-Right", "Bottom-Left" };
            Serial.printf( "\n✓ Selected: %s\n", cornerNames[displayCorner] );
            break;
        } else {
            Serial.println( "Invalid input. Please enter 1, 2, 3, or 4:" );
        }
    }
}

void calculateConfiguration() {
    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║         MEASUREMENT SUMMARY                ║" );
    Serial.println( "╚════════════════════════════════════════════╝\n" );

    Serial.println( "FLAT UP:" );
    Serial.printf( "  Sensor X: %7.3fg, Y: %7.3fg, Z: %7.3fg\n",
                   measurements[0].accX, measurements[0].accY, measurements[0].accZ );
    
    Serial.println( "\nVERTICAL ROTATIONS:" );
    Serial.println( "┌────────────┬──────────┬──────────┬──────────┐" );
    Serial.println( "│ Angle      │ Sensor X │ Sensor Y │ Sensor Z │" );
    Serial.println( "├────────────┼──────────┼──────────┼──────────┤" );
    for ( int i = 1; i < 5; i++ ) {
        Serial.printf( "│  %3d°      │ %7.3fg │ %7.3fg │ %7.3fg │\n",
                       (i-1) * 90,
                       measurements[i].accX,
                       measurements[i].accY,
                       measurements[i].accZ );
    }
    Serial.println( "└────────────┴──────────┴──────────┴──────────┘\n" );

    // Step 1: Determine Z-axis direction and expected pattern
    Serial.println( "════════════════════════════════════════════" );
    Serial.println( "STEP 1: Determine Z-axis Direction" );
    Serial.println( "════════════════════════════════════════════\n" );
    
    float zFlat = measurements[0].accZ;
    Serial.printf( "Z when flat (display up): %.3f g\n", zFlat );
    
    bool zPointsAway = (zFlat < -0.5f);  // Z+ points down into table → away when vertical
    
    if ( zPointsAway ) {
        Serial.println( "→ Z+ points AWAY from user when vertical (Standard Pattern)" );
        Serial.println( "  Expected: X:(-1,0,+1,0), Y:(0,-1,0,+1)" );
    } else {
        Serial.println( "→ Z+ points TOWARD user when vertical (Reversed Pattern)" );
        Serial.println( "  Expected: X:(+1,0,-1,0), Y:(0,+1,0,-1)" );
    }

    // Define expected patterns
    struct Pattern {
        int x[4];  // Expected X values (normalized to -1, 0, +1)
        int y[4];  // Expected Y values
    };
    
    Pattern expectedPattern = zPointsAway ? 
        Pattern{{-1,0,1,0}, {0,-1,0,1}} :  // Standard (Z+ away)
        Pattern{{1,0,-1,0}, {0,1,0,-1}};   // Reversed (Z+ toward)

    // Step 2: Test all 8 axis remappings
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "STEP 2: Find Axis Remapping" );
    Serial.println( "════════════════════════════════════════════\n" );
    Serial.println( "Testing 8 possible axis remappings..." );

    struct RemapConfig {
        const char *exprX;
        const char *exprY;
        float (*getX)(const Measurement&);
        float (*getY)(const Measurement&);
    };

    RemapConfig remaps[] = {
        {"(acc.x)",    "(acc.y)",    [](const Measurement& m){ return m.accX; },  [](const Measurement& m){ return m.accY; }},
        {"(acc.x)",    "((-acc.y))", [](const Measurement& m){ return m.accX; },  [](const Measurement& m){ return -m.accY; }},
        {"((-acc.x))", "(acc.y)",    [](const Measurement& m){ return -m.accX; }, [](const Measurement& m){ return m.accY; }},
        {"((-acc.x))", "((-acc.y))", [](const Measurement& m){ return -m.accX; }, [](const Measurement& m){ return -m.accY; }},
        {"(acc.y)",    "(acc.x)",    [](const Measurement& m){ return m.accY; },  [](const Measurement& m){ return m.accX; }},
        {"(acc.y)",    "((-acc.x))", [](const Measurement& m){ return m.accY; },  [](const Measurement& m){ return -m.accX; }},
        {"((-acc.y))", "(acc.x)",    [](const Measurement& m){ return -m.accY; }, [](const Measurement& m){ return m.accX; }},
        {"((-acc.y))", "((-acc.x))", [](const Measurement& m){ return -m.accY; }, [](const Measurement& m){ return -m.accX; }}
    };

    int bestRemap = -1;
    int deviceHome = -1;  // Which rotation index (0-3) is the device home
    
    for ( int r = 0; r < 8; r++ ) {
        int matches = 0;
        int homeIndex = -1;
        
        for ( int rot = 0; rot < 4; rot++ ) {
            float boardX = remaps[r].getX(measurements[rot+1]);  // +1 because measurements[0] is FLAT
            float boardY = remaps[r].getY(measurements[rot+1]);
            
            // Normalize to -1, 0, +1
            int normX = (abs(boardX) > 0.7f) ? (boardX > 0 ? 1 : -1) : 0;
            int normY = (abs(boardY) > 0.7f) ? (boardY > 0 ? 1 : -1) : 0;
            
            // Check if matches expected pattern
            if ( normX == expectedPattern.x[rot] && normY == expectedPattern.y[rot] ) {
                matches++;
            }
            
            // Find which measurement index matches pattern position 0
            // This tells us the offset between user's "home" and pattern start
            if ( normX == expectedPattern.x[0] && normY == expectedPattern.y[0] ) {
                homeIndex = rot;  // This rotation index is where pattern starts
            }
        }
        
        if ( matches == 4 ) {
            bestRemap = r;
            deviceHome = homeIndex;
            Serial.printf( "✓ Remap %d: X=%s, Y=%s → 4/4 matches!\n",
                           r, remaps[r].exprX, remaps[r].exprY );
            break;
        }
    }

    if ( bestRemap == -1 ) {
        Serial.println( "\n⚠ ERROR: No axis remapping matched expected pattern!" );
        Serial.println( "  Check measurements and try again." );
        return;
    }

    // Step 3: Calculate LUT mapping
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "STEP 3: Calculate Display Rotation LUT" );
    Serial.println( "════════════════════════════════════════════\n" );
    
    const char* cornerNames[] = {"Top-Left", "Top-Right", "Bottom-Right", "Bottom-Left"};
    Serial.printf( "Device home: Physical rotation %d°\n", deviceHome * 90 );
    Serial.printf( "Pixel 0 corner: %s\n", cornerNames[displayCorner] );
    
    // Display offset: TL=0°, TR=270°, BR=180°, BL=90°
    int displayOffsets[] = {0, 270, 180, 90};
    int displayOffset = displayOffsets[displayCorner];
    
    Serial.printf( "Display offset: %d°\n", displayOffset );
    Serial.println( "\nNOTE: LUT rotations are inverted from physical rotation" );
    Serial.println( "      (device rotates CW → content rotates CCW to stay upright)" );
    
    // Determine what getOrientation() will return for each measurement
    // This simulates the runtime behavior of the IMU getOrientation() function
    int orientationEnums[4];  // What enum value getOrientation returns for each measurement
    for ( int i = 0; i < 4; i++ ) {
        float boardX = remaps[bestRemap].getX(measurements[i+1]);
        float boardY = remaps[bestRemap].getY(measurements[i+1]);
        
        // Simulate getOrientation() logic
        if ( abs(boardX) < 0.5f && abs(boardY) > 0.7f ) {
            orientationEnums[i] = (boardY > 0) ? 2 : 0;  // ROTATE_180 or ROTATE_0
        } else if ( abs(boardX) > 0.7f && abs(boardY) < 0.5f ) {
            orientationEnums[i] = (boardX > 0) ? 1 : 3;  // ROTATE_90 or ROTATE_270
        } else {
            orientationEnums[i] = -1;  // Should not happen if pattern matched
        }
    }

    printConfiguration( remaps[bestRemap].exprX, remaps[bestRemap].exprY, "(acc.z)",
                       deviceHome, displayOffset, orientationEnums );
}

void printConfiguration( const char* remapX, const char* remapY, const char* remapZ,
                        int deviceHome, int displayOffset, int orientationEnums[4] ) {
    
    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║         BOARD CONFIGURATION VALUES         ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println( "\nCopy these lines into your board config header:" );
    Serial.println( "\n// IMU Configuration (from calibration tool)" );
    Serial.printf( "#define IMU_AXIS_REMAP_X    %s\n", remapX );
    Serial.printf( "#define IMU_AXIS_REMAP_Y    %s\n", remapY );
    Serial.printf( "#define IMU_AXIS_REMAP_Z    %s\n", remapZ );
    
    Serial.println( "" );
    Serial.printf( "\n// Device home at physical %d°, display offset = %d°\n", deviceHome * 90, displayOffset );
    Serial.println( "#define DEVICE_ORIENTATION_TO_LUT_MAP { \\" );
    
    const char* lutNames[] = {
        "Orientation::ROTATE_0",
        "Orientation::ROTATE_90",
        "Orientation::ROTATE_180",
        "Orientation::ROTATE_270"
    };
    
    // Build LUT based on what getOrientation() actually returns
    // LUT is indexed by the Orientation enum value, not physical rotation
    int lut[6];  // 0=ROTATE_0, 1=ROTATE_90, 2=ROTATE_180, 3=ROTATE_270, 4=FLAT, 5=UNKNOWN
    for ( int i = 0; i < 6; i++ ) lut[i] = -1;  // Initialize to invalid
    
    // For each physical position, calculate correct LUT value
    for ( int physPos = 0; physPos < 4; physPos++ ) {
        int enumValue = orientationEnums[physPos];  // What getOrientation() returns
        int physicalAngle = physPos * 90;
        
        // Calculate LUT value: invert rotation and apply offsets
        int lutAngle = (360 - physicalAngle + displayOffset + (deviceHome * 90)) % 360;
        int lutIndex = lutAngle / 90;
        
        lut[enumValue] = lutIndex;  // Map enum value to LUT
    }
    
    // FLAT and UNKNOWN use same LUT as device home
    int homeEnumValue = orientationEnums[deviceHome];
    lut[4] = lut[homeEnumValue];  // FLAT
    lut[5] = lut[homeEnumValue];  // UNKNOWN
    
    // Print the LUT array
    for ( int i = 0; i < 4; i++ ) {
        Serial.printf( "    %s,  /* getOrientation()=%s → LUT_%d */ \\\n",
                       lutNames[lut[i]], lutNames[i], lut[i] * 90 );
    }
    Serial.printf( "    %s,  /* FLAT → same as device home */ \\\n", lutNames[lut[4]] );
    Serial.printf( "    %s   /* UNKNOWN → same as device home */ \\\n", lutNames[lut[5]] );
    
    Serial.println( "}" );
    
    // Build reverse mapping: enum value → physical angle (for debugging logs)
    Serial.println( "\n// Reverse mapping for debug logging: enum → physical angle" );
    Serial.println( "#define ORIENTATION_ENUM_TO_PHYSICAL_ANGLE { \\" );
    
    int enumToPhysical[6];  // Maps enum value to physical angle
    for ( int i = 0; i < 6; i++ ) enumToPhysical[i] = -1;  // Initialize
    
    // Build reverse mapping from the forward mapping we created
    for ( int physPos = 0; physPos < 4; physPos++ ) {
        int enumValue = orientationEnums[physPos];
        enumToPhysical[enumValue] = physPos * 90;  // Map enum to physical angle
    }
    enumToPhysical[4] = -1;  // FLAT has no angle
    enumToPhysical[5] = -1;  // UNKNOWN has no angle
    
    for ( int i = 0; i < 4; i++ ) {
        Serial.printf( "    %d,  /* %s → Physical %d° */ \\\n",
                       enumToPhysical[i], lutNames[i], enumToPhysical[i] );
    }
    Serial.printf( "    -1,  /* FLAT */ \\\n" );
    Serial.printf( "    -1   /* UNKNOWN */ \\\n" );
    Serial.println( "}" );
    
    Serial.println( "\n════════════════════════════════════════════\n" );
}


//  --- EOF --- //

