void changeTallyChannel() {
/*  Allows the user to select the active tally channel. 
 *      - function will return after a period of inactivity, restoring
 *        the active channel to what it was prior to the call
*/

    unsigned long updateTimeout;
    uint8_t tallyChanNow = tallyStatus.tChannel;     // keep the tally channel state we had on entry

    drawGlyph(glyphMap[tallyStatus.tChannel], tallychangecolor);    // display the current tally channel
   
    while ( dButt.read() );                     // don't proeeed until the button is released.
    updateTimeout = millis() + 30000;           // delete this line if you don't want to leave after a period of inactivity
     
    do {
        if ( millis() >= updateTimeout ) {          // remove this 'if' clause if you don't want to leave after a period of inactivity
            tallyStatus.tChannel = tallyChanNow;    // we timed out, user didn't confirm the change, so restore the tally channel as it was on entry...
            return;                                 //   and head back to the barn
        }
        dButt.read();                              // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + 30000;      // kick the timeout timer (delete this line if you don't want to leave after a period of inactivity)
            if (tallyStatus.tChannel >= tallyStatus.tChanMax) tallyStatus.tChannel = 1; // loop the channel number back to 1 if it would otherwise exceed the max channel #
            else tallyStatus.tChannel++;
            drawGlyph(glyphMap[tallyStatus.tChannel], tallychangecolor);
        }
        if (dButt.pressedFor(1500)) {                              // user wants to confirm the change and exit so...
            drawGlyph(GLF_CK, gtgcolor);
            if (tallyChanNow != tallyStatus.tChannel) {                 // if the active channel changed...
                stcPrefs.begin("STCPrefs", PREFS_RW);                   //   open up the pref namespace for R/W
                stcPrefs.putUChar("talChan", tallyStatus.tChannel);     //   save the new tally channel in NVS
                stcPrefs.end();                                         //   close the prefs namespace
            }    
            while ( dButt.read() );                             // don't proeeed until the button is released.
            delay(500);
            return;
        }
    } while (true);
    dButt.read();
    return;
    
}   // closing brace for changeTallyChannel()

void changeTallyMode() {
/*  Allows the user to flip the STAC operaing mode
    between "camera operator" and "talent"
 *      - function will return after a period of inactivity, restoring
 *        the operating mode to what it was prior to the call
*/
    unsigned long updateTimeout;
    bool ctModeNow = ctMode;

    if (ctMode) drawGlyph(GLF_C, tallymodecolor);   // display the current tally mode...
    else drawGlyph(GLF_T, tallymodecolor);
 
    while ( dButt.read() );                     // don't proeeed until the button is released
    updateTimeout = millis() + 30000;           // initialize the timeout timer 
     
    do {
        if (millis() >= updateTimeout) {
            ctMode = ctModeNow;                     // we timed out, user didn't confirm the change, restore the ctMode as it was on entry...
            return;                                 // and head back to the barn
        }

        dButt.read();                                       // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + 30000;               // kick the timeout timer
            ctMode = !ctMode;                               // btn clicked so we want to flip the current ctMode...
            if (ctMode) drawGlyph(GLF_C, tallymodecolor);   //   and display the new operating mode
            else drawGlyph(GLF_T, tallymodecolor);
        }
        if (dButt.pressedFor(1500)) {              // user wants to confirm the change and exit so...
            drawGlyph(GLF_CK, gtgcolor);
            if (ctModeNow != ctMode) {                  // if the tally mode changed...
                stcPrefs.begin("STCPrefs", PREFS_RW);   //   open up the pref namespace for R/W
                stcPrefs.putBool("ctMde", ctMode);      //   save the new ctMode in NVS
                stcPrefs.end();                         //   close the prefs namespace
            }
            while ( dButt.read() );               // don't proeeed until the button is released
            delay(500);
            return;
        }
    } while (true);

}   // closing brace for changeTallyMode()

void changeStartupMode() {
/*  Allows the user to flip the STAC startup mode
 *  between "Auto" and "Standard"
 *      - function will return after a period of inactivity, restoring
 *        the startup mode to what it was prior to the call
*/
    unsigned long updateTimeout;
    bool suModeNow = autoStart;
    if (autoStart) drawGlyph(GLF_A, startchangecolor);     // display the current startup mode
    else drawGlyph(GLF_S, startchangecolor);

    while ( dButt.read() );                     // don't proeeed until the button is released
    updateTimeout = millis() + 30000;               // initialize the timeout timer 
     
    do {
        if (millis() >= updateTimeout) {
            autoStart = suModeNow;                  // we timed out, user didn't confirm the change, restore the startup mode as it was on entry...
            return;                                 // and head back to the barn
        }

        dButt.read();                           // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + 30000;                      // kick the timeout timer
            autoStart = !autoStart;                                // btn clicked so we want to flip the current startup mode...
            if (autoStart) drawGlyph(GLF_A, startchangecolor);     // and display the new operating mode
            else drawGlyph(GLF_S, startchangecolor);
        }
        if ( dButt.pressedFor(1500) ) {              // user wants to confirm the change and exit so...
            drawGlyph(GLF_CK, gtgcolor);
            if (suModeNow != autoStart) {               // if the startup mode changed...
                stcPrefs.begin("STCPrefs", PREFS_RW);   //   open up the pref namespace for R/W              
                stcPrefs.putBool("aStart", autoStart);  //   save the new autoStart mode in NVS
                stcPrefs.end();                         //   close the prefs namespace
            }
            while ( dButt.read() );                 // don't proeeed until the button is released
            delay(500);
            return;
        }
    } while (true);

}   // closing brace for changeStartupMode()
