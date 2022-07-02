    // send the serial port info dump header

    Serial.print("\r\n\r\n");                                               // clear out the serial buffer
    Serial.println("\r\n\r\n=======================================");
    Serial.println("                 STAC");
    Serial.println("   A Smart Tally ATOM Matrix Client");
    Serial.println("            by: Team STAC");
    Serial.println("https://github.com/Xylopyrographer/STAC\r\n");
    Serial.print("     Software Version: ");
    Serial.println(swVer);
    Serial.print("   Configuration SSID: ");
    Serial.println(stacID);
    Serial.println("     Configuration IP: 192.168.6.14");
    Serial.println("     -------------------------------");

    // end send the serial port info dump header
