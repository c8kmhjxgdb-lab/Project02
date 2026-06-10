cmake_minimum_required(VERSION 3.20)

# vcpkg toolchain (must be set BEFORE project())
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(Starchild2D VERSION 0.1.0 LANGUAGES CXX)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

get_target_property(GLM_INCLUDE_DIR glm::glm-header-only INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "GLM include dir: ${GLM_INCLUDE_DIR}")

set(VCPKG_INSTALL_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows")
find_path(LUAJIT_INCLUDE_DIR NAMES lua.h PATH_SUFFIXES luajit)
find_library(LUAJIT_LIBRARY NAMES luajit luajit-5.1 lua51)
if(NOT LUAJIT_INCLUDE_DIR OR NOT LUAJIT_LIBRARY)
    message(FATAL_ERROR "Could not find LuaJIT")
endif()

add_library(StarchildEngine STATIC
    src/Engine/Audio/AudioSystem.cpp
    src/Engine/Camera/Camera2D.cpp
    src/Engine/Physics/PhysicsWorld.cpp
    src/Engine/Renderer/DecorRenderer.cpp
    src/Engine/Renderer/DialogueUI.cpp
    src/Engine/Renderer/Draw2D.cpp
    src/Engine/Renderer/MiniMap.cpp
    src/Engine/Renderer/ParticleSystem.cpp
    src/Engine/Renderer/PostProcess.cpp
    src/Engine/Renderer/TextRenderer.cpp
    src/Engine/Scripting/LuaVM.cpp
)

add_library(StarchildGame STATIC
    src/Game/Ability/BondTechnique.cpp
    src/Game/Ability/Lightning.cpp
    src/Game/Ability/Projectile.cpp
    src/Game/Ability/Shield.cpp
    src/Game/Ability/SuperStrength.cpp
    src/Game/AI/Enemy.cpp
    src/Game/AI/PathfindingSystem.cpp
    src/Game/Building/BuildingPhysicsSync.cpp
    src/Game/Building/BuildingPlacementRules.cpp
    src/Game/Building/BuildingStats.cpp
    src/Game/Building/BuildingSystem.cpp
    src/Game/Building/Furniture.cpp
    src/Game/Controllers/AbilityInputController.cpp
    src/Game/Controllers/BuildingInputController.cpp
    src/Game/Controllers/InputController.cpp
    src/Game/Controllers/InteractionInputController.cpp
    src/Game/Controllers/MainMenuInputController.cpp
    src/Game/Data/DialogueScriptLoader.cpp
    src/Game/Data/EmotionConfigLoader.cpp
    src/Game/Data/FurnitureDefinitionLoader.cpp
    src/Game/Data/LuaConfigRepository.cpp
    src/Game/Data/QuestDefinitionLoader.cpp
    src/Game/Data/SaveMigration.cpp
    src/Game/Data/SaveRepository.cpp
    src/Game/Data/SaveSerializer.cpp
    src/Game/Data/TimeConfigLoader.cpp
    src/Game/Data/ToyDefinitionLoader.cpp
    src/Game/Data/WeatherConfigLoader.cpp
    src/Game/Drop.cpp
    src/Game/Emotion/EmotionSystem.cpp
    src/Game/Emotion/VentAnimation.cpp
    src/Game/Health.cpp
    src/Game/Inventory/Inventory.cpp
    src/Game/Presentation/AbilityView.cpp
    src/Game/Presentation/AimReticleView.cpp
    src/Game/Presentation/BuildingView.cpp
    src/Game/Presentation/EntityView.cpp
    src/Game/Presentation/GameRenderer.cpp
    src/Game/Presentation/HudView.cpp
    src/Game/Presentation/MainMenuView.cpp
    src/Game/Presentation/MiniMapPresenter.cpp
    src/Game/Presentation/ParticleView.cpp
    src/Game/Presentation/PresentationModelBuilder.cpp
    src/Game/Presentation/WindowTitlePresenter.cpp
    src/Game/Presentation/WorldPresentationBuilder.cpp
    src/Game/Presentation/WorldRenderer.cpp
    src/Game/Presentation/WorldSignView.cpp
    src/Game/Quest/QuestSystem.cpp
    src/Game/Scenes/MainMenuScene.cpp
    src/Game/Scenes/SceneManager.cpp
    src/Game/Scenes/WorldScene.cpp
    src/Game/Services/AbilityUpdateService.cpp
    src/Game/Services/AudioService.cpp
    src/Game/Services/CombatCollisionService.cpp
    src/Game/Services/CombatService.cpp
    src/Game/Services/DialogueService.cpp
    src/Game/Services/EnemySpawnService.cpp
    src/Game/Services/GameSessionService.cpp
    src/Game/Services/NoticeService.cpp
    src/Game/Services/PlayerInputQuery.cpp
    src/Game/Services/PlayerLifecycleService.cpp
    src/Game/Services/PlayerMotionService.cpp
    src/Game/Services/ProgressionUpdateService.cpp
    src/Game/Services/ProjectileTrailService.cpp
    src/Game/Services/RegionService.cpp
    src/Game/Services/RegionUpdateService.cpp
    src/Game/Services/SaveApplier.cpp
    src/Game/Services/SaveGameService.cpp
    src/Game/Services/SaveSnapshotBuilder.cpp
    src/Game/Services/WorldCombatUpdateService.cpp
    src/Game/Services/WorldNpcUiUpdateService.cpp
    src/Game/Services/WorldQuery.cpp
    src/Game/Services/WorldUpdateService.cpp
    src/Game/Social/DialogueTree.cpp
    src/Game/Social/NPC.cpp
    src/Game/StateContexts.cpp
    src/Game/Toy/MiniCarGame.cpp
    src/Game/Toy/ToySystem.cpp
    src/Game/World/DecorationGen.cpp
    src/Game/World/MapRegion.cpp
    src/Game/World/MapTileManager.cpp
    src/Game/World/RegionFactory.cpp
    src/Game/World/RegionManager.cpp
    src/Game/World/TerrainGenerator.cpp
    src/Game/World/TileMap.cpp
    src/Game/World/TimeSystem.cpp
    src/Game/World/WeatherSystem.cpp
    src/Game/World/WeatherTypes.cpp
)

add_library(StarchildApp STATIC
    src/App/Application.cpp
    src/App/GameBootstrap.cpp
    src/App/GameLoop.cpp
    src/App/WindowEvents.cpp
)

add_executable(Starchild2D
    src/main.cpp
)

function(configure_starchild_target target_name)
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${GLM_INCLUDE_DIR}
        ${VCPKG_INSTALL_DIR}/include/SDL2
        ${LUAJIT_INCLUDE_DIR}
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX- /source-charset:utf-8 /execution-charset:utf-8)
        target_compile_definitions(${target_name} PUBLIC
            _CRT_SECURE_NO_WARNINGS
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            SDL_MAIN_HANDLED
            $<$<CONFIG:Debug>:DEBUG>
        )
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_definitions(${target_name} PUBLIC
            $<$<CONFIG:Debug>:DEBUG>
        )
    endif()
endfunction()

configure_starchild_target(StarchildEngine)
configure_starchild_target(StarchildGame)
configure_starchild_target(StarchildApp)
configure_starchild_target(Starchild2D)

target_link_libraries(StarchildEngine PUBLIC
    SDL2::SDL2
    GLEW::GLEW
    box2d::box2d
    glm::glm
    sol2::sol2
    nlohmann_json::nlohmann_json
    ${LUAJIT_LIBRARY}
)

target_link_libraries(StarchildGame PUBLIC
    StarchildEngine
)

target_link_libraries(StarchildApp PUBLIC
    StarchildGame
)

target_link_libraries(Starchild2D PRIVATE
    StarchildApp
)

if(WIN32)
    target_link_libraries(StarchildEngine PUBLIC gdi32)
endif()

if(MSVC AND CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /I\"${GLM_INCLUDE_DIR}\" /I\"${VCPKG_INSTALL_DIR}/include\"")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I\"${GLM_INCLUDE_DIR}\" /I\"${VCPKG_INSTALL_DIR}/include\"")
endif()

file(COPY ${CMAKE_SOURCE_DIR}/assets/shaders
     DESTINATION ${CMAKE_BINARY_DIR}/assets/)

file(COPY ${CMAKE_SOURCE_DIR}/assets/scripts
     DESTINATION ${CMAKE_BINARY_DIR}/assets/)

file(COPY ${CMAKE_SOURCE_DIR}/assets/audio
     DESTINATION ${CMAKE_BINARY_DIR}/assets/)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Starchild2D)

if(BUILD_TESTING)
    add_executable(SaveDataRoundTripTest
        tests/SaveDataRoundTripTest.cpp
    )
    configure_starchild_target(SaveDataRoundTripTest)
    target_link_libraries(SaveDataRoundTripTest PRIVATE StarchildGame)
    add_test(NAME SaveDataRoundTripTest COMMAND SaveDataRoundTripTest)

    add_executable(StoryProgressTest
        tests/StoryProgressTest.cpp
    )
    configure_starchild_target(StoryProgressTest)
    target_link_libraries(StoryProgressTest PRIVATE StarchildGame)
    add_test(NAME StoryProgressTest COMMAND StoryProgressTest)

    add_executable(CombatCastServiceTest
        tests/CombatCastServiceTest.cpp
    )
    configure_starchild_target(CombatCastServiceTest)
    target_link_libraries(CombatCastServiceTest PRIVATE StarchildGame)
    add_test(NAME CombatCastServiceTest COMMAND CombatCastServiceTest)

    add_executable(QuestSystemFlowTest
        tests/QuestSystemFlowTest.cpp
    )
    configure_starchild_target(QuestSystemFlowTest)
    target_link_libraries(QuestSystemFlowTest PRIVATE StarchildGame)
    add_test(NAME QuestSystemFlowTest COMMAND QuestSystemFlowTest)

    add_executable(RegionServiceDoorTest
        tests/RegionServiceDoorTest.cpp
    )
    configure_starchild_target(RegionServiceDoorTest)
    target_link_libraries(RegionServiceDoorTest PRIVATE StarchildGame)
    add_test(NAME RegionServiceDoorTest COMMAND RegionServiceDoorTest)

    add_executable(DialogueServiceRewardTest
        tests/DialogueServiceRewardTest.cpp
    )
    configure_starchild_target(DialogueServiceRewardTest)
    target_link_libraries(DialogueServiceRewardTest PRIVATE StarchildGame)
    add_test(NAME DialogueServiceRewardTest COMMAND DialogueServiceRewardTest)

    add_executable(AudioManifestTest
        tests/AudioManifestTest.cpp
    )
    configure_starchild_target(AudioManifestTest)
    target_link_libraries(AudioManifestTest PRIVATE StarchildEngine)
    add_test(NAME AudioManifestTest COMMAND AudioManifestTest)

    add_executable(SaveApplierIntegrationTest
        tests/SaveApplierIntegrationTest.cpp
    )
    configure_starchild_target(SaveApplierIntegrationTest)
    target_link_libraries(SaveApplierIntegrationTest PRIVATE StarchildGame)
    add_test(NAME SaveApplierIntegrationTest COMMAND SaveApplierIntegrationTest)

    add_executable(CombatCollisionServiceTest
        tests/CombatCollisionServiceTest.cpp
    )
    configure_starchild_target(CombatCollisionServiceTest)
    target_link_libraries(CombatCollisionServiceTest PRIVATE StarchildGame)
    add_test(NAME CombatCollisionServiceTest COMMAND CombatCollisionServiceTest)

    add_executable(InputControllerBoundaryTest
        tests/InputControllerBoundaryTest.cpp
    )
    configure_starchild_target(InputControllerBoundaryTest)
    target_link_libraries(InputControllerBoundaryTest PRIVATE StarchildGame)
    add_test(NAME InputControllerBoundaryTest COMMAND InputControllerBoundaryTest)
endif()
