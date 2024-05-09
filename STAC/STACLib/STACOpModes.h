
// STACOpModes.h

void changeTallyChannel( Preferences &prefSpace, const char *nameSpace, const char *keyName, stac_ops_t &_stacOps ) {
    /*  allows the user to select the active tally channel.
     *      - any change to the tally channel updates the operating parameter and is saved to NVS
     *      - function will return after a period of inactivity
    */

    unsigned long updateTimeout;
    uint8_t tallyChanNow = _stacOps.tChannel;   // grab the tally channel we had on entry

    if ( _stacOps.tChannel < 9 ) {
        drawGlyph( glyphMap[ _stacOps.tChannel ], tallychangecolor, SHOW );     // display the current tally channel
    }
    else {
        drawGlyph( glyphMap[ _stacOps.tChannel - 8 ], tallychangecolorSDI, SHOW );  // display the current tally channel
    }

    while ( dButt.read() );                     // don't proeeed until the button is released.
    updateTimeout = millis() + OP_MODE_TIMEOUT;

    do {
        if ( millis() >= updateTimeout ) {          /* we timed out so head back to the barn */
             _stacOps.tChannel = tallyChanNow;      // but restore the channel number we had on entry first
            return;
        }
        dButt.read();                               // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT;
            if ( _stacOps.tModel == "V-60HD" ) {
                if ( _stacOps.tChannel >= _stacOps.tChanMax ) {
                    _stacOps.tChannel = 1;          // loop the channel number back to 1 if it would otherwise exceed the max channel #
                }
                else {
                    _stacOps.tChannel++;
                }
                drawGlyph( glyphMap[ _stacOps.tChannel ], tallychangecolor, 1 );
            }
            else {  /* switch model is V-160HD */
                // advance the tally channel and/or wrap it around if needed
                if ( _stacOps.tChannel <  9  && _stacOps.tChannel == _stacOps.tChanHDMIMax ) {
                    _stacOps.tChannel = 9;
                }
                else if ( _stacOps.tChannel > 8 && _stacOps.tChannel == _stacOps.tChanSDIMax + 8 ) {
                    _stacOps.tChannel = 1;
                }
                else {
                    _stacOps.tChannel++;
                }
                if ( _stacOps.tChannel < 9 ) {
                    drawGlyph( glyphMap[ _stacOps.tChannel ], tallychangecolor, 1 );            // display the current V160 HDMI tally channel
                }
                else {
                    drawGlyph( glyphMap[ _stacOps.tChannel - 8 ], tallychangecolorSDI, 1 );     // display the current V160 SDI tally channel
                }
            }
        }
        if ( dButt.pressedFor( SELECT_TIME ) ) {                /* user wants to confirm the change and exit so... */
            drawGlyph( GLF_CK, gtgcolor, SHOW );                //  confirm to the user
            if ( tallyChanNow != _stacOps.tChannel ) {          /* if the active channel changed... */
                prefSpace.begin( nameSpace, PREFS_RW );         //   open up the pref namespace for R/W
                prefSpace.putUChar( keyName, _stacOps.tChannel );    //   save the new tally channel in NVS
                prefSpace.end();                                //   close the prefs namespace
            }
            while ( dButt.read() );     // don't proeeed until the button is released.
            delay( GUI_PAUSE_SHORT );   // "GUI" delay
            return;
        }
    } while ( true );
    return;

}   // end changeTallyChannel()

void changeTallyMode( Preferences &prefSpace, const char *nameSpace, const char *keyName, stac_ops_t &_stacOps, bool disIsSetup = false ) {
    /*  allows the user to flip the STAC display mode
     *   between "camera operator" and "talent"
     *      - function will return after a period of inactivity
     *      - any change to the tally mode is saved to NVS
    */

    unsigned long updateTimeout;
    bool ctModeNow = _stacOps.ctMode;

    if ( !disIsSetup ) {        /* peripheral mode needs this check as it sets the display before calling this function  */
        drawGlyph( ( ctModeNow ? GLF_C : GLF_T ), tallymodecolor, SHOW ); // show the glyph corresponding to the mode
    }

    while ( dButt.read() );                                 // wait until the button is released
    updateTimeout = millis() + OP_MODE_TIMEOUT;             // initialize the timeout timer

    do {
        if ( millis() >= updateTimeout ) {
            return;                                         // we timed out, user didn't confirm the change, head back to the barn
        }

        dButt.read();                                       // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT;     // kick the timeout timer
            ctModeNow = !ctModeNow;                         // btn clicked so we want to flip the current ctMode...
            drawGlyph( ( ctModeNow ? GLF_C : GLF_T ), tallymodecolor, SHOW ); // show the glyph corresponding to the mode
        }
        if ( dButt.pressedFor( SELECT_TIME ) ) {            // user wants to confirm the change and exit so...
            drawGlyph( GLF_CK, gtgcolor, SHOW );
            if ( ctModeNow != _stacOps.ctMode ) {           // if the tally mode changed...
                _stacOps.ctMode = ctModeNow;
                prefSpace.begin( nameSpace, PREFS_RW );     //   open up the pref namespace for R/W
                prefSpace.putBool( keyName, ctModeNow );    //   save the new ctMode in NVS
                prefSpace.end();                            //   close the prefs namespace
            }
            while ( dButt.read() );                         // don't proeeed until the button is released
            delay( GUI_PAUSE_SHORT );
            return;
        }
    } while ( true );
    return;

}   // end changeTallyMode()

void changeStartupMode( Preferences &prefSpace, const char *nameSpace, const char *keyName, stac_ops_t &_stacOps ) {
    /*  Allows the user to flip the STAC startup mode
     *   between "Auto" and "Standard"
     *      - function will return after a period of inactivity
     *      - any change to the startup mode is saved to NVS
    */

    unsigned long updateTimeout;
    bool suModeNow = _stacOps.autoStart;
    if ( suModeNow ) {
        drawGlyph( GLF_A, startchangecolor, SHOW );     // display the current startup mode
    }
    else {
        drawGlyph( GLF_S, startchangecolor, SHOW );
    }
    while ( dButt.read() );                             // don't proeeed until the button is released
    updateTimeout = millis() + OP_MODE_TIMEOUT;         // initialize the timeout timer

    do {
        if ( millis() >= updateTimeout ) {
            return;                                     // we timed out, user didn't confirm the change, head back to the barn
        }

        dButt.read();                           // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT;             // kick the timeout timer
            suModeNow = !suModeNow;                                 // btn clicked so we want to flip the current startup mode...
            drawGlyph( ( suModeNow ? GLF_A : GLF_S ), startchangecolor, SHOW ); // show the glyph corresponding to the mode
        }
        if ( dButt.pressedFor( SELECT_TIME ) ) {        // user wants to confirm the change and exit so...
            drawGlyph( GLF_CK, gtgcolor, SHOW );
            if ( suModeNow != _stacOps.autoStart ) {                 /* if the startup mode changed... */
                _stacOps.autoStart = suModeNow;
                prefSpace.begin( nameSpace, PREFS_RW );     //   open up the pref namespace for R/W
                prefSpace.putBool( keyName, suModeNow );    //   save the new autoStart mode in NVS
                prefSpace.end();                            //   close the prefs namespace
            }
            while ( dButt.read() );                 // don't proeeed until the button is released
            delay( GUI_PAUSE_SHORT );
            return;
        }
    } while ( true );

}   // end changeStartupMode()

uint8_t updateBrightness( Preferences &prefSpace, const char *nameSpace, const char *keyName, uint8_t brLevel, bool noSetup = false ) {
    /*  allows the user to select the brightness of the display.
     *      - function will return after a period of inactivity
     *      - any change to the brightness level is saved to NVS
     *      - "brLevel" is an index into the brightmap[] array, not an absolute brightness value
     *      - uses the brightness level to index into the glyphMap[] to fetch the bitmap to display the digit for that level
     *      - call with noSetup = true if the display is already set up with the current brightness level before calling this function
    */

    unsigned long updateTimeout;
    uint8_t brLevelNow = brLevel;       // "brightness" is a mapping table index, not an absolute display brightness value

    if ( !noSetup ) {
        disFillPix( RGB_COLOR_WHITE, NO_SHOW );                     // fill the display...
        drawOverlay( GLF_EN, RGB_COLOR_BLACK, NO_SHOW );            //  blank out the inside three columns...
        drawOverlay( glyphMap[ brLevel ], RGB_COLOR_ORANGE, SHOW ); //  and overlay the brightness level number
    }

    while ( dButt.read() );                     // don't proeeed until the button is released.
    updateTimeout = millis() + OP_MODE_TIMEOUT;

    do {
        if ( millis() >= updateTimeout ) {                  /* we timed out, user didn't confirm the change */
            brLevel = brLevelNow;                           //  restore disLevel as it was on entry...
            disSetBright( brightMap[ brLevelNow ], SHOW );  //  reset the display to that brightness...
            dButt.read();
            return brLevelNow;                              //  and head back to the barn
        }

        dButt.read();
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + OP_MODE_TIMEOUT;
            if ( brLevel >= brightLevels ) {
                brLevel = 1;
            }
            else {
                brLevel++;
            }
            drawOverlay( GLF_EN, RGB_COLOR_BLACK, NO_SHOW );                // blank out the inside three columns...
            drawOverlay( glyphMap[ brLevel ], RGB_COLOR_ORANGE, NO_SHOW );  // and overlay the new brightness setting number
            disSetBright( brightMap[ brLevel ], SHOW );                     // set the display to the new brightness setting
        }

        if ( dButt.pressedFor( SELECT_TIME ) ) {            /* user wants to confirm the change and exit so... */
            drawGlyph( GLF_CK, gtgcolor, SHOW );            //   let the user know we're good to go
            if ( brLevelNow != brLevel ) {                  /* if the brightness level changed... */
                prefSpace.begin( nameSpace, PREFS_RW );     //   open up the pref namespace for R/W
                prefSpace.putUChar( keyName, brLevel );     //   save the new brightness in NVS
                prefSpace.end();                            //   close the prefs namespace
            }
            while ( dButt.read() );                         // don't proeeed until the button is released.
            delay( GUI_PAUSE_SHORT );                                   // "GUI" pause
            return brLevel;
        }
    } while ( true );

}    // end updateBrightness()


//  --- EOF ---
