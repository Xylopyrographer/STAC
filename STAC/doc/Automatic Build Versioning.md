# Automatic Build Versioning

**Script:** `scripts/build_version.py`  
**Generated:** `include/build_info.h`  
**Status:** Automatic - runs before every build  
**Version:** 3.0.0+

---

## Overview

V3.0 uses **fully automatic build versioning** that generates unique build identifiers without any manual steps. The system automatically:

1. Extracts version from `Device_Config.h`
2. Generates MD5 hash of all source files (build number)
3. Retrieves git commit and branch information
4. Creates build timestamp
5. Generates `build_info.h` header

---

## What Gets Generated

Every build creates `include/build_info.h`:

```cpp
#define BUILD_VERSION "3.0.0-RC.9"       // From Device_Config.h
#define BUILD_NUMBER "a1b2c3"            // MD5 hash (last 6 chars)
#define BUILD_GIT_COMMIT "0cb6394"       // Git short hash
#define BUILD_GIT_BRANCH "v3_RC"         // Current branch
#define BUILD_TIMESTAMP "2025-11-21 14:30:45"
#define BUILD_DATE "2025-11-21"
#define BUILD_FULL_VERSION "3.0.0-RC.9 (a1b2c3)"
#define BUILD_INFO_STRING "3.0.0-RC.9 build:a1b2c3 git:0cb6394 2025-11-21"
```

---

## Usage in Code

```cpp
#include "build_info.h"

void printVersion() {
    Serial.println("Version: " BUILD_FULL_VERSION);
    Serial.println("Build:   " BUILD_INFO_STRING);
}
```

---

## Build Number Explained

**Build number** = Last 6 characters of MD5 hash of all source files

- **Unique**: Different source = different hash
- **Reproducible**: Same source = same hash  
- **Immediate**: Changes when any source file modified
- **Git independent**: Shows actual code state (including uncommitted changes)

### Build Number vs Git Commit

| Feature | Build Number | Git Commit |
|---------|-------------|------------|
| Tracks | Source file contents | Git repo state |
| Changes when | Any source modified | Git commit made |
| Shows uncommitted | Yes | No |

**Use both**: Git commit for version control, build number for actual code state!


## Build Console Output

```
============================================================
Build Information Generated:
  Version:      3.0.0-RC.9
  Build Number: a1b2c3
  Git Commit:   0cb6394
  Git Branch:   v3_RC
  Build Time:   2025-11-21 14:30:45
============================================================
```

---

## Verifying Builds

Two STAC devices with **same build number** have:
- Identical source code
- Same functional behaviour

Different build numbers mean:
- Source code differs
- May behave differently

---

**Last Updated:** November 21, 2025  

