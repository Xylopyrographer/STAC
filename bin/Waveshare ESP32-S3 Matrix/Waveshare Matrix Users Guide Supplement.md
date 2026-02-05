# Waveshare ESP32-S3-Matrix Users Guide Supplement

This document outlines the differences in the STAC software for the Waveshare ESP32-S3-Matrix development board. Will be calling this the Waveshare Matrix for short.

This device is a bit different in that Waveshare does not manufacture an enclosure for it and during development I didn't create one either as 3D design isn't (yet) in my repertoire.

But it was the first device I used as a platform to extend the number of devices that could be used as a STAC so it has a bit of a special place in my heart.

So the practical use of the Waveshare Matrix is a bit of an exercise left to the user.

---

## Buttons

The binary files assume the "display" button is connected to GPIO7 and GND. That pin was picked as a breadboard compatible tactile button plugs into a breadboard with no extra wires required (my test board has a right-angle header soldered to that side of the board).

The very tiny reset button is at the top when mounted that way on the breadboard so it's still accessible.

A practical case design would provide a bit of a chin on the top case to facilitate adding buttons for both reset and the display buttons. Accommodation would also need to be made if you wanted an enclosure that could hold a connector of some sort to support Peripheral Mode. Adding a tinted cover over the matrix would be a nice touch.

---

## Display

The display letters, numbers and icons are modified to make use of the larger 8×8 matrix.

The design isn't so different as to be unrecognizable from the ATOM Matrix so the *STAC Users Guide* is the place to go for that information.

### Orientation

The Waveshare Matrix does respect the orientation of the device when started in that display objects are rotated to match the physical orientation detected.

### Brightness Setting

The LEDs on this device are stupid bright. There are eight display brightness settings. If you do put this in an enclosure, be aware that this thing gets pretty hot with higher settings.

---

## Peripheral Mode
 
### Operating in Peripheral Mode

The Waveshare Matrix supports Peripheral Mode operation by monitoring the state of GPIO5 and GPIO6 as per the table below.

Selecting those pins was arbitrary based on the breadboard setup I have.

<table>
	<tbody><center>
		<tr>
			<td rowspan="2"><b>GPIO5<br>(TS_0)</b></td>
			<td rowspan="2"><b>GPIO6<br>(TS_1)</b></td>
			<td colspan="2"><center><b>Displayed Tally State</center></td>
		</tr>
		<tr>
			<td><center><b>Talent</center></b></td>
			<td><center><b>Camera Operator</center></b></td>
		</tr>
		<tr>
			<td><center>0</center></td>
			<td><center>0</center></td>
			<td><center>PVW</center></td>
			<td><center>X</center></td>
		</tr>
		<tr>
			<td><center>0</center></td>
			<td><center>1</center></td>
			<td><center>PVW</center></td>
			<td><center>UNSELECTED</center></td>
		</tr>
		<tr>
			<td><center>1</center></td>
			<td><center>0</center></td>
			<td><center>PVW</center></td>
			<td><center>PVW</center></td>
		</tr>
		<tr>
			<td><center>1</center></td>
			<td><center>1</center></td>
			<td><center>PGM</center></td>
			<td><center>PGM</center></td>
		</tr>
	</center></tbody>
</table>

**Where:**<br>
&nbsp;&nbsp;&nbsp;**1** = logic high (3.3V)<br>
&nbsp;&nbsp;&nbsp;**0** = logic low (0V)<br>
&nbsp;&nbsp;&nbsp;**PVW** (Preview, Standby, Selected) = GREEN<br>
&nbsp;&nbsp;&nbsp;**PGM** (Program, Live, On Air) = RED<br>
&nbsp;&nbsp;&nbsp;**UNSELECTED** = Purple checkerboard pattern<br>
&nbsp;&nbsp;&nbsp;**X** (Unknown) = Orange X

> **Note:**
> 
> Be aware that the GPIO pins use 3.3V logic levels and are not 5V tolerant. Driving any GPIO pin beyond 3.3V will irreparably damage the device.

### Normal State (Controller) Output

When operating in **normal state** (not Peripheral Mode), the Waveshare Matrix outputs its current tally state on pins GPIO5 and GPIO6 as per the table below.

| GPIO5<br>(TS_0) | GPIO6<br>(TS_1) | ROLAND SWITCH<br>TALLY STATE |
|:---:|:---:|:---:|
| 0 | 0 | UNKNOWN |
| 0 | 1 | UNSELECTED |
| 1 | 0 | PVW |
| 1 | 1 | PGM |

**Where:**<br>
&nbsp;&nbsp;&nbsp;**1** = logic high (3.3V output)<br>
&nbsp;&nbsp;&nbsp;**0** = logic low (0V output)


<!-- EOF -->
