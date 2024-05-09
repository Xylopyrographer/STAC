
// STACInfoHeader.h

void sendHeader( wifi_info_t &_wifiStat ) {
    // send the serial port info dump header
    Serial.print( "\r\n\r\n" );
    Serial.println( "==========================================" );
    Serial.println( "                  STAC" );
    Serial.println( "        A Roland Smart Tally Client" );
    Serial.println( "             by: Team STAC" );
    Serial.println( "      github.com/Xylopyrographer/STAC" );
    Serial.println();
    Serial.print(   "    Version: " ); Serial.println( swVer );
    Serial.print(   "    Core: " ); Serial.println( ardesp32Ver );
    Serial.print(   "    SDK: " ); Serial.println( espidfsdk );
    Serial.print(   "    Setup SSID: "); Serial.println( _wifiStat.stacID );
    Serial.println( "    Setup URL: http://setup.local" );
    Serial.println( "    Setup IP: 192.168.6.14" );
    Serial.print(   "    MAC: "); Serial.println( WiFi.macAddress() );
    Serial.println( "  --------------------------------------" );
    Serial.flush();
    return;
}   // end sendHeader()

void sendWifiGood() {
    Serial.print(   "    WiFi connected. IP: " ); Serial.println( WiFi.localIP() );
    Serial.println( "==========================================" );
    Serial.flush();
    return;
}

void sendModelChange() {
    Serial.println( " ***       Switch Model changed       ***" );
    Serial.println( " ***  User run-time parameters reset  ***" );
    Serial.println( "      ------------------------------" );
    Serial.flush();
    return;
}

void sendConfigDone() {
    Serial.println( "   ***** Configuration complete *****" );
    Serial.println( "==========================================" );
    Serial.flush();
    return;
}

void sendReset() {
    Serial.println( "   ***** Performing factory reset *****" );
    Serial.println( "==========================================\r\n\r\n" );
    Serial.flush();
    return;
}

void sendNewPrefs() {
    Serial.println( "    ****** New preferences layout ******");
    Serial.println( "    ****** Configuration required ******");
    Serial.flush();
    return;
}

void sendFooter( stac_ops_t &_stacOps, wifi_info_t &_wifiStat ) {
    Serial.print(   "    WiFi Network SSID: " ); Serial.println( _wifiStat.networkSSID );
    Serial.print(   "    Switch IP: " ); Serial.println( _wifiStat.stIP );
    Serial.print(   "    Switch Port #: " ); Serial.println( _wifiStat.stPort );
    Serial.println( "  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" );
    Serial.print(   "    Configured for Model: " ); Serial.println( _stacOps.tModel );
    Serial.print(   "    Active Tally Channel: " );
    if ( _stacOps.tModel == "V-60HD" ) {
        Serial.println( _stacOps.tChannel );
        Serial.print( "    Max Tally Channel: " ); Serial.println( _stacOps.tChanMax );
    }
    else {  /* switch model is V-160HD */
        if ( _stacOps.tChannel > 8 ) {
            Serial.print( "SDI " ); Serial.println( _stacOps.tChannel - 8 );
        }
        else {
            Serial.print( "HDMI " ); Serial.println( _stacOps.tChannel );
        }
            Serial.print( "    Max HDMI Tally Channel: " ); Serial.println( _stacOps.tChanHDMIMax );
            Serial.print( "    Max SDI Tally Channel: " ); Serial.println( _stacOps.tChanSDIMax );
    }
    Serial.print(   "    Tally Mode: " ); Serial.println( _stacOps.ctMode ? "Camera Operator" : "Talent" );
    Serial.print(   "    Auto start: "); Serial.println( _stacOps.autoStart ? "Enabled" : "Disabled" );
    Serial.print(   "    Brightness Level: " ); Serial.println( _stacOps.disLevel );
    Serial.print(   "    Polling Interval: " ); Serial.print( _stacOps.stsPollInt ); Serial.println( " ms" );
    Serial.println( "==========================================" );
    Serial.flush();
    return;
}

void sendPeripheral( bool &_pmCTmode, uint8_t &_pmBright ) {
    Serial.println( ">>>> OPERATING IN PERIPHERAL MODE <<<<" );
    Serial.print(   "    Tally Mode: "); Serial.println( _pmCTmode ? "Camera Operator" : "Talent" );
    Serial.print(   "    Brightness Level: " ); Serial.println( _pmBright );
    Serial.println( "=======================================" );
    Serial.flush();
    return;
}

void sendOTA() {
    Serial.println( "    ***** Updating STAC firmware *****" );
    Serial.println( "    Connect to the STAC SSID WiFi AP," );
    Serial.println( "    then browse to http://update.local");
    Serial.println( "===========================================" );
    Serial.flush();
    return;
}

void sendOTAStat( bool udOK, String &udFileName, size_t udSize, String &udStatus ) {
/* called by the firmware update lambda function to send
 *  results of the firmware update to the Serial port.
*/
    if ( udOK ) {
            Serial.println( "  ******* Firmware update done *******" );
            Serial.print(   " File: "); Serial.println( udFileName );
            Serial.print(   " Bytes written: "); Serial.println( udSize );
            Serial.print(   " Status: "); Serial.println( udStatus );
    }
    else {
            Serial.println( " ******* FIRMWARE UPDATE FAILED *******" );
            Serial.print(   " Tried with file: " ); Serial.println( udFileName );
            Serial.print(   " Reason: " ); Serial.println( udStatus );
            Serial.println( " Ensure the correct \"STAC_XXXXX.BIN\"" );
            Serial.println( " file was selected" );
    }
    Serial.println( "              Restarting..." );
    Serial.println( "=========================================\r\n\r\n" );
    Serial.flush();

    return;

}   // end udSerialStat()


//  --- EOF ---
