## Source Conventions

- **C++ files are CRLF.** Do **not** overwrite whole `.cpp`/`.h` files with the `Write` tool on Windows — it can corrupt line endings into `\r\r\n`. Use `Edit` for surgical hunks. For new/large files, write via PowerShell `UTF8Encoding($false)`.
- `PascalCase` for variables. `*.generated.h` is always the **last** include in a header. Prefer forward declarations in headers; put implementation includes in `.cpp`.
- Never `new` a `UObject` — use `NewObject` / `CreateDefaultSubobject` / spawn APIs.
- Before modifying code, read the related files first and decide based on the existing implementation.

## Unreal Engine Basics

- Check the project's actual engine version before relying on any API — be cautious of deprecated or outdated APIs, includes, or practices.

### Coordinate system

UE uses a Z-up, left-handed coordinate system:

- **X-axis (Red):** forward/backward. Positive = forward.
- **Y-axis (Green):** left/right. Positive = right.
- **Z-axis (Blue):** up/down. Positive = up.

### Units

| Quantity | Unit |
|---|---|
| Distance/Length | Centimeters (cm) |
| Mass | Kilograms (kg) |
| Time | Minutes (min), Seconds (s) |
| Angles | Degrees (deg) |
| Speed/Velocity | Meters per Second (m/s) |
| Temperature | Celsius (C) |
| Force | Newtons (N) |
| Torque | Newton Meters (N·m) |
