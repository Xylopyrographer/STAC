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
    STATE_CORNER_IDENTIFY,    // NEW: Identify top-left corner
    STATE_POSITION_90,
    STATE_WAIT_INPUT_90,
    STATE_POSITION_180,
    STATE_WAIT_INPUT_180,
    STATE_POSITION_270,
    STATE_WAIT_INPUT_270,
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
int currentCornerTest = 0; // Which corner we're currently lighting (0-3)

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
void identifyTopLeftCorner();
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
            currentCornerTest = 0;  // Start corner identification
            currentState = STATE_CORNER_IDENTIFY;
            break;

        case STATE_CORNER_IDENTIFY:
            identifyTopLeftCorner();
            // identifyTopLeftCorner() will handle state transition when complete
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
    Serial.println( "║       IMU CALIBRATION TOOL v2.0            ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println( "\nThis tool will measure accelerometer readings as you" );
    Serial.println( "position the device in 6 orientations." );
    Serial.println( "\nSTEPS:" );
    Serial.println( "  1. Flat with display facing up (determines Z-axis)" );
    Serial.println( "  2. Vertical at your chosen home position" );
    Serial.println( "  3. Display corner identification (absolute reference)" );
    Serial.println( "  4-6. Three more rotations: 90°, 180°, 270°" );
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
            Serial.println( "STEP 2/6: Device at HOME position" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n2. Hold device VERTICAL in your chosen orientation" );
            Serial.println( "   (This is your HOME position)" );
            Serial.println( "   (Can be USB up, down, left, or right - your choice!)" );
            break;
            
        case 2:  // 90°
            Serial.println( "STEP 4/6: Device rotated 90° clockwise" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n4. Rotate device 90° CLOCKWISE (keep vertical!)" );
            Serial.println( "   (90° from your home position)" );
            break;
            
        case 3:  // 180°
            Serial.println( "STEP 5/6: Device rotated 180° clockwise" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n5. Rotate device 180° CLOCKWISE (keep vertical!)" );
            Serial.println( "   (180° from your home position)" );
            break;
            
        case 4:  // 270°
            Serial.println( "STEP 6/6: Device rotated 270° clockwise" );
            Serial.println( "════════════════════════════════════════════" );
            Serial.println( "\n6. Rotate device 270° CLOCKWISE (keep vertical!)" );
            Serial.println( "   (270° from your home position)" );
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

void identifyTopLeftCorner() {
    static bool firstCall = true;
    
    if ( firstCall ) {
        Serial.println( "\n════════════════════════════════════════════" );
        Serial.println( "STEP 3/6: Display Corner Identification" );
        Serial.println( "════════════════════════════════════════════\n" );
        Serial.println( "We will light each corner sequentially." );
        Serial.println( "Watch the display and identify which corner is TOP-LEFT.\n" );
        Serial.println( "Make sure device is at HOME position!\n" );
        Serial.println( "Press ENTER to start..." );
        waitForEnter();
        firstCall = false;
        currentCornerTest = 0;
        display->clear();
    }
    
    // Light the current corner being tested
    const char* cornerNames[] = {"Top-Left", "Top-Right", "Bottom-Right", "Bottom-Left"};
    
    // Show pixel at corner
    // Corner 0 (TL) = pixel 0, Corner 1 (TR) = pixel 4, Corner 2 (BR) = pixel 24, Corner 3 (BL) = pixel 20
    // For 5x5 matrix: TL=0, TR=4, BR=24, BL=20
    int pixelIndices[] = {0, 4, 24, 20};
    
    display->clear();
    display->showPixelAtIndex( pixelIndices[currentCornerTest] );
    
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.printf( "Lighting corner %d/4: %s\n", currentCornerTest + 1, cornerNames[currentCornerTest] );
    Serial.println( "════════════════════════════════════════════\n" );
    Serial.println( "Is this the TOP-LEFT corner?" );
    Serial.println( "  Y = Yes (this is top-left)" );
    Serial.println( "  N = No (try next corner)" );
    Serial.print( "\nEnter Y or N: " );
    
    while ( true ) {
        if ( Serial.available() ) {
            char input = Serial.read();
            // Clear rest of buffer
            while ( Serial.available() ) {
                Serial.read();
            }
            
            if ( input == 'Y' || input == 'y' ) {
                displayCorner = currentCornerTest;
                Serial.printf( "\n✓ Top-Left corner identified: %s\n", cornerNames[displayCorner] );
                display->clear();
                firstCall = true;  // Reset for next calibration run
                currentRotation = 90;
                currentState = STATE_POSITION_90;  // Move to next rotation
                return;
            } else if ( input == 'N' || input == 'n' ) {
                currentCornerTest = (currentCornerTest + 1) % 4;
                return;  // Will call again in next loop iteration
            } else {
                Serial.print( "\nInvalid input. Enter Y or N: " );
            }
        }
        delay( 10 );
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

    // Step 2: Use corner identification to determine absolute orientation
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "STEP 2: Determine Absolute Orientation" );
    Serial.println( "════════════════════════════════════════════\n" );
    
    const char* cornerNames[] = {"Top-Left", "Top-Right", "Bottom-Right", "Bottom-Left"};
    Serial.printf( "User identified top-left corner: %s\n", cornerNames[displayCorner] );
    
    // Corner tells us the physical rotation of the display relative to the IMU
    // displayCorner: 0=TL, 1=TR, 2=BR, 3=BL
    // If user sees TR as top-left, display is rotated 270° CCW (= 90° CW) from IMU frame
    int displayRotationFromIMU = displayCorner * 90;  // 0°, 90°, 180°, 270°
    Serial.printf( "Display rotated %d° CW from IMU sensor frame\n", displayRotationFromIMU );
    
    // Now we know ABSOLUTE orientation at measurement[1]:
    // The user chose this as "home" and identified which corner is top-left
    // This breaks the rotational symmetry!
    
    // Step 3: Derive axis remapping from absolute reference
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "STEP 3: Calculate Axis Remap from Absolute Reference" );
    Serial.println( "════════════════════════════════════════════\n" );
    
    // Rotate the expected pattern by the corner offset to get absolute pattern
    Pattern absolutePattern;
    for ( int i = 0; i < 4; i++ ) {
        int sourceIdx = (i + displayCorner) % 4;
        absolutePattern.x[sourceIdx] = expectedPattern.x[i];
        absolutePattern.y[sourceIdx] = expectedPattern.y[i];
    }
    
    Serial.printf( "Absolute pattern (accounting for display rotation):\n" );
    Serial.printf( "  X: (%2d, %2d, %2d, %2d)\n", 
                   absolutePattern.x[0], absolutePattern.x[1], 
                   absolutePattern.x[2], absolutePattern.x[3] );
    Serial.printf( "  Y: (%2d, %2d, %2d, %2d)\n",
                   absolutePattern.y[0], absolutePattern.y[1], 
                   absolutePattern.y[2], absolutePattern.y[3] );

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
    int deviceHome = -1;  // Which absolute pattern position (0-3) matches user's home (measurement[1])
    
    for ( int r = 0; r < 8; r++ ) {
        int matches = 0;
        int homePos = -1;
        
        for ( int rot = 0; rot < 4; rot++ ) {
            float boardX = remaps[r].getX(measurements[rot+1]);  // +1 because measurements[0] is FLAT
            float boardY = remaps[r].getY(measurements[rot+1]);
            
            // Normalize to -1, 0, +1
            int normX = (abs(boardX) > 0.7f) ? (boardX > 0 ? 1 : -1) : 0;
            int normY = (abs(boardY) > 0.7f) ? (boardY > 0 ? 1 : -1) : 0;
            
            // Check if matches absolute pattern (not relative pattern!)
            if ( normX == absolutePattern.x[rot] && normY == absolutePattern.y[rot] ) {
                matches++;
            }
            
            // Find which absolute pattern position matches user's home (measurement[1], rot=0)
            if ( rot == 0 ) {  // This is user's chosen home
                // Find which absolute pattern position this matches
                for ( int absPos = 0; absPos < 4; absPos++ ) {
                    if ( normX == absolutePattern.x[absPos] && normY == absolutePattern.y[absPos] ) {
                        homePos = absPos;
                        break;
                    }
                }
            }
        }
        
        if ( matches == 4 ) {
            bestRemap = r;
            deviceHome = homePos;
            Serial.printf( "✓ Remap %d: X=%s, Y=%s → 4/4 matches!\n",
                           r, remaps[r].exprX, remaps[r].exprY );
            Serial.printf( "  User's home (measurement[1]) matches absolute pattern position %d\n", deviceHome );
            break;
        }
    }

    if ( bestRemap == -1 ) {
        Serial.println( "\n⚠ ERROR: No axis remapping matched absolute pattern!" );
        Serial.println( "  Check measurements and try again." );
        return;
    }

    // Step 4: Calculate LUT mapping
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "STEP 4: Calculate Display Rotation LUT" );
    Serial.println( "════════════════════════════════════════════\n" );
    
    Serial.printf( "Device home: measurement[1] (user's chosen orientation)\n" );
    Serial.printf( "Top-left corner: %s (displayCorner=%d)\n", cornerNames[displayCorner], displayCorner );
    Serial.printf( "Display rotation from IMU: %d°\n", displayRotationFromIMU );
    
    // Calculate display content rotation needed to compensate for physical display orientation
    // If TR is in TL corner → display rotated 270° CW → content needs 90° CW to compensate
    // If BL is in TL corner → display rotated 90° CW → content needs 270° CW to compensate
    int displayOffsets[] = {0, 90, 180, 270};  // TL, TR, BR, BL
    int displayOffset = displayOffsets[displayCorner];
    
    Serial.printf( "Display offset: %d° (content rotation to compensate for physical mounting)\n", displayOffset );
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
    
    // For each measurement, calculate correct LUT value
    // measurements[1] = user's home (physical 0°)
    // measurements[2] = 90° CW from home (physical 90°)  
    // measurements[3] = 180° CW from home (physical 180°)
    // measurements[4] = 270° CW from home (physical 270°)
    for ( int measurementIdx = 0; measurementIdx < 4; measurementIdx++ ) {
        int enumValue = orientationEnums[measurementIdx];  // What getOrientation() returns at this measurement
        int physicalAngle = measurementIdx * 90;  // Physical angle from user's home (0°, 90°, 180°, 270°)
        
        // Calculate LUT value: invert rotation and apply display offset
        // At physical 0° (home), we want display at displayOffset
        // At physical 90° CW, we want display rotated 90° CCW from that = displayOffset - 90°
        int lutAngle = (displayOffset - physicalAngle + 360) % 360;
        int lutIndex = lutAngle / 90;
        
        lut[enumValue] = lutIndex;  // Map enum value to LUT
        
        Serial.printf( "  Measurement[%d]: physical %d°, getOrientation()=%d, display needs %d° → lut[%d]=%d\n",
                       measurementIdx, physicalAngle, enumValue, lutAngle, enumValue, lutIndex );
    }
    
    // FLAT and UNKNOWN: When device is physically flat (not vertical),
    // getOrientation() returns ROTATE_0, so always use lut[0]
    int homeEnumValue = orientationEnums[deviceHome];
    lut[4] = lut[0];  // FLAT → use lut[0] (what to display when getOrientation()=ROTATE_0)
    lut[5] = lut[0];  // UNKNOWN → use lut[0]
    Serial.printf( "DEBUG: deviceHome=%d, orientationEnums[%d]=%d, lut[%d]=%d, lut[0]=%d (FLAT)\n",
                   deviceHome, deviceHome, homeEnumValue, homeEnumValue, lut[homeEnumValue], lut[0] );
    
    // Print the LUT array
    for ( int i = 0; i < 4; i++ ) {
        Serial.printf( "    %s,  /* getOrientation()=%s → LUT_%d */ \\\n",
                       lutNames[lut[i]], lutNames[i], lut[i] * 90 );
    }
    Serial.printf( "    %s,  /* FLAT → uses lut[0] */ \\\n", lutNames[lut[4]] );
    Serial.printf( "    %s   /* UNKNOWN → uses lut[0] */ \\\n", lutNames[lut[5]] );
    
    Serial.println( "}" );
    
    // Build reverse mapping: enum value → physical angle (for debugging logs)
    Serial.println( "\n// Reverse mapping for debug logging: enum → physical angle" );
    Serial.println( "#define ORIENTATION_ENUM_TO_PHYSICAL_ANGLE { \\" );
    
    int enumToPhysical[6];  // Maps enum value to physical angle
    for ( int i = 0; i < 6; i++ ) enumToPhysical[i] = -1;  // Initialize
    
    // Build reverse mapping
    // measurements[1] = user's home (physical 0°)
    // measurements[2] = 90° CW from home (physical 90°)  
    // measurements[3] = 180° CW from home (physical 180°)
    // measurements[4] = 270° CW from home (physical 270°)
    for ( int measurementIdx = 0; measurementIdx < 4; measurementIdx++ ) {
        int enumValue = orientationEnums[measurementIdx];
        int physicalAngle = measurementIdx * 90;  // 0°, 90°, 180°, 270° from user's home
        enumToPhysical[enumValue] = physicalAngle;
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

