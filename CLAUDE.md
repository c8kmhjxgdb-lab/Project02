# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Starchild2D** (星愿之子) — a 2D top-down action RPG built in C++17 with OpenGL rendering, Box2D physics, and Lua scripting. The project is currently in **Stage 6** (Multi-region, Pathfinding, Save/Load, Day/Night) of an 8-stage development plan.

## Build & Run

```bash
# Configure (from project root)
cmake -B build

# Build
cmake --build build --config Release   # Release build
cmake --build build --config Debug     # Debug build

# Run (must run from build/ dir so shaders resolve correctly)
cd build && ./Release/Starchild2D.exe

# Visual Studio
# Open build/ directory in VS — Starchild2D is set as startup project automatically
```

The project uses **vcpkg** for dependency management (manifest mode via `vcpkg.json`). Dependencies are auto-installed during CMake configure if `VCPKG_ROOT` is set or vcpkg exists in the repo root.

**Key dependencies:** SDL2, GLEW, Box2D v3, GLM, sol2, LuaJIT, nlohmann-json (see `vcpkg.json` for full list)

## Game Loop & Rendering Pipeline

Each frame (fixed 60 FPS, dt = 1/60):

1. **Input** — Poll SDL2 events, apply forces to player body
2. **Physics** — Step Box2D world
3. **Update** — Camera follow, projectiles, enemies, drops, particles, emotion system
4. **Collisions** — Projectile vs enemy, enemy vs player, drop collection
5. **Render to FBO** — TileMap (frustum-culled) → water → low decors → sorted objects/enemies/Princess → high decors (y-sorted) → character (SDF) → projectiles → particles
6. **Post-process** — Apply vignette/desaturation based on grievance
7. **Render UI** — Health bar, dialogue UI, minimap (direct to screen, orthographic)

## Architectural Patterns

- **Component-based design** — `HealthComponent`, `ProjectileManager`, `EnemyManager` as reusable object pools
- **Entity IDs** — Type-safe handles (`ProjectileId`, `EnemyId`, `b2BodyId`) instead of raw indices
- **Callback-driven events** — Health hurt/death, dialogue node enter/end, emotion change, collision — loose coupling between systems
- **Immediate-mode rendering** — `Draw2D` batches draw calls into a single VBO flush per frame
- **Y-sorted rendering** — Objects on the `Objects` layer are sorted by world Y coordinate for correct depth ordering with tall decorations (trees)

## Architecture

### Source Layout

```
src/
├── main.cpp                    # Game entry, main loop, GameState
├── Engine/
│   ├── Physics/
│   │   └── PhysicsWorld.h/.cpp # Box2D world wrapper (zero gravity, clampVelocity)
│   ├── Renderer/
│   │   ├── Draw2D.h/.cpp       # Batched immediate-mode 2D drawing
│   │   ├── ParticleSystem.h/.cpp  # GPU particle effects
│   │   ├── PostProcess.h/.cpp  # FBO-based post-processing (vignette)
│   │   ├── DialogueUI.h/.cpp   # Dialogue UI rendering
│   │   ├── DecorRenderer.h/.cpp   # SDF decoration rendering (trees, bushes)
│   │   ├── MiniMap.h/.cpp      # Minimap texture renderer
│   │   └── RenderLayer.h       # Render layer enum + Renderable sort key
│   ├── Camera/
│   │   └── Camera2D.h/.cpp     # 2D orthographic camera with smooth follow
│   └── Scripting/
│       └── LuaVM.h/.cpp        # Lua virtual machine (sol2 wrapper)
├── Game/
│   ├── World/
│   │   ├── TileMap.h/.cpp          # Tile-based map with procedural generation
│   │   ├── MapTileManager.h/.cpp   # Tile↔Box2D sync, modification tracking
│   │   ├── TerrainGenerator.h/.cpp # Perlin noise terrain generation
│   │   ├── Decoration.h            # Decor types and instance data
│   │   ├── DecorationGen.h/.cpp    # Procedural decoration placement
│   │   ├── RegionManager.h/.cpp    # Multi-region world management
│   │   ├── TimeSystem.h/.cpp       # Day/night cycle
│   │   └── WeatherSystem.h/.cpp    # Weather effects (rain, snow)
│   ├── Social/
│   │   ├── NPC.h/.cpp          # NPC base class with schedule system
│   │   ├── Princess.h          # Princess NPC (affection 0-1000, following; header-only)
│   │   └── DialogueTree.h/.cpp # Dialogue tree (Lua-driven, branching)
│   ├── Emotion/
│   │   ├── EmotionSystem.h/.cpp # Grievance system (0-100)
│   │   └── VentAnimation.h/.cpp # Venting animation
│   ├── Ability/
│   │   ├── Projectile.h/.cpp   # Projectile/missile system
│   │   ├── SuperStrength.h/.cpp # Grab/throw via distance joint
│   │   ├── Lightning.h/.cpp    # Chain lightning, auto-targeting
│   │   ├── Shield.h/.cpp       # Rotating barrier, reflects enemies
│   │   └── BondTechnique.h/.cpp # Princess combo attack
│   ├── AI/
│   │   ├── Enemy.h/.cpp        # Enemy AI (Chaser, Shooter, Exploder)
│   │   └── PathfindingSystem.h/.cpp # A* pathfinding for NPCs
│   ├── Health.h/.cpp           # HP/damage with death callbacks
│   ├── Drop.h/.cpp             # Item drop system with pickup detection
│   └── SaveSystem.h/.cpp       # Save/load game state (JSON serialization)
└── Utils/
    ├── Math.h                  # Math utilities (normalize, distance, direction)
    └── ShaderUtils.h           # Shared shader loading (createShaderProgram)
```

### Key Systems

**Core APIs**

- **Draw2D** — Batched immediate-mode 2D API. Call `Draw2D::beginFrame()`, issue draw calls (`drawRectFilled`, `drawCircle`, `drawLine`, `drawRectGradient`), then `Draw2D::endFrame()` to flush.
- **Camera2D** — Orthographic camera with position, zoom, rotation. Provides `getViewProjMatrix()`, `smoothFollow()`, and screen↔world coordinate conversion.
- **PhysicsWorld** — Box2D wrapper with zero gravity. Use `PhysicsWorld::clampVelocity(bodyId, maxSpeed)` for speed limiting (static helper, avoids code duplication).
- **ShaderUtils** — Shared shader loading utilities (`loadShaderFile`, `compileShader`, `createShaderProgram`). Include `Utils/ShaderUtils.h` for shader creation.

**World & Map**

- **TileMap** — Grid-based map with 12+ tile types (Grass, Dirt, Stone, Water, Wall, Path, Sand, Lava, DeepWater, Bridge, Door, Portal, Snow). Each tile has a `TileDef` with passability, movement cost, physics body flag, and damage-per-second. Supports procedural generation with deterministic seeds and frustum-culled rendering.
- **MapTileManager** — Coordinates all tile modifications between `TileMap` and Box2D. Automatically creates/destroys physics bodies when tiles change. Tracks modifications for save/load.
- **TerrainGenerator** — Perlin noise + fBm terrain generation producing coherent biomes (deep water → water → sand → grass → dirt → stone → wall).
- **Decoration / DecorRenderer** — SDF-rendered decorations (trees, bushes, flowers, rocks) with instanced rendering. Tall decorations (trees) participate in y-sorted rendering.
- **RegionManager** — Multi-region world management. Each region has its own TileMap, spawn points, and properties. Supports transitions via Portal tiles.
- **TimeSystem** — Day/night cycle with configurable day length. Affects lighting, NPC schedules, and visibility.
- **WeatherSystem** — Dynamic weather (rain, snow, clear) with particle effects and gameplay modifiers.
- **SaveSystem** — JSON-based save/load for game state including player progress, map modifications, NPC states, and world time.
- **PathfindingSystem** — A* pathfinding for NPC movement across the tile map, respecting passability and movement costs.

**Rendering**

- **RenderLayer** — 7-layer rendering system: Ground → Water → DecorLow → Objects (y-sorted) → DecorHigh → Effects → UI.
- **PostProcess** — FBO-based post-processing pipeline. Vignette effect scales with grievance.
- **ParticleSystem** — GPU-rendered particle effects with types: Point, Circle, Spark (with gravity), Trail. Methods: `emit()`, `emitBurst()` (explosions), `emitRing()` (halo effects). Used for muzzle flash, dash particles, enemy death effects.
- **MiniMap** — Lazy-updated minimap texture (updates every 1s or on demand).

**Scripting & Dialogue**

- **LuaVM** — Lua scripting via sol2. Load scripts from `assets/scripts/`, bind C++ functions, execute Lua code.
- **DialogueTree** — Load dialogue trees from Lua files. Supports branching choices with affection changes.

**Characters & AI**

- **NPC/Princess** — NPCs with schedule-based movement. Princess has affection system (0-1000) with 5 levels: Stranger, Acquaintance, Friend, CloseFriend, Beloved. Princess has `ultimateCharge` (0-100) for bond technique system.
- **EmotionSystem** — Grievance system (0-100). At 70+, triggers vignette post-processing and ×0.7 speed reduction.
- **EnemyManager** — Spawns and renders enemies (Chaser, Shooter, Exploder types).

**Combat & Abilities**

- **ProjectileManager** — SDF-rendered projectiles (fireballs, ice spikes) with collision detection.
- **HealthComponent** — Tracks HP, handles damage and healing with death callbacks.
- **SuperStrength** — Distance-joint based grabbing and throwing of objects.
- **Lightning** — Auto-targeting chain lightning that jumps between nearby enemies.
- **Shield** — Rotating barrier that reflects enemy collisions.
- **BondTechnique** — Princess combo attack triggered when ultimate charge reaches 100.
- **Drop** — Item drop system with pickup detection.

**Implemented Abilities:**
| Ability | Key | Effect |
|---------|-----|--------|
| Fireball | J | 25 damage, 400 speed, 2s lifetime, particle trail |
| Ice Spike | L | 20 damage, 500 speed, applies 2s slow debuff |
| Dash | Space | 0.2s invincible dash, 1s cooldown |
| Super Strength | K | Grab/throw objects via Box2D distance joint |
| Lightning | Q | Auto-targeting chain lightning, jumps between enemies |
| Shield | F | Rotating barrier, reflects enemy collisions, 15 mana |
| Bond Technique | G | Princess combo attack when ultimateCharge reaches 100 |

**Planned (Not Yet Implemented):**
| Ability | Description |
|---------|-------------|
| Flight | Height simulation, shadow scaling, fog layer, replaces dash |

### Data Flow

```
Input → Physics → Update → Collisions → Render(FBO) → PostProcess → UI
          ↑          ↑                                  ↑
      Box2D     EmotionSystem                     Vignette/Desaturate
          ↑     Princess/AI/Drops                       (grievance-driven)
    MapTileManager
```

### Important Design Decisions

- **Zero gravity** — Box2D world uses `b2Gravity(0, 0)` for top-down perspective.
- **Fixed timestep** — Game runs at fixed 60 FPS (dt = 1/60) regardless of monitor refresh rate.
- **SDF rendering** — Characters, projectiles, and decorations use signed distance field rendering (no sprites).
- **Object pools** — Projectiles, enemies, particles use pre-allocated pools to avoid runtime allocation.
- **Type-safe entity IDs** — `ProjectileId`, `EnemyId`, `b2BodyId` are distinct types, not raw integers.
- **Tile modification through MapTileManager** — Never call `TileMap::setTile()` directly; always use `MapTileManager::setTile()` to keep physics in sync.

### Physics

Box2D with **zero gravity** (top-down perspective). Player is a dynamic body with linear damping; walls from TileMap are static bodies created/managed by `MapTileManager`. Uses the **Box2D v3 C-style API** (`b2CreateWorld`, `b2Body_ApplyForceToCenter`, etc.), not the old C++ class API.

## Development Phases

| Stage | Status | Focus |
|-------|--------|-------|
| 1 | ✅ Done | SDL2 window, OpenGL, Box2D, WASD movement |
| 2 | ✅ Done | Draw2D, Camera2D, SDF character, TileMap |
| 3 | ✅ Done | Combat (projectiles, enemies, damage, drops, particles) |
| 4 | ✅ Done | Lua scripting, dialogue, emotion system |
| 5 | ✅ Done | All abilities (fireball, ice, lightning, shield, dash, super strength, bond technique), particle trails, map upgrade |
| 6 | ✅ Done | Multi-region, A* pathfinding, save/load, day/night cycle, weather |
| 7-8 | Pending | Indoor details, building mode, furniture, audio, UI polish, main story |

See `doc/plan5.md` for stage 5 details, `doc/plan6.md` for stage 6 implementation plan, and `doc/技术栈.md` for the full 8-stage roadmap.

## Controls (Current Build)

- **WASD / Arrow keys** — Move player
- **Mouse wheel / +/-** — Zoom in/out
- **J** — Fire fireball projectile
- **L** — Fire ice spike projectile (slow effect)
- **Q** — Cast chain lightning (auto-targeting, jumps between enemies)
- **G** — Bond technique (Princess combo attack, requires ultimate charge)
- **Space** — Dash (0.2s invincible, 1s cooldown)
- **F** — Activate shield (rotating barrier, reflects enemies, 15 mana)
- **K** — Grab/throw object (super strength)
- **E** — Interact (talk to Princess, vent at home)
- **W/S or ↑/↓** — Navigate dialogue choices (when in dialogue)
- **H** — Debug: heal 30 HP (Debug builds only)
- **1/2/3** — Change character expression (normal/happy/sad)
- **F5** — Quick save
- **F9** — Quick load
- **Esc** — Exit

## Important Patterns

- **Shader loading**: Use `createShaderProgram()` from `Utils/ShaderUtils.h` — shaders are loaded from `assets/shaders/` relative to the working directory.
- **Lua scripts**: Copied to `build/assets/scripts/` by CMake. Load via `luaVM.loadFile()` or `lua.state().script_file()`.
- **GLEW**: Requires `glewExperimental = GL_TRUE` before `glewInit()` for OpenGL 3.3 Core.
- **Box2D v3 API**: Uses the new C-style API (`b2CreateWorld`, `b2Body_ApplyForceToCenter`, etc.), not the old C++ class API.
- **No console window** in release builds (MSVC: `SDL_MAIN_HANDLED` + Windows subsystem).
- **Working directory**: Asset paths resolve relative to CWD. The binary must be run from `build/`, OR the VS debugger working directory can be set to project root via `VS_DEBUGGER_WORKING_DIRECTORY`. Running from the wrong directory will fail to find shaders and Lua scripts.
- **UTF-8 source files**: MSVC may have issues with Chinese characters in source files. Use English comments to avoid compilation errors.

## Project Structure

```
assets/
├── shaders/                      # GLSL shaders (copied to build/)
│   ├── character.vert/.frag
│   ├── projectile.vert/.frag
│   ├── enemy.vert/.frag
│   ├── particle.vert/.frag
│   ├── draw2d.vert/.frag
│   ├── postprocess.vert/.frag
│   └── decor.vert/.frag          # Decoration SDF rendering
└── scripts/                      # Lua scripts (copied to build/)
    ├── dialogues/
    │   ├── first_meeting.lua
    │   └── daily_greetings.lua
    ├── abilities.lua
    ├── emotion_config.lua
    └── npc_schedules.lua
```

## Debugging

- Compile with `/W4` warnings enabled (configured in CMakeLists.txt)
- No test framework is currently integrated; visual verification is the primary testing method
- Debug builds define `DEBUG` automatically for conditional compilation
- See `doc/plan5.md` for stage 5 verification steps and acceptance criteria

## Common Pitfalls

| Pitfall | Solution |
|---------|----------|
| Shaders/Lua not found at runtime | Binary must be run from `build/` directory, or set `VS_DEBUGGER_WORKING_DIRECTORY` to project root |
| Box2D v2 vs v3 API confusion | Uses v3 C-style API (`b2Body_ApplyForceToCenter`), NOT old C++ class API (`body->ApplyForce`) |
| Tile modification without physics sync | Never call `TileMap::setTile()` directly; always use `MapTileManager::setTile()` |
| UTF-8 source encoding issues | Use English comments, or `/source-charset:utf-8 /execution-charset:utf-8` (already configured) |
| GLEW initialization failure | `glewExperimental = GL_TRUE` must be set before `glewInit()` |
| FBO vs direct rendering | Post-process renders to FBO; UI elements (health, dialogue, minimap) render directly to screen |
| OpenGL 3.3 Core requirement | All shaders use `#version 330 core`; requires compatible GPU/driver |

## Common Development Tasks

### Adding a new shader
1. Create `.vert` and `.frag` files in `assets/shaders/`
2. The CMake build copies all shaders to `build/assets/shaders/`
3. Load at runtime via `createShaderProgram("assets/shaders/name.vert", "assets/shaders/name.frag")`
4. Remember to run from the `build/` directory so paths resolve correctly

### Adding a new Lua script
1. Create `.lua` file in `assets/scripts/` (or subdirectory)
2. CMake copies to `build/assets/scripts/`
3. Load via `luaVM.loadFile("assets/scripts/path/to/script.lua")`

### Adding a new source file
1. Add `.h/.cpp` in the appropriate `src/` subdirectory
2. Add to `target_sources()` in `CMakeLists.txt`
3. Include via `#include "Relative/Path/To/Header.h"` (src/ is in include path)

### Adding a new tile type
1. Add to `TileType` enum in `TileMap.h`
2. Add a `TileDef` entry in the `TILE_DEFS` array with passability, color, physics flag, etc.
3. Add the color mapping in `TileMap::getTileColor()`
4. Re-run `cmake -B build` if the enum size changes (affects serialization)

### Debug build with console output
Build with `cmake --build build --config Debug`. Debug-only code can be guarded with `#ifdef DEBUG` (automatically defined in Debug config).

## Documentation

- `doc/技术栈.md` — Full 8-stage roadmap and architecture vision
- `doc/plan2.md` — Stage 2 implementation details (rendering system)
- `doc/plan3.md` — Stage 3 combat system details
- `doc/plan4.md` — Stage 4 Lua scripting and emotion system
- `doc/plan5.md` — Stage 5 map system upgrade (passability, terrain, decorations, minimap)
- `doc/map-spec.md` — Map system specification (multi-region, pathfinding, save/load)
