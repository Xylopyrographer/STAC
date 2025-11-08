## Ambiguous `rgb_t` warnings

Those IntelliSense diagnostics mean: in the translation unit(s) where `LiteLED.h` is parsed, the unqualified name `rgb_t` refers to two different, unrelated types, so the IDE can’t decide which one you intend.

### The two conflicting definitions

1. Library (global namespace) – from llrgb.h:
```cpp
// struct with r,g,b byte fields
typedef struct {
    union { uint8_t r; uint8_t red; };
    union { uint8_t g; uint8_t green; };
    union { uint8_t b; uint8_t blue; };
} rgb_t;
```

2. Your project (`STAC::Display`) – from Colors.h:
```cpp
namespace STAC { namespace Display {
    using rgb_t = uint32_t; // 0x00RRGGBB packed color
}}
```

### Why they become ambiguous

In main.cpp you have:
```cpp
using namespace STAC;
using namespace STAC::Display;
```
Those `using namespace` directives pull `STAC::Display::rgb_t` into the current (global) scope of that translation unit. When the compiler (or IntelliSense) processes headers included after those directives (e.g., `LiteLED.h` via your display headers), any unqualified `rgb_t` now finds:

- The global struct `::rgb_t`
- The alias `STAC::Display::rgb_t` (imported by the using directives)

Result: name lookup yields two distinct types with the same identifier, so references like:
```cpp
esp_err_t setPixel( size_t num, rgb_t color, bool show = false );
```
are flagged “ambiguous”. The specific lines (188, 197, 204, 225) all mention an unqualified `rgb_t` in method declarations.

Important: This may be an IntelliSense (IDE) diagnostic only. The actual compiler can also complain depending on include order; either way, it’s a genuine name collision you should resolve.

### Options to fix the `rgb_t` ambiguity

Pick ONE of these approaches (they can be combined, but one is sufficient):

1. Remove broad `using namespace` directives (recommended):
   - Delete `using namespace STAC;` and `using namespace STAC::Display;` from headers and source files.
   - Qualify names explicitly: `STAC::Display::rgb_t`, `StandardColors::GREEN`, etc.
   - In `.cpp` you can add targeted aliases:
     ```cpp
     using STACrgb = STAC::Display::rgb_t;
     ```
     and keep using `STACrgb` locally.

2. Qualify the library’s usage inside your code:
   - Whenever you intend the library struct, write `::rgb_t`.
   - Whenever you intend your packed 32‑bit color, write `STAC::Display::rgb_t`.
   - (Still keep or remove the `using namespace`; explicit qualification resolves ambiguity.)

3. Rename your project type (if feasible):
   - Change `using rgb_t = uint32_t;` to something like `using rgb32_t = uint32_t;`
   - Update references. This permanently avoids clashes with libraries that choose `rgb_t` for a struct.

4. Wrap library include before using directives:
   - Include `LiteLED.h` prior to any `using namespace STAC::Display;`
   - This may quiet IntelliSense in some cases, but it’s brittle; later headers might reintroduce ambiguity. Prefer a real fix.

5. Create conversion helpers and always use one form:
   ```cpp
   inline ::rgb_t toLiteRGB(STAC::Display::rgb_t c) {
       return {
           .r = uint8_t((c >> 16) & 0xFF),
           .g = uint8_t((c >> 8) & 0xFF),
           .b = uint8_t(c & 0xFF)
       };
   }
   ```
   Then always pass `::rgb_t` to LiteLED APIs. This approach still needs name disambiguation.

### Choosing which rgb_t to use

- LiteLED APIs that accept `rgb_t` expect the struct (three separate bytes).
- Your display abstraction currently uses the packed 32‑bit form. Make sure you’re converting appropriately before calling LiteLED functions, or adopt the struct throughout and only pack/unpack at boundaries.

### Minimal, low-impact fix

The least intrusive change is to remove `using namespace STAC::Display;` from main.cpp (and any headers) and qualify the few color references:

Replace:
```cpp
display->setPixel(centerPixel, STACColors::POWER_ON, false);
```
with:
```cpp
display->setPixel(centerPixel, STAC::Display::STACColors::POWER_ON, false);
```

Then use `STAC::Display::rgb_t` explicitly. That prevents the alias from polluting the global scope and leaves the library’s `rgb_t` unambiguous.

### Verifying the fix

After adjusting:
- IntelliSense should stop flagging ambiguous `rgb_t` in `LiteLED.h`.
- Build should succeed unchanged (logic not altered).

### Summary

You have two distinct color representations sharing the same identifier due to a `using namespace` directive. Remove or restrict the directive and/or qualify the names to eliminate ambiguity. The `#warning` is an intentional library notice about unavailable DMA and can be safely ignored unless you need that feature.

