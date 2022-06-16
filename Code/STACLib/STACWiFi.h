WiFiState connectToWifi(WfState wifistate) {
/*  This is the "Connect to WiFi" function
*/
    unsigned long wfTimeout;
    wifistate.wfconnect = false;
    wifistate.timeout = true;
    int wfStatus = WiFi.status();

    WiFi.disconnect(true);
    WiFi.setHostname( stacID.c_str() );         // set the STAC hostname
    WiFi.setSleep(false);                       // have to set this mode or else the display goes nuts when polling

    WiFi.mode(WIFI_STA);                        // set WiFi station mode. Needs to be done *after* setHostmame & other set calls

    wfTimeout = millis() + WIFI_CONNECT_TIMEOUT;
    WiFi.begin(networkSSID, networkPass);
    
    while ( ( wfStatus != WL_CONNECTED ) && ( wfTimeout >= millis() ) ) {
        delay(250);                             // take a pause as it takes some time for the WiFi routines to do their thing in the backgroud
        wfStatus = WiFi.status();
    }
    
    if (wfStatus == WL_CONNECTED) {
        wifistate.wfconnect = true;
        wifistate.timeout = false;
    }
    else {
        WiFi.disconnect();
    }
    return wifistate;
    
}    // closing brace for connectToWifi()


