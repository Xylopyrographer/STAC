
void sendHeader() { 
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
    Serial.print(   "    Setup SSID: "); Serial.println( stacID );
    Serial.println( "    Setup URL: http://setup.local" );
    Serial.println( "    Setup IP: 192.168.6.14" );
    Serial.print(   "    MAC: "); Serial.println( WiFi.macAddress() );
    Serial.println( "   --------------------------------" );
    Serial.flush();

    return;

}   // end sendHeader()
