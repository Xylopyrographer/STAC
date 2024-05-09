# STAC  
## (Smart Tally Atom Client)  
**A Roland Smart Tally Client**  

An Arduino sketch designed to run on an [M5Stack ATOM Matrix](https://docs.m5stack.com/#/en/core/atom_matrix) board.

Its purpose is to monitor the tally status of a single video input channel 
of a Roland video device that implements their Smart Tally protocol.

Starting with STAC v2.2.0, the following Roland video switches are supported:

* V-60HD
* V-160HD

The sketch uses WiFi to connect to the same network as that of the Roland device.

For the Roland video input channel being monitored, STAC will set
the colour of the display on the ATOM:

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

## Installation

### Download the Files

1. Here in GitHub, on the right side of the window, click the green "Latest" button under "Releases".
1. Click "Source code" choosing either `zip` or `tar.gz` as you prefer.
1. Unpack the downloaded `STAC-X.Y.Z` file.

### Using a Pre-Built Version

1. Download an ESP flasher. A recommended one is *esphome-flasher*. Available from their [Github page](https://github.com/esphome/esphome-flasher).

1. Connect an ATOM Matrix to your computer.

1. Open the ESP flasher that you downloaded.
    + Select the **device serial port** *(from the "Serial port" list in esphome-flasher)* and then select the STAC software file *(clicking on "Browse" in esphome-flasher)*.
    + The STAC software file you want is in the `bin` folder which is inside the `STAC` folder of the Release zip archive you downloaded. The file name is something like:  `STAC_211-4c417e.bin`.

1. Install the software onto the ATOM Matrix *(using esphome-flasher, click "Flash ESP")*.<br>If you're using *esphome-flasher*, also click the "View Logs" button as it will show how things are progressing along with a nice welcome message from the STAC.

Done! Your ATOM Matrix is now a STAC 👍. Next steps are in the *STAC User Guide* in the `Documentation` folder of the Release archive.

### Using an IDE

1. Move the unpacked archive you downloaded to a folder where you keep your Arduino sketches.

1. Install the libraries:<br>
The following libraries are required to compile this sketch.<br>

    + [LiteLED](https://github.com/Xylopyrographer/LiteLED) by Xylopyrographer
    + [XP_Button](https://github.com/Xylopyrographer/XP_Button) by Xylopyrographer
    + [I2C_MPU6886](https://github.com/tanakamasayuki/I2C_MPU6886) by TANAKA Masayuki

    All of these can be installed using the Arduino IDE Library Manager.


**Build System Compatibility**

This sketch has been built and tested using:    
    
+ ESP arduino-esp32 core version 2.0.14.
  + core v2.0.5 is the minimum required. *Older cores will not work*.
  + *do not* use the esp32 core version supplied by M5Stack.
+ Arduino IDE version 2.3.2
+ Depending on how support for the ATOM Matrix was added, selecting either the M5Stick-C, or the M5Stack-ATOM as the target board should work.
+ To compile, make sure the `STACLib` folder is located in the same folder as the `STAC.ino` file.
+ Starting with STAC software v2.1, the Partition Scheme must be set to "Minimal SPIFFS (Large APPS with OTA)".

*Note:* STAC is not compatible with ESP arduino-esp32 core version 3.
<br><br>

---
### Revision History

**2024-05-09:** Revise for STAC v2.2.0, adding V-160HD support.<br>
**2023-09-09:** Revise required libraries, Build System Compatibility.<br>
**2023-02-21:** Include info on using an ESP flasher.<br>
**2023-02-04:** Revised for software v2.1.<br>
**2022-07-09:** Revised arduino-esp32 core compatibility information. Added required libraries.<br>
**2022-01-04:** Added information on Peripheral Mode. Minor format changes.<br>
**2021-05-20:** Direction to use the `Release` version in place of the `main` branch.<br>
**2021-04-07:** Delete config info. Minor format changes.<br>
