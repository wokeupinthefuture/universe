# Agent Context

## Project

C++ game engine with hot-reload support. Win32 + DirectX 11.

## Critical: Unity Build

This project uses unity builds - `.cpp` files are `#include`d into other `.cpp` files (mainly `main.cpp` and `game.cpp`). This means:
- **No `using namespace` statements** - causes collisions
- All symbols share the same translation unit
- Be careful with `static` variables

## Code Style

| Element | Convention | Example |
|---------|------------|---------|
| Types | PascalCase | `RenderState`, `Entity` |
| Functions/vars | camelCase | `renderInit`, `drawCommands` |
| Constants/macros | UPPER_CASE | `MAX_SHADER_VARIABLES` |

- C-style: structs + free functions, minimal classes
- Global state via `g_context` pointer
- Arena allocation: `arenaAlloc()`, `arrayInit()`
- Arrays are trivial types only (`std::is_trivial_v`)

## Key Files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Host exe, window, asset loading, hot-reload |
| `src/game.cpp` | Game DLL entry, gameplay, update+render |
| `src/context.*` | Global `Context` struct, memory arenas |
| `src/platform_win32.cpp` | Win32: window, input, file IO |
| `src/renderer_dx11.cpp` | DX11 renderer, resource cache |
| `src/entity.*` | Transform hierarchy, lights, cameras |
| `src/common/*` | Arena, arrays, math, strings, logging |

## Memory

Three arenas:
- `platformMemory` - app lifetime
- `gameMemory` - cleared on hot reload
- `tempMemory` - per-frame scratch

## Build & Run

```powershell
cmake -S . -B out/build
cmake --build out/build --config Debug
.\out\build\Debug\universe.exe
```
