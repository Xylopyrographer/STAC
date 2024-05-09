
// STACIMU.h

orientation_t getOrientation() {
    /* Retrieves the current orientation of the IMU.
        - returns one of six possible orientations: UP; DOWN; LEFT; RIGHT; FLAT; UNKNOWN.
        - for the M5Stack ATOM MATRIX, the USB port is the reference point.
        - "UP" = device is vertical with the USB port at the bottom.
        - "DOWN = device vertical with the USB port at the top.
        - "LEFT" = device vertical with the display facing you and the USB port to the left.
        - "RIGHT" = device vertical with the display facing you and the USB port to the right.
        - "FLAT" = device is horizontal, display is facing up (or down - though that would be impractical).
        - "UNKNOWN" = none of the above.
    */

    I2C_MPU6886 imu( I2C_MPU6886_DEFAULT_ADDRESS, Wire );   // IMU object
    orientation_t stacState = UNKNOWN;      // set the default orientation of the STAC

    Wire.begin( I_SDA, I_SCL, 100000L );    // enable the I2C bus (SDA pin, SCL pin, Clock frequency)

    bool accel = imu.begin() != -1;
    if ( !accel ) {
        log_e( "Could not initialize the IMU error." );
        Wire.flush();       // we're all done with the IMU - release the I2C bus resources & power down peripheral
        return stacState;
    }

    // IMU accelerometer parameters
    const float LOW_TOL = 100.0;
    const float HIGH_TOL = 900.0;
    const float MID_TOL = LOW_TOL + ( HIGH_TOL - LOW_TOL ) / 2.0;

    float accX = 0, accY = 0, accZ = 0;

    imu.getAccel( &accX, &accY, &accZ );    // Call into the accelerometer to get the current values

    float scaledAccX = accX * 1000;
    float scaledAccY = accY * 1000;
    float scaledAccZ = accZ * 1000;

    if ( ( abs( scaledAccX ) < HIGH_TOL ) && ( abs( scaledAccY ) > MID_TOL ) && ( abs( scaledAccZ ) < HIGH_TOL ) ) {
        if ( scaledAccY > 0 ) {                 // Device is oriented Up
            stacState = UP;
        }
        else {                                  // scaledAccY < 0: Device is oriented Down
            stacState = DOWN;
        }
    }
    else if ( ( abs( scaledAccX ) > MID_TOL ) && ( abs( scaledAccY ) < HIGH_TOL ) && ( abs( scaledAccZ ) < HIGH_TOL ) ) {
        if ( scaledAccX > 0 ) {                 // Device is oriented Right
            stacState = RIGHT;
        }
        else {                                  // scaledAccX < 0: Device is oriented Left
            stacState = LEFT;
        }
    }
    else {
        stacState = FLAT;
    }

    Wire.flush();   // done with the IMU - release all I2C bus resources, power down peripheral

    return stacState;

}   // end getOrientation()


//  --- EOF ---
