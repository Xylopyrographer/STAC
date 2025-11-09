**Xylopyrographer: Please analyse the code base and tell me where, based on the taly state,  is the color to send to the display selected?**

GitHub Copilot: Created 4 todos

Searched for files matching `**/TallyStateManager.*`, 4 matches

Read `file:///Users/robl/Documents/PlatformIO/Projects/STAC3/pioarduino/include/State/TallyStateManager.h`

Read `file:///Users/robl/Documents/PlatformIO/Projects/STAC3/pioarduino/src/State/TallyStateManager.cpp`

Searched for files matching `**/SystemState.*`, 4 matches

Read `file:///Users/robl/Documents/PlatformIO/Projects/STAC3/pioarduino/include/State/SystemState.h)

Read [](file:///Users/robl/Documents/PlatformIO/Projects/STAC3/pioarduino/src/State/SystemState.cpp`

Searched for files matching `**/OperatingModeManager.*`, 4 matches

Read `file:///Users/robl/Documents/PlatformIO/Projects/STAC3/pioarduino/include/State/OperatingModeManager.h`

Searched text for `STACColors|StandardColors|stateToColor|getStateColor|setPixel\(|fill\(|boardColor` (`**/src/**`), 20 results

Read `file:///Users/robl/Documents/PlatformIO/Projects/STAC3/pioarduino/src/main.cpp`

The tally-driven display color is selected in one central place: the static mapping function `TallyStateManager::stateToColor` (file TallyStateManager.cpp). Everything else just asks the manager for the current color and pushes it to the display.

### Where the color is chosen

File: TallyStateManager.cpp

Function: `Display::color_t TallyStateManager::stateToColor( TallyState state )`

Mapping logic:

- PROGRAM    → `STACColors::PROGRAM` (red)
- PREVIEW    → `STACColors::PREVIEW` (green)
- UNSELECTED → `STACColors::UNSELECTED` (blue)
- NO_TALLY   → `StandardColors::PURPLE` (purple)
- ERROR      → `STACColors::ALERT` (red alert)
- default    → `StandardColors::BLACK`

`getStateColor()` just calls `stateToColor(currentState)`.

### How it reaches the display

In `main.cpp`, whenever tally changes (button click, GROVE input, initial setup, post‑mode toggle), the code calls:

```cpp
display->fill( systemState->getTallyState().getStateColor(), true );
```

Lines (from your current main.cpp): initial setup and lines 199, 215, 250, 267 (plus the initial fill).

Display implementation (e.g., Display8x8.cpp) takes that `color_t` (your packed `rgb_t` alias) and converts it to board-specific ordering with `boardColor()` before setting pixels.

### Flow summary

1. Event (button press, peripheral read) triggers `TallyStateManager::setState(newState)`.
2. `currentState` updated; optional callback fires.
3. Caller requests the color via `getStateColor()`.
4. Display layer (`display->fill(...)`) receives packed color.
5. Hardware display implementation remaps bytes (`boardColor`) and writes them out.

### Key files/functions

- Color mapping: `TallyStateManager.cpp → stateToColor`
- State access: `TallyStateManager.h → getStateColor`
- Usage sites (apply to display): main.cpp multiple `display->fill(...)` calls
- Pixel writing pipeline: `Display8x8.cpp` (and analogous 5x5) `→ fill`, `setPixel`, which calls `boardColor` then `LiteLED`.

### If you need to change colors

Edit only the switch in `stateToColor`. All callers will reflect the new mapping without further changes.

Let me know if you’d like a refactor (e.g., data-driven table instead of switch) or to add brightness differences (e.g., ERROR using a brighter variant).
