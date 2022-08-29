
ORIENTATION getOrientation( ) {
    /* Code to retrieve the current orientation of the STAC.
        - uses the IMU built into the ATOM to return one of 
          four possible orientations, UP, DOWN, LEFT, RIGHT
     */
     
    ORIENTATION stacState = ORIENTATION::UP;
  
    float accX=0, accY=0, accZ = 0 ;      
  
    imu.getAccel(&accX, &accY, &accZ);   // Call into the accelerometer to get the current values 
    
    float scaledAccX = accX * 1000;
    float scaledAccY = accY * 1000;
    float scaledAccZ = accZ * 1000;

    if( ( abs(scaledAccX) < HIGH_TOL ) && ( abs(scaledAccY) > MID_TOL ) && ( abs(scaledAccZ) < HIGH_TOL ) )
    {
        if ( scaledAccY > 0 )                   // Device is oriented Up            
        {
            stacState = ORIENTATION::UP;
            log_i( "Scaled Y > 0,  Device is oriented Up" ) ;
        }
        else // scaledAccY < 0                  // Device is oriented Down
        {
            stacState = ORIENTATION::DOWN;
            log_i( "Scaled Y < 0,  Device is oriented Down" ) ;
        }
    }
    else if( ( abs(scaledAccX) > MID_TOL ) && ( abs(scaledAccY) < HIGH_TOL ) && ( abs(scaledAccZ) < HIGH_TOL ) )
    {
        if( scaledAccX > 0 )                    // Device is oriented Right
        {
            stacState = ORIENTATION::RIGHT;
            log_i( "Scaled X > 0,  Device is oriented Right" ) ;
        }
        else // scaledAccX < 0                  // Device is oriented Left
        {
            stacState = ORIENTATION::LEFT;
            log_i( "Scaled X < 0,  Device is oriented Left" ) ;
        }
    }
    else
    {
        log_i( "Orientation is flat: setting orientation Up." );
    }  

    return stacState ;
    
}   // closing brace for getOrientation()

void rotateGlyphs( ORIENTATION stacOrientation ) {
    /*  Rotates the entire glyph matrix in memory depending upon the vertical orientation of the STAC
    */
     
    // Initalize the rotation vectors
    int rotate_0[25] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
    int rotate_90[25] = {20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4};
    int rotate_180[25] = {24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    int rotate_270[25] = {4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20};
  
    // Initalize the rotation LUT
    int * rotation_LUT = NULL;
  
    // Determine which rotation to use
    if ( stacOrientation == ORIENTATION::DOWN )                 // Upside Down
      rotation_LUT = rotate_180;
    else if ( stacOrientation == ORIENTATION::LEFT )            // Rotated to left 
      rotation_LUT = rotate_90;
    else if ( stacOrientation == ORIENTATION::RIGHT )           // Rotated to Right
      rotation_LUT = rotate_270;
    else
      rotation_LUT = rotate_0; 

    // Perform the rotation
    for ( int glyphID = 0 ; glyphID < TOTAL_GLYPHS; glyphID++ )
    {
        for ( int loop_pix = 0; loop_pix < 25; loop_pix++ )
        {
          glyphMap[glyphID][ loop_pix ] = baseGlyphMap[glyphID][ rotation_LUT[ loop_pix ] ] ;
        }
    }

    return;

}   // closing brace for rotateGlyphs()
