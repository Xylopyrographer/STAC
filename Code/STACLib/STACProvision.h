    // ~~~~~~~~~~~~~~ Start all the checks for provisioning & initialization stuff for normal operating mode ~~~~~~~~~~~~~~
    
    stcPrefs.begin("STCPrefs", PREFS_RO);                   // open our preferences in R/O mode.
    provisioned = false;
    if ( stcPrefs.isKey("pVis") ); {
        provisioned = stcPrefs.getBool("pVis");             // check to see if we've been provisioned already. provisioned = true if so
    }  

    bootPrefs = 0;
    if ( stcPrefs.isKey("prefVer") ) {
        bootPrefs = stcPrefs.getUShort("prefVer");          // set the prefs layout version we had when last run/powered up
    }          
    stcPrefs.end();

    if (provisioned && (bootPrefs != NOM_PREFS_VERSION)) {    // if we've been provisioned before but the current Prefs layout is different than the last boot...
        
        stcPrefs.begin("STCPrefs", PREFS_RW);   // open the namespace in R/W mode
        stcPrefs.clear();                       // wipe our namespace...
        stcPrefs.end();                         // close the preferences...
        provisioned = false;                    // we are no longer provisioned       
    }


    if (!provisioned) {                         // if not ever provisioned before, or we wiped the namespace because of a new Preferences layout...
        // add to the serial port info dump
        Serial.println("   ***** STAC NOT PROVISIONED *****");
        Serial.println("=======================================");
        // end add to the serial port info dump
                
        stcPrefs.begin("STCPrefs", PREFS_RW);   // create and open the namespace in R/W mode

        // create the namespace keys and store the factory defaults
        stcPrefs.putBool("pVis", false);
        stcPrefs.putString("swVersion", swVer);
        stcPrefs.putUShort("prefVer", NOM_PREFS_VERSION);
        stcPrefs.putULong("pPoll", 300);

        stcPrefs.putString("stnSSID", "");
        stcPrefs.putString("stnPass", "");
        stcPrefs.putString("stswIP", "");
        stcPrefs.putUShort("stswPort", 80);
        
        stcPrefs.putUChar("talChan", 1);
        stcPrefs.putUChar("talMax", 6);
        stcPrefs.putUChar("curBright", 10);
        stcPrefs.putBool("ctMde", true);
        stcPrefs.putBool("aStart", false);

        stcPrefs.end();
        // factory default values are now saved
                                                
        drawGlyph(GLF_CFG, alertcolor);                     // let the user know this is a first time run - configuration required
        flashDisplay(4, 500, 20);
        disSetBright(10);
        
        while ( dButt.read() );                            // wait for the button to be released
        
    }   // end brace for if (!provisioned)

    if ( dButt.read() && provisioned ) {                   // if the button is down at restart and we are provisioned...
        drawGlyph(GLF_CFG, warningcolor);                  //  let the user know they are about to change the provisioning info
        flashDisplay(4, 500, 20);
        disSetBright(10);
        unsigned long resetTime = millis() + 2000;
        provisioned = false;                                // ... then the user wants to reconfigure the STAC, unless...
        while (dButt.read()) {
            if (millis() >= resetTime) {                    // ... the button is long pressed, in which case...
                drawGlyph(GLF_FM, alertcolor);              // ... the user wants to do a factory reset.
                drawOverlay(GLF_CK, GRB_COLOR_GREEN);       // confirm to the user...
                
                // add to the serial port info dump
                Serial.println(" ***** PERFORMING FACTORY RESET *****");
                Serial.println("\r\n              Restarting...");
                Serial.println("=======================================\r\n\r\n");
                // end add to the serial port info dump
                
                stcPrefs.begin("STCPrefs", PREFS_RW);       // open the normal operating mode prefs in R/W mode...
                stcPrefs.clear();                           //  wipe the namespace...
                stcPrefs.end();                             //  close the preferences
                stcPrefs.begin("PModePrefs", PREFS_RW);     // open the peripheral mode prefs in R/W mode...
                stcPrefs.clear();                           //  wipe the namespace...
                stcPrefs.end();                             //  close the preferences
                
                while ( dButt.read() );                     // wait for the button to be released            
                delay(1000);                                // wait a bit for the "GUI"
                disClear();                                 // clear the display
                ESP.restart();                              // restart the STAC
            }
        }
    }   // closing brace for if button down
        
    if (!provisioned) {
        Serial.println(" ***** WAITING FOR PROVISIONING *****");
        Serial.println("=======================================");

        sConfigData = getCreds(stacID, swVer);              // go get the WiFi provisioning data from the user's web browser
        
        drawGlyph(GLF_CK, gtgcolor);                        // confirm to the user we got the provisioning data
        delay(1000);                                        // park a bit for the "GUI"
        
        stcPrefs.begin("STCPrefs", PREFS_RW);               // open our preferences in R/W mode
        stcPrefs.putString("stnSSID", sConfigData.pSSID);   // save the provisioning data
        stcPrefs.putString("stnPass", sConfigData.pPass);
        stcPrefs.putString("stswIP", sConfigData.pSwitchIP);
        stcPrefs.putUShort("stswPort", sConfigData.pPort);
        stcPrefs.putULong("pPoll", sConfigData.pPollInt);
        stcPrefs.putUChar("talMax", sConfigData.ptChanMax);
        if (stcPrefs.getUChar("talChan") > sConfigData.ptChanMax) {     // make sure the active tally channel is not > max tally
            stcPrefs.putUChar("talChan", 1);
        }
        stcPrefs.putBool("pVis", true);
        stcPrefs.end();
        provisioned = true;
        
        Serial.println("  ***** PROVISIONING COMPLETE *****");
        Serial.println("=======================================");
    }

    // go get & set the runtime ops parms from NVS :)     
    stcPrefs.begin("STCPrefs", PREFS_RO);                       // open our preferences in R/O mode
    // read and set the operational parameters from NVS
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
    // runtime ops parms are set

    disSetBright(currentBrightness);
    tallyStatus.tState = "NO_INIT";                                 // initialize the control flags
    lastTallyState = "NO_TALLY";
    wifiStatus.wfconnect = false;
    wifiStatus.timeout = false;

    disClear();
    disDrawPix(PO_PIXEL, GRB_COLOR_GREEN);                   // turn the power LED green

    // add to the info dump to the serial port   
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
    Serial.println("     =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    Serial.print("    Auto start: ");
        if (autoStart) Serial.println("Enabled");
        else Serial.println("Disabled");
    Serial.print("    Operating Mode: ");
        if (ctMode) Serial.println("Camera Operator");
        else Serial.println("Talent");
    Serial.print("    Active Tally Channel: ");
    Serial.println(tallyStatus.tChannel);
    Serial.print("    Brightness Level: ");
    Serial.println(currentBrightness / 10);

    Serial.println(F("======================================"));
    // end add to the info dump to the serial port

    pinMode(TS_0, OUTPUT);      // set the GROVE GPIO 
    pinMode(TS_1, OUTPUT);      //   pins as outputs
    GROVE_UNKNOWN;              // send the tally state to the GROVE pins
    
    delay(1000);                // all the background setup is done. Whew. Almost there... Pause for the "GUI"
     
    // ~~~~~~~~~~~~~~ End all the checks for provisioning & initialization stuff for normal operating mode ~~~~~~~~~~~~~~
