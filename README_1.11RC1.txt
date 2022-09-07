This is the last version of STAC that runs under arduino-esp32 core version 1.0.6. It is being tucked away here for posterity.

It was intended to be a version that would run under core 1.0.6 and the new version 2 cores. However, the version 2 cores introduced incompatibilities in the WiFi client methods that made implementing and maintaining workarounds to run under either core something I deemed not worth the effort.

To find those incompatibilities, a fair bit of debug code was added to this version in the main STAC.ino and STACSTS.h file (the latter located in the STACLib folder). This debug code, and the #define used to enable it remain.

The documentation for all the changes in this version has not been revised, however most changes are internal and so the STAC version 1.10 docs should be accurate enough from a user's perspective to get the STAC up and running.

The newer STAC version 2 code stream is fully tested and released with all documents up to date.

If you do want to run this under core 1.0.6, you will need to first install these libraries:

FastLED           // for driving the ATOM matrix display
JC_Button         // for driving the ATOM display button.
I2C_MPU6886       // for driving the ATOM IMU.

The modified Preferences library should also be installed as per the STAC version 1.10 release notes. The modified library is also part of the STAC version 1.10 release. (arduino-esp32 core v2 and greater include these changes and so the custom Preferences library is not needed.)

The libraries provided by M5Stack for the ATOM Matrix are not used in this version. See the STAC version 2 release notes if you're curious.

FWIW, if you really need a version that runs under core 1.0.6 that has some of the  improvements in this version, you could try grabbing the version 2.0 release and then replace the STACLIB/STACSTS.h file in version 2 with the 1.11_RC-1 version from here. Completely untested and unsupported but something you could tinker with.

Xylopyrographer
2022-08-29
