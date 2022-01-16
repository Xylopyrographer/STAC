# STAC  
## (Smart Tally Atom Client)  
**A Roland Smart Tally Client**  

An Arduino sketch designed to run on an [M5Stack ATOM Matrix](https://docs.m5stack.com/#/en/core/atom_matrix) board.

Its purpose is to monitor the tally status of a single video input channel 
of a Roland video device that implements their proprietary Smart Tally protocol.

The sketch uses WiFi to connect to the same network as that of the Roland device.

For the Roland video input channel being monitored, STAC will set
the colour of the display on the ATOM:

+ when in "Camera Operator" mode, to:  
     - RED if the channel is in PGM (Progam or onair)
     - GREEN if the channel is in PVW (Preview or selected)
     - "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected).  
+ when in "Talent" mode, to:
     - RED if the channel is in PGM (Progam or onair)
     - GREEN otherwise

Configuration of the STAC for the WiFi credentials and IP address, port number and number of tally channels of the Roland switch is done using a web browser.

There is also a Peripheral Mode where one STAC can be connected to another via a cable and operate as in Talent mode without the need for configuration or to establish a WiFi connection.

Lots of good information in the *STAC User Guide*, located in the `Documentation` folder along with other interesting bits in there as well.

### To use this sketch:

1. Here in GitHub, on the right side of the window, click the green "Latest" button under "Releases".
1. Click "Source code" chosing either `zip` or `tar.gz` as you prefer.
1. Unpack the downloaded `STAC-X.Y.Z` file
1. Rename the resulting folder from  `STAC-X.Y.Z` to just `STAC`
1. Move the `STAC` folder to your Arduino sketch folder.


---


|Use of this sketch requires a modified set of libraries. <br>See the`libraries_README.md` file in the `libraries` folder.|  
:---:


|This sketch has been built and tested using<br>version 1.8.19 of the Arduino IDE and with<br>arduino-esp32 core version 1.0.6.<br>It has not been tested with the version 2 Release Candidates<br>of the Arduino IDE nor any version of the arduino-esp32 core<br>other than 1.0.6, including the version 2.x.y cores.|
:---:


---

### Document Revision History
**2022-01-04:** Added information on Peripheral Mode. Minor format changes.<br>
**2021-05-20:** Direction to use the `Release` version in place of the `main` branch.<br>
**2021-04-07:** Delete config info. Minor format changes.<br>
