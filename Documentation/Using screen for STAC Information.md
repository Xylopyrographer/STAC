# Using `screen` for STAC Information

On startup, the STAC sends an information and status page to its USB port.

To view this, you need an application on your computer that can take in serial data from a USB port.

macOS, through its Terminal app supports the `screen` serial monitor app which can be used to view this startup information.

**To use `screen` to receive information from a STAC:**

1. Open the Terminal app on your Mac.
1. Connect the STAC to a USB port.
1. In Terminal, find the STAC serial address by typing:
`ls /dev/cu.usbserial*`
1. In the list that is returned, look for a line like:<br>`/dev/cu.usbserial-3552F606F3`
1. Select and copy the above line.
1. In the Terminal window, type: `screen`; a space; then paste in the line from above; and then type `115200`.<br>The complete line should look something like this:<br>
`screen /dev/cu.usbserial-3552F606F3 115200`
1. Hit the return key.
1. Restart the STAC.

In the Terminal window, you should see the STAC startup information.
Something like:

```
=======================================
                 STAC
   A Smart Tally ATOM Matrix Client
            by: Team STAC
https://github.com/Xylopyrographer/STAC

     Software Version: 2.0
   Configuration SSID: STAC-78B5927E
     Configuration IP: 192.168.6.14
     -------------------------------
    WiFi Network SSID: StreamNet
    Smart Tally IP: 192.168.1.13
    Port #: 80
    Max Tally Channel: 8
    Polling Interval: 300 ms
     =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    Auto start: Disabled
    Operating Mode: Camera Operator
    Active Tally Channel: 5
    Brightness Level: 1
======================================
```
Items above the "`=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=`" line are set when configuring the STAC via a web browser. Items below that line are set using the STAC display button.

If the STAC has not been configured, it'll tell you that. And it will let you know when it's waiting for the configuration data. Then once configured, it'll send the rest of the information to the screen.

If it is operating in Peripheral Mode, it'll send that info out, something like...

```
=======================================
                 STAC
   A Smart Tally ATOM Matrix Client
            by: Team STAC
https://github.com/Xylopyrographer/STAC

     Software Version: 2.0
   Configuration SSID: STAC-78B5927E
     Configuration IP: 192.168.6.14
     -------------------------------
     OPERATING IN PERIPHERAL MODE
          Brightness Level: 1
=======================================

```
In Peripheral Mode, only the display brightness level can be set, using the display button.

When doing a Factory Reset, it'll send out information to let you know how that's progressing.

You can select and copy this information to paste into another application.

**To quit `screen`:**

1. In the Terminal window, while holding down the `control` key, type `a` then `\`.
1. A prompt will pop up at the bottom of the screen. Release the `control` key. Confirm you want to quit by typing `y`.
1. Quit the Terminal app.

**Note:**<br>
If you see a bunch of garbage on the screen after typing the `screen` command, or the `screen` command gives an error message, ensure that:

* the `/dev/...` line was copied and pasted correctly;
* there are no extra spaces in the command line;
* the last thing on the line is exactly `115200`.

Computers are fussy that way.

### Other Systems

The `screen` app is also available on most *nix distributions. The serial communications port descriptor is different, but once you discover that, use it on the command line and things should function as described above.

Windows systems have a number of options for serial communications programs. Consult the documentation for the connection syntax. The STAC only communicates at 115200 baud.