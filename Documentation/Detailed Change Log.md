
## Detailed Change Log

Intended for developers and others interested in the nitty gritty.
### Version 2.1

* added the ability to do OTA firmware updates via a web browser.
    * routines are in `STACUtil.h`. 
    * implemented using the `WebServer` library.
* now using the mDNS library when the STAC is operating a wiFi access point for configuration & firmware updates.
* all HTML code for configuration and OTA updates moved to 'STACsuPages.h' and `STACotaPages.h`respectively.
    * all HTML is now HTML5 compliant.
* refactoring of the initilization code in `STACProvision.h`.
* refacrtoring of `getCreds()` in `STACUtil.h` to use the WebServer library.
* redo of the display drawing routines to increase speed by reducing the number of calls to `FastLED.show()`.
    * added a function parameter to optionally refresh the display post-execution.
    * redid all the calls throughout the code to match match the above.
    * due to changing the overloads for the `drawPix()` functions, renamed functions that draw to the display using `x` & `y`  corordinates to `drawPixXY()
* a number of changes throughout to make the code more modular and to convert a number of routines to be functions rather than in-line includes.
* for Peripheral Mode, added the ability to change the Tally Mode to either Camera Operator or Talent.
* the display brightness is now set via a LUT that maps the brightness level to an absolute value as used by `FastLED()`.
* rename of `STACDisplay.h`, `STACGlyph.h` and `STACIMU.h` to `STACDisplay5.h`, `STACGlyph5.h` and `STACIMU5.h` respectively.
    * the "5" indicating use for 5 x 5 matrix displays.
* layout of the serial data dumps modified.
* as always, a number of housekeeping items addressed.

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
  * add appropriate `#include` statements to `STAC.ino`.

* Rewrite of the `loop()` tally display code to better facilitate error handling.
  * error handling should now be done in `loop()`.

* Add user configurable polling interval via the web config page.
  * add polling interval as an NVS item.
  * increment the NVS `NOM_PREFS_VERSION`.

* Correct improper use of the `Preferences` library.

* No longer reformats the NVS when doing a factory reset.
  * clears all `Preferences` name spaces instead.

* Change layout of the startup data dump.
  * display the arduino-esp32 core version used.
  * display the ESP-IDF SDK version used. 
  * show the polling interval.
  * items above the "`-----`" line are hard coded.
  * items above the "`=-=-=`" line are web config items.
  * items below the "`=-=-=`" line are run-time configurable.

* Add macros for setting the tally state on the GROVE connector.

* Set WiFi hostname to the STAC ID.

* Set the normal operating mode default polling interval to 300 ms.

* Set the Peripheral Mode polling interval to 0 ms.

* Remove left over debug code in `parseForm()`.

* Change all `drawGlyph()` `colors[]` parameter data type to `const int`.

* Remove `buttonClicked()` function in favour of native `JC_Button` library calls.
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
  
* Add filtering for erroneous `stClient` connection cases.
  * require eight consecutive errors of the same type before the glyph for that error type is displayed.
  * exception is when there is no connection at all to the STS (aka "BigOrangeX"), for which the error glyph is always displayed immediately.
  * all error counters are reset after an error glyph is displayed,

* Add setting of STS re-poll interval on errors by error type, independent of the re-poll interval; currently set as:
  * 1500 ms for "BigOrangeX"
  * 1000 ms for unknown "BigRedX"
  * 50 ms for all other errors

* Add `stClient.stop()` if we have an STS connection but the STS response timed out.

* Display the arduino-esp32 core version used on the STAC web config form.

* Display the ESP-IDF SDK version used on the STAC web config form.

* Add STAC software build number.
    * displayed in brackets after the software number on the Serial data dump and on the STAC web config form.
    * see `Documentation/Creating the build number.md` for details.
