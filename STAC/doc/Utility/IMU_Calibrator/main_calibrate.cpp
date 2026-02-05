// main_calibrate.cpp
// IMU Calibration Tool v3.0 - Pattern-Based Approach
//
// Interactive development tool to determine IMU mounting configuration.
// Outputs board config values for copy/paste into board config header.
//
// Pattern-Based Calibration Methodology:
// ---------------------------------------
// IMU accelerometer readings follow predictable patterns as device rotates.
// There are only 2 base patterns (determined by Z-axis direction), each with
// 4 rotational positions. By identifying which pattern we see at each rotation,
// we can derive the correct axis remapping and display rotation LUT.
//
// Key Insights:
// 1. Pattern matching is simpler and more reliable than formula-based calculation
// 2. Z-axis direction (from FLAT measurement) selects the pattern table
// 3. Display corner position (pixel 0) breaks rotational symmetry
// 4. All configuration can be derived from pattern numbers + modular arithmetic
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

// Pattern definition structure
struct Pattern {
    int x[ 4 ]; // X values for pattern positions 0-3
    int y[ 4 ]; // Y values for pattern positions 0-3
};

// The two base patterns (determined by Z-axis direction)
// Entries ordered so consecutive 90° CW rotations increment index by +1 (mod 4)
// Verified empirically on ATOM Matrix and Waveshare ESP32-S3-Matrix
//
// Z+ AWAY pattern: Corrected order for +1 increment
// Entry order: (X,Y) = (+1,0), (0,-1), (-1,0), (0,+1)
const Pattern PATTERN_Z_AWAY = {{1, 0, -1, 0}, {0, -1, 0, 1}};

// Z+ TOWARD pattern: Theoretical (Y inverted from Z_AWAY)
// Entry order: (X,Y) = (+1,0), (0,+1), (-1,0), (0,-1)
const Pattern PATTERN_Z_TOWARD = {{1, 0, -1, 0}, {0, 1, 0, -1}};

// Function declarations
void printWelcome();
void printInstructions( int step );
void waitForEnter();
void takeMeasurement( int index );
void identifyTopLeftCorner();
int identifyPatternNumber( float x, float y, bool zPointsAway );
void calculateConfiguration();
void printConfiguration( const char* remapX, const char* remapY, const char* remapZ,
                         int deviceHome, int displayOffset, int orientationEnums[ 4 ] );

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
    Serial.println( "║       IMU CALIBRATION TOOL v3.0            ║" );
    Serial.println( "║         Pattern-Based Approach             ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println( "\nThis tool identifies IMU accelerometer patterns to derive" );
    Serial.println( "the correct axis remapping and display rotation configuration." );
    Serial.println( "\nSTEPS:" );
    Serial.println( "  1. Flat with display facing up (determines Z-axis direction)" );
    Serial.println( "  2. Vertical at your chosen home position (any orientation)" );
    Serial.println( "  3. Display corner identification (breaks rotational symmetry)" );
    Serial.println( "  4-6. Three more rotations: 90°, 180°, 270° from home" );
    Serial.println( "\nPattern-based calibration is simpler and more reliable!" );
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
    const char *cornerNames[] = {"Top-Left", "Top-Right", "Bottom-Right", "Bottom-Left"};

    // Calculate corner pixel indices from board config
    const int width = DISPLAY_MATRIX_WIDTH;
    int pixelIndices[] = {0, width - 1, width *width - 1, width *( width - 1 )};

    Serial.printf( "DEBUG: Showing pixel at buffer index %d\n", pixelIndices[ currentCornerTest ] );

    display->clear();
    display->showPixelAtIndex( pixelIndices[ currentCornerTest ] );

    Serial.println( "\n════════════════════════════════════════════" );
    Serial.printf( "Lighting corner %d/4: %s\n", currentCornerTest + 1, cornerNames[ currentCornerTest ] );
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
                Serial.printf( "\n✓ Top-Left corner identified: %s\n", cornerNames[ displayCorner ] );
                display->clear();
                firstCall = true;  // Reset for next calibration run
                currentRotation = 90;
                currentState = STATE_POSITION_90;  // Move to next rotation
                return;
            }
            else if ( input == 'N' || input == 'n' ) {
                currentCornerTest = ( currentCornerTest + 1 ) % 4;
                return;  // Will call again in next loop iteration
            }
            else {
                Serial.print( "\nInvalid input. Enter Y or N: " );
            }
        }
        delay( 10 );
    }
}

// Identify which pattern number (0-3) matches the given accelerometer readings
// Returns: 0-3 for valid pattern, -1 if no match
int identifyPatternNumber( float x, float y, bool zPointsAway ) {
    // Normalize accelerometer values to -1, 0, +1
    int normX = ( abs( x ) > 0.7f ) ? ( x > 0 ? 1 : -1 ) : 0;
    int normY = ( abs( y ) > 0.7f ) ? ( y > 0 ? 1 : -1 ) : 0;

    // Select pattern based on Z direction
    const Pattern& pattern = zPointsAway ? PATTERN_Z_AWAY : PATTERN_Z_TOWARD;

    // Find which pattern position matches
    for ( int i = 0; i < 4; i++ ) {
        if ( pattern.x[ i ] == normX && pattern.y[ i ] == normY ) {
            return i;  // Found match at position i
        }
    }

    return -1;  // No match found
}

void calculateConfiguration() {
    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║         MEASUREMENT SUMMARY                ║" );
    Serial.println( "╚════════════════════════════════════════════╝\n" );

    Serial.println( "FLAT UP:" );
    Serial.printf( "  Sensor X: %7.3fg, Y: %7.3fg, Z: %7.3fg\n",
                   measurements[ 0 ].accX, measurements[ 0 ].accY, measurements[ 0 ].accZ );

    Serial.println( "\nVERTICAL ROTATIONS:" );
    Serial.println( "┌────────────┬──────────┬──────────┬──────────┐" );
    Serial.println( "│ Angle      │ Sensor X │ Sensor Y │ Sensor Z │" );
    Serial.println( "├────────────┼──────────┼──────────┼──────────┤" );
    for ( int i = 1; i < 5; i++ ) {
        Serial.printf( "│  %3d°      │ %7.3fg │ %7.3fg │ %7.3fg │\n",
                       ( i - 1 ) * 90,
                       measurements[ i ].accX,
                       measurements[ i ].accY,
                       measurements[ i ].accZ );
    }
    Serial.println( "└────────────┴──────────┴──────────┴──────────┘\n" );

    // Step 1: Determine Z-axis direction and expected pattern
    Serial.println( "════════════════════════════════════════════" );
    Serial.println( "STEP 1: Determine Z-axis Direction" );
    Serial.println( "════════════════════════════════════════════\n" );

    float zFlat = measurements[ 0 ].accZ;
    Serial.printf( "Z when flat (display up): %.3f g\n", zFlat );

    bool zPointsAway = ( zFlat < -0.5f ); // Z+ points down into table → away when vertical

    if ( zPointsAway ) {
        Serial.println( "→ Z+ points AWAY from user when vertical" );
        Serial.println( "  Pattern: X:(+1,0,-1,0), Y:(0,+1,0,-1)" );
    }
    else {
        Serial.println( "→ Z+ points TOWARD user when vertical" );
        Serial.println( "  Pattern: X:(+1,0,-1,0), Y:(0,-1,0,+1)" );
    }

    // Step 2: Identify pattern numbers at each rotation
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "STEP 2: Identify Pattern Numbers" );
    Serial.println( "════════════════════════════════════════════\n" );

    int patternNumbers[ 4 ]; // Pattern numbers for rotations 0°, 90°, 180°, 270°
    bool allPatternsValid = true;

    for ( int i = 0; i < 4; i++ ) {
        patternNumbers[ i ] = identifyPatternNumber(
                                  measurements[ i + 1 ].accX, // +1 because measurements[0] is FLAT
                                  measurements[ i + 1 ].accY,
                                  zPointsAway
                              );

        if ( patternNumbers[ i ] == -1 ) {
            Serial.printf( "✗ Rotation %d°: NO MATCH (x=%.3f, y=%.3f)\n",
                           i * 90, measurements[ i + 1 ].accX, measurements[ i + 1 ].accY );
            allPatternsValid = false;
        }
        else {
            Serial.printf( "✓ Rotation %d°: Pattern #%d (x=%.3f, y=%.3f)\n",
                           i * 90, patternNumbers[ i ],
                           measurements[ i + 1 ].accX, measurements[ i + 1 ].accY );
        }
    }

    if ( !allPatternsValid ) {
        Serial.println( "\n⚠ ERROR: Some patterns could not be identified!" );
        Serial.println( "  Check measurements and try again." );
        return;
    }

    // Validate pattern sequence (should always increment by +1 for 90° CW rotation)
    // Pattern arrays are reindexed so consecutive entries = consecutive rotations
    bool sequenceValid = true;

    for ( int i = 0; i < 3; i++ ) {
        int expected = ( patternNumbers[ i ] + 1 ) % 4;
        if ( patternNumbers[ i + 1 ] != expected ) {
            Serial.printf( "⚠ Warning: Pattern sequence break at %d° → %d°: expected pattern #%d, got #%d\n",
                           i * 90, ( i + 1 ) * 90, expected, patternNumbers[ i + 1 ] );
            sequenceValid = false;
        }
    }

    if ( sequenceValid ) {
        Serial.println( "✓ Pattern sequence validated (each 90° CW → +1 increment)" );
    }

    // Step 3: Determine axis remapping from pattern numbers
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "STEP 3: Determine Axis Remapping" );
    Serial.println( "════════════════════════════════════════════\n" );

    // The pattern number tells us which axis remap to use
    // Pattern is defined as (X, Y) values at rotations (0°, 90°, 180°, 270°)
    // We need to find which raw sensor axis combination produces this pattern

    struct RemapConfig {
        const char *exprX;
        const char *exprY;
        float ( *getX )( const Measurement & );
        float ( *getY )( const Measurement & );
    };

    RemapConfig remaps[] = {
        {
            "(acc.x)",    "(acc.y)",    []( const Measurement & m ){ return m.accX; },  []( const Measurement & m ) {
                return m.accY;
            }
        },
        {
            "(acc.x)",    "((-acc.y))", []( const Measurement & m ){ return m.accX; },  []( const Measurement & m ) {
                return -m.accY;
            }
        },
        {
            "((-acc.x))", "(acc.y)",    []( const Measurement & m ){ return -m.accX; }, []( const Measurement & m ) {
                return m.accY;
            }
        },
        {
            "((-acc.x))", "((-acc.y))", []( const Measurement & m ){ return -m.accX; }, []( const Measurement & m ) {
                return -m.accY;
            }
        },
        {
            "(acc.y)",    "(acc.x)",    []( const Measurement & m ){ return m.accY; },  []( const Measurement & m ) {
                return m.accX;
            }
        },
        {
            "(acc.y)",    "((-acc.x))", []( const Measurement & m ){ return m.accY; },  []( const Measurement & m ) {
                return -m.accX;
            }
        },
        {
            "((-acc.y))", "(acc.x)",    []( const Measurement & m ){ return -m.accY; }, []( const Measurement & m ) {
                return m.accX;
            }
        },
        {
            "((-acc.y))", "((-acc.x))", []( const Measurement & m ){ return -m.accY; }, []( const Measurement & m ) {
                return -m.accX;
            }
        }
    };

    int bestRemap = -1;

    // Test each remap to see which one produces the identified pattern numbers
    for ( int r = 0; r < 8; r++ ) {
        bool matches = true;

        for ( int rot = 0; rot < 4; rot++ ) {
            float boardX = remaps[ r ].getX( measurements[ rot + 1 ] );
            float boardY = remaps[ r ].getY( measurements[ rot + 1 ] );

            int testPattern = identifyPatternNumber( boardX, boardY, zPointsAway );

            if ( testPattern != patternNumbers[ rot ] ) {
                matches = false;
                break;
            }
        }

        if ( matches ) {
            bestRemap = r;
            Serial.printf( "✓ Found matching axis remap: X=%s, Y=%s\n",
                           remaps[ r ].exprX, remaps[ r ].exprY );
            break;
        }
    }

    if ( bestRemap == -1 ) {
        Serial.println( "\n⚠ ERROR: No axis remapping matched the pattern sequence!" );
        Serial.println( "  This should not happen. Check sensor readings." );
        return;
    }

    // Step 4: Calculate LUT mapping using pattern-based approach
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "STEP 4: Calculate Display Rotation LUT" );
    Serial.println( "════════════════════════════════════════════\n" );

    const char *cornerNames[] = {"Top-Left", "Top-Right", "Bottom-Right", "Bottom-Left"};
    Serial.printf( "Top-left corner identified: %s (corner #%d)\n",
                   cornerNames[ displayCorner ], displayCorner );
    Serial.printf( "Home position (0°) has pattern #%d\n", patternNumbers[ 0 ] );

    // LUT numbering: 0=ROTATE_0, 1=ROTATE_90, 2=ROTATE_180, 3=ROTATE_270
    // Corner offset: TL=0, TR=1, BR=2, BL=3 (number of 90° CW rotations from TL)
    // Baseline LUT needed at home = corner offset (display needs rotation to compensate)

    // Calculate what orientation enum getOrientation() will return at each measurement
    // This requires simulating the IMU getOrientation() logic with our identified remap
    int orientationEnums[ 4 ]; // What enum value getOrientation returns for each measurement

    // Use EXACT same thresholds as runtime MPU6886_IMU.cpp
    constexpr float LOW_TOL = 100.0f;
    constexpr float HIGH_TOL = 900.0f;
    constexpr float MID_TOL = LOW_TOL + ( HIGH_TOL - LOW_TOL ) / 2.0f;  // 500.0f
    constexpr float ACCL_SCALE = 1000.0f;

    for ( int i = 0; i < 4; i++ ) {
        float boardX = remaps[ bestRemap ].getX( measurements[ i + 1 ] );
        float boardY = remaps[ bestRemap ].getY( measurements[ i + 1 ] );

        // Scale exactly as runtime does
        float scaledAccX = boardX * ACCL_SCALE;
        float scaledAccY = boardY * ACCL_SCALE;

        // Simulate getOrientation() logic EXACTLY as in MPU6886_IMU.cpp
        if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) > MID_TOL ) {
            orientationEnums[ i ] = ( scaledAccY > 0 ) ? 3 : 1; // ROTATE_270 or ROTATE_90
        }
        else if ( abs( scaledAccX ) > MID_TOL && abs( scaledAccY ) < HIGH_TOL ) {
            orientationEnums[ i ] = ( scaledAccX > 0 ) ? 2 : 0; // ROTATE_180 or ROTATE_0
        }
        else {
            orientationEnums[ i ] = -1; // Should not happen if pattern matched
        }

        Serial.printf( "  Rotation %3d°: getOrientation() returns enum %d, pattern #%d\n",
                       i * 90, orientationEnums[ i ], patternNumbers[ i ] );
    }

    // Pattern-based LUT calculation
    // At home (0°): Need display rotation = displayCorner offset
    // At each rotation: LUT offset follows pattern offset

    Serial.println( "\nPattern-Based LUT Calculation:" );
    Serial.printf( "  Home pattern #%d → Baseline LUT #%d (corner offset)\n",
                   patternNumbers[ 0 ], displayCorner );
    Serial.println( "  Each 90° CW rotation → Pattern +1 → LUT +1 (wrap at 4)" );

    printConfiguration( remaps[ bestRemap ].exprX, remaps[ bestRemap ].exprY, "(acc.z)",
                        patternNumbers[ 0 ], displayCorner, orientationEnums );
}

void printConfiguration( const char* remapX, const char* remapY, const char* remapZ,
                         int homePattern, int cornerOffset, int orientationEnums[ 4 ] ) {

    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║         BOARD CONFIGURATION VALUES         ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println( "\nCopy these lines into your board config header:" );
    Serial.println( "\n// IMU Configuration (from calibration tool v3.0 - pattern-based)" );
    Serial.printf( "#define IMU_AXIS_REMAP_X    %s\n", remapX );
    Serial.printf( "#define IMU_AXIS_REMAP_Y    %s\n", remapY );
    Serial.printf( "#define IMU_AXIS_REMAP_Z    %s\n", remapZ );

    Serial.println( "" );
    Serial.printf( "\n// Home pattern #%d, corner offset %d (baseline LUT #%d)\n",
                   homePattern, cornerOffset, cornerOffset );
    Serial.println( "#define DEVICE_ORIENTATION_TO_LUT_MAP { \\" );

    const char *lutNames[] = {
        "Orientation::ROTATE_0",
        "Orientation::ROTATE_90",
        "Orientation::ROTATE_180",
        "Orientation::ROTATE_270"
    };

    // Build LUT: indexed by orientation enum, returns LUT to use
    // Pattern-based: LUT[enum] = (cornerOffset + rotationOffset) % 4
    int lut[ 6 ]; // 0-3 = ROTATE_0/90/180/270, 4=FLAT, 5=UNKNOWN
    for ( int i = 0; i < 6; i++ ) {
        lut[ i ] = -1;
    }

    // For each measurement, calculate the LUT needed
    for ( int measurementIdx = 0; measurementIdx < 4; measurementIdx++ ) {
        int enumValue = orientationEnums[ measurementIdx ];
        int physicalAngle = measurementIdx * 90;  // 0°, 90°, 180°, 270° from home

        // LUT calculation: invert physical rotation and add corner offset
        // Physical 0° (home) → Use LUT = cornerOffset
        // Physical 90° CW → Rotate content 90° CCW = use LUT (cornerOffset + 3) % 4
        // Physical 180° → Rotate content 180° = use LUT (cornerOffset + 2) % 4
        // Physical 270° CW → Rotate content 270° CCW = use LUT (cornerOffset + 1) % 4
        int lutAngle = ( cornerOffset * 90 - physicalAngle + 360 ) % 360;
        int lutIndex = lutAngle / 90;

        lut[ enumValue ] = lutIndex;

        Serial.printf( "  Measurement[%d]: physical %3d°, enum=%d → LUT #%d (%s)\n",
                       measurementIdx, physicalAngle, enumValue, lutIndex, lutNames[ lutIndex ] );
    }

    // FLAT and UNKNOWN: Use same LUT as home position (physical 0°)
    // Home position is always measurement[0], which maps to orientationEnums[0]
    lut[ 4 ] = lut[ orientationEnums[ 0 ] ]; // FLAT → same as home
    lut[ 5 ] = lut[ orientationEnums[ 0 ] ]; // UNKNOWN → same as home

    Serial.println( "\nLUT Map Output:" );
    for ( int i = 0; i < 4; i++ ) {
        Serial.printf( "    %s,  /* enum %d → LUT_%d */ \\\n",
                       lutNames[ lut[ i ] ], i, lut[ i ] * 90 );
    }
    Serial.printf( "    %s,  /* FLAT → same as home */ \\\n", lutNames[ lut[ 4 ] ] );
    Serial.printf( "    %s   /* UNKNOWN → same as home */ \\\n", lutNames[ lut[ 5 ] ] );

    Serial.println( "}" );

    // Reverse mapping: enum → physical angle
    Serial.println( "\n// Reverse mapping for debug logging: enum → physical angle" );
    Serial.println( "#define ORIENTATION_ENUM_TO_PHYSICAL_ANGLE { \\" );

    int enumToPhysical[ 6 ];
    for ( int i = 0; i < 6; i++ ) {
        enumToPhysical[ i ] = -1;
    }

    for ( int measurementIdx = 0; measurementIdx < 4; measurementIdx++ ) {
        int enumValue = orientationEnums[ measurementIdx ];
        int physicalAngle = measurementIdx * 90;
        enumToPhysical[ enumValue ] = physicalAngle;
    }
    enumToPhysical[ 4 ] = -1; // FLAT
    enumToPhysical[ 5 ] = -1; // UNKNOWN

    for ( int i = 0; i < 4; i++ ) {
        Serial.printf( "    %d,  /* %s → Physical %d° */ \\\n",
                       enumToPhysical[ i ], lutNames[ i ], enumToPhysical[ i ] );
    }
    Serial.printf( "    -1,  /* FLAT */ \\\n" );
    Serial.printf( "    -1   /* UNKNOWN */ \\\n" );
    Serial.println( "}" );

    Serial.println( "\n════════════════════════════════════════════\n" );
}


//  --- EOF --- //

