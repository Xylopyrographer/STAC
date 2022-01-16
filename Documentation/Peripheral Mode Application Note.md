# Peripheral Mode Application Note

As described in the *STAC User's Guide*, STAC software starting with version 1.10 adds a Peripheral Mode of operation.

This note goes into what happens behind the scenes with the STAC running a version of software that implements Peripheral Mode: during startup; when operating as a peripheral; and also when in its normal operating state.
<br><br>

## Initiating Client Mode

A STAC is placed into Peripheral Mode by:
    
1. Placing a jumper wire between G22 and G33 on the ATOM rear header block.
1. Applying power either via its USB port or a cable connecting its GROVE port to the GROVE port of another STAC. Do one or the other, not both.

When powered up or when reset, the STAC configures G22 as an output pin and G33 as an input pin (with weak pull down). It then toggles the state of G22 in a sequence and confirms that G33 follows. If G33 follows throughout the entire sequence, it is assumed the "Peripheral Mode jumper" is in place. 

If G33 fails to follow G22 at any point during the sequence, the STAC assumes the jumper is not in place and it will start up in its normal operation state.

In either case G22 is then driven low.

After the STAC confirms the jumper is in place, it then configures G26 and G32 on its GROVE (HY2.0-4P) connector as input pins (with weak pull downs) and enters Peripheral Mode.
<br><br>

## Operation in Peripheral Mode

In Peripheral Mode, all normal operation state and features of the STAC are disabled. The STAC will not connect to a WiFi network and it will ignore any Tally Channel, Operating Mode and Startup Mode settings. The Display Brightness select feature remains enabled as does the orientation feature so the Brightness Select numbers are displayed right side to.

In Peripheral Mode, the STAC continually monitors the state of G26 and G32 on its GROVE port and uses the state of those inputs to determine the state of its tally display as per the table below.

| G26 | G32 | DISPLAYED<br>TALLY STATE |
|:---:|:---:|:---:|
| 0 | 0 | PVW |
| 0 | 1 | PVW |
| 1 | 0 | PVW |
| 1 | 1 | PGM |

**Where:**<br>
&nbsp;&nbsp;&nbsp;**1** = logic high (true)<br>
&nbsp;&nbsp;&nbsp;**0** = logic low (false)<br>
&nbsp;&nbsp;&nbsp;**PVW** (Preview, Standby, Selected) = GREEN<br>
&nbsp;&nbsp;&nbsp;**PGM** (Program, Live, On Air) = RED

Here's the cool part. This allows the opportunity for developers to use a STAC set in Peripheral Mode as a tally indicator driven by an external piece of gear. Simply connect a cable to the GROVE port, apply power and drive pins G26 and G32 as needed.

<center>==**Be aware that the ATOM Matrix GPIO pins use 3.3V logic levels and are not 5V tolerant.<br>Driving any GPIO pin beyond 3.3 V will irreparably damage the device.**==</center>
<br><br>

##Normal State (Controller) Output Signals

Starting with software v1.10, a STAC running it its normal operating state (not in Peripheral Mode) also acts as a controller by outputting the current tally state of the video channel it is monitoring to its GROVE connector. Pins G26 and G32 on this port are set as per the table below.

| G26 | G32 | ROLAND SWITCH<br>TALLY STATE
|:---:|:---:|:---:|
| 0 | 0 | UNKNOWN |
| 0 | 1 | UNSELECTED |
| 1 | 0 | PVW |
| 1 | 1 | PGM |

**Where:**<br>
&nbsp;&nbsp;&nbsp;**1** = logic high (true)<br>
&nbsp;&nbsp;&nbsp;**0** = logic low (false)<br>
&nbsp;&nbsp;&nbsp;**UNKNOWN** = the STAC is not connected to the Roland switch; or no information was reported from the last poll; or the return from the last poll was not an expected response; or the STAC is in an error state.<br>
&nbsp;&nbsp;&nbsp;**UNSELECTED** = the video channel being monitored is neither in PVW or PGM.<br>
&nbsp;&nbsp;&nbsp;**PVW** = the channel being monitored is in preview (Preview, Standby, Selected)<br>
&nbsp;&nbsp;&nbsp;**PGM** = the channel being monitored is in program (Program, Live, On Air)

This allows the opportunity for developers to use a STAC running in its normal state to provide tally information to another piece of gear.

To do this, connect a cable to the GROVE port, apply power and monitor the state of pins G26 and G32.

This could allow the STAC to:

- drive a larger or brighter indictor;
- act as a wireless tally relay;
- be integrated as an indicator and controller into a system such as [Tally Arbiter](http://www.tallyarbiter.com).

The "UNSELECTED" state is useful as it indicates normal operation and communication of the controller STAC with the Roland (or emulated) device being monitored even if that channel is not active in either PGM or PVW.

With proper cabling, multiple STACS operating in Peripheral Mode could be connected in parallel to a STAC operating in its normal state. No testing has been done to confirm how many STACs could be driven in this manner. Power supply would be an issue to consider as well.
<br><br>


## Disabling Client Mode

To disable Peripheral Mode and return the STAC it to its normal operating state:

1. Remove power from the Peripheral Mode STAC.
1. Remove the GROVE cable connecting the STACs. **Do not** use the GROVE cable to power up two STAC's that are in their normal oprating state.
1. Remove the Peripheral Mode jumper wire from the rear of the STAC.
1. Power up the STAC via its USB port.

<br>

## Notes

1. ==**Be aware that the ATOM Matrix GPIO pins use 3.3V logic levels and are not 5V tolerant.<br>Driving any GPIO pin beyond 3.3V will irreparably damage the device.**==
1. The serial data dump on power up or restart when in Peripheral Mode (see the *Using screen for STAC Information.md* document) will report that the STAC is in Peripheral Mode and the brightness level set.
1. The *Peripheral Mode* section of the *User's Guide* provides more information on the GROVE cable.
<br><br>

---

## Document Revision History  

**2022-01-04:** First release.<br>
