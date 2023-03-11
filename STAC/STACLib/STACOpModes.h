void changeTallyChannel() {
/*  Allows the user to select the active tally channel. 
 *      - function will return after a period of inactivity, restoring
 *        the active channel to what it was prior to the call
*/

    unsigned long updateTimeout;
    uint8_t tallyChanNow = tallyStatus.tChannel;     // keep the tally channel state we had on entry

    drawGlyph( glyphMap[tallyStatus.tChannel], tallychangecolor, 1 );    // display the current tally channel
   
    while ( dButt.read() );                     // don't proeeed until the button is released.
    updateTimeout = millis() + OP_MODE_TIMEOUT; // delete this line if you don't want to leave after a period of inactivity
     
    do {
        if ( millis() >= updateTimeout ) {          // remove this 'if' clause if you don't want to leave after a period of inactivity
            tallyStatus.tChannel = tallyChanNow;    // we timed out, user didn't confirm the change, so restore the tally channel as it was on entry...
            return;                                 //   and head back to the barn
        }
        dButt.read();                              // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT; // kick the timeout timer (delete this line if you don't want to leave after a period of inactivity)
            if ( tallyStatus.tChannel >= tallyStatus.tChanMax ) {
                tallyStatus.tChannel = 1;           // loop the channel number back to 1 if it would otherwise exceed the max channel #
            }
            else tallyStatus.tChannel++;
            drawGlyph( glyphMap[ tallyStatus.tChannel ], tallychangecolor, 1 );
        }
        if ( dButt.pressedFor( SELECT_TIME ) ) {                        // user wants to confirm the change and exit so...
            drawGlyph( GLF_CK, gtgcolor, 1 );
            if ( tallyChanNow != tallyStatus.tChannel ) {               // if the active channel changed...
                stcPrefs.begin( "STCPrefs", PREFS_RW );                 //   open up the pref namespace for R/W
                stcPrefs.putUChar( "talChan", tallyStatus.tChannel );   //   save the new tally channel in NVS
                stcPrefs.end();                                         //   close the prefs namespace
            }    
            while ( dButt.read() );             // don't proeeed until the button is released.
            delay( 500 );
            return;
        }
    } while ( true );
    return;

}   // end changeTallyChannel()

void changeTallyMode() {
/*  Allows the user to flip the STAC display mode
 *  between "camera operator" and "talent"
 *      - function will return after a period of inactivity, restoring
 *        the operating mode to what it was prior to the call
*/
    unsigned long updateTimeout;
    bool ctModeNow = ctMode;

    if ( ctMode ) drawGlyph( GLF_C, tallymodecolor, 1 );    // display the current tally mode...
    else drawGlyph( GLF_T, tallymodecolor, 1 );
 
    while ( dButt.read() );                                 // wait until the button is released
    updateTimeout = millis() + OP_MODE_TIMEOUT;             // initialize the timeout timer 
     
    do {
        if ( millis() >= updateTimeout ) {
            ctMode = ctModeNow;                     // we timed out, user didn't confirm the change, restore the ctMode as it was on entry...
            return;                                 // and head back to the barn
        }

        dButt.read();                                       // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT;     // kick the timeout timer
            ctMode = !ctMode;                               // btn clicked so we want to flip the current ctMode...
            if ( ctMode ) drawGlyph( GLF_C, tallymodecolor, 1 );    //   and display the new operating mode
            else drawGlyph( GLF_T, tallymodecolor, 1 );
        }
        if ( dButt.pressedFor( SELECT_TIME ) ) {             // user wants to confirm the change and exit so...
            drawGlyph( GLF_CK, gtgcolor, 1 );
            if ( ctModeNow != ctMode ) {                      // if the tally mode changed...
                stcPrefs.begin( "STCPrefs", PREFS_RW );     //   open up the pref namespace for R/W
                stcPrefs.putBool( "ctMde", ctMode );        //   save the new ctMode in NVS
                stcPrefs.end();                             //   close the prefs namespace
            }
            while ( dButt.read() );                         // don't proeeed until the button is released
            delay( 500 );
            return;
        }
    } while ( true );
    return;

}   // end changeTallyMode()

bool changeTallyMode( Preferences& prefSpace, const char * nameSpace, const char * keyName, bool ctMode, bool disIsSetup = false ) {
/*  Allows the user to flip the STAC tally mode
 *  between "camera operator" and "talent"
 *      - function will return after a period of inactivity, restoring
 *        the tally mode to what it was prior to the call
 *      - overloaded version of changeTallyMode() used when operating in Peripheral Mode
*/
    unsigned long updateTimeout;
    bool ctModeNow = ctMode;

    if ( !disIsSetup ) {
        if ( ctMode ) drawGlyph( GLF_C, tallymodecolor, 1 );    // display the current tally mode...
        else drawGlyph( GLF_T, tallymodecolor, 1 );
    }
    while ( dButt.read() );                                 // wait until the button is released
    updateTimeout = millis() + OP_MODE_TIMEOUT;             // initialize the timeout timer 
     
    do {
        if ( millis() >= updateTimeout ) {
            ctMode = ctModeNow;                     // we timed out, user didn't confirm the change, restore the ctMode as it was on entry...
            return ctMode;                          // and head back to the barn
        }

        dButt.read();                                       // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT;     // kick the timeout timer
            ctMode = !ctMode;                               // btn clicked so we want to flip the current ctMode...
            if ( ctMode ) {
                drawGlyph( GLF_C, tallymodecolor, 1 );      //   and display the new operating mode
            }
            else drawGlyph( GLF_T, tallymodecolor, 1 );
        }
        if ( dButt.pressedFor( SELECT_TIME ) ) {        // user wants to confirm the change and exit so...
            drawGlyph( GLF_CK, gtgcolor, 1 );
            if ( ctModeNow != ctMode ) {                  // if the tally mode changed...
                prefSpace.begin( nameSpace, PREFS_RW ); //   open up the pref namespace for R/W
                prefSpace.putBool( keyName, ctMode );   //   save the new ctMode in NVS
                prefSpace.end();                        //   close the prefs namespace
            }
            while ( dButt.read() );                     // don't proeeed until the button is released
            delay( 500 );
            return ctMode;
        }
    } while (true);

}   // end changeTallyMode()

void changeStartupMode() {
/*  Allows the user to flip the STAC startup mode
 *  between "Auto" and "Standard"
 *      - function will return after a period of inactivity, restoring
 *        the startup mode to what it was prior to the call
*/
    unsigned long updateTimeout;
    bool suModeNow = autoStart;
    if ( autoStart ) {
        drawGlyph( GLF_A, startchangecolor, 1 );   // display the current startup mode
    }
    else drawGlyph( GLF_S, startchangecolor, 1 );

    while ( dButt.read() );                         // don't proeeed until the button is released
    updateTimeout = millis() + OP_MODE_TIMEOUT;     // initialize the timeout timer 
     
    do {
        if ( millis() >= updateTimeout ) {
            autoStart = suModeNow;                  // we timed out, user didn't confirm the change, restore the startup mode as it was on entry...
            return;                                 // and head back to the barn
        }

        dButt.read();                           // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT;             // kick the timeout timer
            autoStart = !autoStart;                                 // btn clicked so we want to flip the current startup mode...
            if ( autoStart ) {
                drawGlyph( GLF_A, startchangecolor, 1 );            // and display the new operating mode
            }
            else drawGlyph( GLF_S, startchangecolor, 1 );
        }
        if ( dButt.pressedFor( SELECT_TIME ) ) {        // user wants to confirm the change and exit so...
            drawGlyph( GLF_CK, gtgcolor, 1 );
            if ( suModeNow != autoStart ) {                 // if the startup mode changed...
                stcPrefs.begin( "STCPrefs", PREFS_RW );     //   open up the pref namespace for R/W              
                stcPrefs.putBool( "aStart", autoStart );    //   save the new autoStart mode in NVS
                stcPrefs.end();                             //   close the prefs namespace
            }
            while ( dButt.read() );                 // don't proeeed until the button is released
            delay( 500 );
            return;
        }
    } while ( true );

}   // end changeStartupMode()

uint8_t updateBrightness( Preferences& prefSpace, const char * nameSpace, const char * keyName, uint8_t brightness, bool noSetup = false ) {
/*  Allows the user to select the brightness of the display. 
 *      - function will return after a period of inactivity, restoring
 *         the brightness to what it was prior to the call
 *      - any change to the brightness level is saved to NVS using 
 *         the "prefsSpace" object in the namespace "namespace" under the key "keyname"
 *      - "uint8_t brightness" is a level (index into the brightmap[] array), not an absolute brightness value
 *      - uses the brightness level to index into the glyphMap[] to fetch the bitmap to display the digit for that level
 *      - call with noSet = true if the display is already set up with the current brightness level before calling this function
*/
    unsigned long updateTimeout;                    // delete this line if you don't want to bug out after a period of inactivity
    uint8_t brightnessNow = brightness;             // "brightness" is a level, not an absolute display brightness value

    if ( !noSetup ) {
        disFillPix( GRB_COLOR_WHITE, 0 );                           // fill the display...
        drawOverlay( GLF_EN, GRB_COLOR_BLACK, 0 );                  // blank out the inside three columns...
        drawOverlay( glyphMap[ brightness ], GRB_COLOR_ORANGE, 1 ); // and overlay the brightness level number
    }
        
    while ( dButt.read() );                     // don't proeeed until the button is released.
    updateTimeout = millis() + OP_MODE_TIMEOUT; // delete this line if you don't want to bug out after a period of inactivity

    do {
        if ( millis() >= updateTimeout ) {      // delete this if clause if you don't want to bug out after a period of inactivity
            brightness = brightnessNow;         // we timed out, user didn't confirm the change, restore currentBrightness as it was on entry...
            disSetBright( brightMap[ brightness ] );    // reset the display to that brightness...
            dButt.read();
            return brightness;                  // and head back to the barn
        }

        dButt.read();
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT;     // delete this line if you don't want to bug out after a period of inactivity
            if ( brightness >= brightLevels ) brightness = 1;
            else brightness++;                    
            drawOverlay( GLF_EN, GRB_COLOR_BLACK, 0 );                      // blank out the inside three columns...
            drawOverlay( glyphMap[ brightness ], GRB_COLOR_ORANGE, 0 );     // and overlay the new brightness setting number
            disSetBright( brightMap[ brightness ] );                        // set the display to the new brightness setting
        }

        if ( dButt.pressedFor( SELECT_TIME ) ) {                // user wants to confirm the change and exit so...
            drawGlyph( GLF_CK, gtgcolor, 1 );                   //   let the user know we're good to go
            if ( brightnessNow != brightness ) {                // if the brightness level changed...
                prefSpace.begin( nameSpace, PREFS_RW );         //   open up the pref namespace for R/W
                prefSpace.putUChar( keyName, brightness );      //   save the new brightness in NVS
                prefSpace.end();                                //   close the prefs namespace
            }
            while ( dButt.read() );                             // don't proeeed until the button is released.
            delay( 500 );
            return brightness;
        }
    } while ( true );

}    // end updateBrightness()
