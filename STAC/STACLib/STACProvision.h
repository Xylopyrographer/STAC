
// STACProvision.h

void pvStateArm( uint8_t glyph[], const crgb_t colors[], unsigned long &armTime, bool push = true ) {
    /* - function to set up the display and timing for dealing with display
     *   button presses of varying lengths during power-on or reset
     * - called from STACProvision.h
    */
    drawGlyph( glyph, colors, push );
    delay( 250 );
    flashDisplay( 4, 500, brightMap[ 1 ] );
    armTime = millis() + NEXT_STATE_TIME;

    return;

}   // end pvStateArm

void STACProvision( Preferences &_stcPrefs, stac_ops_t &_stacOps, tState_t &_tallyStat, wifi_info_t &_wifiStat ) {
    /*  This is where all the startup checks are done to figure out if we can use the
     *  existing configuration, what to do if the button is down at startup and such.
     *  You might need a coffee before digging into the logic here. :)
    */
    bool provisioned;       // true if the WiFi provisioning has been done
    uint16_t bootPrefs;     // the version of the normal operating mode preferences layout the STAC has - from NVS storage at boot time
    bool goodPrefs;         // true if bootPrefs = NOM_PREFS_VERSION

    // check to see if we've been provisioned before and if it's OK to use the current Preferences layout if so
    _stcPrefs.begin( "STCPrefs", PREFS_RO );            // open our preferences in R/O mode.
    provisioned = false;
    if ( _stcPrefs.isKey( "pVis" ) ) {
        provisioned = _stcPrefs.getBool( "pVis" );      // check to see if we've been provisioned already. pVis = true if so
    }
    if ( !provisioned ) {
        Serial.println("      ***** STAC not configured *****");
    }
    bootPrefs = 0;
    goodPrefs = false;
    if ( _stcPrefs.isKey( "prefVer" ) ) {
        bootPrefs = _stcPrefs.getUShort( "prefVer" );   // get the prefs layout version we had when last run/powered up
    }
    _stcPrefs.end();

    if ( bootPrefs == NOM_PREFS_VERSION ) {
        goodPrefs = true;
    }
    if ( dButt.read() ) {                               /* button is down on reset or power up */
        // set up the state machine to manage the users button presses & releases
        bool reconfig = false;                          // need a flag for each state
        bool factoryreset = false;
        bool dfu = false;
        bool pvEscape = false;
        unsigned long pvNextArmT = 0UL;
        pvMode_t pvState = UNDEFINED;

        if ( !provisioned || !goodPrefs ) {
            pvStateArm( GLF_UD, alertcolor, pvNextArmT, SHOW );     // set up for DFU_PEND
            pvState = DFU_PEND;
        }
        else {
            pvStateArm( GLF_CFG, warningcolor, pvNextArmT, SHOW );  // set up for CFG_PEND
            pvState = CFG_PEND;
        }

        do {
            dButt.read();
            switch ( pvState ) {
                case CFG_PEND:                          // reconfig pending state
                    if ( dButt.wasReleased() ) {
                        reconfig = true;
                        factoryreset = false;
                        dfu = false;
                        pvEscape = true;
                    }
                    else if ( millis() >= pvNextArmT ) {
                        drawGlyph( GLF_FM, alertcolor, NO_SHOW );       // set up for FR_PEND
                        drawOverlay( GLF_CK, RGB_COLOR_GREEN , SHOW );
                        delay( 250 );
                        flashDisplay( 4, 500, brightMap[ 1 ] );
                        pvNextArmT = millis() + NEXT_STATE_TIME;
                        pvState = FR_PEND;
                    }
                    break;
                case FR_PEND:                           // factory reset pending state
                    if ( dButt.wasReleased() ) {
                        reconfig = false;
                        factoryreset = true;
                        dfu = false;
                        pvEscape = true;
                    }
                    else if ( millis() >= pvNextArmT ) {
                        pvStateArm( GLF_UD, alertcolor, pvNextArmT, SHOW );    // set up for DFU_PEND
                        pvState = DFU_PEND;
                    }
                    break;
                case DFU_PEND:                          // dfu mode pending state
                    if ( dButt.wasReleased() ) {
                        reconfig = false;
                        factoryreset = false;
                        dfu = true;
                        pvEscape = true;
                    }
                    break;
            }   // end switch( pvState )
        } while ( !pvEscape );

        pvEscape = false;
        pvState = UNDEFINED;
        pvNextArmT = 0UL;

        if ( reconfig ) {
            STACconfig( _stcPrefs, _wifiStat, provisioned, goodPrefs );     // go get new config data
        }
        else if ( factoryreset ) {
            STACreset( _stcPrefs );     // this function results in a reset so we never return
        }
        else if ( dfu ) {
            STACupdate();               // this function results in a reset so we never return
        }
    }   // end button down on power-up or reset checks

    if ( provisioned && !goodPrefs ) {
        sendNewPrefs();     // add to the serial port info dump
    }

    if ( !provisioned || !goodPrefs ) {             /* let the user know we need to configure the STAC */
        drawGlyph( GLF_CFG, alertcolor, SHOW );
        flashDisplay( 4, 500, brightMap[ 1 ] );
        disSetBright( brightMap[ 1 ], SHOW );
        STACconfig( _stcPrefs, _wifiStat, provisioned, goodPrefs );
    }

    // go get & set the runtime ops parms from NVS :)
    _stcPrefs.begin( "STCPrefs", PREFS_RO );                // open our preferences in R/O mode
    _stacOps.tModel = stcPrefs.getString( "stswModel" );
    _stacOps.stsPollInt = stcPrefs.getULong( "pPoll" );
    _stacOps.disLevel = stcPrefs.getUChar( "curBright" );
    _stacOps.ctMode = stcPrefs.getBool( "ctMde" );
    _stacOps.autoStart = stcPrefs.getBool( "aStart" );
    _stacOps.tChannel = stcPrefs.getUChar( "talChan" );
    if ( _stacOps.tModel == "V-60HD" ) {                    /* fetch the ops parameters specific to the V-60HD */
        _stacOps.tChanMax = stcPrefs.getUChar( "talMax" );
    }
    else {                                                  /* fetch the ops parameters specific to the V-160HD */
        if ( _stacOps.tChannel > 8  ) {
            _stacOps.tChanBank = "sdi_";
        }
        else {
            _stacOps.tChanBank = "hdmi_";
        }
        _stacOps.tChanHDMIMax = stcPrefs.getUChar( "talMaxHDMI" );
        _stacOps.tChanSDIMax = stcPrefs.getUChar( "talMaxSDI" );
        tallyStat.tLanUID = stcPrefs.getString( "stswLanUID" );
        tallyStat.tLanPW = stcPrefs.getString( "stswLanPW" );
    }

    String tempstring;
    tempstring = _stcPrefs.getString( "stnSSID" );              // ST WiFi SSID
    tempstring.concat( '\0' );                                  // Make it a C-string
    tempstring.toCharArray( _wifiStat.networkSSID, tempstring.length() );

    tempstring = _stcPrefs.getString( "stnPass" );              // ST WiFi Password
    tempstring.concat( '\0' );                                  // Make it a C-string
    tempstring.toCharArray( _wifiStat.networkPass, tempstring.length() );

    tempstring = _stcPrefs.getString( "stswIP" );               // ST switch IP address
    _wifiStat.stIP.fromString( tempstring );                    // convert to an IPAddress type & save into stIP

    _wifiStat.stPort = _stcPrefs.getUShort( "stswPort" );       // ST port number

    _stcPrefs.end();                                            // close our preferences namespace
    // runtime ops params are set

    disSetBright( brightMap[ _stacOps.disLevel ], SHOW );
    sendFooter( _stacOps, _wifiStat );                          // add to the serial port info dump

    // ~~~~~~~~~~~~~~ End all the checks for provisioning & initialization stuff for normal operating mode ~~~~~~~~~~~~~~
    return;

}   // end STACProvision()


//  --- EOF ---
