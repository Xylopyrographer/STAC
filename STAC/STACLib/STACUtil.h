
// STACUtil.h

String genSTACid() {
/*  Generates and returns the STAC ID
 *  based on the ESP-32 chip MAC.
 *  - depends on <Esp.h>
 *  - idPrefix must be defined as a global outside this function
*/

    String idSuffix = "-";
    char bufr[ 3 ] = { };                   // buffer for a formatted MAC id byte
    uint8_t stacMAC[ 6 ] = { };             // array for the chip MAC address

    esp_efuse_mac_get_default( stacMAC );   // high byte of the MAC is at array index 0.
    for ( uint8_t i = 5; i >= 2; i-- ) {
        snprintf( bufr, 3, "%02X", stacMAC[ i ] );
        idSuffix.concat( bufr );
    }
    return ID_PREFIX + idSuffix;

}   // end genSTACid()

void getCreds( prov_info_t &_stCreds, wifi_info_t &_wifiStat ) {
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
 *  The web pages are defined as globals outside this function.
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

    bool gotForm = false;                           // set to true to break out of the handle client

    WiFi.mode( WIFI_MODE_NULL );                                // turn off the Wi-Fi radio
    while ( WiFi.getMode() != WIFI_MODE_NULL ) delay( 10 );     // ensure the radio is off.
    WiFi.mode( WIFI_AP );                                       // configure the WiFi mode to AP
    while ( WiFi.getMode() != WIFI_MODE_AP ) delay( 10 );       // ensure we're configured in AP mode.

    WiFi.softAP( _wifiStat.stacID.c_str(), suPwd, suChan, suHideSSID, suMaxConnect ); // set all the WiFi stuff for the AP and turn it on
    WiFi.softAPsetHostname( suStachost );                       // set the STAC mDNS AP mode hostname
    WiFi.softAPConfig( suIP, suIP, suNMask );                   // set all the networking stuff for the AP

    MDNS.begin( suStachost );                       // fire up the mDNS responder

    // create the index HTML page
    String suForm =
        String ( suFormOpen ) +
        "Unit: " + _wifiStat.stacID + "<br>" +
        "MAC: " + WiFi.macAddress() + "<br><br>" +
        "Version: " + swVer + "<br>" +
        "Core: " + ardesp32Ver + "<br>" +
        "SDK: " + espidfsdk + "<br>" +
        String( suFormClose );

    // register the suServer endpoint handlers
    suServer.on( "/", HTTP_GET,
    [&]() {
        suServer.send( 200, "text/html", suForm );
    }
    );

    // "/" POST returns the model of the switch
    suServer.on( "/", HTTP_POST,
    [&]() {
        _stCreds.pModel = suServer.arg( "stModel" );              // get form argument values by name
        if ( _stCreds.pModel == "V-60HD" ) {
            suServer.send( 200, "text/html", suConfigV60 );     // send the V-60HD setup page
        }
        else {
            suServer.send( 200, "text/html", suConfigV160 );    // send the V-160HD setup page
        }
    }
    );  // end "/" POST handler

    // "/parse" POST handles config information for model V-60HD
    suServer.on( "/parse", HTTP_POST,
    [&]() {
        suServer.send( 200, "text/html", suReceived );
        _stCreds.pSSID = suServer.arg( "SSID" );                // get form argument values by name
        _stCreds.pPass = suServer.arg( "pwd" );
        _stCreds.pSwitchIP = suServer.arg( "stIP" );
        _stCreds.pPort = (uint16_t)suServer.arg( "stPort" ).toInt();
        _stCreds.ptChanMax = (uint8_t)suServer.arg( "stChan" ).toInt();
        _stCreds.pPollInt = (unsigned long)suServer.arg( "pollTime" ).toInt();

        suServer.close();                                   // all done so, stop listening for requests from the client
        MDNS.end();                                         // shut down the mDNS responder
        unsigned long pauseTime = millis() + 500UL;         // pause before shutting down so the server stuff gets done
        while ( pauseTime >= millis() ) yield();
        WiFi.mode( WIFI_MODE_NULL );                                // turn off the Wi-Fi radio
        while ( WiFi.getMode() != WIFI_MODE_NULL ) delay( 10 );     // ensure the radio is off.
        pauseTime = millis() + 500UL;
        while ( pauseTime >= millis() );
        gotForm = true;
    }
    );  // end "/parse" handler

    // "/parse2" POST handles config information for model V-160HD
    suServer.on( "/parse2", HTTP_POST,
    [&]() {
        suServer.send( 200, "text/html", suReceived );
       _stCreds.pSSID = suServer.arg( "SSID" );              // get form argument values by name
       _stCreds.pPass = suServer.arg( "pwd" );
       _stCreds.pSwitchIP = suServer.arg( "stIP" );
       _stCreds.pPort = (uint16_t)suServer.arg( "stPort" ).toInt();
       _stCreds.pLanUID = suServer.arg( "stnetUser" );
       _stCreds.pLanPW = suServer.arg( "stnetPW" );
       _stCreds.ptChanHDMIMax = (uint8_t)suServer.arg( "stChanHDMI" ).toInt();
       _stCreds.ptChanSDIMax = (uint8_t)suServer.arg( "stChanSDI" ).toInt();
       _stCreds.pPollInt = (unsigned long)suServer.arg( "pollTime" ).toInt();
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
    );  // end "/parse2" handler

    suServer.onNotFound( [&]() { suServer.send( 404, "text/html", suNotFound ); } );
    // end register the server endpoint handlers

    suServer.begin();                   // lets start the setup Web server!
    suServer.enableDelay( false );      // disable the 1 ms delay in WebServer.cpp

    while ( !gotForm ) {                // until we get the form back...
        suServer.handleClient();        //  listen for HTTP requests from the users browser
        yield();
    }

    return;

}   // end getCreds()


void STACconfig( Preferences &_stcPrefs, wifi_info_t &_wifiStat, bool &_provisioned, bool &_goodPrefs ) {
/* - sets up the Preferences namespace in NVS
 * - gets the STAC configuration information via a user's web browser
 * - stores the config info into NVS
 * - if the prefs layout version hasn't changed, retains the current operating parameters
 * - if the switch model changes between configs, the user run-time operating parameters are reset to defaults
*/

    // add to the serial port info dump
    Serial.println( "  ***** Waiting for configuration *****" );

    if ( !_goodPrefs || !_provisioned ) {
        _stcPrefs.begin("STCPrefs", PREFS_RW);               // create or open the namespace in R/W mode
        _stcPrefs.clear();                                   // wipe the namespace
        // create the namespace keys and store the factory defaults
        _stcPrefs.putBool( "pVis", false );
        _stcPrefs.putString( "swVersion", swVer );
        _stcPrefs.putUShort( "prefVer", NOM_PREFS_VERSION );
        _stcPrefs.putString( "lastModel", "" );
        _stcPrefs.putString( "stnSSID", "" );
        _stcPrefs.putString( "stnPass", "" );
        _stcPrefs.putString( "stswIP", "" );
        _stcPrefs.putUShort( "stswPort", 80 );
        _stcPrefs.putULong( "pPoll", 300UL );
        _stcPrefs.putUChar( "talChan", 1 );
        _stcPrefs.putUChar( "talMax", 8 );
        _stcPrefs.putUChar( "talMaxHDMI", 8 );
        _stcPrefs.putUChar( "talMaxSDI", 8 );
        _stcPrefs.putString( "stLANuser", "" );
        _stcPrefs.putString( "stLANpw", "" );
        _stcPrefs.putUChar( "curBright", 1 );
        _stcPrefs.putBool( "ctMde", true );
        _stcPrefs.putBool( "aStart", false );
        _stcPrefs.end();
        // namespace as per the nom prefs layout created with default values saved
    }

    prov_info_t sConfigData = { "NO_MODEL", "NO_SSID", "NO_WFPASS", "NO_IP", 0, "NO_USER", "NO_PW", 0, 0, 0, 0UL };

    getCreds( sConfigData, _wifiStat );     // go get the STAC configuration data from the user's web browser

    drawGlyph( GLF_CK, gtgcolor, SHOW );    // confirm to the user we got the provisioning data

    // open the namespace in R/W mode & save the provisioning data
    _stcPrefs.begin( "STCPrefs", PREFS_RW );                 // open the namespace in R/W mode
    _stcPrefs.putString( "stswModel", sConfigData.pModel );
    _stcPrefs.putString( "stnSSID", sConfigData.pSSID );     // save the provisioning data
    _stcPrefs.putString( "stnPass", sConfigData.pPass );
    _stcPrefs.putString( "stswIP", sConfigData.pSwitchIP );
    _stcPrefs.putUShort( "stswPort", sConfigData.pPort );
    _stcPrefs.putULong( "pPoll", sConfigData.pPollInt );
    _stcPrefs.putUChar( "talMax", sConfigData.ptChanMax );
    // check to see if we've had a model change & reset the run-time configurable ops parameters if so
    if ( _stcPrefs.getString( "lastModel" ) == "" ) {              /* if first run, the defaults are already set */
        _stcPrefs.putString( "lastModel", sConfigData.pModel );     /*  so just save the model */
    }
    else if ( _stcPrefs.getString( "lastModel" ) != sConfigData.pModel ) {    /* if model has changed, reset the run-time configurable parameters */
        _stcPrefs.putUChar( "talChan", 1 );
        _stcPrefs.putUChar( "curBright", 1 );
        _stcPrefs.putBool( "ctMde", true );
        _stcPrefs.putBool( "aStart", false );
        _stcPrefs.putString( "lastModel", sConfigData.pModel );
        sendModelChange();                      // add to the info header dump to the serial port
    }
    // end model change check

    if ( sConfigData.pModel == "V-60HD" ) {
        if ( _stcPrefs.getUChar( "talChan" ) > sConfigData.ptChanMax ) {    /* make sure the active tally channel is not > max tally */
            _stcPrefs.putUChar( "talChan", 1 );
        }
    }

    if ( sConfigData.pModel == "V-160HD" ) {                                /* make sure the active tally channel is not > max tally */
        _stcPrefs.putUChar( "talMaxHDMI", sConfigData.ptChanHDMIMax );
        _stcPrefs.putUChar( "talMaxSDI", sConfigData.ptChanSDIMax );
        _stcPrefs.putString( "stLANuser", sConfigData.pLanUID );
        _stcPrefs.putString( "stLANpw", sConfigData.pLanPW );

        if ( _stcPrefs.getUChar( "talChan" ) > 8 ) {
            uint8_t tempSDI = _stcPrefs.getUChar( "talChan" ) - 8;
            if ( tempSDI > sConfigData.ptChanSDIMax ) {
                _stcPrefs.putUChar( "talChan", 9 );
            }
        }
        else if (_stcPrefs.getUChar( "talChan" ) > sConfigData.ptChanHDMIMax ) {
            _stcPrefs.putUChar( "talChan", 1 );
        }
    }

    _stcPrefs.putBool( "pVis", true );
    _stcPrefs.end();
    _provisioned = true;

    sendConfigDone();               // add to the info header dump to the serial port
    delay( GUI_PAUSE_TIME );        // park a bit for the "GUI"

    return;

}   // end STACconfig()

void STACreset( Preferences &_stcPrefs ) {
    /* clears the STAC NVS namespaces */
    sendReset();            // add to the serial port info dump

    _stcPrefs.begin( "STCPrefs", PREFS_RW );    // open the normal operating mode prefs in R/W mode...
    _stcPrefs.clear();                          //  wipe the namespace...
    _stcPrefs.end();                            //  close the preferences
    _stcPrefs.begin( "PModePrefs", PREFS_RW );  // open the peripheral mode prefs in R/W mode...
    _stcPrefs.clear();                          //  wipe the namespace...
    _stcPrefs.end();                            //  close the preferences

    delay( GUI_PAUSE_TIME );
    flashDisplay( 1, 500, brightMap[ 1 ] );
    while ( true ) {
        yield();        // park the bus
    }

    return;     // we'll never get here, but keeps the compiler happy

}   // end STACreset()


void STACupdate() {
/* - routine to perform an OTA update of the STAC firmware
 * - the STAC fires up a WiFi AP
 * - user specifies via a web browser the firmware file to use.
 * - always forces a restart of the STAC
*/

    sendOTA();          // add to the serial port info dump

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

    WiFi.mode( WIFI_MODE_NULL );                                // turn off the Wi-Fi radio
    while ( WiFi.getMode() != WIFI_MODE_NULL ) delay( 10 );     // ensure the radio is off.
    WiFi.mode( WIFI_AP );                                       // configure the WiFi mode to AP
    while ( WiFi.getMode() != WIFI_MODE_AP ) delay( 10 );       // ensure we're configured in AP mode.

    WiFi.softAP( wifiStat.stacID.c_str(), udpwd, udwifiChan, udhideSSID, udmaxConnect ); // set all the WiFi stuff for the AP and turn it on
    WiFi.softAPsetHostname( udstachost );                       // set the STAC mDNS AP mode hostname
    WiFi.softAPConfig( udIP, udIP, udNMask );                   // set all the networking stuff for the AP

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
        sendOTAStat( udOK, udFileName, udSize, udStatus ); // send the update status to the Serial port
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
    STACserver.enableDelay( false );    // disable the 1 ms delay in WebServer.cpp

    while ( true ) {
        STACserver.handleClient();      // listen for HTTP requests from a client browser
        yield();
    }

    return;     // we'll never get here, but keeps the compiler happy

}   // end updateSTAC()


// ---- EOF ----
