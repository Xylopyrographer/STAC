# Using `screen` for STAC Information

On startup, the STAC sends an information and status page to its USB port.

To view this, you need an application on your computer that can take in serial data from a USB port.

macOS, through its Terminal app supports the `screen` serial monitor app which can be used to view this startup information.

Terminal commands to connect to a STAC.

1. Open the Terminal app on your Mac.
1. Connect the STAC to a USB port.
1. In terminal, find the STAC serial address by typing:
`ls /dev/cu.usbserial*`
1. Look for a line like: `/dev/cu.usbserial-3552F606F3`
1. Select and copy the above.
1. In the Terminal window, type `screen ` and then paste in the line from above and then type `115200`. <br>The complete line should look something like this:<br>
`screen /dev/cu.usbserial-3552F606F3 115200`
1. Hit the return key.
1. Restart the STAC.

In the Terminal window, you should see the STAC startup information.
Something like:

```
======================================
                STAC
   A Smart Tally ATOM Matrix Client
         by: Xylopyrographer
https://github.com/Xylopyrographer/STAC

    Software Version: 1.8
    WiFi Network SSID: StreamNet
    Smart Tally IP: 192.168.4.101
    Port #: 80
    Auto start: Disabled
    Operating Mode: Camera Operator
    Active Tally Channel: 4
    Max Tally Channel: 6
    Brightness Level: 1
    Configuration SSID: STAC_245B937E
======================================

```
If the STAC has not been configured, it'll tell you that. And let you know when it's waiting for the configuration data. Then once configured, it'll send the rest of the information.

You can select and copy this information to paste into another application.

To quit `screen`:

1. In the Terminal window, hold down the `control` and `A` key and then the `\` key.
1. Confirm you want to quit by typing 'y'.
1. Quit the Terminal app.

If you see a bunch of garbage on the screen after typing the `screen` command, or the screen command gives an error message, ensure that:

* the `/dev/...` line was copied and pasted correctly;
* there are no extra spaces in the command line;
* the last thing on the line is exactly `115200`.

Computers are fussy that way.
