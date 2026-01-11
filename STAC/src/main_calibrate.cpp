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
    STATE_CALCULATE,
    STATE_COMPLETE
};

// Measurement data for each rotation
struct Measurement {
    float accX;
    float accY;
    float accZ;
    char userResponse; // T, B, L, R
};

// Global state
CalibrationDisplayType* display = nullptr;
std::unique_ptr<IIMU> imu;
CalibrationState currentState = STATE_WELCOME;
Measurement measurements[4]; // 0°, 90°, 180°, 270°
int currentRotation = 0;

// Color constants
const uint32_t COLOR_GREEN = 0x00FF00;
const uint32_t COLOR_ORANGE = 0xFF8800;
const uint32_t COLOR_RED = 0xFF0000;
const uint32_t COLOR_BLUE = 0x0000FF;

// Function declarations
void printWelcome();
void printInstructions(int rotation);
void waitForEnter();
char waitForResponse();
void takeMeasurement(int index);
void calculateConfiguration();
void printConfiguration(const char* remapX, const char* remapY, const char* remapZ, int rotationOffset);

void setup() {
    Serial.begin(115200);
    delay(1000); // Wait for serial connection
    
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════════╗");
    Serial.println("║     STAC IMU Calibration Tool v1.0.0       ║");
    Serial.println("╚════════════════════════════════════════════╝");
    Serial.println();

    // Initialize display
    display = new CalibrationDisplayType();
    if (!display->begin()) {
        Serial.println("ERROR: Failed to initialize display");
        display->showMessage("DISPLAY FAIL", COLOR_RED);
        while (true) delay(1000);
    }
    Serial.println("✓ Display initialized");
    #if defined(DISPLAY_TYPE_LED_MATRIX)
        display->clear(); // LED can't show text, just clear
    #else
        display->showMessage("CALIBRATE", COLOR_GREEN);
        delay(1000);
    #endif

    // Initialize IMU
    imu = Hardware::IMUFactory::create();
    if (!imu || !imu->begin()) {
        Serial.println("ERROR: Failed to initialize IMU");
        display->showMessage("IMU FAIL", COLOR_RED);
        while (true) delay(1000);
    }
    Serial.println("✓ IMU initialized");
    Serial.printf("  Type: %s\n", imu->getType());
    delay(100); // Allow IMU to stabilize

    currentState = STATE_WELCOME;
}

void loop() {
    switch (currentState) {
        case STATE_WELCOME:
            printWelcome();
            waitForEnter();
            currentRotation = 0;
            currentState = STATE_POSITION_0;
            break;

        case STATE_POSITION_0:
            printInstructions(0);
            display->showTopMarker();
            currentState = STATE_WAIT_INPUT_0;
            break;

        case STATE_WAIT_INPUT_0:
            takeMeasurement(0);
            measurements[0].userResponse = waitForResponse();
            currentRotation = 90;
            currentState = STATE_POSITION_90;
            break;

        case STATE_POSITION_90:
            printInstructions(90);
            display->showTopMarker();
            currentState = STATE_WAIT_INPUT_90;
            break;

        case STATE_WAIT_INPUT_90:
            takeMeasurement(1);
            measurements[1].userResponse = waitForResponse();
            currentRotation = 180;
            currentState = STATE_POSITION_180;
            break;

        case STATE_POSITION_180:
            printInstructions(180);
            display->showTopMarker();
            currentState = STATE_WAIT_INPUT_180;
            break;

        case STATE_WAIT_INPUT_180:
            takeMeasurement(2);
            measurements[2].userResponse = waitForResponse();
            currentRotation = 270;
            currentState = STATE_POSITION_270;
            break;

        case STATE_POSITION_270:
            printInstructions(270);
            display->showTopMarker();
            currentState = STATE_WAIT_INPUT_270;
            break;

        case STATE_WAIT_INPUT_270:
            takeMeasurement(3);
            measurements[3].userResponse = waitForResponse();
            currentState = STATE_CALCULATE;
            break;

        case STATE_CALCULATE:
            Serial.println("\n════════════════════════════════════════════");
            Serial.println("Calculating configuration...");
            display->showMessage("CALC...", COLOR_BLUE);
            calculateConfiguration();
            currentState = STATE_COMPLETE;
            break;

        case STATE_COMPLETE:
            display->clear();  // Clear display when done
            Serial.println("\n════════════════════════════════════════════");
            Serial.println("Calibration complete!");
            Serial.println("Copy the values above into your board config.");
            Serial.println("\nReset device to run calibration again.");
            while (true) delay(1000); // Halt
            break;
    }
}

void printWelcome() {
    Serial.println("\nThis tool will determine your IMU mounting configuration.");
    Serial.println("\nYou will:");
    Serial.println("  1. Hold the board in HOME position (USB port DOWN)");
    Serial.println("  2. Note where a reference marker appears");
    Serial.println("  3. Rotate board 90° clockwise and repeat");
    Serial.println("  4. Continue for 180° and 270°");
    Serial.println("\nThe tool will then calculate the configuration values");
    Serial.println("needed for your board config file.");
    Serial.println("\n════════════════════════════════════════════");
    Serial.println("\nPress ENTER to begin...");
}

void printInstructions(int rotation) {
    Serial.println("\n════════════════════════════════════════════");
    Serial.printf("STEP %d/4: Rotation %d°\n", (rotation / 90) + 1, rotation);
    Serial.println("════════════════════════════════════════════");
    
    if (rotation == 0) {
        Serial.println("\n1. Hold board VERTICALLY with USB port at BOTTOM");
        Serial.println("   (This is your HOME position)");
    } else {
        Serial.printf("\n1. Rotate board %d° CLOCKWISE from HOME\n", rotation);
    }
    
    Serial.println("2. Look at the display");
    Serial.println("3. Note where the reference marker appears");
    Serial.println("\nPress ENTER when ready...");
    waitForEnter();
}

void waitForEnter() {
    while (!Serial.available()) {
        delay(10);
    }
    // Clear the input buffer
    while (Serial.available()) {
        Serial.read();
    }
}

char waitForResponse() {
    Serial.println("\nWhere is the reference marker?");
    Serial.println("  T = Top");
    Serial.println("  B = Bottom");
    Serial.println("  L = Left");
    Serial.println("  R = Right");
    Serial.print("\nEnter your response (T/B/L/R): ");
    
    char response = 0;
    while (true) {
        if (Serial.available()) {
            response = toupper(Serial.read());
            if (response == 'T' || response == 'B' || response == 'L' || response == 'R') {
                Serial.println(response);
                // Clear remaining input
                while (Serial.available()) Serial.read();
                break;
            }
        }
        delay(10);
    }
    
    return response;
}

void takeMeasurement(int index) {
    Serial.println("\nReading IMU data...");
    
    // Read raw accelerometer data from IMU
    if (!imu->getRawAcceleration(measurements[index].accX, 
                                  measurements[index].accY, 
                                  measurements[index].accZ)) {
        Serial.println("ERROR: Failed to read IMU data");
        measurements[index].accX = 0.0f;
        measurements[index].accY = 0.0f;
        measurements[index].accZ = 0.0f;
    }
    
    Serial.printf("  AccX: %7.3f  AccY: %7.3f  AccZ: %7.3f\n",
                  measurements[index].accX,
                  measurements[index].accY,
                  measurements[index].accZ);
}

void calculateConfiguration() {
    Serial.println("\n════════════════════════════════════════════");
    Serial.println("MEASUREMENT DATA:");
    Serial.println("════════════════════════════════════════════");
    
    for (int i = 0; i < 4; i++) {
        Serial.printf("\nRotation %d°:\n", i * 90);
        Serial.printf("  AccX: %7.3f  AccY: %7.3f  AccZ: %7.3f\n",
                      measurements[i].accX,
                      measurements[i].accY,
                      measurements[i].accZ);
        Serial.printf("  User saw marker at: %c\n", measurements[i].userResponse);
    }
    
    Serial.println("\n════════════════════════════════════════════");
    Serial.println("ANALYSIS:");
    Serial.println("════════════════════════════════════════════");
    
    // Analyze which sensor axis corresponds to which board axis
    // At 0° (USB down), marker at top, gravity should be along board Y-axis
    // At 90° CW, marker rotates, gravity should be along board X-axis
    
    // Find dominant axis at each rotation
    char axisMap[4]; // Which sensor axis (X/Y/Z) is dominant at each rotation
    bool axisSign[4]; // Sign of dominant axis (true = positive)
    
    for (int i = 0; i < 4; i++) {
        float absX = abs(measurements[i].accX);
        float absY = abs(measurements[i].accY);
        float absZ = abs(measurements[i].accZ);
        
        if (absX > absY && absX > absZ) {
            axisMap[i] = 'X';
            axisSign[i] = (measurements[i].accX > 0);
        } else if (absY > absX && absY > absZ) {
            axisMap[i] = 'Y';
            axisSign[i] = (measurements[i].accY > 0);
        } else {
            axisMap[i] = 'Z';
            axisSign[i] = (measurements[i].accZ > 0);
        }
        
        Serial.printf("  Rotation %d°: Dominant axis = %c%c, Marker at %c\n",
                      i * 90, 
                      axisSign[i] ? '+' : '-',
                      axisMap[i],
                      measurements[i].userResponse);
    }
    
    // Determine axis remapping
    // At 0° (USB down): marker at top = board needs Y-axis up when sensor shows dominant axis
    // At 90° (USB right): marker rotates = board needs X-axis right when sensor shows dominant axis
    
    const char* remapX = "acc.x";  // Default: no remapping
    const char* remapY = "acc.y";
    const char* remapZ = "acc.z";
    int rotationOffset = 0;
    
    // Determine mapping from 0° and 90° measurements
    if (axisMap[0] == 'Y' && axisMap[1] == 'X') {
        // Standard mapping: Board Y = Sensor Y, Board X = Sensor X
        if (axisSign[0]) {
            remapY = "acc.y";
        } else {
            remapY = "(-acc.y)";
        }
        
        if (axisSign[1]) {
            remapX = "acc.x";
        } else {
            remapX = "(-acc.x)";
        }
        
        // Check if marker position matches expected
        if (measurements[0].userResponse == 'T' && measurements[1].userResponse == 'R') {
            rotationOffset = 0;
        } else if (measurements[0].userResponse == 'R' && measurements[1].userResponse == 'B') {
            rotationOffset = 90;
        } else if (measurements[0].userResponse == 'B' && measurements[1].userResponse == 'L') {
            rotationOffset = 180;
        } else if (measurements[0].userResponse == 'L' && measurements[1].userResponse == 'T') {
            rotationOffset = 270;
        }
    } else if (axisMap[0] == 'X' && axisMap[1] == 'Y') {
        // Swapped: Board Y = Sensor X, Board X = Sensor Y
        if (axisSign[0]) {
            remapY = "acc.x";
        } else {
            remapY = "(-acc.x)";
        }
        
        if (axisSign[1]) {
            remapX = "acc.y";
        } else {
            remapX = "(-acc.y)";
        }
        
        // Determine rotation offset based on marker positions
        if (measurements[0].userResponse == 'T' && measurements[1].userResponse == 'R') {
            rotationOffset = 0;
        } else if (measurements[0].userResponse == 'R' && measurements[1].userResponse == 'B') {
            rotationOffset = 90;
        } else if (measurements[0].userResponse == 'B' && measurements[1].userResponse == 'L') {
            rotationOffset = 180;
        } else if (measurements[0].userResponse == 'L' && measurements[1].userResponse == 'T') {
            rotationOffset = 270;
        }
    }
    
    remapZ = "acc.z"; // Z is typically not remapped
    
    Serial.printf("\nCalculated mapping:\n");
    Serial.printf("  Board X = Sensor %s\n", remapX);
    Serial.printf("  Board Y = Sensor %s\n", remapY);
    Serial.printf("  Board Z = Sensor %s\n", remapZ);
    Serial.printf("  Rotation offset = %d°\n", rotationOffset);
    
    printConfiguration(remapX, remapY, remapZ, rotationOffset);
}

void printConfiguration(const char* remapX, const char* remapY, const char* remapZ, int rotationOffset) {
    Serial.println("\n╔════════════════════════════════════════════╗");
    Serial.println("║         BOARD CONFIGURATION VALUES         ║");
    Serial.println("╚════════════════════════════════════════════╝");
    Serial.println("\nAdd these lines to your board config header:");
    Serial.println("\n// IMU Configuration (from calibration tool)");
    Serial.printf("#define IMU_AXIS_REMAP_X    (%s)\n", remapX);
    Serial.printf("#define IMU_AXIS_REMAP_Y    (%s)\n", remapY);
    Serial.printf("#define IMU_AXIS_REMAP_Z    (%s)\n", remapZ);
    
    const char* offsetName = "OrientationOffset::OFFSET_0";
    if (rotationOffset == 90) offsetName = "OrientationOffset::OFFSET_90";
    else if (rotationOffset == 180) offsetName = "OrientationOffset::OFFSET_180";
    else if (rotationOffset == 270) offsetName = "OrientationOffset::OFFSET_270";
    
    Serial.printf("#define IMU_ROTATION_OFFSET %s\n", offsetName);
    Serial.println("\n════════════════════════════════════════════\n");
}


//  --- EOF --- //
