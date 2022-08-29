
TallyState getTallyState(TState tally) {
/*  Establishes the connection to the Smart Tally server and returns the response  after the server is sent a GET request.
    The long form of the GET request returns an HTTP header, followed by the payload (status state).
    The short form of the GET request returns only the status state.
    This function only sends and responds to the short form.
     
        stClient.print("GET /tally/" +  TALLY_CHAN + "/status HTTP/1.0\r\n\r\n");   // Uses the long form of the tally status request
        stClient.print("GET /tally/" +  TALLY_CHAN + "/status\r\n\r\n");            // Uses the short form of the tally status request
*/

    int maxloops;
    bool breakFlag = false;
    String reply = "";
    tally.tState = "NO_INIT";
    tally.tConnect = false;
    tally.tTimeout = true;
    tally.tNoReply = true;

    if ( !stClient.connected() ) {
        // try to connect to the ST server
        maxloops = 0;
        while ( !breakFlag && maxloops < 10 ) {
            if ( stClient.connect(stIP, stPort, 100) ) {   // try to connect to the server, waiting up to 100 ms
                tally.tConnect = true;
                breakFlag = true;
            }
            else {
                // ST server connect attempt failed; wait 1 ms before trying again.
                maxloops++;
                delay(1);
            }
        }
        if (maxloops >= 10) {
            // we tried to get an ST server connection for (100 * 10) ms & failed
            tally.tState = "NO_STS";
            
            if ( DB_MODE ) { Serial.print( __LINE__ + 2 ); Serial.print(": In getTallyState. tally.tState = "); Serial.println(tally.tState); }
            
            return tally;               // unable to get a connection to the ST server
        }
        
    }
    tally.tConnect = true;
    
    // we have a connection to the ST server so send a status request
    stClient.print("GET /tally/");
    stClient.print(tally.tChannel);
    stClient.print("/status\r\n\r\n");

    maxloops = 0;
    while (stClient.available() == 0 && maxloops < 1000) {    // Wait a maximum of 1 second for the ST server's reply to become available
        maxloops++;
        delay(1);                                             // wait a tick before checking again if there is a response from the ST Server.
    }
    if (maxloops >= 1000) {         // response from the ST Server timed out
        tally.tState = "NO_REPLY";
        
        if ( DB_MODE ) { Serial.print(__LINE__ + 2); Serial.print(": In getTallyState. tally.tState = "); Serial.println(tally.tState); }
        
        return tally;
    }
    
    stClient.setTimeout(0);                    // need to override the default 1 sec blocking of the stClient.readString() function
    while (stClient.available() > 0) {
        reply += stClient.readString();        // we have a response from the server
    }
    reply.trim();                             // knock any junk off the front and back of the reply (trim is done in-place)
    
    if ( reply.length() == 0 ) {
        tally.tState = "EMPTY_REPLY";
    }
    else {
        tally.tState = reply;
        tally.tNoReply = false;
    }
    tally.tTimeout = false;

    if ( DB_MODE ) { Serial.print( __LINE__ + 2 ); Serial.print(": In getTallyState. tally.tState = "); Serial.println(tally.tState); }

    return tally;                              // and head back to the barn

}   // closing brace for getTallyState()

