# Universe (C++ game)

## Code style

### General
- C-style modules with structs + free functions; minimal class usage.
- Global `g_context` for cross-system access.
- Hot-reload friendly: game code in a DLL and state stored in arenas.

### Naming and layout
- Types are `PascalCase` (`Context`, `RenderState`, `Entity`).
- Functions and variables are `camelCase` (`renderInit`, `gameUpdateAndRender`, `drawCommands`).
- Constants/macros are `UPPER_CASE` (`MAX_SHADER_VARIABLES`, `Megabytes()`).

### Compilation model
- "Unity build" style: `.cpp` files are included directly in other `.cpp` files (e.g. `main.cpp`, `game.cpp`).
- Platform-specific code is guarded by `PLATFORM_TYPE` macros.

### Memory and containers
- Arena allocation is explicit. `arenaAlloc()` and `arrayInit()` are the defaults.
- Arrays are trivial types only (`std::is_trivial_v`) and do not manage ownership.

### Math and transforms
- GLM math types (`vec3`, `mat4`, `quat`) are used throughout.
- Transform updates flow through `updateTransform()`, which recomputes world matrices and camera MVP data.

### Error handling
- Assertions and runtime checks use `ENSURE`, `LOGIC_ERROR`, and `HR_ASSERT` macros.
- Logging goes through `logInfo`, `logError`, etc. in `src/common/log.hpp`.

## Architecture

### High-level layout
- `src/main.cpp` is the host executable. It opens the window, loads assets, and hot-reloads the game DLL.
- `src/game.cpp` is the game DLL entry point. It owns gameplay state and drives update + render.
- `src/context.*` owns global state (`Context`) and the memory arenas used by the rest of the game.
- `src/platform_win32.cpp` provides the Win32 layer (window, input events, file IO, dynamic libs).
- `src/renderer_dx11.cpp` is a DirectX 11 renderer with its own resource cache and constant buffer mapping.
- `src/entity.*` implements a transform hierarchy and light/camera helpers.
- `src/geometry.*`, `src/shaders.*`, `src/texture.hpp` handle mesh generation/loading, shader variables, and textures.
- `src/gui.*` integrates ImGui for tooling/UI.
- `src/common/*` contains arena memory, arrays, math, string utils, and logging.

### Runtime flow
1) `main()` sets up `Context`, opens the window, and loads assets from `resources/`.
2) The game DLL is loaded, `gameInit()` builds initial entities and renderer resources.
3) The main loop polls platform events, detects window resize and DLL changes, and calls
   `gameUpdateAndRender()` every frame.
4) On reload, `gamePreHotReload()` is called, arenas are cleared, the DLL is reloaded, and `gamePostHotReload()` runs.

### Data ownership and memory
- Three arenas: `platformMemory` (app lifetime), `gameMemory` (cleared on hot reload), and `tempMemory` (per-frame).
- Arrays are fixed-capacity, arena-backed containers (`Array<T>`). Capacity is set at startup or reload.
- Most systems access state through `Context` and the global `g_context` pointer.

### Rendering
- `RenderState` holds draw commands, generated meshes, and loaded textures.
- Shaders use a fixed layout `Shaders::Variables` constant buffer; CPU-side mapping is built in `renderer_dx11.cpp`.
- `DrawCommand` stores shader choice, culling, flags, textures, and per-draw variables.
- Grid and skybox are special-cased (wireframe, culling, depth write).

### Entities and scene graph
- `Entity` has local/world transforms, optional draw command, and light/camera data.
- Parent-child relationships update transforms recursively via `updateTransform()`.
- Lights update shader variables across all drawables when changed.

### Assets
- Meshes are loaded from `.obj` files at startup (`Platform::loadAsset`, `loadMesh`).
- Textures and cubemaps are loaded with `stb_image` and uploaded to DX11 at renderer init.
- Generated meshes (triangle/quad/cube/sphere/grid) are created on demand.

### Input and GUI
- Win32 message handling writes into `InputState`. Helpers like `isKeyPressed()` read from it.
- ImGui is initialized in `guiInit()` and integrated into the render loop in `gameUpdateAndRender()`.

## Build

This project uses CMake and builds a host executable plus a game DLL.

```powershell
cmake -S . -B out/build
cmake --build out/build --config Debug
```

## Run

Run the host executable so it can hot-reload the game DLL:

```powershell
.\out\build\Debug\universe.exe
```
