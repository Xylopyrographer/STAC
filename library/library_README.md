## Enhanced M5Atom Libraries
When M5Stack introduced the ATOM matrix the product API documentation include a function for filling the display with a single colour.

For reasons unknown, this function was not implemented in their library files.

So, I modified the following files to correct this oversight.

* M5Atom.h
* utility/LED\_Display.h
* utility/LED\_Display.cpp

To get the STAC sketch to compile, you'll have to replace the files provided by M5Stack with these modified versions. (Or, in the STAC sketch, replace every call to `M5.dis.fillpix();` with your own routine.)

### To use these libraries:
To use the modified libraries:

1. Navigate to your Arduino M5Atom libraries directory.
I'm not certain where this is kept on all platforms or where it was placed during your install. Searching for a folder named M5Atom should turn it up.<br><br>On macOS the default location is:<br><br>
`/Users/*YOUR_USER_NAME*/Documents/Arduino/libraries/M5Atom`<br><br>Inside that folder:

1. Rename the file *M5Atom.h* to something like *M5Atom_Original.h*
1. Copy the modified *M5Atom.h* file into this folder
1. Open the *utilities* folder that is inside the *M5Atom* folder.
2. Rename the file *LED\_Display.h* to something like *LED\_Display\_Original.h*
3. Rename the file *LED\_Display.cpp* to something like *LED\_Display\_Original.cpp*
4. Copy the modified *LED\_Display.h* and *LED\_Display.cpp* files into the *utilities* folder.

Note: You will have to repeat these steps whenever M5Stack updates their M5Atom libraries as their new version will overwrite these modified files.|
:---

---
## Improved Preferences.cpp
After doing a deep dive into the Espressif ESP 32 IDF NVS storage methods and comparing their documentation to the Arduino ESP32 library, I modified the *Preferences.cpp* file to ensure changes made to the NVS when using certain Arduino *Preferences* function calls were committed. Those changes are documented in the modified *Prefernces.h* file here.

While not essential for the STAC sketch to compile, I do believe these modified methods are more robust.

### To use this library
1. Navigate to your Arduino ESP32 folder.<br>
I'm not certain where this is kept on all platforms or where it was placed during your install. Searching for a file named *Preferences.cpp* should turn it up.<br><br>On macOS the default location is:<br><br>
`/Users/*YOUR_USER_NAME*/Library/Arduino15/packages/esp32/hardware/esp32/1.0.6/libraries/Preferences/src`  <br>

Note: As newer versions of the ESP32 Arduino libraries are released, the location will change to reflect the version number of those libraries. Thus, in the path above, you may need to replace *1.0.6* with the current version number.|
:--

Inside the *src* folder:

1. Rename the file *Preferences.cpp* to something like *Preferences_Original.cpp*
1. Copy the modified *Preferences.cpp* file into the *src* folder

You will have to repeat these steps whenever the Arduino ESP32 libraries are updated as the new version will overwrite this modified file.|
:--

A request has been submitted to the Arduino ESP32 team to incorporate these changes into the official release.

---