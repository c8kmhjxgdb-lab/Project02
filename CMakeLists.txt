cmake_minimum_required(VERSION 3.20)

# vcpkg toolchain (must be set BEFORE project())
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(Starchild2D VERSION 0.1.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find packages
find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Get glm include directory from the imported target
get_target_property(GLM_INCLUDE_DIR glm::glm-header-only INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "GLM include dir: ${GLM_INCLUDE_DIR}")

# Main executable
add_executable(Starchild2D
    src/main.cpp
    src/Engine/Renderer/Draw2D.cpp
    src/Engine/Renderer/ParticleSystem.cpp
    src/Engine/Renderer/PostProcess.cpp
    src/Engine/Renderer/DialogueUI.cpp
    src/Engine/Camera/Camera2D.cpp
    src/Engine/Physics/PhysicsWorld.cpp
    src/Engine/Scripting/LuaVM.cpp
    src/Game/World/TileMap.cpp
    src/Game/World/MapTileManager.cpp
    src/Game/World/TerrainGenerator.cpp
    src/Game/World/DecorationGen.cpp
    src/Game/World/MapRegion.cpp
    src/Game/World/RegionManager.cpp
    src/Game/World/TimeSystem.cpp
    src/Game/World/WeatherSystem.cpp
    src/Engine/Renderer/DecorRenderer.cpp
    src/Engine/Renderer/MiniMap.cpp
    src/Game/Health.cpp
    src/Game/Drop.cpp
    src/Game/Ability/Projectile.cpp
    src/Game/Ability/SuperStrength.cpp
    src/Game/AI/Enemy.cpp
    src/Game/AI/PathfindingSystem.cpp
    src/Game/Social/NPC.cpp
    src/Game/Social/DialogueTree.cpp
    src/Game/Emotion/EmotionSystem.cpp
    src/Game/Emotion/VentAnimation.cpp
    src/Game/SaveSystem.cpp
)

# Include directories
set(VCPKG_INSTALL_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows")
target_include_directories(Starchild2D PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${GLM_INCLUDE_DIR}
    ${VCPKG_INSTALL_DIR}/include/SDL2
)

# Workaround for vcpkg MSBuild toolchain not respecting target_include_directories
# for non-header-only libraries in MSVC generator
if(MSVC AND CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /I\"${GLM_INCLUDE_DIR}\" /I\"${VCPKG_INSTALL_DIR}/include\"")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I\"${GLM_INCLUDE_DIR}\" /I\"${VCPKG_INSTALL_DIR}/include\"")
endif()

# LuaJIT headers are in a luajit/ subdirectory, but sol2 expects lua.h at top level
find_path(LUAJIT_INCLUDE_DIR NAMES lua.h PATH_SUFFIXES luajit)
if(LUAJIT_INCLUDE_DIR)
    target_include_directories(Starchild2D PRIVATE ${LUAJIT_INCLUDE_DIR})
endif()

# Link libraries
target_link_libraries(Starchild2D PRIVATE
    SDL2::SDL2
    GLEW::GLEW
    box2d::box2d
    glm::glm
    sol2::sol2
    nlohmann_json::nlohmann_json
)

# LuaJIT doesn't provide CMake config, link directly
find_path(LUAJIT_INCLUDE_DIR NAMES lua.h PATH_SUFFIXES luajit)
find_library(LUAJIT_LIBRARY NAMES luajit luajit-5.1 lua51)
if(LUAJIT_INCLUDE_DIR AND LUAJIT_LIBRARY)
    target_include_directories(Starchild2D PRIVATE ${LUAJIT_INCLUDE_DIR})
    target_link_libraries(Starchild2D PRIVATE ${LUAJIT_LIBRARY})
else()
    message(FATAL_ERROR "Could not find LuaJIT")
endif()

# Windows subsystem (no console window in release)
if(MSVC)
    target_compile_options(Starchild2D PRIVATE /W4 /WX- /source-charset:utf-8 /execution-charset:utf-8)
    target_compile_definitions(Starchild2D PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        SDL_MAIN_HANDLED
    )
else()
    target_compile_options(Starchild2D PRIVATE -Wall -Wextra -Wpedantic)
endif()

# DEBUG define for debug-only features (debug keys, extra logging)
target_compile_definitions(Starchild2D PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

# Copy shader files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets/shaders
     DESTINATION ${CMAKE_BINARY_DIR}/assets/)

# Copy Lua scripts to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets/scripts
     DESTINATION ${CMAKE_BINARY_DIR}/assets/)

# Set as startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Starchild2D)
