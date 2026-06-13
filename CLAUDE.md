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

**VS Code debugging:** `.vscode/launch.json` provides cppdbg/cppvsdbg profiles. The `cwd` defaults to `${fileDirname}`, so when launching the executable directly set it to `${workspaceFolder}/build/Release` (or `Debug`) — otherwise asset paths will fail to resolve.

## Repository Layout

The codebase has been refactored from a monolithic `main.cpp` into a layered architecture:

- [src/main.cpp](src/main.cpp) — minimal entry point; delegates to `Application::run()`.
- [src/App/](src/App/) — application shell: `Application` (SDL/OpenGL setup), `GameBootstrap` (initialization/shutdown), `GameLoop` (main loop + scene management), `WindowEvents`.
- [src/Engine/](src/Engine/) — reusable engine systems:
  - [Renderer/](src/Engine/Renderer/): `Draw2D` (batched geometry), `TextRenderer`, `ParticleSystem`, `PostProcess` FBO, `DialogueUI`, `DecorRenderer`, `MiniMap`, `RenderLayer` ordering.
  - [Physics/](src/Engine/Physics/): zero-gravity Box2D world wrapper.
  - [Camera/](src/Engine/Camera/): 2D orthographic camera with smooth follow.
  - [Scripting/](src/Engine/Scripting/): Lua VM wrapper around sol2.
  - [Audio/](src/Engine/Audio/): `AudioSystem` for sound effects and music.
- [src/Game/](src/Game/) — gameplay systems:
  - **Core:** `GameState` (facade holding all subsystems), `Health.*`, `Drop.*`.
  - [World/](src/Game/World/): `TileMap`, `MapTileManager`, `TerrainGenerator`, `DecorationGen`, `MapRegion`/`RegionManager`, `TimeSystem`, `WeatherSystem`.
  - [Ability/](src/Game/Ability/): `Projectile` pool, `SuperStrength`, `Shield`, `Lightning`, `BondTechnique`.
  - [AI/](src/Game/AI/): `EnemyManager`, `PathfindingSystem` (A*).
  - [Social/](src/Game/Social/): `NPC`, `DialogueTree`, `Princess` (affection + following).
  - [Emotion/](src/Game/Emotion/): `EmotionSystem`, `VentAnimation`.
  - [Building/](src/Game/Building/), [Inventory/](src/Game/Inventory/), [Quest/](src/Game/Quest/), [Toy/](src/Game/Toy/): Stage 7 systems.
  - [Scenes/](src/Game/Scenes/): `IScene` interface, `SceneManager`, `AppMode` (MainMenu / Playing).
  - [Services/](src/Game/Services/): update services for combat, abilities, world, regions, progression, input, dialogue, save, audio, notices, enemy spawn, etc.
  - [Controllers/](src/Game/Controllers/): `InputController`, `InputState`, mode-specific input handlers.
  - [Presentation/](src/Game/Presentation/): views and presenters — `WorldRenderer`, `HudView`, `EntityView`, `AbilityView`, `BuildingView`, `MiniMapPresenter`, etc.
  - [Data/](src/Game/Data/): save/load infrastructure — `SaveSerializer`, `SaveMigration`, `SaveRepository`, `SaveSnapshotBuilder`, `SaveApplier`, config loaders for furniture/toys/quests/time/weather/emotion/dialogue.
  - `SaveSystem.*` — JSON save/load orchestration.
- [src/Utils/](src/Utils/): `Math.h` (deterministic noise, angle/lerp, collision tests), `ShaderUtils.h` (shader compilation).

CMake manually lists source files in [CMakeLists.txt](CMakeLists.txt). When adding a `.cpp`, add it to the `add_executable(Starchild2D ...)` list.

[assets/shaders/](assets/shaders/) and [assets/scripts/](assets/scripts/) are copied into `build/assets/` by CMake at configure time (not on every build).

[doc/](doc/) holds the Chinese roadmap and implementation notes. Key entries: [doc/技术栈.md](doc/技术栈.md), [doc/plan7.md](doc/plan7.md), [doc/map-spec.md](doc/map-spec.md), and the `怀旧主题-*.md` series.

## Runtime Architecture

Each frame uses a fixed 60 FPS timestep (`dt = 1.0f / 60.0f`):

```text
Input -> Physics -> Update services -> Manual collision checks -> Render to FBO -> Post-process -> UI direct to screen
```

The main loop ([src/App/GameLoop.cpp](src/App/GameLoop.cpp)) delegates to `SceneManager`, which routes to the active `IScene` (`MainMenu` or `Playing`). The scene calls into `WorldUpdateService` and the various `*Service` update functions, then renders via the `Presentation` layer.

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
- **Asset working directory:** Runtime paths like `assets/shaders/...` and `assets/scripts/...` are relative. Use the documented `cd build && ./Release/Starchild2D.exe` pattern unless debugger working directory is configured appropriately.
- **Shaders/scripts copy:** CMake uses `file(COPY ...)` for [assets/shaders/](assets/shaders/) and [assets/scripts/](assets/scripts/), so adding a file under [assets/](assets/) requires **re-running cmake configure** (`cmake -B build`) — incremental builds do not pick up new files.
- **Encoding:** MSVC compile options include `/source-charset:utf-8 /execution-charset:utf-8`. Prefer English comments in source when possible to avoid encoding-related build issues; Chinese comments already in the tree are kept.
- **Release subsystem:** MSVC release builds are configured for no console window via SDL main handling and Windows subsystem-related settings. Use `--config Debug` to keep a console and the `DEBUG` define (debug-only keys, extra logging).
- **Determinism:** Procedural terrain/decorations/AI are seed-driven. Use the helpers in [src/Utils/Math.h](src/Utils/Math.h) (`Math::hashRandom`, `Math::noise2D`) for new procedural content so save/load stays reproducible.
- **Commit messages:** Use short Chinese summaries like `修复Bug`, `阶段7-添加家具系统`, `日志`. Include the stage or subsystem when useful.

## Save/Load Notes

Save/load is JSON-based and uses world seed plus recorded state/modifications where implemented. Some state restoration may still be incomplete or staged; inspect [src/Game/Data/SaveSerializer.h](src/Game/Data/SaveSerializer.h) and [src/Game/Services/SaveApplier.h](src/Game/Services/SaveApplier.h) before assuming a field is fully persisted and reapplied. F5 = quick save, F9 = quick load. The Stage 7 schema is being extended with `coins`, `completedQuests`, `collectedItems`, region deltas, and (in progress) furniture placement, toy unlocks, 童心值, and weather/time snapshots.

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
