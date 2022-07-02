/* STAC Web Server Functions */

String decodeURL(const char* ptr) {
/* Converts a URL encoded string back to its plain text form 
   
   Usage: 
    const char* stringToDecode = "abcd%24%26%40%29%28-987";
    String decodedString = decodeURL(stringToDecode);
     - or, putting it another way -
    String stringToDecode = "abcd%24%26%40%29%28-987";
    String decodedString = decodeURL(stringToDecode.c_str());

   Example:
    Encoded URL: abcd%24%26%40%29%28-987
    Decoded plain text: abcd$&@)(-987

   Modified 2021-04-23 to decode '+' chars to spaces.

Credit to reddit user u/truetofiction
Source: https://www.reddit.com/r/arduino/comments/m2uw0g/replacing_hex_with_ascii/
*/

    const size_t size = strlen(ptr);                // size of the string, in characters
    String buffer;  
    buffer.reserve(size);                           // buffer on heap in String object

     for (size_t i = 0; i < size; i++) {
        char c = ptr[i];                            // get character       
        if (c == '%' && i + 2 < size) {             // if hex follows...
            char tmp[3]{};
            memcpy(tmp, &ptr[i + 1], 2);            // make a temporary copy of the hex data (i+1, i+2)
            c = (char) strtol(tmp, nullptr, 16);    // convert from hex string to long (ASCII)
            i += 2;                                 // skip the characters we just processed for the next loop
        }
        else {
            if (c == '+') {
                c = ' ';                            // decode a + to a space
            }
        }
        buffer.concat(c);                           // save the current character
    }
    return buffer;

}   // closing brace for decodeURL()


void sendForm(WiFiClient theClient, String &swVer) {
/* Sends the HTML configuration form web page to the user's browser
 HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
 and a content-type so the client knows what's coming, then a blank line
 then an optional payload (data) and then a blank line
*/
  
    theClient.println("HTTP/1.1 200 OK");
    theClient.println("Content-type: text/html");
    theClient.println("Accept-Language: en-us");
    theClient.println("Cache-Control: no-store");
    theClient.println("Connection: Keep-Alive");
    theClient.println();
    theClient.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>");
    theClient.println("<link rel=\"icon\" href=\"data:,\">");   // prevent 'favicon' requests
    theClient.println("<body><h1 align=\"center\"><font face=\"Helvetica, Arial, sans-serif\">STAC Configuration</font></h1><div align=\"center\">");
    theClient.println("<form method=\"post\"><font face=\"Helvetica, Arial, sans-serif\">");
    theClient.println("<label for=\"SSID:\">Network SSID:</label><input id=\"SSID\" name=\"SSID\" autofocus=\"\" required=\"\" type=\"text\" maxlength=\"32\"><br><br>");
    theClient.println("<label for=\"Password:\">Password:</label><input id=\"pwd\" name=\"pwd\" type=\"text\" size=\"20\" maxlength=\"63\"><br><br>");
    theClient.println("<label for=\"Smart Tally IP:\">Smart Tally IP:</label><input id=\"stIP\" name=\"stIP\" size=\"15\" ");
    theClient.println("pattern=\"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$\" required=\"\" type=\"text\"><br><br>");
    theClient.println("<label for=\"stPort\">port #:</label><input id=\"stPort\" name=\"stPort\" size=\"5\" min=\"0\" max=\"65353\" required=\"\" type=\"number\" value=\"80\"><br><br>");
    theClient.println("<label for=\"stChan\"># of channels:</label><input id=\"stChan\" name=\"stChan\" size=\"3\" min=\"1\" max=\"8\" required=\"\" type=\"number\" value=\"6\"><br><br>");
    theClient.println("<label for=\"pollTime\">Polling interval (ms):</label><input id=\"pollTime\" name=\"pollTime\" size=\"6\" min=\"175\" max=\"2000\" required=\"\" type=\"number\" value=\"300\"><br><br>");
    theClient.println("<input value=\"Submit\" type=\"submit\">&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<input type=\"Reset\"></font></form><br>");
    theClient.print("<p align=\"center\"><font face=\"Helvetica, Arial, sans-serif\">STAC software version: ");
    theClient.print(swVer);
    theClient.println("</font></p></body>");
    theClient.println();
    theClient.println();
     
}   // closing brace for sendForm()

void send404(WiFiClient theClient) {
//  Sends a "not found" HTML response to the user's web browser

    theClient.println("HTTP/1.1 404 Not Found");
    theClient.println("Cache-Control: no-store, max-age=0");
    theClient.println("Connection: Keep-Alive");
    theClient.println();
    theClient.println();

}   // closing brace for send404()

void sendtftf(WiFiClient theClient) {
/*  Sends the HTML page acknowledging receipt of the 
 *  configuration data back to the users web browser.
 */

    theClient.println("HTTP/1.1 200 OK");
    theClient.println("Content-type: text/html");
    theClient.println("Cache-Control: no-store");
    theClient.println("Clear-Site-Data: \"*\"");
    theClient.println("Connection: closed");
    theClient.println();
    theClient.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=0.86, maximum-scale=1.0, minimum-scale=0.86\"></head><body>");
    theClient.println("<div align=\"center\"><h1><font face=\"Helvetica, Arial, sans-serif\">");
    theClient.println("STAC configuration received.<br>");
    theClient.println("</font></h1><font size=\"+2\" face=\"Helvetica, Arial, sans-serif\">");
    theClient.println("Close this window and reconnect <br>");
    theClient.println("to your regular WiFi network.<br><br>");
    theClient.println("Consult the manual <br>");
    theClient.println("if you need to reconfigure this device.");
    theClient.println("</font></div></body></html>");
    theClient.println();
    theClient.println();
    theClient.flush();      // wait and make sure everything has been sent 
                            // (important to do this time as we shut down the AP after sending this response back to the user's browser)
    
}   // closing brace for sendtftf()

provData_t parseForm(String &formData) {
/*  Extracts the data from the POST response returned by the client web browser
 *      - Example last line of a POST response:
 *        SSID=SandyShores222&pwd=flatBUSH%40%26%24%29%28%3B%3A%2F-3546&stIP=192.168.2.132&stPort=80&stChan=7
 */

    String payload = "";
    String tempString;
    unsigned int llIndex;
    provData_t stCred;
 
    llIndex = formData.lastIndexOf("SSID=");                                // the last line of the POST reply is the only thing we want
    if (llIndex != -1) payload = formData.substring(llIndex);               // throw away everything except the last line
    else payload = formData;                                                // should never need this check but just in case...
    payload.trim();                                                         //   knock any junk off the front and back of the last line of the reply
    
    // parse the payload from the end of the string to the start

    tempString = payload.substring(payload.lastIndexOf("&") + 10);          // pull out the polling interval; 10 = length of "&pollTime="...
    stCred.pPollInt = (unsigned long)tempString.toInt();                        // convert (cast) it to an 4-byte uint and save it
    payload = payload.substring(0, payload.lastIndexOf("&"));               // throw away the part of payload that held the poll time   
    
    tempString = payload.substring(payload.lastIndexOf("&") + 8);           // pull out the max # of tally channels; 8 = length of "&stChan="...
    stCred.ptChanMax = (uint8_t)tempString.toInt();                         //   convert (cast) it to an 8-bit uint and save it   
    payload = payload.substring(0, payload.lastIndexOf("&"));               //   throw away the part of payload that held the tally channel

    tempString = payload.substring(payload.lastIndexOf("&") + 8);           // pull out the port #; 8 = length of "&stPort="...
    stCred.pPort = (uint16_t)tempString.toInt();                            //   convert (cast) it to a uint16_t and save it      
    payload = payload.substring(0, payload.lastIndexOf("&"));               //   throw away the part of payload that held the port number

    stCred.pSwitchIP = payload.substring(payload.lastIndexOf("&") + 6);     // pull out IP address of the Roland switch and save it; 6 = length of "&stIP=" 
    payload = payload.substring(0, payload.lastIndexOf("&"));               //   throw away the part of payload that held the IP address
    
    tempString = payload.substring(payload.lastIndexOf("&") + 5);           // pull out the WiFi password; 5 = length of "&pwd="...
    stCred.pPass = decodeURL( tempString.c_str() );                         //   decode the password and save it
    payload = payload.substring(0, payload.lastIndexOf("&"));               //   throw away the part of payload that held the password
    
    tempString = payload.substring(5);                                      // pull out the SSID of the WiFi network; 5 = length of "SSID="
    stCred.pSSID = decodeURL(tempString.c_str());                           //   decode the SSID and save it

    return stCred;

}   // closing brace for parseForm()


provData_t getCreds(String &stacUUID, String &swVersion) {
/*  Big honkin' routine that retrevies the configuration/provisiong data for 
 *  the WiFi network that the Roland switch is connected to.
 *      - sets the STAC as a WiFi access point using a device unique SSID
 *      - waits for the user's web browser to connect
 *      - sends the form to the browser for the user to fill out
 *      - waits for the form to be returned
 *      - confirms to the user that the form was received (on the browser and on the STAC)
 *      - shuts down the access point
 *      - returns the configuration/provisiong data to the calling function
 *      - and then goes for a beer.
 *
 *  The IP and password for the access point are set in this function.
 */

    provData_t stProv;                          // structure to hold the data from the web page form from the user

    const char* password = "1234567890";        // the password for our WiFi AP
    const IPAddress configIP(192, 168, 6, 14);        // sets the IP...
    const IPAddress gateway(192, 168, 6, 14);         // ...gateway...
    const IPAddress NMask(255, 255, 255, 0);          // ...and network mask of the AP
    WiFiServer server(80);                      // init a server class & set the AP to listen for inbound connections on port 80
    bool hideSSID = false;                      // false to broadcast the SSID of our AP network, true to hide it
    uint8_t wifiChan = 1;                       // WiFi channel to use. Default is 1, max is 13
    uint8_t maxConnect = 1;                     // maximum # of client connections allowed to our AP
    WiFiClient scClient;                        // we need a WiFi client to talk to the user's web browser
    
    String clData = "";                         // for holdng the input from the user's web browser
    bool formReceived;                          // true when we get the POST form back from the user's web browser

    // set up the WiFi access point
    WiFi.mode(WIFI_AP);                                                 // configure the WiFi mode to AP 
    while (WiFi.getMode() != 2) delay(10);                              // best practise to ensure we're configured in AP mode. "2" = "WIFI_AP_CONNECTED" (or something like it).
    WiFi.softAP(stacUUID.c_str(), password, wifiChan, hideSSID, maxConnect);  // set the SSID, password, etc. of our AP (all the WiFi stuff)
    WiFi.softAPConfig(configIP, gateway, NMask);                        // set the IP address, gateway and network mask of our AP (all the networking stuff)
    server.begin();                                                     // fire up the AP server

    // let's go fetch the info from the user's web browser...
    formReceived = false;
    do {
        scClient = server.available();                                  // listen for incoming clients
        if (scClient) {                                                 // if a new client connects...
            clData = "";                                                // clear out any old client data received                        
            while (scClient.connected()) {                              // loop while the client's connected
                if (scClient.available()) {                             // if there's something from from the client...
                    while (scClient.available()) {                      // while there is still stuff being sent by the client... 
                        clData += scClient.readString();                //   suck in the entire response from the client (timeout for the .readString() is set above)
                    }
                    if (clData.indexOf("GET / HTTP/") >= 0) {           // goody! a root request...
                        formReceived = false;                           // but we're stil waiting on the form...
                        sendForm(scClient, swVersion);                  // so send the form
                        break;                                          // break out of while (scClient.connected())
                    }           
                    else if (clData.indexOf("POST /") >= 0) {           // incoming form response from the user's web browser!!
                        formReceived = true;                            // this is the data we're looking for!
                        sendtftf(scClient);                             // acknowledge receipt of the form and send the "so long and thanks for all the fish" page
                        break;                                          // break out of while (scClient.connected())
                    }
                    else {                                              // the client is asking for something we don't have
                        formReceived = false;                           // this isn't the reply we're looking for.                   
                        send404(scClient);                              // send a no can do response
                        break;                                          // break out of while (scClient.connected())
                    }
                }  // closing brace for if (scClient.available())               
            }  // closing brace for while (scClient.connected())
        }  // closing brace for if (scClient)
    } while (!formReceived);                                            // stay here forever until the POST is received
    
    // we got the goods so, shut down the access point
    scClient.stop();                        // stop listening for incoming stuff
    delay(500);                             // seems to be needed for the client browser to accept the final response
    server.end();                           // close the connection
    delay(500);                             // seems to be needed for the client browser to accept the final response
    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_NULL);              // turn off the WiFi AP
    stProv = parseForm(clData);             // extract the data from the POSTed form...
    
    return stProv;                          // and head back to the barn

}   // closing brace for getCreds()


/* End STAC Web Server Functions */
