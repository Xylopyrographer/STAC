
ORIENTATION getOrientation( ) {
/* Retrieves the current orientation of the IMU.
    - returns one of six possible orientations: UP; DOWN; LEFT; RIGHT; FLAT; UNDEFINED.
    - for the M5Stack ATOM MATRIX, the USB port is the reference point.
    - "UP" = device is vertical with the USB port at the bottom.
    - "DOWN = device vertical with the USB port at the top.
    - "LEFT" = device vertical with the display facing you and the USB port to the left.
    - "RIGHT" = device vertical with the display facing you and the USB port to the right.
    - "FLAT" = device is horizontal, display is facing up (or down - though that would be impractical).
    - "UNDEFINED" = none of the above.
*/
     
    ORIENTATION stacState = ORIENTATION::UP;
  
    float accX = 0, accY = 0, accZ = 0 ;      
  
    imu.getAccel( &accX, &accY, &accZ );   // Call into the accelerometer to get the current values 
    
    float scaledAccX = accX * 1000;
    float scaledAccY = accY * 1000;
    float scaledAccZ = accZ * 1000;

    if ( ( abs( scaledAccX ) < HIGH_TOL ) && ( abs( scaledAccY ) > MID_TOL ) && ( abs( scaledAccZ ) < HIGH_TOL ) ) {
        if ( scaledAccY > 0 ) {                 // Device is oriented Up            
            stacState = ORIENTATION::UP;
        }
        else {                                  // scaledAccY < 0: Device is oriented Down
            stacState = ORIENTATION::DOWN;
        }
    }
    else if ( ( abs( scaledAccX ) > MID_TOL ) && ( abs( scaledAccY ) < HIGH_TOL ) && ( abs( scaledAccZ ) < HIGH_TOL ) ) {
        if ( scaledAccX > 0 ) {                  // Device is oriented Right
            stacState = ORIENTATION::RIGHT;
        }
        else {                                  // scaledAccX < 0: Device is oriented Left
            stacState = ORIENTATION::LEFT;
        }
    }
    else {
        stacState = ORIENTATION::FLAT;
    }  

    return stacState ;
    
}   // end getOrientation()

void rotateGlyphs( ORIENTATION stacOrientation ) {
    //  copies the entire glyph matrix into memory, rotated according to the physical orientation of the STAC
     
    // Initalize the rotation vectors
    uint8_t rotate_0[ MATRIX_LEDS ] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24 };
    uint8_t rotate_90[ MATRIX_LEDS ] = { 20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4 };
    uint8_t rotate_180[ MATRIX_LEDS ] = { 24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 };
    uint8_t rotate_270[ MATRIX_LEDS ] = { 4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20 };
  
    // Initalize the rotation LUT
    uint8_t * rotation_LUT = NULL;
  
    // Determine which rotation to use
    if ( stacOrientation == ORIENTATION::DOWN )
      rotation_LUT = rotate_180;
    else if ( stacOrientation == ORIENTATION::LEFT )
      rotation_LUT = rotate_90;
    else if ( stacOrientation == ORIENTATION::RIGHT )
      rotation_LUT = rotate_270;
    else
      rotation_LUT = rotate_0;

    // Perform the rotation
    for ( uint8_t glyphID = 0 ; glyphID < TOTAL_GLYPHS; glyphID++ ) {
        for ( uint8_t loop_pix = 0; loop_pix < MATRIX_LEDS; loop_pix++ ) {
          glyphMap[glyphID][ loop_pix ] = baseGlyphMap[ glyphID ][ rotation_LUT[ loop_pix ] ];
        }
    }

    return;

}   // end rotateGlyphs()
