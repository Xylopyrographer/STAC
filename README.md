# STAC  
## (Smart Tally Atom Client)  
**A Roland Smart Tally Client**  

An Arduino sketch designed to run on an [M5Stack ATOM Matrix](https://docs.m5stack.com/#/en/core/atom_matrix) board.

Its purpose is to monitor the tally status of a single video input channel 
of a Roland video device that implements their Smart Tally protocol.

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

There is also a Peripheral Mode where one STAC can be connected to another via a cable and operate as in Talent mode without the need for configuration or to establish a WiFi connection.

Lots of good information in the *STAC User Guide*, located in the `Documentation` folder along with other interesting bits there as well.

### To use this sketch:

1. Here in GitHub, on the right side of the window, click the green "Latest" button under "Releases".
1. Click "Source code" choosing either `zip` or `tar.gz` as you prefer.
1. Unpack the downloaded `STAC-X.Y.Z` file
1. Rename the resulting folder from  `STAC-X.Y.Z` to just `STAC`
1. Move the `STAC` folder to your Arduino sketch folder.


### Required Libraries:

The following libraries are required to compile this sketch.

1. [FastLED](https://github.com/FastLED/FastLED) by Daniel Garcia
1. [JC_Button](https://github.com/JChristensen/JC_Button) by Jack Christensen
1. [I2C_MPU6886](https://github.com/tanakamasayuki/I2C_MPU6886) by TANAKA Masayuki


<<<<<<< HEAD
It is recommended that the modified Preferences library files be used. <br>See the`libraries_README.md` file in the `libraries` folder.|  
:---:
=======
All of these can be installed using the Arduino IDE Library Manager.
>>>>>>> main

When compiling, you may see an message that JC_Button may not work as expected with the ESP32. This can be ignored.
<br>

### Build System Compatibility:

This sketch has been built and tested using:

+ arduino-esp32 core version 2.0.3
  + this is the minimum required core version; _older cores will not work_.
+ Arduino IDE version 1.8.19
+ Arduino IDE version 2.0.0-rc8
+ Visual Studio Code version 1.68.1
    - using Microsoft Arduino extension version 0.4.12
+ Depending on how support for the ATOM Matrix was added, selecting the M5StickC, M5StickC Plus, or the ATOM MATRIX as the target board should work.

### Revision History
**2022-07-01:** Revised arduino-esp32 core compatibility information. Added required libraries.<br>
**2022-01-04:** Added information on Peripheral Mode. Minor format changes.<br>
**2021-05-20:** Direction to use the `Release` version in place of the `main` branch.<br>
**2021-04-07:** Delete config info. Minor format changes.<br>
