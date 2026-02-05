# STAC  
**Smart Tally Atom Client**

A wireless tally light system for Roland video switches.

Its purpose is to monitor via WiFi the tally status of a single video input channel of a Roland video device that implements their Smart Tally protocol.

It supports the following Roland video switches:

* V-60HD
* V-160HD

For the Roland video input channel being monitored, STAC will set the colour of the display:

+ when in "Camera Operator" mode, to:  
     - RED if the channel is in PGM (Program or onair)
     - GREEN if the channel is in PVW (Preview or selected)
     - "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected).  
+ when in "Talent" mode, to:
     - RED if the channel is in PGM (Program or onair)
     - GREEN otherwise

Configuration of the STAC for the WiFi credentials and IP address, port number and number of tally channels of the Roland switch is done using a web browser.

There is also a Peripheral Mode where one STAC can be connected to another via a cable and operate without the need for set up or to establish a WiFi connection.

Lots of good information in the *STAC User Guide*, located in the `Documentation` folder. Other interesting bits are there as well.

## Supported Devices

This release will run on the following devices:

- LilyGo T-Display
- LilyGo T-QT
- M5Stack ATOM Matrix
- M5Stack StickC Plus
- Waveshare ESP32-S3 Matrix
- Xorigin AI PI-Lite


## Installation

### Using a Pre-Built Version

Pre-built binaries are provided for the devices listed above. For information on installing, jump over to the `bin` folder and run through `BIN_README.md`.

Once you've flashed your device it is now a STAC 👍. Next steps are in the *STAC User Guide* in the `Documentation` folder of the Release archive.

### Using an IDE

This release is a complete rewrite of the codebase. 

The `STAC` folder contains all the source files. In there as well is a `doc` folder that holds a good measure of information on how to modify and build the software. There are also some very nifty things in the `utility` folder.

Developing requires VS Code with the pioarduino extension installed. With this release, STAC has outgrown the Arduino IDE.

The Arduino framework is used, and this release is was done using ESP-arduino core version 3.3.2. It should be compatible with newer versions but would require testing.

One (of many) new architectural features is the ability to port the software to new hardware via a configuration file. Good chance that's all you'll need to do to add a device. Details in the `STAC/doc` folder.

To download the files:

1. Here in GitHub, on the right side of the window, click the green "Latest" button under "Releases".
1. Click "Source code" choosing either `zip` or `tar.gz` as you prefer.
1. Unpack the downloaded `STAC-X.Y.Z` file and have at 'er.


## Compatibility

This release is a complete rewrite of the STAC software. If you are new to STAC, follow the installation instructions above to get up and running and you can skip the rest of this section.

If you already have ATOM Matrix STAC devices, the fundamental purpose of the STAC&mdash;to relay tally information&mdash;has not changed. At the end of the day, that is what this release still does very well.

From a users point of view, this release brings:
- significant improvements in configuring and managing multiple devices with the ability to create, save and load from a configuration file and use copy and paste functions as well;
- ability to perform maintenance tasks via the web interface;
- a single web access point for all functions setup and maintenance.

Depending on your environment, these may be useful. Check out the *STAC User Guide*.

**Do note that upgrading to this version will require that the:**<br>
- **firmware is flashed as per the installation instructions as if this was the very first time using the ATOM as a STAC**<br>
- **setup be redone;** however, the new setup features makes this much much easier.

**You cannot do an OTA update to install this release.** Too much has changed under the hood to have made that possible.

All that said, if you're happy as a clam with what you've got that's great. STAC's running this or any other release are fully compatible with their main purpose&mdash;keeping everyone in the know  as to the tally status of the camera you're running!

<br><br>

---

### Revision History

**2026-02-05:** Revise for STAC v3.0.0.<br>
**2024-05-09:** Revise for STAC v2.2.0, adding V-160HD support.<br>
**2023-09-09:** Revise required libraries, Build System Compatibility.<br>
**2023-02-21:** Include info on using an ESP flasher.<br>
**2023-02-04:** Revised for software v2.1.<br>
**2022-07-09:** Revised arduino-esp32 core compatibility information. Added required libraries.<br>
**2022-01-04:** Added information on Peripheral Mode. Minor format changes.<br>
**2021-05-20:** Direction to use the `Release` version in place of the `main` branch.<br>
**2021-04-07:** Delete config info. Minor format changes.<br>

<!-- EOF -->
