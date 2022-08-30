## Improved Preferences Library
After doing a deep dive into the Espressif ESP 32 IDF NVS storage methods and comparing their documentation to the Arduino ESP32 library, the *Preferences.cpp* file was modified to ensure changes made to the NVS when using certain Arduino *Preferences* function calls were committed to NVS. Those changes are documented in the modified *Preferences.h* file here.

While not essential for the STAC sketch to compile, I do believe these modified methods are more robust.

### To use this library

Navigate to your Arduino ESP32 folder.

We're not certain where this is kept on all platforms or where it was placed during your install. Searching for a file named *Preferences.cpp* should turn it up.<br><br>On macOS the default location is:<br>
`/Users/*YOUR_USER_NAME*/Library/Arduino15/packages/esp32/hardware/esp32/1.0.6/libraries/Preferences/src`<br><br>On Windows the default location is:<br>
`C:\Users\*YOUR_USER_NAME*\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\Preferences\src`<br>

|&mdash; Note &mdash;<br>As newer versions of the ESP32 Arduino libraries are released,<br>the location will change to reflect the version number of those libraries.<br>Thus, in the path above, you may need to replace *1.0.6* with the current version number.|
:--:  
<br>

Inside the *src* folder:

1. Rename the file `Preferences.cpp` to something like `Preferences.cpp.orig`
2. Copy the modified `Preferences.cpp` file from this repository into the `src` folder

&mdash; Note &mdash;<br>You will have to repeat these steps whenever the Arduino ESP32 <br>libraries are updated as the new version will overwrite this modified file.|
:--:
<br>
A request has been submitted to the fantastic [arduino-esp32](https://github.com/espressif/arduino-esp32) team on GitHub to incorporate these changes into the official release.

