/*
This is where all the startup checks are done to figure out if we can use the 
existing configuration, what to do if the button is down at startup and such.
You might need a coffee before digging into the logic in here. :)
*/

    unsigned long nextStateVal = 0UL;      // # ms value the button has to be down at to trigger the next state
    
    // check to see if we've been provisioned before and if it's OK to use the current Preferences layout if so
    stcPrefs.begin( "STCPrefs", PREFS_RO );             // open our preferences in R/O mode.
    provisioned = false;
    if ( stcPrefs.isKey( "pVis" ) ) {
        provisioned = stcPrefs.getBool( "pVis" );       // check to see if we've been provisioned already. pVis = true if so
    }
    if ( !provisioned ) {
        Serial.println("      ***** STAC not configured *****");  
    }
    bootPrefs = 0;
    goodPrefs = false;
    if ( stcPrefs.isKey( "prefVer" ) ) {
        bootPrefs = stcPrefs.getUShort( "prefVer" );    // get the prefs layout version we had when last run/powered up
    }
    stcPrefs.end();

    if ( bootPrefs == NOM_PREFS_VERSION ) goodPrefs = true;

    if ( dButt.read() ) {                                           // button is down on reset or power up
        bool reconfig = false, factoryreset = false, dfu = false;   // need a flag for each action

        if ( !provisioned || !goodPrefs ) {
            drawGlyph( GLF_CFG, alertcolor, 1 );        // let the user know device needs initial or fresh configuration
        }
        else {
            drawGlyph( GLF_CFG, warningcolor, 1 );      // let the user know they are about to change the provisioning info
            reconfig = true;
            factoryreset = false;
            dfu = false;
        }
        flashDisplay( 4, 500, brightMap[ 1 ] );
        disSetBright( brightMap[ 1 ] );
        nextStateVal = millis() + NEXT_STATE_TIME;

        while ( dButt.read() && !dfu ) {
            if ( millis() >= nextStateVal && !factoryreset ) {     // long press
                if ( provisioned && goodPrefs ) {
                    reconfig = false;
                    factoryreset = true;
                    dfu = false;
                    drawGlyph( GLF_FM, alertcolor, 0 );
                    drawOverlay(GLF_CK, GRB_COLOR_GREEN , 1);
                }
                else {
                    reconfig = false;
                    factoryreset = false;
                    dfu = true;                             // if not provisioned or prefs don't match, this lets DFU happen after a long press.
                    drawGlyph( GLF_UD, alertcolor, 1 );
                }
                flashDisplay( 4, 500, brightMap[ 1 ] );     // let the user know we're armed to do a firmware update
                disSetBright( brightMap[ 1 ] );
                nextStateVal = millis() + NEXT_STATE_TIME;                    // the next button down timing interval comes after the display flashing time
            }            
            if ( millis() >=  nextStateVal ) {              // 2x long press press
                reconfig = false;
                factoryreset = false;
                dfu = true;                
                drawGlyph( GLF_UD, alertcolor, 1 );
                flashDisplay( 4, 500, brightMap[ 1 ] );     // let the user know we're armed to do a firmware update
                disSetBright( brightMap[ 1 ] );
            }           
        } // while ( dButt.read() && !dfu );
        nextStateVal = 0UL;
        
        if ( reconfig ) {
            STACconfig( provisioned, goodPrefs  );
        }
        else if ( factoryreset ) {
            STACreset();
        }
        else if ( dfu ) {
            while ( dButt.read() );
            STACupdate();
        }
        else {      // we got here because we need provisioning & button was released before the long press time.
            STACconfig( provisioned, goodPrefs );
        }
    }

    if ( provisioned && !goodPrefs ) {        
        Serial.println( "  ****** New preferences layout ******");
        Serial.println( "  ****** Configuration required ******");
    }
    
    if ( !provisioned || !goodPrefs ) {
        disClear( 1 );
        drawGlyph( GLF_CFG, alertcolor, 1 );
        flashDisplay( 4, 500, brightMap[ 1 ] );     // let the user know we're armed to do a factory reset
        disSetBright( brightMap[ 1 ] );
        STACconfig( provisioned, goodPrefs );
    }
    
    // go get & set the runtime ops parms from NVS :)     
    stcPrefs.begin("STCPrefs", PREFS_RO);                       // open our preferences in R/O mode
    stsPollInt = stcPrefs.getULong("pPoll");
    currentBrightness = stcPrefs.getUChar("curBright");
    tallyStatus.tChannel = stcPrefs.getUChar("talChan");
    tallyStatus.tChanMax = stcPrefs.getUChar("talMax");
    ctMode = stcPrefs.getBool("ctMde");
    autoStart = stcPrefs.getBool("aStart");
    
    String tempstring;   
    tempstring = stcPrefs.getString("stnSSID");                 // ST WiFi SSID  
    tempstring.concat('\0');                                    // Make it a C-string  
    tempstring.toCharArray(networkSSID, tempstring.length());   

    tempstring = stcPrefs.getString("stnPass");                 // ST WiFi Password      
    tempstring.concat('\0');                                    // Make it a C-string  
    tempstring.toCharArray(networkPass, tempstring.length());

    tempstring = stcPrefs.getString("stswIP");                  // ST switch IP address    
    stIP.fromString(tempstring);                                // convert to an IPAddress type & save into stIP

    stPort = stcPrefs.getUShort("stswPort");                    // ST port number
 
    stcPrefs.end();                                             // close our preferences namespace
    // runtime ops params are set

    disClear( 0 );                   
    disSetBright( brightMap[ currentBrightness ] ); 
    disDrawPix( PO_PIXEL, GRB_COLOR_GREEN, 1 );                 // turn on the power LED

    // add to the info header dump to the serial port   
    Serial.print("    WiFi Network SSID: ");
    Serial.println(networkSSID);
    Serial.print("    Smart Tally IP: ");
    Serial.println(stIP);
    Serial.print("    Port #: ");
    Serial.println(stPort);
    Serial.print("    Max Tally Channel: ");
    Serial.println(tallyStatus.tChanMax);
    Serial.print("    Polling Interval: ");
    Serial.print(stsPollInt);
    Serial.println(" ms");
    Serial.println("    =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    Serial.print("    Active Tally Channel: ");
    Serial.println(tallyStatus.tChannel);
    Serial.print("    Tally Mode: ");
        if (ctMode) Serial.println("Camera Operator");
        else Serial.println("Talent");
    Serial.print("    Auto start: ");
        if (autoStart) Serial.println("Enabled");
        else Serial.println("Disabled");
    Serial.print("    Brightness Level: ");
    Serial.println( currentBrightness );
    Serial.println( "=========================================" );
    // end add to the info dump to the serial port
    
    delay( GUI_PAUSE_TIME );    // all the background setup is done. Whew. Almost there... Pause for the "GUI"
     
    // ~~~~~~~~~~~~~~ End all the checks for provisioning & initialization stuff for normal operating mode ~~~~~~~~~~~~~~
