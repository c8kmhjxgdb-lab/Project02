# Repository Guidelines

## Project Structure & Module Organization

Starchild2D is a C++17 top-down action RPG. Main application setup and the game loop live in `src/main.cpp`. Reusable engine code is under `src/Engine/`: `Renderer/`, `Physics/`, `Camera/`, and `Scripting/`. Gameplay systems are under `src/Game/`: `World/`, `Ability/`, `AI/`, `Social/`, `Emotion/`, plus save and health systems. Shared helpers live in `src/Utils/`.

Assets are stored in `assets/`: GLSL files in `assets/shaders/` and Lua gameplay/dialogue scripts in `assets/scripts/`. CMake copies these into `build/assets/`; run the executable from `build/` so relative asset paths resolve. Planning and design notes are in `doc/`. Treat `build/` as generated output.

## Build, Test, and Development Commands

- `cmake -B build`: configure the project and install vcpkg manifest dependencies when `VCPKG_ROOT` is set.
- `cmake --build build --config Release`: build the release executable at `build/Release/Starchild2D.exe`.
- `cmake --build build --config Debug`: build with the `DEBUG` define enabled.
- `cd build && ./Release/Starchild2D.exe`: run the game with copied assets available.
- `cmake -B build -G "Visual Studio 17 2022" -A x64`: regenerate a Visual Studio solution when needed.

There is currently no `ctest` suite or integrated unit-test command.

## Coding Style & Naming Conventions

Use C++17 and keep source/header pairs together by feature, for example `WeatherSystem.cpp` and `WeatherSystem.h`. Class and system names use PascalCase; methods, locals, and data members generally use lower camelCase. Follow the existing four-space indentation style. Prefer English source comments for encoding stability, though existing Chinese comments are part of the project. When adding a `.cpp`, list it explicitly in `CMakeLists.txt`.

## Testing Guidelines

Verification is manual: build the relevant configuration, run from `build/`, and exercise the affected gameplay path. For asset or script changes, confirm CMake has copied files into `build/assets/`. Include Debug checks when touching debug-only controls or code guarded by `DEBUG`.

## Commit & Pull Request Guidelines

Recent commits use short Chinese summaries such as `修复Bug`, `日志`, and `阶段1-6`. Keep commits concise and action-oriented; include the stage or subsystem when useful, for example `阶段6-修复天气显示`. Pull requests should describe the gameplay/system change, list manual verification steps, link related issues or docs, and include screenshots or short clips for rendering/UI changes.

## Agent-Specific Instructions

Do not modify generated build artifacts unless the task explicitly requires it. Preserve update ordering in `src/main.cpp`, especially physics, collision cleanup, rendering, and save/load sequencing. Use `MapTileManager` for gameplay-relevant tile edits so physics and persistence stay synchronized.
