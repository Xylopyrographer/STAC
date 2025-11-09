### Here's what happens:

1. **Startup Animation** - RGB color sweep (red, green, blue)
    - Startup Animation confirmed but then the display starts flashing teal
2. **Operating Mode Detection** - Should say "NORMAL" (no jumper)
    - Serial monitor shows as per below. Does not show "`NORMAL`": 
    
    ```
    ╔════════════════════════════════════════════╗
    ║          STAC Application Starting         ║
    ╚════════════════════════════════════════════╝

    Board: M5Stack ATOM Matrix
    Software: v2.3.0

    --- Hardware Initialization ---

    --- Network & Storage ---
        [  1918][E][Preferences.cpp:506] getString(): nvs_get_str len fail: ssid NOT_FOUND

    ╔════════════════════════════════════════════╗
    ║               STAC Ready!                  ║
    ╚════════════════════════════════════════════╝
    ```


3. **Tally State** - Starts at NO_TALLY (purple display)
    - Yes u
4. **Button Short Press** - Cycles through:
   - Purple (NO_TALLY)
   - Green (PREVIEW)
   - Red (PROGRAM)
   - Blue (UNSELECTED)
       - State changes when button is pressed but returns to flashing teal when released.
5. **Button Long Press** - Enters provisioning mode (teal pulsing)
    - the device entrs this mode after the startup animation.
6. **Serial Output** - Clean startup messages and state changes
    - state changes are not seen on the serial monitor

### Peripheral Mode Test (if you have two ATOMs):

1. Connect pins 3-4 on one ATOM (peripheral jumper)
2. Reset it
3. Should detect "PERIPHERAL MODE"
    - not seen
4. Connect GROVE pins between the two
5. Button presses on Normal mode device should change Peripheral mode display!
    - PMode device display does not change when button on the NMode device is pressed.

Long press on the button of the PMode device does put it into "flashing teal" mode on its display.

