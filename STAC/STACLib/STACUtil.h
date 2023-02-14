
/*~~~~ GROVE Pins Macros  - sends the tally status to the GPIO pins ~~~~*/
#define GROVE_PGM digitalWrite( TS_1, HIGH ); digitalWrite( TS_0, HIGH )
#define GROVE_PVW digitalWrite( TS_1, HIGH ); digitalWrite( TS_0, LOW )
#define GROVE_NO_SEL digitalWrite( TS_1, LOW ); digitalWrite( TS_0, HIGH )
#define GROVE_UNKNOWN digitalWrite( TS_1, LOW ); digitalWrite( TS_0, LOW )

provData_t getCreds() {
/*  Big honkin' routine that retrevies the configuration/provisiong data for the STAC 
 *      - sets the STAC as a WiFi access point using a device unique SSID
 *      - waits for the user's web browser to connect
 *      - sends the form to the browser for the user to fill out
 *      - waits for the form to be returned
 *      - confirms to the user that the form was received
 *      - shuts down the access point
 *      - returns the configuration/provisiong data to the calling function
 *      - and then goes for a beer.
 *
 *  The IP and password for the access point are set in this function.
 */

    WebServer suServer( 80 );                       // need a server to interact with the user's web browser
    
    const char* suStachost = "setup";               // the hostname used for mDNS
    const char* suPwd = "1234567890";               // the password for our WiFi AP
    const IPAddress suIP( 192, 168, 6, 14 );        // IP...
    const IPAddress suGateway( 0, 0, 0, 0 );        // gateway...
    const IPAddress suNMask( 255, 255, 255, 0 );    // and network mask of the AP
    uint8_t suChan = 1;                             // AP WiFi channel to use. Default is 1, max is 13
    bool suHideSSID = false;                        // false to broadcast the SSID of our AP network, true to hide it
    uint8_t suMaxConnect = 1;                       // maximum # of client connections allowed to our AP

    provData_t stCred;                              // struct that holds the info returned from the setup form
    bool gotForm = false;                           // set to true to break out of the handle client
    
    #include "./STACsuPages.h"                      // HTML pages sent to the user's browser

    WiFi.mode( WIFI_MODE_NULL );                                // turn off the Wi-Fi radio
    while ( WiFi.getMode() != WIFI_MODE_NULL ) delay( 10 );     // ensure the radio is off.
    WiFi.mode( WIFI_AP );                                       // configure the WiFi mode to AP 
    while ( WiFi.getMode() != WIFI_MODE_AP ) delay( 10 );       // ensure we're configured in AP mode.
    
    WiFi.softAP( stacID.c_str(), suPwd, suChan, suHideSSID, suMaxConnect ); // set all the WiFi stuff for the AP and turn it on
    WiFi.softAPsetHostname( suStachost );                           // set the STAC mDNS AP mode hostname
    WiFi.softAPConfig( suIP, suIP, suNMask );                       // set all the networking stuff for the AP

    MDNS.begin( suStachost );                       // fire up the mDNS responder

    // register the suServer endpoint handlers
    suServer.on( "/", HTTP_GET, 
    [&]() { 
        String suForm =
        String ( suFormOpen ) +
        stacID + "<br>"
        "MAC: " + WiFi.macAddress() + "<br><br>" +
        "Version: " + swVer + "<br>" +
        "Core: " + ardesp32Ver + "<br>" +
        "SDK: " + espidfsdk + "<br>" +
        String( suFormClose );     
        suServer.send( 200, "text/html", suForm ); 
    } );
    
    suServer.on( "/parse", HTTP_POST, 
    [&]() {
        suServer.send( 200, "text/html", suReceived );
        stCred.pSSID = suServer.arg( "SSID" );                                  // get form argument values by name
        stCred.pPass = suServer.arg( "pwd" );
        stCred.pSwitchIP = suServer.arg( "stIP" );
        stCred.pPort = (uint16_t)suServer.arg( "stPort" ).toInt();
        stCred.ptChanMax = (uint8_t)suServer.arg( "stChan" ).toInt();
        stCred.pPollInt = (unsigned long)suServer.arg( "pollTime" ).toInt();
        suServer.close();                                   // all done so, stop listening for requests from the client
        MDNS.end();                                         // shut down the mDNS responder
        unsigned long pauseTime = millis() + 500UL;         // pause before shutting down so the server stuff gets done
        while ( pauseTime >= millis() );
        WiFi.mode( WIFI_MODE_NULL );                                // turn off the Wi-Fi radio
        while ( WiFi.getMode() != WIFI_MODE_NULL ) delay( 10 );     // ensure the radio is off.
        pauseTime = millis() + 500UL;
        while ( pauseTime >= millis() );
        gotForm = true;
    }
    );  // end of the ".on /parse" handler
    
    suServer.onNotFound( [&]() { suServer.send( 404, "text/html", suNotFound ); } );
    // end register the server endpoint handlers

    suServer.begin();                   // lets start the setup Web server!

    while( !gotForm ) {                 // listen for HTTP requests from a client browser
        suServer.handleClient();        // until we get the form back
        yield();
    }
    return stCred;

}   // end getCreds()

void udSerialStat( bool udOK, String udFileName, size_t udSize, String udStatus ) {
/* 
 * Called by the firmware update lambda function to send 
 * results of the firmware update to the Serial port.
*/
    if ( udOK ) {   
            Serial.println( "******* Firmware update done *******" );
            Serial.print(   "File: "); Serial.println( udFileName);
            Serial.print(   "Bytes written: "); Serial.println( udSize);
            Serial.print(   "Status: "); Serial.println( udStatus );
    }        
    else {                    
            Serial.println( "******* FIRMWARE UPDATE FAILED *******" );
            Serial.print(   "Tried with file: "); Serial.println( udFileName);
            Serial.print(   "Reason: "); Serial.println( udStatus );
            Serial.println( "Ensure the correct \"STAC_XXXXX.BIN\"" );
            Serial.println( "file was selected");
    }
    Serial.println( "              Restarting..." );
    Serial.println( "=========================================\r\n\r\n" );
    Serial.flush();

    return;
    
}   // end udSerialStat()

void STACconfig( bool &provisioned, bool &goodPrefs ) {
/* - sets up the Preferences namespace in NVS
 * - gets the STAC configuration information via a user's web browser
 * - stores the config info into NVS
 * - if the prefs layout version hasn't changed, retains the current operating parameters
*/

    // add to the serial port info dump   
    Serial.println( "  ***** Waiting for configuration *****" );

    if ( !goodPrefs || !provisioned ) {
        stcPrefs.begin("STCPrefs", PREFS_RW);               // create or open the namespace in R/W mode
        stcPrefs.clear();                                   // wipe the namespace
        // create the namespace keys and store the factory defaults
        stcPrefs.putBool( "pVis", false );
        stcPrefs.putString( "swVersion", swVer );
        stcPrefs.putUShort( "prefVer", NOM_PREFS_VERSION );

        stcPrefs.putString( "stnSSID", "" );
        stcPrefs.putString( "stnPass", "" );
        stcPrefs.putString( "stswIP", "" );
        stcPrefs.putUShort( "stswPort", 80 );
        stcPrefs.putUChar( "talMax", 6 );
        stcPrefs.putULong( "pPoll", 300 );
    
        stcPrefs.putUChar( "talChan", 1 );
        stcPrefs.putUChar( "curBright", 1 );
        stcPrefs.putBool( "ctMde", true );
        stcPrefs.putBool( "aStart", false );
        stcPrefs.end();
        // namespace as per the nom prefs layout created with default values saved
    }

    provData_t sConfigData = getCreds();                // go get the STAC configuration data from the user's web browser
    drawGlyph( GLF_CK, gtgcolor, 1 );                   // confirm to the user we got the provisioning data

    stcPrefs.begin("STCPrefs", PREFS_RW);               // open the namespace in R/W mode
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
    Serial.println( "   ***** Configuration complete *****" );          // add to the info header dump to the serial port
    Serial.println( "=========================================" );
    Serial.flush();
    delay( GUI_PAUSE_TIME );        // park a bit for the "GUI"
    
    return;
    
}   // end STACconfig()

void STACreset() {
/* Wipes the STAC NVS */

    // add to the serial port info dump
    Serial.println(     "  ***** Performing factory reset *****" );
    Serial.println(     "=========================================\r\n\r\n" );
    Serial.flush();
    // end add to the serial port info dump
    
    stcPrefs.begin("STCPrefs", PREFS_RW);       // open the normal operating mode prefs in R/W mode...
    stcPrefs.clear();                           //  wipe the namespace...
    stcPrefs.end();                             //  close the preferences
    stcPrefs.begin("PModePrefs", PREFS_RW);     // open the peripheral mode prefs in R/W mode...
    stcPrefs.clear();                           //  wipe the namespace...
    stcPrefs.end();                             //  close the preferences

    delay( GUI_PAUSE_TIME );
    flashDisplay( 1, 500, brightMap[ 1 ] );
    while ( true ) {
        yield;
    }    

    return;     // we'll never get here, but keeps the compiler happy

}   // end STACreset()

void STACupdate() {
/* - routine to perform an OTA update of the STAC firmware
 * - the STAC fires up a WiFi AP
 * - user specifies via a web browser the firmware file to use.
 * - always forces a restart of the STAC
*/

    // add to the serial port info dump
    Serial.println(     "    ***** Updating STAC firmware *****" );
    Serial.println(     "=========================================" );
    Serial.flush();
    // end add to the serial port info dump

    WebServer STACserver( 80 );                     // need a server to interact with the user's web browser

    const char* udstachost = "update";              // the hostname used for mDNS
    const char* udpwd = "1234567890";               // the password for our WiFi AP
    const IPAddress udIP( 192, 168, 6, 14 );        // IP...
    const IPAddress udgateway( 0, 0, 0, 0 );        // gateway...
    const IPAddress udNMask( 255, 255, 255, 0 );    // and network mask of the AP
    uint8_t udwifiChan = 1;                         // WiFi channel to use. Default is 1, max is 13
    bool udhideSSID = false;                        // false to broadcast the SSID of our AP network, true to hide it
    uint8_t udmaxConnect = 1;                       // maximum # of client connections allowed to our AP
    
    String udFileName = "";                         // name of the firmware file used for the update
    size_t udSize = 0;                              // # of bytes written by Update
    bool udOK = true;                               // true if the Update succeded, false otherwise.
    String udStatus = "No Error";                   // the return status string from Update
    String udMessage = "";                          // body of the Update return status message

    #include "./STACotaPages.h"                     // HTML pages sent to the user's browser

    WiFi.mode( WIFI_MODE_NULL );                                // turn off the Wi-Fi radio
    while ( WiFi.getMode() != WIFI_MODE_NULL ) delay( 10 );     // ensure the radio is off.
    WiFi.mode( WIFI_AP );                                       // configure the WiFi mode to AP 
    while ( WiFi.getMode() != WIFI_MODE_AP ) delay( 10 );       // ensure we're configured in AP mode.
    
    WiFi.softAP( stacID.c_str(), udpwd, udwifiChan, udhideSSID, udmaxConnect ); // set all the WiFi stuff for the AP and turn it on
    WiFi.softAPsetHostname( udstachost );                                       // set the STAC mDNS AP mode hostname
    WiFi.softAPConfig( udIP, udIP, udNMask );                       // set all the networking stuff for the AP

    MDNS.begin( udstachost );                       // fire up the mDNS responder

    // register the STACserver endpoint handlers
    STACserver.on( "/", HTTP_GET, [&]() { STACserver.send( 200, "text/html", udIndex ); } );
    STACserver.on( "/update", HTTP_POST, 
    [&]() { /* function called after the file upload handler function returns */
        if ( udOK ) {                                       // assemble the body of the update results status page
            udMessage = String( udGood )
            + udFileName
            + String( "<br><br>Bytes written: ")
            + String( udSize )
            + String( "<br>Status: " ) 
            + udStatus;
            drawGlyph( GLF_CK, gtgcolor, 1 );
        }
        else {
            udMessage = String( udFail ) 
            + udStatus 
            + String( "<br><br>Ensure the correct<br>\"<strong>STAC_xxxx.bin</strong>\"<br>file was selected.<br>" );
            drawGlyph( GLF_BX, alertcolor, 1 );
        }
        udMessage = udPageOpen + udMessage + udPageClose;   // assemble the update results status page to the user's browser

        STACserver.send( 200, "text/html", udMessage );     // send the update results status page to the user's browser
        STACserver.close();                                 // all done so, stop listening for requests from the client
        MDNS.end();                                         // shut down the mDNS responder      
        udSerialStat( udOK, udFileName, udSize, udStatus ); // send the update status to the Serial port
        unsigned long udPauseTime = millis() + 500UL;       // pause before shutting down so the server stuff gets done
        while ( udPauseTime >= millis() ) delay( 10 );;
        WiFi.mode( WIFI_MODE_NULL );                                // turn off the Wi-Fi radio
        while ( WiFi.getMode() != WIFI_MODE_NULL ) delay( 10 );     // ensure the radio is off.
        udPauseTime = millis() + 500UL;
        while ( udPauseTime >= millis() );
        ESP.restart();
    },
    [&]() { /* function called to handle the file upload & writing that file to the OTA partition in flash */
        HTTPUpload& fwFile = STACserver.upload();
        if ( fwFile.status == UPLOAD_FILE_START ) {
            udFileName = fwFile.filename;
            Update.begin();
        }
        else if ( fwFile.status == UPLOAD_FILE_WRITE ) {
            Update.write( fwFile.buf, fwFile.currentSize );
        } 
        else if ( fwFile.status == UPLOAD_FILE_END ) {
            Update.end( true );
            udOK = true;
            udSize = fwFile.totalSize;
        }
        if ( Update.hasError() ) {
            udStatus = Update.errorString();
            udOK = false;
            Update.end();
        }
    }
    );  // end of the ".on /update" handler
    
    STACserver.onNotFound( [&]() { STACserver.send( 404, "text/html", udNotFound ); } );
    // end register the server endpoint handlers

    STACserver.begin();                 // lets start the OTA Web server!
    
    while( true ) {
        STACserver.handleClient();      // listen for HTTP requests from a client browser
        yield();
    }
    
    return;     // we'll never get here, but keeps the compiler happy

}   // end updateSTAC()

void pvStateArm( uint8_t glyph[], const CRGB colors[], unsigned long &armTime, bool show = true ) {
    /* - function to set up the display and timing for dealing with display
     *    button presses of varying lengths during power-on or reset
     * - called from STACProvision.h
    */
    drawGlyph( glyph, colors, show );
    flashDisplay( 4, 500, brightMap[ 1 ] );
    armTime = millis() + NEXT_STATE_TIME;

}   // end pvStateArm

// ---- EOF ----
