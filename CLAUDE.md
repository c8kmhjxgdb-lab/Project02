# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Starchild2D** (星愿之子) is a C++17 2D top-down action RPG using SDL2 for window/input, OpenGL 3.3 Core + GLEW for rendering, Box2D v3 for physics, LuaJIT/sol2 for scripting, GLM for math, and nlohmann-json for save data. The game is currently mid-Stage 7 of the roadmap ([doc/plan7.md](doc/plan7.md)): Stage 6 foundations (multi-region maps, A* pathfinding, save/load, day/night, weather) are done; Stage 7 is in progress and adds secret-base building, furniture, inventory, toys, quests, and a 0–1000 童心值 (childlike-heart) meter that drives endings. Stage 8 (audio, UI polish, main story) is still future work. A companion agent guide with build/style/test conventions lives at [AGENTS.md](AGENTS.md).

## Build, Run, and Verification

```bash
# Configure from repository root. vcpkg manifest dependencies are installed if
# VCPKG_ROOT is set, or if a vcpkg checkout exists at ./vcpkg.
cmake -B build

# Build
cmake --build build --config Release
cmake --build build --config Debug

# Run from build/ so copied assets resolve correctly
cd build && ./Release/Starchild2D.exe
cd build && ./Debug/Starchild2D.exe
```

Visual Studio generators are also used in this project when needed:

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake -B build -G "Visual Studio 18 2025" -A x64
```

There is currently no integrated test framework, no `ctest` configuration, and no single-test command. Verification is primarily: build the relevant config, run the game from [build/](build/), and manually/visually check gameplay. Debug builds define `DEBUG` and include debug-only functionality.

## Repository Layout

- [src/main.cpp](src/main.cpp) owns SDL/OpenGL setup, the monolithic `GameState`, initialization order, input, fixed-step game loop, collision orchestration, save/load hotkeys, and render sequencing. It is the single biggest file in the repo (~3.3k LOC) — most engine wiring lives there rather than in a framework.
- [src/Engine/](src/Engine/) contains reusable engine systems:
  - [Renderer/](src/Engine/Renderer/): `Draw2D` (batched geometry), `TextRenderer`, particle rendering, `PostProcess` FBO, `DialogueUI`, `DecorRenderer`, `MiniMap`, and `RenderLayer` ordering types.
  - [Physics/](src/Engine/Physics/): zero-gravity Box2D world wrapper and helpers.
  - [Camera/](src/Engine/Camera/): 2D orthographic camera and smooth follow.
  - [Scripting/](src/Engine/Scripting/): Lua VM wrapper around sol2; talks to dialogue trees and ability config.
- [src/Game/](src/Game/) contains gameplay systems:
  - [World/](src/Game/World/): `TileMap`, `MapTileManager` (use this for gameplay tile edits), `TerrainGenerator`, `DecorationGen`, `MapRegion`/`RegionManager`, `TimeSystem`, `WeatherSystem`.
  - [Ability/](src/Game/Ability/): `Projectile` pool, `SuperStrength`, `Shield`, `Lightning`, `BondTechnique`.
  - [AI/](src/Game/AI/): `Enemy` manager and `PathfindingSystem` (A*).
  - [Social/](src/Game/Social/): `NPC`, Lua-driven `DialogueTree`, and the header-only `Princess` (affection + following).
  - [Emotion/](src/Game/Emotion/): grievance / venting animation (short-term mood in Stage 7; long-term 童心值 lives on `GameState`).
  - [Building/](src/Game/Building/), [Inventory/](src/Game/Inventory/), [Quest/](src/Game/Quest/), [Toy/](src/Game/Toy/): Stage 7 systems for secret-base placement, item ownership, quest tracking, and toys.
  - `Health.*`, `Drop.*`, `SaveSystem.*` sit at the Game root and are shared by everything — health, pickups, JSON save/load.
- [src/Utils/](src/Utils/) holds small helpers: `Math.h` (deterministic 2D noise, angle/lerp helpers, AABB/circle tests) and `ShaderUtils.h` (file → program compile).
- [assets/shaders/](assets/shaders/) and [assets/scripts/](assets/scripts/) are copied into `build/assets/` by CMake at configure time (not on every build).
- [doc/](doc/) holds the Chinese roadmap and implementation notes. Key entries: [doc/技术栈.md](doc/技术栈.md), [doc/plan5.md](doc/plan5.md), [doc/plan6.md](doc/plan6.md), [doc/plan7.md](doc/plan7.md), [doc/map-spec.md](doc/map-spec.md), and the `怀旧主题-*.md` series that reimagine the game as a 六一 / 8090-00 后 nostalgia universe (world, 9 princesses, NPCs, BOSSes, music cues, promo script).
- [AGENTS.md](AGENTS.md) is a sibling guide covering agent-facing conventions (commit message style, manual verification flow, build-artifact discipline) and overlaps with this file on a few points — read it alongside.

CMake manually lists source files in [CMakeLists.txt](CMakeLists.txt). When adding a `.cpp`, add it to the `add_executable(Starchild2D ...)` list.

## Runtime Architecture

Each frame uses a fixed 60 FPS timestep (`dt = 1.0f / 60.0f`):

```text
Input -> Physics -> Update systems -> Manual collision checks -> Render to FBO -> Post-process -> UI direct to screen
```

Rendering is mostly shader/SDF/math based rather than sprite based. The main world render path draws tile map and water, low decorations, y-sorted objects/enemies/Princess, high decorations, character, projectiles, particles, then applies post-process effects before drawing UI such as health, dialogue, and minimap.

Most gameplay collision handling is explicit distance/shape logic in the main loop rather than Box2D contact callbacks. Preserve update ordering carefully, especially enemy/projectile cleanup after collision processing.

## Important Patterns and Constraints

- **Box2D v3 C API:** Use `b2WorldId`, `b2BodyId`, `b2CreateBody`, `b2Body_ApplyForceToCenter`, etc. Do not use the older Box2D C++ `body->...` API.
- **Top-down physics:** The Box2D world is zero gravity. Player and enemies are dynamic bodies with damping/speed limiting, not platformer physics.
- **Tile edits:** Use `MapTileManager` for tile modifications that matter to gameplay. Do not call `TileMap::setTile()` directly when physics sync or save tracking is needed.
- **Regions:** Region state is managed through `RegionManager`/`MapRegion`. After transitions, the standalone `GameState::tileMap` is not the authoritative current map.
- **Entity pools and IDs:** Projectiles, enemies, and particles use manager-owned pools and type-safe IDs such as `ProjectileId` and `EnemyId`. Be careful with pointers returned from manager vectors; avoid cleanup before consumers finish using them.
- **Exploder enemies:** Dead-state exploders may still need explosion processing, so code paths intentionally use active enemies rather than only alive enemies in some places.
- **Draw2D batching:** Call `Draw2D::beginFrame(viewProj)`, issue draw calls, then `Draw2D::endFrame()` to flush. Mixing in other GL draws mid-frame will corrupt the batch.
- **GLEW initialization:** Set `glewExperimental = GL_TRUE` before `glewInit()` for OpenGL 3.3 Core.
- **Render-layer ordering:** Use the `RenderLayer` enum from [src/Engine/Renderer/RenderLayer.h](src/Engine/Renderer/RenderLayer.h) to sort y-coordinated draws (low decor → objects/Princess/enemies → high decor → character → projectiles → particles).
- **Asset working directory:** Runtime paths like `assets/shaders/...` and `assets/scripts/...` are relative. Use the documented `cd build && ./Release/Starchild2D.exe` pattern unless debugger working directory is configured appropriately. A `.vscode/launch.json` is provided for VS Code cppdbg/cppvsdbg attach/launch profiles; `cwd` defaults to `${fileDirname}`, so set it to `${workspaceFolder}/build/Release` (or `Debug`) when launching the executable directly.
- **Shaders/scripts copy:** CMake uses `file(COPY ...)` for [assets/shaders/](assets/shaders/) and [assets/scripts/](assets/scripts/), so adding a file under [assets/](assets/) requires **re-running cmake configure** (`cmake -B build`) — incremental builds do not pick up new files.
- **Encoding:** MSVC compile options include `/source-charset:utf-8 /execution-charset:utf-8`. Prefer English comments in source when possible to avoid encoding-related build issues; Chinese comments already in the tree are kept.
- **Release subsystem:** MSVC release builds are configured for no console window via SDL main handling and Windows subsystem-related settings. Use `--config Debug` to keep a console and the `DEBUG` define (debug-only keys, extra logging).
- **Determinism:** Procedural terrain/decorations/AI are seed-driven. Use the helpers in [src/Utils/Math.h](src/Utils/Math.h) (`Math::hashRandom`, `Math::noise2D`) for new procedural content so save/load stays reproducible.

## Save/Load Notes

Save/load is JSON-based and uses world seed plus recorded state/modifications where implemented. Some state restoration may still be incomplete or staged; inspect [src/Game/SaveSystem.h](src/Game/SaveSystem.h) and [src/main.cpp](src/main.cpp) before assuming a field is fully persisted and reapplied. F5 = quick save, F9 = quick load. The Stage 7 schema is being extended with `coins`, `completedQuests`, `collectedItems`, region deltas, and (in progress) furniture placement, toy unlocks, 童心值, and weather/time snapshots.

## Controls Worth Knowing for Manual Verification

- WASD / Arrow keys: move
- Mouse wheel / `+` / `-`: zoom
- J: fireball
- L: ice spike
- Q: chain lightning
- F: shield
- G: bond technique
- Space: flight in current implementation
- K: grab/throw
- E: interact/dialogue/vent
- F5 / F9: quick save/load
- H: debug heal in Debug builds
- Esc: exit
