# Architecture Route 3 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Drive the remaining architecture route from transitional boundaries toward smaller contexts, stronger tests, and an updated roadmap.

**Architecture:** Keep the current directory layout and target split stable. Continue replacing broad `GameState&` consumers with focused contexts and presenters while keeping compatibility adapters at scene and app boundaries.

**Tech Stack:** C++17, SDL2, OpenGL/GLEW, Box2D, LuaJIT/sol2, nlohmann_json, CMake/CTest.

---

### Task 1: Update Route Document From Current Code

**Files:**
- Modify: `doc/架构分层与重构路线.md`

- [x] **Step 1: Reconcile stage status**

Update the "代码现状快照" section so phase 1, 2, 6, and 7 are marked as effectively completed with follow-up risks, while phase 3 and 4/5 are marked as active.

- [x] **Step 2: Replace next priorities**

Replace the current priority list with route-3 batches:

1. Presentation context extraction.
2. Save/load application tests.
3. Input and UI/audio context narrowing.
4. Real state sub-object extraction after test coverage grows.
5. Audio asset pipeline after real assets exist.

- [x] **Step 3: Verify document readability**

Run: `Get-Content -Path 'doc\架构分层与重构路线.md' | Select-String -Pattern '当前下一步优先级|阶段 3 现状|阶段 6 现状|阶段 7 现状'`

Expected: each section appears with current wording.

### Task 2: Extract World Presentation Context Builder

**Files:**
- Create: `src/Game/Presentation/WorldPresentationBuilder.h`
- Create: `src/Game/Presentation/WorldPresentationBuilder.cpp`
- Modify: `src/Game/Scenes/WorldScene.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Add builder interface**

Create a `WorldPresentationBuilder::buildRenderContext(GameState&)` function returning `GameRenderer::WorldRenderContext`.

- [x] **Step 2: Move render context assembly**

Move the current anonymous `makeRenderContext(GameState&)` implementation from `WorldScene.cpp` into the new builder.

- [x] **Step 3: Simplify WorldScene**

Make `WorldScene::render()` call `WorldPresentationBuilder::buildRenderContext(gs)` and remove now-unused includes.

- [x] **Step 4: Add new source to CMake**

Add `src/Game/Presentation/WorldPresentationBuilder.cpp` to `StarchildGame`.

- [x] **Step 5: Verify build**

Run: `cmake --build build --config Release`

Expected: exit code 0.

### Task 3: Add SaveApplier Integration Test

**Files:**
- Create: `tests/SaveApplierIntegrationTest.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Build a focused fixture**

Create a test fixture with a Box2D world, player body, `GameState`, region manager initialization, audio disabled, and enough systems initialized to call `SaveGameService::applySaveData`.

- [x] **Step 2: Test full save application effects**

Assert that applying a save:

- resets transient input/build/toy/dialogue/combat state;
- restores player position, health, mana, coins, weather, time, emotion, and total play time;
- transitions to the saved region;
- refreshes minimap dimensions and notice text;
- clears projectiles/enemies/drops/particles.

- [x] **Step 3: Register the test**

Add a `SaveApplierIntegrationTest` executable linked to `StarchildGame` and an `add_test` entry.

- [x] **Step 4: Verify test**

Run: `cmake --build build --config Release --target SaveApplierIntegrationTest`

Expected: exit code 0.

Run: `cd build; ctest -C Release -R SaveApplierIntegrationTest --output-on-failure`

Expected: 1/1 tests pass.

### Task 4: Wire UI SFX Through Input Paths

**Files:**
- Modify: `src/Game/Controllers/MainMenuInputController.cpp`
- Modify: `src/Game/Controllers/InputController.cpp`

- [x] **Step 1: Add menu navigation SFX**

Call `AudioService::playUiSfx(gs.audioSystem, "navigate")` when the selected menu item changes.

- [x] **Step 2: Add confirm/cancel SFX**

Call `AudioService::playUiSfx(gs.audioSystem, "confirm")` before menu activation and save/load actions. Call `"cancel"` when Escape leaves build mode or requests scene exit.

- [x] **Step 3: Verify fallback manifest coverage**

Ensure `AudioSystem::loadManifest()` registers fallback IDs for `sfx/ui/navigate`, `sfx/ui/confirm`, and `sfx/ui/cancel`.
`AudioManifestTest` also asserts `sfx/ui/navigate` and `sfx/ui/confirm` are registered.

- [x] **Step 4: Verify build and audio test**

Run: `cmake --build build --config Release --target AudioManifestTest`

Expected: exit code 0.

Run: `cd build; ctest -C Release -R AudioManifestTest --output-on-failure`

Expected: 1/1 tests pass.

### Task 5: Final Build, Test, and Roadmap Sync

**Files:**
- Modify: `doc/架构分层与重构路线.md`

- [x] **Step 1: Update roadmap with completed batch**

Add a dated note that the route-3 first execution batch extracted world presentation building, added save-apply integration coverage, and wired UI SFX events.

- [x] **Step 2: Run full verification**

Run: `cmake --build build --config Release`

Expected: exit code 0.

Run: `cd build; ctest -C Release --output-on-failure`

Expected: all registered tests pass.

- [x] **Step 3: Inspect source diff**

Run: `git diff --stat -- CMakeLists.txt doc src tests docs`

Expected: only source, docs, tests, and plan files are part of the intentional diff.

### Task 6: Add Collision and Input Boundary Tests

**Files:**
- Create: `tests/CombatCollisionServiceTest.cpp`
- Create: `tests/InputControllerBoundaryTest.cpp`
- Modify: `src/Game/Controllers/AbilityInputController.cpp`
- Modify: `CMakeLists.txt`
- Modify: `doc/架构分层与重构路线.md`

- [x] **Step 1: Add combat collision coverage**

`CombatCollisionServiceTest` covers projectile hits, owner-body skip behavior, and dead-player contact damage guard.

- [x] **Step 2: Add input boundary coverage**

`InputControllerBoundaryTest` covers ability input guards, flight start guards, build-mode toggling, and build-mode mouse-wheel consumption.

- [x] **Step 3: Verify red-green fix**

Initial targeted run failed with `left mouse does not cast while build mode is active`.
`AbilityInputController::handleMouseButtonDown()` now returns early when gameplay actions are not allowed.

- [x] **Step 4: Verify targeted tests**

Run: `ctest -C Release -R "CombatCollisionServiceTest|InputControllerBoundaryTest" --output-on-failure`

Expected: 2/2 tests pass.

### Task 7: Move Scene Adapters Into Scene Classes

**Files:**
- Modify: `src/Game/Scenes/MainMenuScene.h`
- Modify: `src/Game/Scenes/MainMenuScene.cpp`
- Modify: `src/Game/Scenes/WorldScene.h`
- Modify: `src/Game/Scenes/WorldScene.cpp`
- Modify: `src/Game/Scenes/SceneManager.h`
- Modify: `src/Game/Scenes/SceneManager.cpp`
- Modify: `doc/架构分层与重构路线.md`

- [x] **Step 1: Add MainMenuScene factory**

`MainMenuScene::create()` returns an internal `IScene` implementation and keeps existing namespace functions as the compatible implementation surface.

- [x] **Step 2: Add WorldScene factory**

`WorldScene::create()` returns an internal `IScene` implementation that owns `WorldScene::State`.

- [x] **Step 3: Simplify SceneManager state**

`SceneManager::State` no longer stores `WorldScene::State`; `SceneManager::createState()` now creates scenes via `MainMenuScene::create()` and `WorldScene::create()`.

- [x] **Step 4: Verify game library build**

Run: `cmake --build build --config Release --target StarchildGame`

Expected: exit code 0.
