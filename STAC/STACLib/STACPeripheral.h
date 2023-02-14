// =================== START CODE FOR PERIPHERAL MODE OERATION ===================

    bool pmBitOut = 0;                      // we'll set the PM_CK_OUT pin to this value
    bool pmBitIn = 0;                       // we'll read the PM_CK_IN pin into this
    bool inPMode = false;                   // true if we decide we're in Peripheral Mode, false otherwise.
    unsigned long nextCheck = 0;            // compare system ms against this value to decide if we should go look for a tally status change
    unsigned long pmNextStateTime = 0UL;    // button down for this ms longer = set up for next state
    bool pmPrefsOK = false;                 // true if the peripheral mode preferences layout matches the version for this software version

    pinMode( PM_CK_OUT, OUTPUT );           // we toggle this pin to see if PM_CK_IN follows
    pinMode( PM_CK_IN, INPUT_PULLDOWN );    // we read this pin to see if it follows PM_CK_OUT

    // If the Peripheral Mode jumper wire is in place, the PM_CK_IN pin should
    //  follow the state of the PM_CK_OUT pin as we toggle it a few times.

    for ( int i = 1; i <= PM_CK_CNT; i++ ) {
        pmBitOut = !pmBitOut;
        digitalWrite( PM_CK_OUT, pmBitOut );
        delay( i * 13 );                        // "random" delay
        pmBitIn = digitalRead( PM_CK_IN );
        if ( pmBitIn != pmBitOut ) {
            digitalWrite( PM_CK_OUT, LOW );
            break;                              // input failed to match output so, assume no jumper & get outta Dodge.
        }
        if ( i == PM_CK_CNT ) {
            digitalWrite( PM_CK_OUT, LOW );
            inPMode = true;                     // all "ins" matched all "outs", assume jumper is in place, so let's go in to Peripheral Mode
        }
    }

    if ( inPMode ) {
        // ~~~~~~~~~~~~~~ Start all the initialization stuff for Peripheral Mode ~~~~~~~~~~~~~~        
        Preferences pmPrefs;                // need a Preferences object to read, write & retain the Peripheral Mode display brightness level.
        uint8_t pmBright = 1;               // holds the current brightness level of the display
        bool pmCTmode = false;              // display mode: false = talent mode, true = camera operator mode
        uint8_t tsNow = 0;                  // holds the tally state after doing a read of the GROVE pins
        uint8_t tsLast = 0xff;              // holds tally state of the previous read of the GROVE pins (init to an impossible value to force tally refresh)
        pinMode( TS_0, INPUT_PULLDOWN );    // set the ATOM GROVE GPIO pins to inputs with the internal pulldown active 
        pinMode( TS_1, INPUT_PULLDOWN );

        // Time to go figure out if we've ever operated in Peripheral Mode before & do the first time set up in NVS if needed
        pmPrefs.begin( "PModePrefs", PREFS_RO );        // open or create the Pref namespace
        if ( pmPrefs.isKey( "pmPrefVer" ) ) {           // no key = first time ever in Peripheral Mode
            if ( pmPrefs.getUShort( "pmPrefVer" ) == PM_PREFS_VERSION ) pmPrefsOK = true;
        }
        if ( !pmPrefsOK ) {
            pmPrefs.end();                              // close the namespace & reopen it in RW mode & 
            pmPrefs.begin( "PModePrefs", PREFS_RW );    //  create the keys & save the factory defaults 
            pmPrefs.putUShort( "pmPrefVer", PM_PREFS_VERSION );
            pmPrefs.putBool( "pmct", false );
            pmPrefs.putUChar( "pmbrightness", 1 );
            pmBright = 1;                               // set the default brightness run time level
            pmCTmode = false;                           // set the default camera/talent display mode
        }
        else {
           pmBright = pmPrefs.getUChar( "pmbrightness" );   // get the brightness run time level
           pmCTmode = pmPrefs.getBool( "pmct" );            // get the tally mode
        }
        pmPrefs.end();      // close the namespace
        // Done setting the Peripheral Mode run parameters from NVS

        disSetBright( brightMap[ pmBright ] );              // set the display brightness

        // finish the startup data dump to the serial monitor
        Serial.println( "     OPERATING IN PERIPHERAL MODE" );        // finish the serial port information dump 
        Serial.print( "    Tally Mode: ");
            if ( pmCTmode ) Serial.println( "Camera Operator" );
            else Serial.println( "Talent" );
        Serial.print( "    Brightness Level: " );
        Serial.println( pmBright );
        Serial.println( "=======================================" );

        // confirm to the user that we're in Peripheral Mode
        drawGlyph( GLF_P, perifmodecolor, 1 );      // draw the "in Peripheral Mode" thingy           
        flashDisplay( 4, 500, brightMap[ pmBright ] );  // flashy thingy
        delay( GUI_PAUSE_TIME );
        drawGlyph( GLF_CK, perifmodecolor, 1 );     // draw the "in Peripheral Mode" confirmation
        delay( GUI_PAUSE_TIME );
        disClear();
        // confirmation done
        
        disDrawPix( PO_PIXEL, PO_COLOR, 1 );    // turn on the power LED
        while ( dButt.read() );                 // wait for the button to be released
        nextCheck = millis();                   // force a recheck of the tally state

        // ~~~~~~~~~~~~~~ Finished all the initialization stuff for Peripheral Mode ~~~~~~~~~~~~~~

        do {                                    // We don't leave setup() if we're in Peripheral Mode
            if ( millis() >= nextCheck ) {
                nextCheck = millis() + PM_POLL_INT;
                tsNow = ( ( digitalRead( TS_1 ) ? 2 : 0 ) + ( digitalRead( TS_0 ) ? 1 : 0 ) );              
                if ( tsNow != tsLast ) {
                    tsLast = tsNow;
                    if ( tsNow != 0 ) {
                        if ( tsNow == 3 ) disFillPix( PGM, 0 );                     // set the display to PGM
                        else if ( tsNow == 2 ) disFillPix( PVW, 0 );                // set the display to PVW
                        else {
                            if ( pmCTmode ) drawGlyph( GLF_DF, unselectedcolor, 0 );    //  set the display to the unselected glyph
                            else disFillPix( PVW, 0 );
                        }
                        disDrawPix( PO_PIXEL, PO_COLOR, 1);                         // turn on the power LED
                    }
                    else {
                        if ( pmCTmode ) drawGlyph( GLF_BX, warningcolor, 1 );
                        else {
                             disFillPix( PVW, 0 );
                             disDrawPix( PO_PIXEL, PO_COLOR, 1);                    // turn on the power LED
                        }
                    }
                }
            }
            dButt.read();
            if ( dButt.pressedFor( SELECT_TIME ) ) {
                pmNextStateTime = millis() + SELECT_TIME;
                disFillPix( GRB_COLOR_WHITE, 0 );                           // fill the display...
                drawOverlay( GLF_EN, GRB_COLOR_BLACK, 0 );                  // blank out the inside three columns...
                drawOverlay( glyphMap[ pmBright ], GRB_COLOR_ORANGE, 1 ); // and overlay the brightness level number
                while ( true ) {
                    dButt.read();
                    if ( dButt.isPressed() && millis() >= pmNextStateTime ) {
                        pmCTmode = changeTallyMode( pmPrefs, "PModePrefs", "pmct", pmCTmode, 0 );
                        break;
                    }
                    if ( dButt.isReleased() && millis() < pmNextStateTime ) {
                        pmBright = updateBrightness( pmPrefs, "PModePrefs", "pmbrightness", pmBright, 1 );
                        break;   
                    }
                }    
                disClear();
                disDrawPix( PO_PIXEL, PO_COLOR, 1 );    // turn on the power LED
                tsLast = 0xff;                          // set the last tally state to an impossible value to force a refresh
                nextCheck = millis();                   // ... and set the check time to now
            }
        } while ( true ); // end do loop

    }    // end "if (inPMode)"

// =================== END CODE FOR PERIPHERAL MODE OERATION ===================
