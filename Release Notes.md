# STAC Release Notes

## 1.8

__New Feature__

* Adds Autostart Mode.
    * On startup when autostart is set, if no button press is seen for about 10 seconds, the STAC will jump right to the WiFi connect sequence and then start displaying the tally status of the active channel.
More in the User's Guide.

__Fixes__

* When configuring the STAC via a web browser, the SSID and password fields now accept spaces as given instead of converting them to '+' characters.

## 1.7.1.1

Minor bug fix release.

No changes to the STAC code.

Only change is to the modified Preferences.cpp file in the `/libraries/Preferences/ `folder.

It was revised to correct a coding error in writing to the `log_e` file in 
the `clear()` and `end()` methods.

This bug would only show in a very very rare case should a commit to the NVS memory fail when calling one of those methods.

If you did not replace the standard `Preferences.cpp` file with the modified one, there is nothing you need to do. The version running in your STAC is all good.

If you did replace the standard `Preferences.cpp` file, follow the directions in the `libraries/libraries_README.md` file using this revised `Preferences.cpp` file. Then re-upload the STAC code into your ATOM and you're good to go.

My apologies. Nothing like writing code to keep one humble.


## 1.7.1
- improve the ST server poll response time by about a gazillion. As a result...
- ...the ST\_POLL\_INTERVAL now does indeed govern the time between ST server polls
- add an information dump to the serial port on startup

## 1.7
- add support for user configurable ST Server port #.
    - Allows STAC to be used in an emulated environment; as in a [Tally Arbiter](http://www.tallyarbiter.com) client

## 1.6
- first public release
