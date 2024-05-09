
// STACWiFi.h

void connectToWifi( wifi_info_t &wifiStat ) {
    //  This is the "Connect to WiFi" function

    unsigned long wfTimeout;
    wifiStat.wfconnect = false;
    wifiStat.timeout = true;
    int wfStatus = WiFi.status();

    WiFi.disconnect( true );
    WiFi.setHostname( wifiStat.stacID.c_str() );    // set the STAC hostname
    WiFi.setSleep( false );                         // have to set this mode or else the display goes nuts when polling

    WiFi.mode( WIFI_STA );                          // set WiFi station mode. Needs to be done *after* setHostmame & other set calls
    wfTimeout = millis() + WIFI_CONNECT_TIMEOUT;
    WiFi.persistent( false );                       // don't autosave the last set of WiFi credentials
    WiFi.begin( wifiStat.networkSSID, wifiStat.networkPass );

    while ( ( wfStatus != WL_CONNECTED ) && ( wfTimeout >= millis() ) ) {
        delay( 250 );                               // pause as it takes some time for the WiFi routines to do their thing in the backgroud
        wfStatus = WiFi.status();
    }

    if ( wfStatus == WL_CONNECTED ) {
        wifiStat.wfconnect = true;
        wifiStat.timeout = false;
    }
    else {
        WiFi.disconnect();
    }
    return;

}   // end connectToWifi()


//  --- EOF ---
