# STAC Release Notes
## v2.2.0

__New__

Add support for the funky Roland V-160HD Streaming Video Switch. (Go Team!) But, it does require the V-160HD be running at least version 3 of the System Software.

Any feedback would be appreciated!

Check the new info in the *STAC Users Guide*, especially the section on *First Time Setup*.

__Improvements__

* When doing an OTA firmware update, the serial monitor output has a couple of "How To" info lines.

__Note__

* Updating to this version will require the STAC to be reconfigured. Refer to that section of the *STAC Users Guide* for details.

## v2.1.1

__Bug Fix__

* _STAC ID_
 
  * If the ESP-32 chip MAC address has trailing zeros, the STAC ID would be truncated. Fixed.

__New__

* Added *"Using CoolTerm for STAC Information.md"* in the *Documentation* folder. CoolTerm is a great little app for viewing the info sent out by the STAC during startup. Check it out!

* The *"README.md"* file contains information on how to install the STAC software to an ATOM directly without the need to install a development environment. Zero programming knowledge needed!. Plug, play and go!
 

## v2.1

__New Features__

* _OTA Software Updates_
 
  * After installing this version, it will be possible to update the STAC software over WiFi using a web browser. This is referred to as an *over the air* or OTA update. Details are in the the *"STAC Users Guide"*.
  * As a result of this change, the STAC will need to be setup again after installing this version.

* _mDNS Support_

  * The mDNS protocol is now supported when the STAC opens up a WiFi hotspot for setup or a software update by pointing a browser to `setup.local` and `update.local` respectively. More information in the *"STAC Users Guide"*.

* _Camera Operator Tally Display in Peripheral Mode_

  * The Tally Display Mode when the STAC is in Peripheral Mode can now be set to either Camera Operator or Talent. More info in, you guessed it, the *"STAC Users Guide"*.


__Improvements__

  * After doing a Factory Reset, the STAC will "park" and not reset itself which would then fire up a hotspot and wait for setup information. 
    
  * Revised the format of the information dump sent to the serial port on startup. The IP address assigned to the STAC is now shown as well.

  * Added a bit more information about the STAC to the Setup browser page.

  * Other internal kibbles and bits tweaked to speed things up—yada, yada...


## v2.0

__New Features__

* _Configurable Polling Interval_
  
  * The amount of time, in milliseconds, between polls of the Smart Tally Server can now be set when configuring the STAC.
  * As a result, if an older version of STAC is running, the STAC will need to be reconfigured using a web browser after installing this update. Refer to the *"STAC Users Guide"*.

__Improvements__

  * The STAC ID is now used as the WiFi hostname, making each STAC uniquely identifiable in the attached devices table of the WiFi router.

  * The WiFi authentication/encryption mode of the STAC when it is being provisioned is now WAP2-PSK (AES).

  *  When operating in busy networks, the "Big Purple X" won't flicker, only activating when there is a number of consecutive communication errors with the Smart Tally Server.

  * Added a "Big Orange X" display error when the STAC cannot connect to the Smart Tally Server. Helps with troubleshooting to distinguish between "Big Purple X" and "Purple Question Mark" type errors. More info in the *"STAC Users Guide"*.

  * Revised the format of the information dump sent to the serial port on startup.
  
  * A bunch of house cleaning and other tweaks to speed things up and what not. You know, the usual stuff developers do when realizing they shouldn't have done something a certain way to begin with.
  
  * Now based on arduino-esp32 core v2.0.3 or greater.
    * To compile the code, arduino-esp32 core v2.0.3 as a minimum is required. Core v1.0.6 is no longer supported.

  * No longer requires modified core or M5Stack libraries.
    * But does require new libraries as per the `README.md` file.   
 
  * A new *"Detailed Change Log"* document intended for developers in the `Documentation` folder. It's the TLDR; version of this document.
   
  * A new *"Creating the build number"* document intended for developers in the `Documentation` folder.


## v1.10

__New Features__

* _Adds Peripheral Mode_
    * Allows one STAC to be connected to another via their GROVE ports. The STAC configured in Peripheral Mode operates as if in Talent Mode but it receives its tally information from the connected STAC without the need to establish a WiFi connection to the Roland device. More in the *"STAC User's Guide"* and the *"Peripheral Mode Application Note"*.

* _Adds Configuration Check_
    * Previously, the configuration of the STAC would be reset with each software update. After updating to version 1.10 you will have to reconfigure the STAC but from then on new software updates will do a check to see if the existing configuration is compatible with that version. If so, the existing configuration information will be retained. Too cool 🤯.
 


## v1.9.1

__Fixes__

Fixes the display glitches during polling of the Smart Tally server.

## v1.9

__New Feature__

* Adds Auto Rotate
    * On startup the STAC determines which way is up and sets the display orientation accordingly. More in the User's Guide.

__Talent Mode__

* When in Talent Mode and WiFi is lost after initial connection on power up or restart, the display switches to the Preview state and remains there until resuming normal operation after the WiFi connection is re-established.


__Information Screen__

* Revised the format of the information dump sent to the serial port on startup.

__Internal Improvements__

* WiFi connection time is much quicker.
* Changes to improve robustness and recovery time if errors occur when communicating with the Smart Tally server.



## v1.8

__New Feature__

* Adds Autostart Mode.
    * On startup when autostart is set, if no button press is seen for about 10 seconds, the STAC will jump right to the WiFi connect sequence and then start displaying the tally status of the active channel.
More in the User's Guide.

__Fixes__

* When configuring the STAC via a web browser, the SSID and password fields now accept spaces as given instead of converting them to '+' characters.

## v1.7.1.1

Minor bug fix release.

No changes to the STAC code.

Only change is to the modified Preferences.cpp file in the `/libraries/Preferences/ `folder.

It was revised to correct a coding error in writing to the `log_e` file in 
the `clear()` and `end()` methods.

This bug would only show in a very very rare case should a commit to the NVS memory fail when calling one of those methods.

If you did not replace the standard `Preferences.cpp` file with the modified one, there is nothing you need to do. The version running in your STAC is all good.

If you did replace the standard `Preferences.cpp` file, follow the directions in the `libraries/libraries_README.md` file using this revised `Preferences.cpp` file. Then re-upload the STAC code into your ATOM and you're good to go.

My apologies. Nothing like writing code to keep one humble.


## v1.7.1
- improve the ST server poll response time by about a gazillion. As a result...
- ...the ST\_POLL\_INTERVAL now does indeed govern the time between ST server polls
- add an information dump to the serial port on startup

## v1.7
- add support for user configurable ST Server port #.
    - Allows STAC to be used in an emulated environment; as in a [Tally Arbiter](http://www.tallyarbiter.com) client

## v1.6
- first public release
