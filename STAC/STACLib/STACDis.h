/*~~~~ Display drawing routines ~~~~*/

void disSetBright(uint8_t level) {
    // replaces M5.dis.setBrightness()
    FastLED.setBrightness(level);
    FastLED.show();
    return;
}

void disDrawPix(uint8_t xpos, uint8_t ypos, CRGB color) {
    // replaces M5.dis.drawpix()
    if ( (xpos >= 5) || (ypos >= 5) ) {
        return;
    }
    leds[xpos + ypos * 5] = color;
    FastLED.show();
    return;
}

void disDrawPix(uint8_t dot, CRGB color)
{
    if (dot >= 25) {
        return;
    }
    leds[dot] = color;
    FastLED.show();
    return;
}

void disClear() {
    // replaces M5.dis.clear()
    for (int8_t i = 0; i < 25; i++) {
        leds[i] = 0;
    }
    FastLED.show();
    return;
}

void disFillPix(CRGB color) {
    // replaces M5.dis.fillpix()
    for (int8_t i = 0; i < 25; i++) {
        leds[i] = color;
    }
    FastLED.show();
    return;
}


void drawGlyph(uint8_t glyph[], const int colors[]) {
/*  Draws a bit mapped image onto the Atom display.
      - glyph[] is the bitmap array of the glyph to draw.
      - in the pair of colors[]; a "0" at glyph[i] will draw color[0]; a "1", color[1]
      - or if you like, "0" and "1" in glyph[i] are the background/foreground color selectors at that pixel location.
*/

  for (int i = 0; i <= 24; i++) {
    disDrawPix(i, colors[glyph[i]]);
  }
}   // closing brace for drawGlyph()

void drawOverlay(uint8_t glyph[], int ovColor) {
/*  Overlays a bit mapped image onto the Atom display.
      - glyph[] is the bitmap array of the glyph to draw.
      - a "1" at glyph[i] will draw ovColor at that pixel location; a "0" will leave that pixel as it is
      - ovColor is a CGRB type
*/

    for (int i = 0; i <= 24; i++) {
        if (glyph[i] == 1) disDrawPix(i, ovColor);
    }

}   // closing brace for drawOverlay()

void flashDisplay(int count, int rate, int brightness) {
/*  Flashes the display brightness between 0 and the "brightness" level passed.
    - "count" is the number of times to flash the display
    - "rate" is the speed at which the display flashes in ms
    - function is blocking
    - on exit, the display brightness remains at the brightness level passed
*/

    for (int i = 0; i < count; i++) {
        disSetBright(0);
        delay(rate / 2);
        disSetBright(brightness);
        delay(rate / 2);
    }
    
}   // closing brace for flashDisplay()

void pulsePix(bool state, CRGB colour) {
/*  turns the four outermost pixels of the display off or on
 *  depending on the state of "state" and using the colour "colour"
 */
    if (state) {
        disDrawPix(0, colour);
        disDrawPix(4, colour);
        disDrawPix(20, colour);
        disDrawPix(24, colour);
    }
    else {
        disDrawPix(0, GRB_COLOR_BLACK);
        disDrawPix(4, GRB_COLOR_BLACK);
        disDrawPix(20, GRB_COLOR_BLACK);
        disDrawPix(24, GRB_COLOR_BLACK);
    }
    
}   // closing brace for pulsePix()

/*~~~~ End Display drawing routines ~~~~*/


uint8_t updateBrightness(Preferences& prefSpace, const char * nameSpace, const char * keyName, uint8_t brightness) {
/*  Allows the user to select the brightness of the display. 
 *      - function will return after a period of inactivity, restoring
 *         the brightness to what it was prior to the call
 *      - any change to the brightness level is saved to NVS using 
 *         the "prefsSpace" object in the namespace "namespace" under the key "keyname"
*/
    unsigned long updateTimeout;                    // delete this line if you don't want to bug out after a period of inactivity
    uint8_t brightnessNow = brightness;
    
    drawGlyph(GLF_EM, brightnesschange);                            // fill the display...
    drawOverlay(GLF_EN, GRB_COLOR_BLACK);                           // blank out the inside three columns...
    drawOverlay(glyphMap[brightness / 10], GRB_COLOR_ORANGE);       // and overlay the brightness setting number
        
    while ( dButt.read() );                     // don't proeeed until the button is released.
    updateTimeout = millis() + 30000;               // delete this line if you don't want to bug out after a period of inactivity

    do {
        if (millis() >= updateTimeout) {         // delete this if clause if you don't want to bug out after a period of inactivity
            brightness = brightnessNow;          // we timed out, user didn't confirm the change, restore currentBrightness as it was on entry...
            disSetBright(brightness);            // reset the display to that brightness...
            dButt.read();
            return brightness;                   // and head back to the barn
        }

        dButt.read();                                   // read & refresh the state of the button
        if ( dButt.wasReleased() ) {
            updateTimeout = millis() + 30000;           // delete this line if you don't want to bug out after a period of inactivity
            if (brightness >= 60) brightness = 10;
            else brightness = brightness + 10;                    
            disSetBright(brightness);                            // set the display to the new brightness setting
            drawOverlay(GLF_EN, GRB_COLOR_BLACK);                        // blank out the inside three columns...
            drawOverlay(glyphMap[brightness / 10], GRB_COLOR_ORANGE);    // and overlay the new brightness setting number
        }

        if ( dButt.pressedFor(1500) ) {                             // user wants to confirm the change and exit so...
            drawGlyph(GLF_CK, gtgcolor);                            //   let the user know we're good to go
            if (brightnessNow != brightness) {                      // if the brightness level changed...
                prefSpace.begin(nameSpace, PREFS_RW);               //   open up the pref namespace for R/W
                prefSpace.putUChar(keyName, brightness);            //   save the new brightness in NVS
                prefSpace.end();                                    //   close the prefs namespace
            }
            while ( dButt.read() );                             // don't proeeed until the button is released.
            delay(500);
            return brightness;
        }
    } while (true);

    dButt.read();
    return brightness;
    
}    // closing brace for updateBrightness()


//  ---- EOF ----
