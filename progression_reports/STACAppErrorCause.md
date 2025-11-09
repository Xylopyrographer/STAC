Searched text for `Hardware::IDisplay`, 2 results

Read [](file:///Users/robl/Documents/PlatformIO/Projects/STAC3/pioarduino/include/Hardware/Display/IDisplay.h)

You’re getting “namespace STAC::Hardware has no member IDisplay” because `IDisplay` lives in `namespace STAC::Display`, not `STAC::Hardware`. In STACApp.h you’ve written:

```
std::unique_ptr<STAC::Hardware::IDisplay> display;
```

But the correct fully-qualified name (per IDisplay.h) is:

```
std::unique_ptr<STAC::Display::IDisplay> display;
```

Why the confusion:

- The path `IDisplay.h` can mislead; the folder “Hardware” doesn’t equal a `Hardware` namespace in that header. Only `STAC::Display` is declared.
- No alias or using-directive maps `Display` into `Hardware`.

How to fix:

1. Change the member declaration (and any other occurrences) to use `STAC::Display::IDisplay`.
2. Optionally add a `using` alias inside the class/namespace if you prefer shorter names:

   ```cpp
   using STACDisplay = STAC::Display::IDisplay;
   std::unique_ptr<STACDisplay> display;
   ```
3. Alternatively, if you intended a `Hardware` namespace wrapper for everything under `Hardware/`, you’d have to edit each header to nest the classes in `namespace STAC { namespace Hardware { ... } }` and then adjust all references. That’s a larger refactor; simplest is just using the existing declared namespace.

Double-check other members:

- `IIMU` likely resides in `STAC::Hardware` or `STAC::Sensors`; verify its actual namespace similarly.
- `IButton`, `GrovePort`, `PeripheralMode`—ensure their namespaces match your qualifiers (search their headers).

Immediate patch (only the display line):

Replace:

`std::unique_ptr<STAC::Hardware::IDisplay> display;`

With:

`std::unique_ptr<STAC::Display::IDisplay> display;`
