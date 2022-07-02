// =================== START CODE FOR PERIPHERAL MODE OERATION ===================

    bool pmBitOut = 0;                      // we'll set the PM_CK_OUT pin to this value
    bool pmBitIn = 0;                       // we'll read the PM_CK_IN pin into this
    bool inPMode = false;                   // true if we decide we're in Peripheral Mode, false otherwise.
    unsigned long nextCheck;                // compare system ms against this value to decide if we should go look for a tally status change

    pinMode(PM_CK_OUT, OUTPUT);             // we toggle this pin to see if PM_CK_IN follows
    pinMode(PM_CK_IN, INPUT_PULLDOWN);      // we read this pin to see if it follows PM_CK_OUT

    // If the Peripheral Mode jumper wire is in place, the PM_CK_IN pin should
    //  follow the state of the PM_CK_OUT pin as we toggle it a few times.

    for ( int i = 1; i <= PM_CK_CNT; i++ ) {
        pmBitOut = !pmBitOut;
        digitalWrite(PM_CK_OUT, pmBitOut);
        delay(i * 13);                      // "random" delay
        pmBitIn = digitalRead(PM_CK_IN);
        if (pmBitIn != pmBitOut) {
            digitalWrite(PM_CK_OUT, LOW);
            break;                          // input failed to match output so, assume no jumper & get outta Dodge.
        }
        if (i == PM_CK_CNT) {
            digitalWrite(PM_CK_OUT, LOW);
            inPMode = true;                 // all "ins" matched all "outs", assume jumper is in place, so let's go in to Peripheral Mode
        }
    }

    if (inPMode) {
        // ~~~~~~~~~~~~~~ Start all the initialization stuff for Peripheral Mode ~~~~~~~~~~~~~~        
        Preferences pmPrefs;                // need a Preferences object to read, write & retain the Peripheral Mode display brightness level.
        uint8_t pmBright;                   // holds the current brightness level of the display
        uint8_t tsNow = 0;                  // holds the tally state after doing a read of the GROVE pins
        uint8_t tsLast = 0xff;              // holds tally state of the previous read of the GROVE pins (init to an impossible value to force tally refresh)
        pinMode(TS_0, INPUT_PULLDOWN);      // set the ATOM GROVE GPIO pins to inputs with the internal pulldown active 
        pinMode(TS_1, INPUT_PULLDOWN);

        // Time to go figure out if we've ever operated in Peripheral Mode before & do the first time set up in NVS if needed
        pmPrefs.begin("PModePrefs", PREFS_RO);              // open or create the Pref namespace
        if ( !pmPrefs.isKey("pmbrightness") ) {             // no key = first time ever in Peripheral Mode
            pmPrefs.end();                                  // close the namespace & 
            pmPrefs.begin("PModePrefs", PREFS_RW);          //  reopen it in RW mode
            pmPrefs.putUChar("pmbrightness", 10);           // create the key & save the factory default 
            pmBright = 10;                                  // set the default brightness run time value
        }
        else {
           pmBright = pmPrefs.getUChar("pmbrightness");     // get the brightness run time value
        }
        pmPrefs.end();      // close the namespace
        // Done setting the Peripheral Mode run parameters from NVS

        disSetBright(pmBright);                 // set the display brightness

        // confirm to the user that we're in Peripheral Mode
        drawGlyph(GLF_P, perifmodecolor);               // draw the "in Peripheral Mode" thingy           
        flashDisplay(4, 500, pmBright);                 // flashy thingy
        delay(1000);
        disClear();
        drawGlyph(GLF_CK, perifmodecolor);              // draw the "in Peripheral Mode" confirmation
        delay(1000);
        disClear();
        // confirmation done
        
        disDrawPix(PO_PIXEL, PO_COLOR);             // turn on the power LED
        while ( dButt.read() );                  // wait for the button to be released
        disFillPix(PVW);                            // set the display to PVW
        disDrawPix(PO_PIXEL, PO_COLOR);             // turn on the power LED

        // finish the startup data dump to the serial monitor
        Serial.println("     OPERATING IN PERIPHERAL MODE");          // finish the serial port information dump 
        Serial.print("          Brightness Level: ");
        Serial.println( pmBright / 10 );
        Serial.println("=======================================");
  
        // ~~~~~~~~~~~~~~ Finished all the initialization stuff for Peripheral Mode ~~~~~~~~~~~~~~

        nextCheck = millis();                       // force a recheck of the tally state

        do {                                        // We don't leave setup() if we're in Peripheral Mode
            dButt.read();
            if ( millis() >= nextCheck ) {
                tsNow = ( ( digitalRead(TS_1) ? 2 : 0 ) + ( digitalRead(TS_0) ? 1 : 0 ) );              
                if ( tsNow != tsLast ) {
                    tsLast = tsNow;
                    if ( tsNow == 3 ) {
                        disFillPix(PGM);                // set the display to PGM
                    }
                    else {
                        disFillPix(PVW);                // set the display to PVW
                    }
                    disDrawPix(PO_PIXEL, PO_COLOR);     // turn on the power LED
                }
                nextCheck = millis();                   // change to 'nextCheck = millis() + X;' to set a non-zero polling interval of X ms 
            }

            if ( dButt.pressedFor(1500) ) {          // user wants to change the display brightness level
                pmBright = updateBrightness(pmPrefs, "PModePrefs", "pmbrightness", pmBright);
                disClear();
                disSetBright(pmBright);             // set the display brightness               
                disFillPix(PVW);                    // set the display to PVW
                disDrawPix(PO_PIXEL, PO_COLOR);     // turn on the power LED
                tsLast = 0xff;                      // set the last tally state to an impossible value to force a refresh
                nextCheck = millis();               // ... and set the check time to now
            }

        } while (true); // closing statement for the do loop

   }    // closing brace for "if (inPMode)"

// =================== END CODE FOR PERIPHERAL MODE OERATION ===================
