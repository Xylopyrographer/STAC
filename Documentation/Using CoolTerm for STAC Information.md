# Using CoolTerm for STAC Information

On startup, the STAC sends an information and status page to its USB port.

To view this, you need an application on your computer that can take in serial data from a USB port.

A good option is CoolTerm. CoolTerm is a simple serial port terminal application very well suited for the task and highly recommended.

It is available from [Roger Meier's Web site](https://freeware.the-meiers.org). If you find it useful, please hit the Donate button! 

**Once CoolTerm is installed:**

1. Launch the CoolTerm app and open a new terminal window.
1. Connect the STAC to a USB port.
1. Using the drop down menu in the lower left of the CoolTerm window, find the STAC serial address. On a macOS system it'll look something like:
`usbserial-3552F606F3`.
1. From the CoolTerm Connections menu, pick Options...
1. In the window that opens, select Serial Port from the list on the left to bring up the Serial Port Options pane.
2. Port: should be set to what you picked in step 3.
2. Set:
    * Baudrate to 115200
    * Data Bits to 8
    * Parity to None
    * Stop bits to 1
    * Under Flow Control; uncheck CTS, DTR and XON
    * Check the boxes for "Software Supported Flow Control" and "Block Keystrokes while flow is halted"
    * Under Initial Line States when Port opens, click the "DTR On" and "RTS On" buttons
1. Hit the OK button.
2. Click the Connect icon in the top toolbar.
1. Restart the STAC.

In the CoolTerm window, you should see the STAC startup information.
Something like:

```
==========================================
                  STAC
        A Roland Smart Tally Client
             by: Team STAC
      github.com/Xylopyrographer/STAC

    Version: 2.2.0 (1a2bc3)
    Core: 2.0.14
    SDK: v4.4.6
    Setup SSID: STAC-1C54267E
    Setup URL: http://setup.local
    Setup IP: 192.168.6.14
    MAC: 94:55:7E:A6:55:2B
  --------------------------------------
    WiFi Network SSID: HammyNet
    Switch IP: 192.168.7.99
    Switch Port #: 80
  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    Configured for Model: V-160HD
    Active Tally Channel: HDMI 2
    Max HDMI Tally Channel: 4
    Max SDI Tally Channel: 7
    Tally Mode: Camera Operator
    Auto start: Disabled
    Brightness Level: 1
    Polling Interval: 300 ms
==========================================
```

The "`WiFi Connected.`" line will show up after the STAC connects to the WiFi network and has an IP address assigned.

If the STAC has not been set up, it'll tell you that. And it will let you know when it's waiting for the setup data. Then once set up, it'll send the rest of the information.

If it is operating in Peripheral Mode, it'll send that info out, something like...

```
==========================================
                  STAC
        A Roland Smart Tally Client
             by: Team STAC
      github.com/Xylopyrographer/STAC

    Version: 2.2.0 (1a2bc3)
    Core: 2.0.14
    SDK: v4.4.6
    Setup SSID: STAC-1C54267E
    Setup URL: http://setup.local
    Setup IP: 192.168.6.14
    MAC: 94:55:7E:A6:55:2B
   --------------------------------
     OPERATING IN PERIPHERAL MODE
    Tally Mode: Talent
    Brightness Level: 1
=======================================```
In Peripheral Mode, only the display brightness level and tally display mode can be set, using the display button.

When doing a Factory Reset, it'll send out information to let you know how that's progressing.

Likewise, if a software update is running, it'll let you know that too.

You can select and copy this information to paste into another application.

**Note:**<br>
If you see a bunch of garbage in the CoolTerm window after restarting the STAC, ensure that CoolTerm is set up as described above—especially the Baudrate setting.

### Other Systems

CoolTerm is available for a number of systems and platforms. The serial communications port descriptor is different for each, but once you discover that, things should function as described above.

### Geek Info

If you're curious, part of the info dump looks like: 

```
    Version: 2.2.0 (1a2bc3)
    Core: 2.0.14
    SDK: v4.4.6
    Setup SSID: STAC-1C54267E
    Setup URL: http://setup.local
    Setup IP: 192.168.6.14
    MAC: 94:55:7E:A6:55:2B
```
Here:

`Version` is the version number of the STAC software. The characters in brackets are the "build number" of this version.

`Core` is the arduino-esp32 core version used by the STAC software version.

`SDK` is the espressif ESP-IDF software development kit version used by the arduino-esp32 core.

`MAC` is the WiFi network interface MAC address.

Useful info if you're interested in tweaking the STAC software. File it in the trivia bucket otherwise.

The other bits of the data dump are covered in the *STAC Users Guide*.