
// STACPeripheral.h

bool STACpmCheck() {
    /*  - checks to see if the STAC Peripheral Mode jumper is in place
     *  - returns true if it is, false otherwise
    */

    bool pmBitOut = 0;                      // we'll set the PM_CK_OUT pin to this value
    bool pmBitIn = 0;                       // we'll read the PM_CK_IN pin into this
    bool inPMode = false;                   // true if we detect that the Peripheral Mode jumper is in place, false otherwise.

    pinMode( PM_CK_OUT, OUTPUT );           // we toggle this pin to see if PM_CK_IN follows
    pinMode( PM_CK_IN, INPUT_PULLDOWN );    // we read this pin to see if it follows PM_CK_OUT

    //  If the Peripheral Mode jumper wire is in place, the PM_CK_IN pin should
    //      follow the state of the PM_CK_OUT pin as we toggle it a few times.
    for ( int i = 1; i <= PM_CK_CNT; i++ ) {
        pmBitOut = !pmBitOut;
        digitalWrite( PM_CK_OUT, pmBitOut );
        delay( i * 7 );                         // "random" delay
        pmBitIn = digitalRead( PM_CK_IN );
        if ( pmBitIn != pmBitOut ) {
            digitalWrite( PM_CK_OUT, LOW );
            break;                              // input failed to match output so, assume no jumper & get outta Dodge.
        }
        if ( i == PM_CK_CNT ) {
            digitalWrite( PM_CK_OUT, LOW );
            inPMode = true;                     // all "ins" matched all "outs", assume jumper is in place & head back to the barn
        }
    }
    pinMode( PM_CK_OUT, INPUT );    // a bit of cleanup
    pinMode( PM_CK_IN, INPUT );
    return inPMode;

}   // end STACpmCheck

void STACPeripheral() {
    /*  - function to operate the STAC in Peripheral Mode
     *  - peripheral mode has its own NVS space independant of that for normal operating mode
     *  - when operating in peripheral mode, this function never returns
    */
    // ~~~~~~~~~~~~~~ Start all the initialization stuff for Peripheral Mode ~~~~~~~~~~~~~~
    Preferences pmPrefs;                // need a Preferences object to read, write & retain the Peripheral Mode display brightness level.

    stac_ops_t pmOps = { "NO_MODEL", 0, 0, "NO_BANK", 0, 0, false, true, 1, PM_POLL_INT };  // holds pm operating parameters

    bool pmPrefsOK = false;             // true if the peripheral mode preferences layout matches the version for this software version
    uint8_t tsNow = 0xaa;               // holds the tally state after doing a read of the GROVE pins
    uint8_t tsLast = 0x55;              // holds tally state of the previous read of the GROVE pins (init to an impossible value to force tally refresh)
    unsigned long nextCheck = 0UL;          // compare system ms against this value to decide if we should go look for a tally status change
    unsigned long pmNextStateTime = 0UL;    // button down for this ms longer = set up for next state
    bool brkFlag = false;                   // to get us out of the pm ops mode change loop
    pinMode( TS_0, INPUT_PULLDOWN );        // set the ATOM GROVE GPIO pins to inputs with the internal pulldown active
    pinMode( TS_1, INPUT_PULLDOWN );

    // time to go figure out if we've ever operated in Peripheral Mode before & do the first time set up in NVS if needed
    pmPrefs.begin( "PModePrefs", PREFS_RO );        // open or create the Pref namespace
    if ( pmPrefs.isKey( "pmPrefVer" ) ) {           // no key = first time ever in Peripheral Mode
        if ( pmPrefs.getUShort( "pmPrefVer" ) == PM_PREFS_VERSION )
            pmPrefsOK = true;
    }
    if ( !pmPrefsOK ) {                             /* first time in PM so set up the NVS namespace & set the factory defaults */
        pmPrefs.end();                              // close the namespace & reopen it in RW mode &
        pmPrefs.begin( "PModePrefs", PREFS_RW );    //  create the keys & save the factory defaults
        pmPrefs.putUShort( "pmPrefVer", PM_PREFS_VERSION );
        pmPrefs.putBool( "pmct", false );
        pmPrefs.putUChar( "pmbrightness", 1 );
        pmOps.disLevel = 1;                         // set the default brightness level
        pmOps.ctMode = false;                       // set the default camera/talent display mode
    }
    else {
       pmOps.disLevel = pmPrefs.getUChar( "pmbrightness" );     // get the brightness run time level
       pmOps.ctMode = pmPrefs.getBool( "pmct" );                // get the tally mode
    }
    pmPrefs.end();                                  // close the namespace
    // done setting the Peripheral Mode operating parameters from NVS

    disSetBright( brightMap[ pmOps.disLevel ] );                // set the display brightness
    sendPeripheral( pmOps.ctMode, pmOps.disLevel );             // add to the serial port info dump

    // confirm to the user that we're in Peripheral Mode
    drawGlyph( GLF_P, perifmodecolor, SHOW );                   // draw the "in Peripheral Mode" thingy
    flashDisplay( 4, 500, brightMap[ pmOps.disLevel ] );        // flashy thingy
    delay( GUI_PAUSE_TIME );
    drawGlyph( GLF_CK, perifmodecolor, SHOW );                  // draw the "in Peripheral Mode" confirmation
    delay( GUI_PAUSE_TIME );
    // confirmation done

    disClear( NO_SHOW );
    disDrawPix( PO_PIXEL, PO_COLOR, SHOW );     // turn on the power LED
    while ( dButt.read() );                     // wait for the button to be released
    nextCheck = millis();                       // force a recheck of the tally state

    // ~~~~~~~~~~~~~~ Finished all the initialization stuff for Peripheral Mode ~~~~~~~~~~~~~~

    while ( true ) {    /* we don't leave this loop if we're in Peripheral Mode */
        if ( millis() >= nextCheck ) {
            nextCheck = millis() + PM_POLL_INT;
            tsNow = (uint8_t)( digitalRead( TS_1 ) ? 2 : 0 ) + (uint8_t)( digitalRead( TS_0 ) ? 1 : 0 );
            if ( tsNow != tsLast ) {
                tsLast = tsNow;
                switch ( tsNow ) {
                    case 3:
                        disFillPix( PGM, NO_SHOW );
                        disDrawPix( PO_PIXEL, PO_COLOR, SHOW);
                        break;
                    case 2:
                        disFillPix( PVW, NO_SHOW );
                        disDrawPix( PO_PIXEL, PO_COLOR, SHOW);
                        break;
                    case 1:
                        if ( pmOps.ctMode ) {
                            drawGlyph( GLF_DF, unselectedcolor, NO_SHOW );
                        }
                        else {
                            disFillPix( PVW, NO_SHOW );
                        }
                        disDrawPix( PO_PIXEL, PO_COLOR, SHOW);
                        break;
                    default:
                        if ( pmOps.ctMode ) {
                            drawGlyph( GLF_BX, warningcolor, SHOW );
                        }
                        else {
                            disFillPix( PVW, NO_SHOW );
                            disDrawPix( PO_PIXEL, PO_COLOR, SHOW);
                        }
                }

            }
        }

        dButt.read();
        if ( dButt.pressedFor( SELECT_TIME ) ) {                            /* user wants to change a pm op mode */
            pmNextStateTime = millis() + SELECT_TIME;
            disFillPix( RGB_COLOR_WHITE, NO_SHOW );                             // fill the display...
            drawOverlay( GLF_EN, RGB_COLOR_BLACK, NO_SHOW );                    //   blank out the inside three columns...
            drawOverlay( glyphMap[ pmOps.disLevel ], RGB_COLOR_ORANGE, SHOW );  //   and overlay the brightness level number

            brkFlag = false;
            pmNextStateTime = millis() + SELECT_TIME;
            do {
                dButt.read();
                if ( dButt.isReleased() && ( pmNextStateTime >= millis() ) ) {
                    pmOps.disLevel = updateBrightness( pmPrefs, "PModePrefs", "pmbrightness", pmOps.disLevel, true );
                    brkFlag = true;
                }
                else if ( dButt.isPressed() && ( pmNextStateTime < millis() ) ) {
                    changeTallyMode( pmPrefs, "PModePrefs", "pmct", pmOps, false );
                    brkFlag = true;
                }
            } while ( !brkFlag );

            disClear( NO_SHOW );
            disDrawPix( PO_PIXEL, PO_COLOR, SHOW ); // turn on the power LED
            tsLast = 0xff;                          // set the last tally state to an impossible value to force a refresh
            nextCheck = millis();                   // ... and set the tally state check time to now
        }
    }

}   // end STACPeripheral()


//  --- EOF ---
