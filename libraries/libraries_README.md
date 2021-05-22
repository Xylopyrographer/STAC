## Enhanced M5Atom Libraries
When M5Stack introduced the ATOM Matrix the product API documentation included a function for filling the display with a single colour.

For reasons unknown, this function is not implemented in their library files (as of version 0.0.1).

So, I modified the following files to correct this oversight (and tweak a few other things too).

* `M5Atom.h`
* `utility/LED_DisPlay.h`
* `utility/LED_DisPlay.cpp`

To get the STAC sketch to compile, you'll have to replace the files provided by M5Stack with these modified versions. Or, in the STAC sketch, replace every call to `M5.dis.fillpix();` with your own routine.

But even if you do that, you'll still have to rename the `LED_DisPlay.h` and `LED_DisPlay.cpp` files in your `Arduino/libraries/M5Atom` folder to correct the typos in the file names to: `LED_Display.h` and `LED_Display.cpp`

### To use these libraries:
To use the modified libraries:

1. Navigate to your Arduino M5Atom libraries directory.
I'm not certain where this is kept on all platforms or where it was placed during your install. Searching for a folder named M5Atom should turn it up.<br><br>On macOS the default location is:<br>
`/Users/*YOUR_USER_NAME*/Documents/Arduino/libraries/M5Atom`<br><br>On Windows the default location is:<br>
`C:\Users\*YOUR_USER_NAME*\Documents\Arduino\libraries\M5Atom`<br><br>Inside that folder:

1. Rename the file `M5Atom.h` to something like `M5Atom.h.orig`
1. Copy the modified `M5Atom.h` file from this repository into the `Arduino/libraries/M5Atom` folder.
1. Open the `utility` folder that is inside the `Arduino/libraries/M5Atom` folder.
1. Rename the file `LED_DisPlay.h` to something like `LED_DisPlay.h.orig`
1. Rename the file `LED_DisPlay.cpp` to something like `LED_Display.cpp.orig`
1. From the `utility` folder in this repository, copy the modified `LED_Display.h` and `LED_Display.cpp` files into the `Arduino/libraries/M5Atom/utility` folder.

&mdash; Note &mdash;<br>You will have to repeat these steps whenever M5Stack updates their <br>M5Atom libraries as their new version will overwrite these modified files.|
:---:
<br>

## Improved Preferences Library
After doing a deep dive into the Espressif ESP 32 IDF NVS storage methods and comparing their documentation to the Arduino ESP32 library, I modified the *Preferences.cpp* file to ensure changes made to the NVS when using certain Arduino *Preferences* function calls were committed to NVS. Those changes are documented in the modified *Preferences.h* file here.

While not essential for the STAC sketch to compile, I do believe these modified methods are more robust.

### To use this library
1. Navigate to your Arduino ESP32 folder.<br>
I'm not certain where this is kept on all platforms or where it was placed during your install. Searching for a file named *Preferences.cpp* should turn it up.<br><br>On macOS the default location is:<br>
`/Users/*YOUR_USER_NAME*/Library/Arduino15/packages/esp32/hardware/esp32/1.0.6/libraries/Preferences/src`<br><br>On Windows the default location is:<br>
`C:\Users\*YOUR_USER_NAME*\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\Preferences\src`<br>

|&mdash; Note &mdash;<br>As newer versions of the ESP32 Arduino libraries are released,the location will change to reflect the version number of those libraries.<br>Thus, in the path above, you may need to replace *1.0.6* with the current version number.|
:--:  
<br>

Inside the *src* folder:

1. Rename the file `Preferences.cpp` to something like `Preferences.cpp.orig`
1. Copy the modified `Preferences.cpp` file from this repository into the `src` folder

&mdash; Note &mdash;<br>You will have to repeat these steps whenever the Arduino ESP32 <br>libraries are updated as the new version will overwrite this modified file.|
:--:
<br>
A request has been submitted to the fantastic Arduino ESP32 team to incorporate these changes into the official release.

