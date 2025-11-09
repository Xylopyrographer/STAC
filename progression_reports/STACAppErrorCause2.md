[{
	"resource": "/Users/robl/Documents/PlatformIO/Projects/STAC3/pioarduino/src/Application/STACApp.cpp",
	"owner": "C/C++: IntelliSense",
	"code": "276",
	"severity": 8,
	"message": "name followed by '::' must be a class or namespace name",
	"source": "C/C++",
	"startLineNumber": 130,
	"startColumn": 33,
	"endLineNumber": 130,
	"endColumn": 47,
	"origin": "extHost1"
}]

The error happens because you’re inside namespace `STAC::Application`, so unqualified Hardware refers to `STAC::Application::Hardware` (which doesn’t exist). Fully qualify the namespace and include the factory header.

/src/Application/STACApp.cpp
name followed by '::' must be a class or namespace name", Line: 130, Column: 33,

The error happens because you’re inside namespace `STAC::Application`, so unqualified Hardware refers to `STAC::Application::Hardware` (which doesn’t exist).

 Fully qualify the namespace and include the factory header.

Suggested fixes:

- Include the correct DisplayFactory header.
- Fully qualify to `STAC::Hardware::DisplayFactory` (or add an alias).

```cpp
// ...existing code...
#include "Hardware/DisplayFactory.h"  // adjust path if different
// ...existing code...

namespace STAC {
    namespace Application {

        // Optional: namespace alias for brevity
        namespace HW = STAC::Hardware;

        bool STACApp::initializeHardware() {
            Serial.println( "\n--- Hardware Initialization ---" );

            // Display
            // ...existing code...
            display = STAC::Hardware::DisplayFactory::create();
            // or with alias:
            // display = HW::DisplayFactory::create();

            // ...existing code...
        }
// ...existing code...
    } // namespace Application
} // namespace STAC
```

If you still get errors, confirm:

- The actual header/namespace for DisplayFactory (path and name).
- That DisplayFactory::create() returns std::unique_ptr to your display interface type.
