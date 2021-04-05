# STAC Users Guide 
## (Smart Tally ATOM Client)
 
**Author:** Xylopyrographer

**A Roland Smart Tally Client**

Take a bit of custom software, pour it into an [M5Stack ATOM Matrix](https://docs.m5stack.com/#/en/core/atom_matrix) board, give it a shake and&mdash;ta da! It becomes a *Smart Tally Atom Client* or **STAC** for short.

Its purpose and sole joy in life is to monitor the tally status of a single video input channel of a Roland video device that implements their proprietary Smart Tally protocol and indicate to you that status via the display of the ATOM.

The STAC uses WiFi to connect to the same network as that of the Roland switch. It continuously polls the switch for the tally status of the video channel being monitored. For that channel, the STAC will set the colour of the its display:

* When in "Camera Operator" mode, to:  
	- RED if the channel is in PGM (Program or onair)
	- GREEN if the channel is in PVW (Preview or selected)
	- "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected)
* When in "Talent" mode, to:  
	- RED if the channel is in PGM (Program or onair)
	- GREEN otherwise

Configuration of the STAC for the WiFi credentials, Roland switch IP address, port number and number of tally channels is done using a web browser.

Setup of the STAC is done using its display button, with feedback provided via the display.

To ensure you see the pictures, make sure the *images* folder is in the same folder as this *STAC Users Manual.md* file.<br><br>

## Getting to Know You

The bits you need to know about when using the STAC are shown in the picture below.
<center>
![M5Atom](./images/STAC_bits.png)
</center>
The Display is, well, just that. A square matrix of colour LED's used to show: the state of the tally channel being monitored; receive feedback on its operation; and for setup. We'll get to those parts in a while.

The Display is also a button that you'll click and press to select and set various operating parameters for the STAC.

The Reset Button is used in conjunction with the Display button to reconfigure the STAC. It can also be used to restart the STAC should that be needed.

The USB port is used to power the STAC. Use one end of a standard USB-C cable to connect here and the other to whatever matches the power supply being used. FWIW, data is not required, only 5 VDC power. There is no power switch on the STAC so once the cable is connected to a power supply it comes to life and is good to go. The power can be disconnected at any time without concern for the state of the STAC.

On the back of the STAC is a dang tiny 2mm machine screw hole that can be used to attach the STAC to whatever you would like.

Note the ventilation holes on the sides of the STAC. To allow for cooling, these should not be blocked when using the device. The display LED's can get a bit toasty, especially when operating at higher brightness levels.
<br><br>
## <a name="QuickStart"></a>Quick Start

We'll get to the initial configuration (and how to change it) later. Here we'll show the normal operating sequence for using the STAC.

Connect one end of a USB-C cable to the STAC and the other to a 5 VDC power source.

The STAC will briefly show a "power on" indicator and then display the tally channel of the Roland Smart Tally device it is monitoring (the active channel).
<center>
![poweron](./images/startup_1.png)
</center>

From here, click the Display Button. The Operating Mode of the STAC is shown. Click once more and the current Brightness Level of the display is shown.
<center>
![currentstate](./images/startup_2.png)
</center>
The active tally channel, the operating mode and the display brightness can each be changed when they are displayed during startup. The display brightness level can also be changed when the STAC is monitoring the tally channel. We're getting to that part too.

When the Brightness Level is displayed, the next click of the display button (we'll just call this the button from here on), will let the STAC loose to do its magic.

The orange "attempting to connect to WiFi" icon is shown and, after a bit if the connection is made, the icon will change to green indicating success.
<center>
![wifiseq](./images/startup_3.png)
</center>
Once WiFi is connected the STAC will start querying the configured Roland Smart Tally device for the tally status of the channel being monitored. The STAC then continuously polls the Roland switch and the display will change according to the state returned by the Roland device like so...
<center>
<a name="tallystates"></a>![tallydisplaystates](./images/operating.png)
</center>
We're going to call this the normal operating state of the STAC.

**Note:** if the STAC is set to "Talent Mode", only the Program and Preview tally states will be shown.

So that's the general flow of things. Now let's get to the nitty gritty...
<br><br>
## <a/ name="FirstTimeConfiguration"></a>First Time Configuration

Until you tell it a few things, the STAC is pretty useless. You'll have to configure it.

To do this you need the:

* SSID and 
* Password of the WiFi network to which your Roland switch is connected
* IP address of the Roland switch
* the port number used by the Roland switch
* maximum tally channel number of the Roland switch.

You'll also need a computer, phone, tablet, or some other device that can connect to a WiFi network and run a web browser as that is the tool you'll use to send the configuration information to the STAC. Here's how you'll do that.

1. Power up the STAC. It will flash the red "configuration required" icon and then hold this icon on the display. The STAC then fires up a WiFi access point so it can communicate with your web browser and waits for the configuration information to be received.  
<center>![STACConfgReqd](./images/stac_config_reqd.png)</center>

1. Using a computer, phone, tablet, or some other smarty pants device, open its WiFi settings and pick the SSID of your STAC. The STAC SSID starts with **"STAC\_"** followed by eight characters. Something like **STAC\_4660A124**. Each STAC has its own unique SSID.
<center>
![STAC WiFi](./images/wifi_connect.png)
</center>
1. Join the STAC WiFi network and when prompted, enter the STAC WiFi password which is <b>1234567890</b>. 

1. Once connected, on your smart device, fire up your web browser, open a new browser window and enter the IP address of the STAC which is **192.168.6.14** hit "go" or the Return key to connect. The STAC Configuration page will pop up.
<center>
![STAC Address](./images/browser_connect.png)
</center>

1. On your browser, fill in the form with the WiFi SSID and Password, the Roland device Smart Tally IP address, its port number and the maximum channel number that you want to monitor. Remember, the Roland device must be on the WiFi network that matches this SSID and Password.

1. Once the information has been entered, tap the "Submit" button to send the form to the STAC.
The STAC will send a confirmation page to your browser, briefly show a confirmation checkmark on the display, and then shut down its WiFi connection.
<center>
![STAC Configuration Page](./images/browser_complete.png)
</center>

1. Close the confirmation window in your browser, reconnect your smart device to your preferred WiFi network and you're done. Like dinner.

After the configuration information is submitted and the STAC confirms receipt, the STAC will start up as described in the [Quick Start](#QuickStart) section above. Unless it doesn't, in which case, pop down to the [Troubleshooting](#Troubleshooting) section and it'll tell you how to fix that up. And since you're wondering yup&mdash;you sure can reconfigure the STAC. There's a how-to [below](#ReconfiguringtheSTAC).

But first, a few things about entering the configuration information in your browser:

* The SSID, Roland IP address, port number and maximum tally channel number are required. The form cannot be submitted with these fields blank.
* The maximum SSID length is 32 characters
* The password can be left empty to allow for open WiFi networks. The password is limited to 63 characters.
* The Roland IP address must be in proper IP address format before the form can be submitted.
* The maximum tally channel number is set to <b>6</b> by default. The highest number that can be entered is <b>8</b>. The smallest is <b>1</b>.
* The port number is set to a default of <b>80</b>. It should only be changed in special situations like described in [Special Configurations](#SpecConfig) below.

Regarding the SSID and Password, be cautious when entering quotes as most devices will by default convert these to "smart quotes" which, as per WiFi standards are not allowed.

If the Submit button is tapped and there is missing or incorrect information entered on the form, a little bubble will pop up by that field prompting you to complete or correct the information.

The SSID and Password fields stop accepting input after their limits are reached.

Taping the "Reset" button on the browser form will clear all information entered and set the port number and number of channels to their defaults, leaving the form open.

At the bottom of the form, the version number of the STAC software is shown as a bit of trivia.

**Some advise:** If you are configuring multiple STACs in one go, it is quite important to close the Confirmation browser window each time before moving onto the next one. Using a desktop or laptop computer and browser is recommend. Some browsers, notably the mobile variety, get hung up when connecting to the IP address of the STAC (as each one is the same) even though the STAC and its WiFi SSID are unique.
<br><br>
## <a name="UpandRunning"></a>Up and Running

OK, you got the STAC configured, it's connected to the WiFi network (of the Roland device) and you're looking to set it to monitor a tally channel, etc. Perfect! Let's get into that...

Once configured and connected to the WiFi network, you can tell the STAC:

* which smart tally channel to monitor
* to operate in "Camera Operator" or "Talent" mode
* to change the brightness level of its display.

To change an operational setting you use the display button. When the parameter is displayed during startup, a single click of the display button will confirm the setting of that parameter and advance to the next one. The parameter sequence is: 

* Active Tally Channel &#8594; 
* Operating Mode &#8594; 
* Brightness Level

Clicking past the Brightness Level display starts the STAC WiFi connect sequence and then on to monitor the active tally channel.

To change the value of a parameter, you press and hold the display button while that parameter is being displayed. After about two seconds, the STAC will move into "select state". The display will change to reflect that and to let you know you're now able to change that parameter. Release the button.

Once the STAC is in a select state, subsequent clicks of the button will advance to the next value of the parameter, looping back to the start when the end value of that parameter is reached.

Once the desired value for that parameter is shown on the display, press and hold the button again for about two seconds. The STAC will briefly display a green checkmark to confirm the change, exit select state and display the new value of the parameter in its normal (non-select state) colours.

From here, a single click will move to the next parameter that can be changed. Or if you press and hold the button, you'll once again enter the select state for that parameter. Wash, rinse, repeat.

That was a lot of words. Let's show you how this works in detail for changing the tally channel. You can then apply that same method to the other two...

### <a name="SettingtheTallyChannel"></a>Setting the Tally Channel

Power up your STAC. When you see the blue active tally number on the display, press and hold down the button. In about two seconds the display will change from a blue number on black background to an orange number on a blue background. The orange colour is a bit of a heads up to let you know the STAC is in a select state and you can now change or select the value. We use this convention with all the parameters when in a select state.
<center>
![Initiate Edit Mode](./images/initiate.png)
</center>

Now click the button once. The tally number will advance by one. Keep clicking the button and watch how the tally number increases and then wraps around to the number **1**. Keep clicking until the desired tally channel is shown. Or until your finger gets tired. Your call.
<center>
![Changing the value](./images/select.png)
</center>

Now press and hold the button. After about two seconds the green confirmed checkmark will briefly appear, the STAC will exit select state and the display will show the new active tally channel as a blue number on a black background.
<center>
![Change confirm](./images/confirm1.png)
</center>Only easy right?

When in a select state, the STAC has a timeout feature. If after entering a select state, no clicks of the display button are detected for about 30 seconds, the select state is cancelled and the display and tally channel will revert back to the value it had before entering select state. From here the button can be clicked to advance to the next parameter or press and hold to enter the select state again.

Point to note here. The maximum tally channel number is set when the STAC is configured via the web browser. If that maximum is set to **1**, you can enter the tally channel select state but clicking the button thereafter will have no effect as the lowest and highest numbers are the same. So if the tally number is not changing past **1**, and you need to select a higher number, you'll need to reconfigure the STAC. No biggie, check out how to do that [below](#ReconfiguringtheSTAC).

The STAC configuration does not require that the maximum tally channel be the same as all the channels of the Roland device that support the Smart Tally protocol. For example, the Roland V-60HD supports up to six Smart Tally channels. However if you only have sources connected to the first four video input channels, you could configure the maximum tally channel on the STAC to be **4**. Then during setup, after selecting channel four, the STAC would wrap back around to channel one.

While we're talking reconfiguring, if your STAC is set to monitor, say channel **6**, and you reconfigure the STAC and set the maximum tally channel to a number less than that, after sending the new configuration information to the STAC the active tally channel will be reset to **1** as the STAC can't monitor a channel higher than the maximum you configured it to. Make sense?

And, oh yeah, the STAC remembers the last set Tally Channel across power cycles. Convenient, yes?

### <a name="SettingtheOperatingMode"></a>Setting the Operating Mode

Operating mode? Yup.

Folks on-stage really only need to know where the cameras are and which camera is currently live. For the person operating the camera it is also useful for them to know whether the channel is inactive (neither in program nor preview) and to be informed if there are any error conditions or operating issues with the STAC. To accommodate this, you can select the Operating Mode of the STAC to either "Camera Operator" or "Talent".

In Talent mode, the display shows red when the tally channel being monitored is live (a.k.a.: Active, Program, PGM, On Air) or green otherwise. Refer back to the picture [up there](#tallystates) in the Quick Start section.

In Camera Operator mode, the display is red when the channel is live, green when in Preview (a.k.a.: Inactive, Standby, PVW) and "dotted purple" when Unselected (neither in Program nor Preview). If the operation of the STAC becomes anything other than normal, the display changes to the alert the operator. More on that down in the [Troubleshooting](#Troubleshooting) section.

The sequence of steps to change the Operating Mode is the same as for setting the tally channel so this section is abbreviated. Check out [Setting the Tally Channel](#SettingtheTallyChannel) above for the long-winded version.

The Operating Mode is displayed as a purple letter on a black background. A **C** means the STAC is in **Camera Operator Mode**. A purple **T** is shown for **Talent Mode**.
<center>
![Operating Mode](./images/displays_opmode.png)
</center>
To change the Operating Mode:

1. When the purple **C** or **T** is shown, press and hold the button.
1. After about two seconds the display will change to an orange letter on a purple background.
1. Click the button to toggle between the two Operating Modes.
1. When the desired mode is shown on the display, press and hold the button.
1. After about two seconds the green confirmed checkmark will be shown briefly and the STAC will exit select mode.
1. The new Operating mode will be displayed as a purple letter on a black background.

You can press and hold again to change the Operating Mode or if you click the button, move on to displaying the next operating parameter.

When in a select state, the STAC has a timeout feature. If after entering the select state, no clicks of the display button are detected for about 30 seconds, the select state is cancelled and the display and Operating Mode will revert back to the value it had before entering the select state. From here the button can be clicked to advance to the next parameter or press and hold to enter the select state again.

The STAC also remembers the last set Operating Mode across power cycles. Genius.

### <a name="SettingtheBrightnessLevel"></a>Setting the Brightness Level

To accommodate a range of ambient light levels when using the STAC, the brightness of the display can be changed. As with changing the active tally channel, the method is the same so this section is abbreviated. Check out [Setting the Tally Channel](#SettingtheTallyChannel) above for the extended directors cut.

The current brightness level is shown as a white number on a spectacular field of red and green dots. Yes, it looks like Christmas. There are six brightness levels, increasing from **1** (lowest) to **6** (brightest). Bet you didn't see that coming. Anyway, as levels are selected, the display brightness changes to match.
<center>
![Brightness Select](./images/displays_bright.png)
</center>
To change the Brightness Level:

1. When the current Brightness Level number is shown shown, press and hold the button.
1. After about two seconds the display will change to an orange number with a white stripe on either side.
1. Click the button to increase the Brightness Level.
1. At Brightness Level 6, the next click will wrap around to Brightness Level 1. You can keep clicking to find the ideal photon bombardment.
1. At the desired Brightness Level, press and hold the button.
1. After about two seconds the green confirmed checkmark will be shown briefly and the STAC will exit the select state.
1. The new Brightness Level is set.

You can press and hold again to change the Brightness Level or if you click the button, the STAC will start the WiFi connection sequence.

When in a select state, the STAC has a timeout feature. If after entering the select state, no clicks of the display button are detected for about 30 seconds, select state is cancelled and the display and Brightness Level will revert back to the value it had before entering the select state. From here the button can be clicked to advance to the WiFi connect sequence or press and hold to enter the select state again.

Here's the bonus feature. *The Brightness Level can be changed while the STAC is monitoring the active tally channel!* Works in either Camera Operator or Talent mode. Mind blowing I know. The method is the same as described here. Just press and hold the display button. The STAC will flip into the Brightness Level select state. Set the level as you'd like, then press and hold the button. The confirmation checkmark will be briefly shown and the STAC will return to monitoring the tally channel. Whoa! And yes, the timeout feature is active here as well.  
<center>![Brightness Op Change](./images/op_brightchange.png)</center>
The STAC engages its elephant brain to remember the last set Brightness Level across power cycles. One and done!
<br><br>
## <a name="ReconfiguringtheSTAC"></a>Reconfiguring the STAC

At some point, probably, one or more of the WiFi SSID, Password or Roland device IP address will change. Or maybe you want to take your STAC on the road. Who doesn't like a road trip? In this case you'll want to reconfigure the STAC to match the new settings. We got you covered.

To reconfigure the STAC you'll use the Reset button and Display button in combination. If this is your first reconfiguration rodeo, best to read though the rest of this section and then come back here. It's OK. We'll wait...

1. Power up the STAC. It will pause at the tally channel display.
1. Press and hold down the Reset button on the side of the STAC.
1. While continuing to hold down the Reset button, press and hold down the Display button.
1. While still holding down the Display button, release the Reset button. (Insert gratuitous Kraken reference.)
1. Release the Display button when the orange reconfiguration icon starts flashing on the display.

If you did this little dance right, (guess you figured it'll take two hands) the orange reconfiguration icon will stay on after flashing. The STAC is now waiting patiently for the new configuration information to head its way.
<center>
![Configuration Change](./images/stac_config_change.png)
</center>
If the STAC returns to the tally display, just give the two-button dance another whirl. You got this.

From here follow the directions under [First Time Configuration](#FirstTimeConfiguration) above; connecting to the STAC by WiFi and using a browser to bring up the Configuration form and so on.

If you mysteriously entered this reconfiguration state by accident or you change your mind at this point, click the Reset button and the STAC will return to its normal startup sequence.

That's all there is to it.
<br><br>
## <a name="FactoryReset"></a>Factory Reset

All righty then. Maybe you want to hand off your STAC to someone else or just for giggles restore the STAC to its "fresh-out-of-the-box" state. Doing this erases the SSID, Password, Roland Smart Tally IP address, port number and maximum tally channel number from the STAC and sets the active tally channel, operating mode and brightness level to their default values. Those defaults are: 

* Tally Channel 1,
* Camera Operator mode and
* Brightness Level 1.

To do a factory reset, you'll follow the same procedure as [Reconfiguring the STAC](#ReconfiguringtheSTAC) except, instead of releasing the Display button at step 5, keep holding down the Display button after the reconfiguration icon stops flashing. In about two seconds, the display will show the factory reset icon. Release the Display button. A moment later the STAC will be completely reset. The red Configuration Required icon will flash and then stay on.
<center>
![Factory Reset](./images/cleanslate.png)
</center>
You can remove power if you'd like or, to put the STAC back into service, follow the directions under [First Time Configuration](#FirstTimeConfiguration) above; connecting to the STAC by WiFi and using a browser to bring up the Configuration form and so on.

Once you've entered factory reset state, there is no turning back. Not a super big deal, just do the configuration thing once more and you're good to go.

## Back to the Beginning

One more thing...

The sections above show how the Reset button is used with the Display button to reconfigure or factory reset the STAC. 

The other trick is, when used by itself, the Reset button will return the STAC to its startup sequence. Why would you want to do this? Most common reason is let's say you just confirmed the tally channel or the operating mode but then realized you actually meant to change one or the other. Yoi!

In this case, click the reset button. The STAC display will go blank, then the power light will come on, followed by display of the active tally channel. Now you can press and hold to change that or click through to the operating mode and put that thing where you want it. The brightness level can be changed while the STAC is in its operating state so no need to restart the STAC in that case, but you could click the reset button to get there too.

Much easier than removing and reconnecting power.


## <a name="SpecConfig"></a>Special Configurations

In most situations the STAC is used to communicate directly with a physical Roland switch or device. When this is the case the IP address and port number entered on the web browser configuration form are the those of that Roland device. Just as described back in [First Time Configuration](#FirstTimeConfiguration).

However, if you are using a STAC as in an environment where the STAC is instead communicating with some other device or service that emulates a Roland Smart Tally switch, it may be required to change the STAC Smart Tally IP address and port number so that the STAC and that system are on the same wavelength. An example would be using the STAC with [Tally Arbiter](http://www.tallyarbiter.com) where tally devices from a variety of providers can be aggregated into a unified tally information system.

Refer to the documentation from the provider of the Roland emulator or service on how to configuration the STAC if this applies to you.

Hey, we kind of got our geek on in here. If this entire section is sounding completely foreign, we  get it. Just ensure when you configure the STAC that the IP address matches the actual Roland device and that the port is set to its default of <b>80</b> and you'll be in business.

<br><br>
## <a name="Troubleshooting"></a>Troubleshooting

Ahh life. That thing that happens when you're busy making other plans. Believe it or not at times the STAC may get uppity and display a couple of things to let you know it's not quite happy.

Fortunately, the list of things that can go wrong is short. (Oh boy, just challenged Murphy to a duel!)

The STAC is pretty robust and it will usually correct itself after encountering an error, returning to its normal operating state in short order.

If something is amiss, take a look below at the symptom and a step or two you can take to fix 'er up will be there. If not, shoot us a note and we'll see what we can do.

But before charging forth, know that the STAC will mask most of the errors on the display when its Operating Mode is set to Talent. No need to confuse the on-stage folks.

### <a name="WiFiDeathLoop"></a>_The WiFi Connect Loop of Death_

**What you're seeing:**<div style="margin-left: 2em;">The STAC display shows the orange connecting to WiFi icon and then after a minute it changes to a flashing red WiFi icon and then back to orange.

This condition is seen in both Operating Modes.
</div>
**What's causing it:**<div style="margin-left: 2em;">The STAC is trying to connect to the WiFi network. It tries this for about a minute and if it can't it flashes the red icon to let you know it's trying but there is still no connection. It keeps trying forever.
</div>

**What to try:**

* *If the STAC has successfully connected this WiFi network before:*
    - Check that the WiFi access point is turned on.
    - Check that the STAC is within range of the access point. Move it nearer to the access point and see if it can connect.
    * Has the SSID or Password of the network been changed? Check with someone that might know. If so, whew&mdash;problem solved. Just pop up to the section on [Reconfiguring the STAC](#ReconfiguringtheSTAC). Check and re-enter the new SSID and Password information, submit that and the other info required and you'll be great.

* *If the STAC has never connected to this network:*  
    - It is probably misconfigured. Likely means one or both of the WiFi SSID or Password was entered incorrectly on the web browser form. Hey, it happens. We're all human. Just pop up to the section on [Reconfiguring the STAC](#ReconfiguringtheSTAC). Check and re-enter the SSID and Password, submit that and the other info required and you'll be great.
</div>

### _Tally Status not Changing_

**What you're seeing:**  
<div style="margin-left: 2em;">The tally status display on the STAC does not change. It's stuck on green, red or "dotted purple".

This condition is seen in both Operating Modes.
</div>
**What's causing it:**  
<div style="margin-left: 2em;">Most likely the STAC is set to monitor a channel on the switch that is not changing.</div>  
**What to try:**<div style="margin-left: 2em;">

* Confirm with the person doing the switching that the correct tally channel is being monitored.
* If not, click the Reset button on the side of the STAC and when the tally channel is displayed, follow the steps under [Setting the Tally Channel](#SettingtheTallyChannel) to change the active channel.
</div>

### <a name="BPX"></a>_A Big Purple X?_

**What you're seeing:**
<div style="margin-left: 2em;">There is is a big purple X on the display!
<center>
<img src="./images/err_stnoconnect.png">
</center>
</div>
**What's causing it:**
<div style="margin-left: 2em;">This is the STAC letting you know that it cannot communicate with the Roland switch.  
Everything else is OK though.</div>

**What to try:**

* *If the STAC has been working with this switch before:*
    * Check that the Roland switch is turned on.
    * Check that the switch is connected to the WiFi network.
    * Has the IP address of the switch changed? Check with someone that might know. If so, whew&mdash;problem solved. Just pop up to the section on [Reconfiguring the STAC](#ReconfiguringtheSTAC). Check and re-enter the new IP Address of the switch, submit that and the other info required and you'll be great.

* *If the STAC has never connected to this switch before:*
    *  The IP address or port number of the switch was probably entered incorrectly on the web browser Configuration form. Pop up to the section on [Reconfiguring the STAC](#ReconfiguringtheSTAC). Check and re-enter the IP Address and port number of the switch, along with the other info required and you'll be good to go.

* *If the STAC is being used in an emulated environment:*
    * The IP address or port number of the STAC and the emulated device are probably not in sync. Check the configuration requirements of the system and change either the configuration of the system or the STAC as in [Reconfiguring the STAC](#ReconfiguringtheSTAC) so they match. See also [Special Configurations](#SpecConfig).

<a name="BPX"></a>**Other Considerations**  
<div style="margin-left: 2em;">It is possible that the big purple X (BPX) will appear and then a second or two later disappear with the normal channel status display shown.

After asking the switch for the status of the active channel, the STAC waits about a second for the reply to come back. If it doesn't, the reply has "timed out" and the BPX is displayed. The STAC is continuously polling the switch and next time round, the reply may come back before the time-out period. In this case the current status of the active channel is shown.

If the BPX appears often but the STAC soon returns to displaying the active channel status, it may mean:

* there are too many devices (STAC or other Smart Tally clients) asking the switch for status, causing a backlog;
* the network is busy with other traffic (live streaming being a big one) causing delays in the communication between the switch and the STAC.

If this seems to be the case, have a chat with the some folks about the state of the network.</div>

### _The Red WiFi Interrupt Screen!_

**What you're seeing:**
<div style="margin-left: 2em;">The STAC was operating normally but a suddenly a flashing red WiFi icon appeared on the display, followed by the orange WiFi icon.

This condition is seen in both Operating Modes.</div>
**What's causing it:**  
<div style="margin-left: 2em;">The STAC was doing its thing but lost its WiFi connection. Bummer. The STAC is now in its WiFi connect sequence and will keep trying to connect.</div>

**What to try:**
<div style="margin-left: 2em;">Check for the same things under [The WiFi Connect Loop of Death](#WiFiDeathLoop).  

After the WiFi connection is re-established, you'll need to click through the normal startup tally channel, operating mode and brightness level displays.</div>

### _Purple Question Mark?_

<b>What you're seeing:</b>
<div style="margin-left: 2em;">There is a purple question mark on the display. It may appear intermittently or for longer periods of time.</div>
<center>
![PQM](./images/err_badreply.png)
</center>**What's causing it:**  
<div style="margin-left: 2em;">Ahh, well. What we have here is a failure to communicate. In a way that the STAC understands. Meaning the STAC sent a tally status request to the switch, and a reply was received before the time-out period, but the reply was gibberish.
</div>

**What to try:**  
<div style="margin-left: 2em;">Probably not a lot on this one. The STAC and the switch should get things sorted the next time they talk. If not, it's *possible* there are network congestion issues like mentioned in the [Other Considerations](#BPX2) bit in the [Big Purple X?](#BPX) problem above.

If the STAC is being used in an emulated environment as in [Special Configurations](#SpecConfig) it's possible the emulated device is not responding in a way the STAC is expecting. If you think this is the case, take a look at the *STAC Communications.md* document in the repository.
</div>

### _The Big Red X_

**What you're seeing:**
<div style="margin-left: 2em;">Well boss, there is a big red X on the display!
<center>
![BRX](./images/err_intern.png)
</center></div>

**What's causing it:**
<div style="margin-left: 2em;">Ack. Really hope you never see this one. It means the STAC had a big internal burp and got pretty confused.</div>

**What to try:**
<div style="margin-left: 2em;">With this one there is really nothing to do. The STAC should sort itself out, recover and carry on with its life as normal.

If it does seem to be mis-behaving for a longer period of time, click the Reset button. You'll have to click through the normal startup tally channel, operating mode and brightness level displays after which the STAC should follow through with the rest of its normal WiFi connect startup sequence and then resume normal operation.</div>
<br><br>
## Acknowledgements

Exactly half an ohnosecond into our very first church livestream it became excruciatingly obvious that radio coms alone weren't going to cut it. Our camera operators *needed* a tally light system. The search began.

Wired systems were plentiful but running extra cable about the building, tethering the cameras down didn't seem the way to go.

A bit of reading showed our newly acquired Roland V-60HD video switch had this funky Smart Tally capability. Surely I figured, that's the solution. We gave that a shot for a few weeks but having our operators use their own phones wasn't working out so well. Plus the displays were too large and the phones had to be be configured apart from their normal setup each time to work properly. Back to the drawing board.

A few WiFi capable commercial options were found but the per unit cost was stupid expensive. And I mean waaaaay stupid for what they did.

More searching led us to the [Tally Arbiter](http://www.tallyarbiter.com) project headed by the amazing Joseph Adams and the [Tech Ministry](https://techministry.blog) folks. I set up Tally Arbiter with help from Joseph and with a bit of time searching and poking about figured how get the Tally Arbiter code running on an [M5Stack](https://m5stack.com) [ATOM](https://docs.m5stack.com/#/en/core/atom_matrix) and our livestream computer. It worked great and the size and low cost of the ATOM units meant it could be easily deployed.

However, we were only running with that single video switch and with the entire production and tech team being volunteers where only a few had a technical background, I was forced to come to the conclusion that as great as Tally Arbiter is, it was beyond the skill set of our team to set up, configure and maintain in a sustainable fashion.

But we had a dedicated WiFi network, these super little ATOM units and the Roland switch that could speak Smart Tally&mdash;so why not just let the two of them talk to each other directly?

There's a phrase about fools rushing in where angels fear to tread. And to not prolong things too much, I jumped in and started to bang my fingers on the keyboard and my head against the wall. With help from newly found on-line friends the first version of what is now STAC was hacked together in a few weeks. Add a dash of feedback from our teams and the feature list and capabilities grew to what is being released today.

So thanks again [Joseph](https://techministry.blog/contact/).

For the support from the folks that hang around the [reddit](http://reddit.com) [Arduino](https://www.reddit.com/r/arduino/), [Arduino Projects](https://www.reddit.com/r/ArduinoProjects/) and [ESP32](https://www.reddit.com/r/esp32/) subs&mdash;I'm grateful.

To the awesome and dedicated folks on our production, tech and worship teams&mdash;applause and praise.

But most of all to my wife who, as companion to a guy who can become obsessively dogged in pursuit of a goal has shown&mdash;once more&mdash;saintly patience and forbearance as I chewed up an inordinate number of hours, days, and weeks to bring this project to fruition. My dear, you are a true angel and I remain your blessed fool. Thank-you.

&nbsp;&nbsp;&nbsp;*- Xylopyrographer*  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*March, 2021*
<br><br><br>
---
---
## LICENSE
This work is made available under a Creative Commons NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0) license.

This is a human-readable summary of (and not a substitute for) the license. The full licensing terms are available at [https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode](https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode) or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042.

**You are free to:**  
<div style="margin-left: 2em;"><b>Share</b> — copy and redistribute the material in any medium or format  
<b>Adapt</b> — remix, transform, and build upon the material
</div>

The licensor cannot revoke these freedoms as long as you follow the license terms.

**Under the following terms:**

<div style="margin-left: 2em;">
<b>Attribution</b> — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
<br><br>
<b>NonCommercial</b> — You may not use the material for commercial purposes.
<br><br>
<b>ShareAlike</b> — If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.
<br><br>
<b>No additional restrictions</b> — You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.
</div>

**Notices:**

<div style="margin-left: 2em;">You do not have to comply with the license for elements of the material in the public domain or where your use is permitted by an applicable exception or limitation.

No warranties are given. The license may not give you all of the permissions necessary for your intended use. For example, other rights such as publicity, privacy, or moral rights may limit how you use the material.
</div>
<br><br>

---

###Revision History

**2021-04-05:** Revise to add user configurable Smart Tally device port number. Other minor changes.  
**2021-03-31:** Add the "Back to the Beginning" section. Other minor editorial changes.  
**2021-03-29:** First release.

---


