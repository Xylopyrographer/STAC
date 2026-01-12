// main_calibrate.cpp
// IMU Calibration Tool
//
// Interactive development tool to determine IMU mounting configuration.
// Outputs board config values for copy/paste into board config header.
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
    STATE_POSITION_0,
    STATE_WAIT_INPUT_0,
    STATE_POSITION_90,
    STATE_WAIT_INPUT_90,
    STATE_POSITION_180,
    STATE_WAIT_INPUT_180,
    STATE_POSITION_270,
    STATE_WAIT_INPUT_270,
    STATE_FLAT_UP,
    STATE_WAIT_INPUT_FLAT_UP,
    STATE_FLAT_DOWN,
    STATE_WAIT_INPUT_FLAT_DOWN,
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
Measurement measurements[ 6 ]; // 0°, 90°, 180°, 270°, FLAT_UP, FLAT_DOWN
int currentRotation = 0;

// Color constants
const uint32_t COLOR_GREEN = 0x00FF00;
const uint32_t COLOR_ORANGE = 0xFF8800;
const uint32_t COLOR_RED = 0xFF0000;
const uint32_t COLOR_BLUE = 0x0000FF;

// Function declarations
void printWelcome();
void printInstructions( int rotation );
void waitForEnter();
void takeMeasurement( int index );
void calculateConfiguration();
void printConfiguration( const char* remapX, const char* remapY, const char* remapZ, int rotationOffset );

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
            currentRotation = 0;
            currentState = STATE_POSITION_0;
            break;

        case STATE_POSITION_0:
            printInstructions( 0 );
            currentState = STATE_WAIT_INPUT_0;
            break;

        case STATE_WAIT_INPUT_0:
            takeMeasurement( 0 );
            currentRotation = 90;
            currentState = STATE_POSITION_90;
            break;

        case STATE_POSITION_90:
            printInstructions( 90 );
            currentState = STATE_WAIT_INPUT_90;
            break;

        case STATE_WAIT_INPUT_90:
            takeMeasurement( 1 );
            currentRotation = 180;
            currentState = STATE_POSITION_180;
            break;

        case STATE_POSITION_180:
            printInstructions( 180 );
            currentState = STATE_WAIT_INPUT_180;
            break;

        case STATE_WAIT_INPUT_180:
            takeMeasurement( 2 );
            currentRotation = 270;
            currentState = STATE_POSITION_270;
            break;

        case STATE_POSITION_270:
            printInstructions( 270 );
            currentState = STATE_WAIT_INPUT_270;
            break;

        case STATE_WAIT_INPUT_270:
            takeMeasurement( 3 );
            currentState = STATE_FLAT_UP;
            break;

        case STATE_FLAT_UP:
            printInstructions( -1 );  // -1 = FLAT_UP
            currentState = STATE_WAIT_INPUT_FLAT_UP;
            break;

        case STATE_WAIT_INPUT_FLAT_UP:
            takeMeasurement( 4 );
            currentState = STATE_FLAT_DOWN;
            break;

        case STATE_FLAT_DOWN:
            printInstructions( -2 );  // -2 = FLAT_DOWN
            currentState = STATE_WAIT_INPUT_FLAT_DOWN;
            break;

        case STATE_WAIT_INPUT_FLAT_DOWN:
            takeMeasurement( 5 );
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
    Serial.println( "rotate and tilt the device through 6 positions." );
    Serial.println( "\nIMPORTANT: For rotations 0°-270°, keep device VERTICAL." );
    Serial.println( "           Only rotate around the vertical axis (Z-axis)." );
    Serial.println( "\nExpected: Z-axis stays ~0g, X/Y rotate between -1g and +1g" );
    Serial.println( "\n════════════════════════════════════════════" );
    Serial.println( "\nPress ENTER to begin..." );
}

void printInstructions( int rotation ) {
    Serial.println( "\n════════════════════════════════════════════" );
    
    if ( rotation == -1 ) {
        // FLAT UP
        Serial.println( "STEP 5/6: Device FLAT, display facing UP" );
        Serial.println( "════════════════════════════════════════════" );
        Serial.println( "\n3. Lay device FLAT on table, display facing UP" );
        Serial.println( "   (Gravity pointing DOWN through display)" );
    } else if ( rotation == -2 ) {
        // FLAT DOWN
        Serial.println( "STEP 6/6: Device FLAT, display facing DOWN" );
        Serial.println( "════════════════════════════════════════════" );
        Serial.println( "\n4. Flip device FLAT, display facing DOWN" );
        Serial.println( "   (Gravity pointing UP through display)" );
    } else {
        // Vertical rotations
        int step = rotation / 90;
        Serial.printf( "STEP %d/6: Device rotated %d° clockwise\n", step + 1, rotation );
        Serial.println( "════════════════════════════════════════════" );

        if ( rotation == 0 ) {
            Serial.println( "\n1. Hold device VERTICAL with USB port DOWN" );
            Serial.println( "   (This is your HOME position - 0°)" );
        } else {
            Serial.printf( "\n2. Rotate device %d° CLOCKWISE (keep vertical!)\n", rotation );
            Serial.printf( "   (USB port now pointing %s)\n", 
                          rotation == 90 ? "LEFT" : 
                          rotation == 180 ? "UP" : "RIGHT" );
        }
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

void calculateConfiguration() {
    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║         MEASUREMENT SUMMARY                ║" );
    Serial.println( "╚════════════════════════════════════════════╝\n" );

    Serial.println( "VERTICAL ROTATIONS:" );
    Serial.println( "┌────────────┬──────────┬──────────┬──────────┐" );
    Serial.println( "│ Angle      │ Sensor X │ Sensor Y │ Sensor Z │" );
    Serial.println( "├────────────┼──────────┼──────────┼──────────┤" );
    for ( int i = 0; i < 4; i++ ) {
        Serial.printf( "│  %3d°      │ %7.3fg │ %7.3fg │ %7.3fg │\n",
                       i * 90,
                       measurements[ i ].accX,
                       measurements[ i ].accY,
                       measurements[ i ].accZ );
    }
    Serial.println( "└────────────┴──────────┴──────────┴──────────┘\n" );

    Serial.println( "FLAT ORIENTATIONS:" );
    Serial.println( "┌────────────┬──────────┬──────────┬──────────┐" );
    Serial.println( "│ Position   │ Sensor X │ Sensor Y │ Sensor Z │" );
    Serial.println( "├────────────┼──────────┼──────────┼──────────┤" );
    Serial.printf( "│ FLAT UP    │ %7.3fg │ %7.3fg │ %7.3fg │\n",
                   measurements[ 4 ].accX,
                   measurements[ 4 ].accY,
                   measurements[ 4 ].accZ );
    Serial.printf( "│ FLAT DOWN  │ %7.3fg │ %7.3fg │ %7.3fg │\n",
                   measurements[ 5 ].accX,
                   measurements[ 5 ].accY,
                   measurements[ 5 ].accZ );
    Serial.println( "└────────────┴──────────┴──────────┴──────────┘\n" );

    // Check if Z-axis remained relatively constant (device was vertical)
    float maxZ = -999.0f;
    float minZ = 999.0f;
    for ( int i = 0; i < 4; i++ ) {
        if ( measurements[ i ].accZ > maxZ ) maxZ = measurements[ i ].accZ;
        if ( measurements[ i ].accZ < minZ ) minZ = measurements[ i ].accZ;
    }
    float zVariation = maxZ - minZ;

    Serial.println( "════════════════════════════════════════════" );
    Serial.println( "ANALYSIS:" );
    Serial.println( "════════════════════════════════════════════\n" );

    Serial.printf( "Z-axis variation (vertical): %.3f g (max=%.3f, min=%.3f)\n", zVariation, maxZ, minZ );
    
    if ( zVariation > 0.3f ) {
        Serial.println( "\n⚠ WARNING: Z-axis varied significantly!" );
        Serial.println( "  Device was NOT held vertical consistently." );
        Serial.println( "  Expected: Z ≈ 0g at all rotations (perpendicular to gravity)" );
        Serial.println( "  Please repeat calibration, keeping device VERTICAL." );
    } else {
        Serial.println( "\n✓ Z-axis remained stable - device was held vertical" );
    }

    // Analyze FLAT measurements
    Serial.printf( "\nZ-axis when FLAT UP:   %.3f g\n", measurements[ 4 ].accZ );
    Serial.printf( "Z-axis when FLAT DOWN: %.3f g\n", measurements[ 5 ].accZ );
    
    float zDelta = measurements[ 4 ].accZ - measurements[ 5 ].accZ;
    Serial.printf( "Z-axis delta (UP - DOWN): %.3f g\n", zDelta );
    
    if ( zDelta > 1.5f ) {
        Serial.println( "→ Sensor Z+ points AWAY from display (toward user when facing screen)" );
    } else if ( zDelta < -1.5f ) {
        Serial.println( "→ Sensor Z+ points TOWARD display (away from user when facing screen)" );
    } else {
        Serial.println( "⚠ WARNING: Cannot determine Z-axis direction - delta too small" );
    }

    Serial.println( "\n════════════════════════════════════════════" );

    // Measurement 0 (0°): Buffer top-left at physical top-left (reference)
    // Measurement 2 (180°): Buffer top-left at physical bottom-right (device rotated 180°)
    // The sensor axis that inverts tells us which is aligned with buffer Y

    float deltaX_180 = measurements[ 2 ].accX - measurements[ 0 ].accX;
    float deltaY_180 = measurements[ 2 ].accY - measurements[ 0 ].accY;

    const char *boardYMapping = nullptr;

    Serial.printf( "180° rotation: ΔX=%7.3f, ΔY=%7.3f\n", deltaX_180, deltaY_180 );

    if ( fabs( deltaX_180 ) > fabs( deltaY_180 ) ) {
        // Sensor X changes most during 180° rotation, so buffer Y = sensor X
        // At 0° if sensor X is negative, buffer down = sensor -X, so buffer Y = sensor X
        // At 0° if sensor X is positive, buffer down = sensor +X, so buffer Y = sensor -X
        boardYMapping = ( measurements[ 0 ].accX < 0 ) ? "(acc.x)" : "((-acc.x))";
        Serial.printf( "  → Buffer Y axis = Sensor X %s\n",
                       ( measurements[ 0 ].accX < 0 ) ? "(no inversion)" : "(inverted)" );
    }
    else {
        // Sensor Y changes most during 180° rotation, so buffer Y = sensor Y
        boardYMapping = ( measurements[ 0 ].accY < 0 ) ? "(acc.y)" : "((-acc.y))";
        Serial.printf( "  → Buffer Y axis = Sensor Y %s\n",
                       ( measurements[ 0 ].accY < 0 ) ? "(no inversion)" : "(inverted)" );
    }

    // Measurement 1 (90°): Buffer top-left at physical top-right (device rotated 90° CW)
    // Compared to measurement 0, tells us which sensor axis is aligned with buffer X

    float deltaX_90 = measurements[ 1 ].accX - measurements[ 0 ].accX;
    float deltaY_90 = measurements[ 1 ].accY - measurements[ 0 ].accY;

    const char *boardXMapping = nullptr;

    Serial.printf( "\n90° rotation: ΔX=%7.3f, ΔY=%7.3f\n", deltaX_90, deltaY_90 );

    if ( fabs( deltaX_90 ) > fabs( deltaY_90 ) ) {
        // Sensor X changes most during 90° CW rotation, so buffer X = sensor X
        // At 90° CW, if sensor X becomes more positive, buffer right = sensor +X, so buffer X = sensor X
        // At 90° CW, if sensor X becomes more negative, buffer right = sensor -X, so buffer X = sensor -X
        boardXMapping = ( measurements[ 1 ].accX > measurements[ 0 ].accX ) ? "(acc.x)" : "((-acc.x))";
        Serial.printf( "  → Buffer X axis = Sensor X %s\n",
                       ( measurements[ 1 ].accX > measurements[ 0 ].accX ) ? "(no inversion)" : "(inverted)" );
    }
    else {
        // Sensor Y changes most during 90° CW rotation, so buffer X = sensor Y
        boardXMapping = ( measurements[ 1 ].accY > measurements[ 0 ].accY ) ? "(acc.y)" : "((-acc.y))";
        Serial.printf( "  → Buffer X axis = Sensor Y %s\n",
                       ( measurements[ 1 ].accY > measurements[ 0 ].accY ) ? "(no inversion)" : "(inverted)" );
    }

    const char *remapX = boardXMapping;
    const char *remapY = boardYMapping;
    const char *remapZ = "(acc.z)";

    // Determine which rotation corresponds to device UP (vertical, gravity pointing down)
    // Look for the measurement with strongest downward acceleration (~-1g)
    Serial.printf( "\nDetecting natural UP orientation:\n" );
    
    int upRotation = 0;
    float maxGravity = 0.0f;
    const char *rotationLabels[] = {"0°", "90°", "180°", "270°"};
    
    for ( int i = 0; i < 4; i++ ) {
        // Calculate total gravity magnitude for this orientation
        float totalG = sqrtf( measurements[ i ].accX * measurements[ i ].accX +
                             measurements[ i ].accY * measurements[ i ].accY +
                             measurements[ i ].accZ * measurements[ i ].accZ );
        
        Serial.printf( "  %3s rotation: |g| = %5.3f\n", rotationLabels[ i ], totalG );
        
        if ( totalG > maxGravity ) {
            maxGravity = totalG;
            upRotation = i;
        }
    }
    
    const char *offsetNames[] = {"OFFSET_0", "OFFSET_90", "OFFSET_180", "OFFSET_270"};
    int offsetValue = upRotation * 90;
    
    Serial.printf( "\n  → Device is UP at %s rotation\n", rotationLabels[ upRotation ] );
    Serial.printf( "  → Glyph map needs %s to align with UP\n", offsetNames[ upRotation ] );

    Serial.printf( "\nCalculated mapping:\n" );
    Serial.printf( "  Board X (horizontal) = Sensor %s\n", remapX );
    Serial.printf( "  Board Y (vertical)   = Sensor %s\n", remapY );
    Serial.printf( "  Board Z              = Sensor %s\n", remapZ );
    Serial.printf( "  Rotation offset      = %s\n", offsetNames[ upRotation ] );

    printConfiguration( remapX, remapY, remapZ, upRotation );
}

void printConfiguration( const char* remapX, const char* remapY, const char* remapZ, int rotationOffset ) {
    const char *offsetNames[] = {"OFFSET_0", "OFFSET_90", "OFFSET_180", "OFFSET_270"};
    
    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║         BOARD CONFIGURATION VALUES         ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println( "\nCopy these lines into your board config header:" );
    Serial.println( "\n// IMU Configuration (from calibration tool)" );
    Serial.printf( "#define IMU_AXIS_REMAP_X    %s\n", remapX );
    Serial.printf( "#define IMU_AXIS_REMAP_Y    %s\n", remapY );
    Serial.printf( "#define IMU_AXIS_REMAP_Z    %s\n", remapZ );
    Serial.printf( "#define IMU_ROTATION_OFFSET OrientationOffset::%s\n", offsetNames[ rotationOffset ] );
    Serial.println( "\n════════════════════════════════════════════\n" );
}


//  --- EOF --- //
