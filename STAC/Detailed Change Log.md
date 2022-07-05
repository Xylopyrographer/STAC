
## Detailed Change Log

Intended for developers and others interested in the nitty gritty.

### Version 2.0

* Require arduino-esp32 core 2.0.3 or greater
    * there are irreconcilable breaking changes in WiFiClient from core 1.0.6

* Replace M5Stack ATOM libraries:
  * M5Stack are making library changes that are not backward compatible.
  * use JC_Button library for the display button;
  * use I2C_MPU6886 library for the IMU;
  * use FastLED library for the display;
    * replace "M5.dis.X" functions with bespoke display drawing routines.
  * directly initialize Serial.
  
* Break code into separate `.h` files by function group into a new STACLib folder.
  * add appropriate #include statements to STAC.ino.

* Rewrite of the loop() tally display code to better facilitate error handling.
  * error handling should now be done in loop().

* Add user configurable polling interval via the web config page.
  * add polling interval as an NVS item.
  * increment the NVS NOM_PREFS_VERSION.

* Correct improper use of the Preferences library.

* No longer reformats the NVS when doing a factory reset.
  * clears all `Preferences` name spaces instead.

* Change layout of the startup data dump.
  * show the polling interval.
  * items above the "`-----`" line are hard coded.
  * items above the "`=-=-=`" line are web config items.
  * items below the "`=-=-=`" line are run-time configurable.

* Add macros for setting the tally state on the GROVE connector.

* Set WiFi hostname to the STAC ID.

* Set the normal operating mode default polling interval to 300 ms.

* Set the Peripheral Mode polling interval to 0 ms.

* Remove left over debug code in `parseForm()`.

* Change all drawGlyph() 'colors[]' parameter data type to 'const int'.

* Remove buttonClicked() function in favour of native JC_Button library calls.
  * remove all references to `btnWas` & `btnNow`.

* Change `stIP` to use data type `IPAddress` instead of `char[]`.

* Fix the "set" order in `STACWiFi.h` so setting the hostname works with core v2.0.3.

* Fix a coding error in `getCreds()` in `STACWeb.h` that greatly improves error handling.

* Add `fetchInfo()` function to `STACUtil.h`.
  * interesting for debugging; not used in normal operation.

* With core v2.0.3 & greater, the default WiFi authentication/encryption 
    mode of the STAC when being provisioned is now WAP2-PSK (AES).

* Rework of the error handling in `loop()`.

* Add "BigOrangeX" display error as notification that there is no STS connection at all (Camera operator mode only).
  
* Add filtering for erroneous connection cases.
  * require eight consecutive errors of the same type before the glyph for that error type is displayed.
  * exception is when there is no connection at all to the STS (aka "BigOrangeX"), for which the error glyph is always displayed immediately.
  * all error counters are reset after an error glyph is displayed,

* Add setting of STS re-poll interval on errors by error type, independent of the re-poll interval; currently set as:
  * 1500 ms for "BigOrangeX"
  * 1000 ms for unknown "BigRedX"
  * 50 ms for all other errors

* Stop the stClient connection if we had a connection but the response timed out.

* Add large number of of log_e() debug statements
  * will probably remove most before final v2.0 release.
