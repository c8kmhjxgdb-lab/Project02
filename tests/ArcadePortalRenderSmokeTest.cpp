#include "App/GameBootstrap.h"
#include "Game/GameState.h"
#include "Game/Scenes/WorldScene.h"
#include "Game/Services/RegionService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/World/MapRegion.h"
#include "Game/World/RegionFactory.h"
#include "TestSupport.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <chrono>
#include <cstdio>
#include <memory>

namespace {

struct SdlRuntime {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;

    SdlRuntime() {
        TestSupport::require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0, "SDL initializes");
        SDL_StopTextInput();
        TestSupport::require(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) == 0,
                             "OpenGL major version is set");
        TestSupport::require(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) == 0,
                             "OpenGL minor version is set");
        TestSupport::require(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                             SDL_GL_CONTEXT_PROFILE_CORE) == 0,
                             "OpenGL core profile is set");
        TestSupport::require(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) == 0,
                             "OpenGL double buffering is set");

        window = SDL_CreateWindow(
            "ArcadePortalRenderSmokeTest",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            800,
            600,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        TestSupport::require(window != nullptr, "hidden OpenGL window is created");

        glContext = SDL_GL_CreateContext(window);
        TestSupport::require(glContext != nullptr, "OpenGL context is created");
        glewExperimental = GL_TRUE;
        TestSupport::require(glewInit() == GLEW_OK, "GLEW initializes");
        while (glGetError() != GL_NO_ERROR) {
        }

        glViewport(0, 0, 800, 600);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    ~SdlRuntime() {
        if (glContext) {
            SDL_GL_DeleteContext(glContext);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }
};

void movePlayerTo(GameState& gs, const glm::vec2& position) {
    b2Body_SetTransform(gs.playerBodyId, b2Vec2{position.x, position.y}, b2Rot{0});
    b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);
}

void enterHomeBaseWithoutFade(GameState& gs) {
    bool previousTransitionEffect = gs.regionManager.isTransitionEffectEnabled();
    gs.regionManager.setTransitionEffectEnabled(false);
    gs.regionManager.transitionTo("home_base", glm::ivec2(9, 11), gs.worldId);
    gs.regionManager.setTransitionEffectEnabled(previousTransitionEffect);

    RegionService::GameplayContext regionGameplay = RegionService::makeGameplayContext(gs);
    RegionService::refreshGameplayContext(regionGameplay);
}

void automaticArcadePortalRendersThroughTransition() {
    SdlRuntime sdl;
    GameState gs{};
    TestSupport::require(GameBootstrap::initialize(gs, sdl.window), "game bootstrap initializes");

    enterHomeBaseWithoutFade(gs);
    MapRegion* homeBase = gs.regionManager.getCurrentRegion();
    TestSupport::require(homeBase != nullptr, "home base is current before portal render smoke");
    movePlayerTo(gs, homeBase->getTileMap().tileToWorld(20, 9));
    gs.camera.position = homeBase->getTileMap().tileToWorld(20, 9);

    WorldScene::State state = WorldScene::createState();
    constexpr float dt = 1.0f / 60.0f;
    bool reachedArcade = false;
    int stableArcadeFrames = 0;

    auto started = std::chrono::steady_clock::now();
    for (int frame = 0; frame < 240; ++frame) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
        }

        WorldScene::update(nullptr, gs, dt, state);
        WorldScene::render(sdl.window, gs, dt, state);

        GLenum err = glGetError();
        TestSupport::require(err == GL_NO_ERROR, "OpenGL stays clean during arcade portal transition");

        if (WorldQuery::isCurrentRegion(gs.regionManager, "popup_arcade") &&
            !gs.regionManager.isTransitioning()) {
            reachedArcade = true;
            ++stableArcadeFrames;
            if (stableArcadeFrames >= 60) {
                break;
            }
        } else if (reachedArcade) {
            stableArcadeFrames = 0;
        }
    }

    auto elapsed = std::chrono::steady_clock::now() - started;
    TestSupport::require(
        std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() < 15,
        "arcade portal render transition and post-entry frames complete without hanging");
    TestSupport::require(reachedArcade, "automatic arcade portal reaches rendered popup arcade");
    TestSupport::require(stableArcadeFrames >= 60,
                         "rendered popup arcade remains stable after entry");

    GameBootstrap::shutdown(gs);
}

void cachedAutomaticArcadePortalRendersImmediateTransition() {
    SdlRuntime sdl;
    GameState gs{};
    TestSupport::require(GameBootstrap::initialize(gs, sdl.window), "game bootstrap initializes for cached portal");

    enterHomeBaseWithoutFade(gs);
    TestSupport::require(gs.regionManager.loadRegion("popup_arcade"), "popup arcade is cached before portal");
    TestSupport::require(gs.regionManager.loadRegion("dark_forest"), "dark forest is cached before portal");
    TestSupport::require(gs.regionManager.loadRegion("mountain_pass"), "mountain pass is cached before portal");
    TestSupport::require(gs.regionManager.loadRegion("coastal_town"), "coastal town is cached before portal");
    TestSupport::require(gs.regionManager.loadRegion("cached_extra_a"), "extra region A is cached before portal");
    TestSupport::require(gs.regionManager.loadRegion("cached_extra_b"), "extra region B is cached before portal");
    gs.regionManager.update(1.0f);

    MapRegion* homeBase = gs.regionManager.getCurrentRegion();
    TestSupport::require(homeBase != nullptr, "home base is current before cached portal smoke");
    movePlayerTo(gs, homeBase->getTileMap().tileToWorld(20, 9));
    gs.camera.position = homeBase->getTileMap().tileToWorld(20, 9);

    WorldScene::State state = WorldScene::createState();
    constexpr float dt = 1.0f / 60.0f;

    WorldScene::update(nullptr, gs, dt, state);
    WorldScene::render(sdl.window, gs, dt, state);

    TestSupport::require(
        WorldQuery::isCurrentRegion(gs.regionManager, "popup_arcade"),
        "cached automatic arcade portal switches immediately to popup arcade");
    TestSupport::require(
        !gs.regionManager.isTransitioning(),
        "cached automatic arcade portal does not leave a pending transition");
    TestSupport::require(glGetError() == GL_NO_ERROR,
                         "OpenGL stays clean during cached arcade portal transition");

    for (int frame = 0; frame < 60; ++frame) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
        }
        WorldScene::update(nullptr, gs, dt, state);
        WorldScene::render(sdl.window, gs, dt, state);
        TestSupport::require(
            WorldQuery::isCurrentRegion(gs.regionManager, "popup_arcade"),
            "cached automatic arcade portal remains in popup arcade after entry");
        TestSupport::require(glGetError() == GL_NO_ERROR,
                             "OpenGL stays clean after cached arcade portal entry");
    }

    GameBootstrap::shutdown(gs);
}

void popupArcadePhysicsBodyBudgetIsBounded() {
    // 回归测试:防止 configurePopupArcade / generateSimpleDungeon 重新把内部
    // 全部格子设为 Stone,导致 buildPhysics 一次性往 Box2D 塞入数千个静态
    // 刚体,fade-out 后 completeTransition 卡死。
    //
    // 实际数:
    // - 修复前 (60×60 全 Stone):~3364 个刚体
    // - 修复后 (60×60 Dirt + 4 条边界石柱 + 墙):~120 个刚体
    // 阈值取 500 留出安全余量,任何把内部改回 Stone 的修改都会立即炸出来。

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{0.0f, 0.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    std::unique_ptr<MapRegion> arcade = RegionFactory::createRegion("popup_arcade");
    TestSupport::require(arcade != nullptr, "popup_arcade region is created for budget check");
    arcade->buildPhysics(worldId);

    size_t bodyCount = arcade->getTileManager().getBodyCount();
    TestSupport::require(bodyCount < 500,
        "popup_arcade physics body count stays under freeze threshold (was 3364 before fix)");

    // 显式打印实际数值,方便排错时确认预算
    std::fprintf(stderr, "[budget] popup_arcade physics body count = %zu\n", bodyCount);

    b2DestroyWorld(worldId);
}

void walkingOntoArcadePortalRendersThroughTransition() {
    SdlRuntime sdl;
    GameState gs{};
    TestSupport::require(GameBootstrap::initialize(gs, sdl.window),
                         "game bootstrap initializes for walking portal");

    enterHomeBaseWithoutFade(gs);
    MapRegion* homeBase = gs.regionManager.getCurrentRegion();
    TestSupport::require(homeBase != nullptr, "home base is current before walking portal");
    movePlayerTo(gs, homeBase->getTileMap().tileToWorld(18, 9));
    gs.camera.position = homeBase->getTileMap().tileToWorld(18, 9);
    gs.input.setKey(SDL_SCANCODE_D, true);

    WorldScene::State state = WorldScene::createState();
    constexpr float dt = 1.0f / 60.0f;
    bool reachedArcade = false;
    int stableArcadeFrames = 0;

    auto started = std::chrono::steady_clock::now();
    for (int frame = 0; frame < 420; ++frame) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
        }

        WorldScene::update(nullptr, gs, dt, state);
        WorldScene::render(sdl.window, gs, dt, state);
        TestSupport::require(glGetError() == GL_NO_ERROR,
                             "OpenGL stays clean while walking onto arcade portal");

        if (WorldQuery::isCurrentRegion(gs.regionManager, "popup_arcade") &&
            !gs.regionManager.isTransitioning()) {
            reachedArcade = true;
            gs.input.setKey(SDL_SCANCODE_D, false);
            ++stableArcadeFrames;
            if (stableArcadeFrames >= 60) {
                break;
            }
        } else if (reachedArcade) {
            stableArcadeFrames = 0;
        }
    }

    auto elapsed = std::chrono::steady_clock::now() - started;
    TestSupport::require(
        std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() < 15,
        "walking arcade portal transition and post-entry frames complete without hanging");
    TestSupport::require(reachedArcade, "walking onto arcade portal reaches popup arcade");
    TestSupport::require(stableArcadeFrames >= 60,
                         "popup arcade remains stable after walking entry");

    GameBootstrap::shutdown(gs);
}

}  // namespace

int main() {
    popupArcadePhysicsBodyBudgetIsBounded();
    automaticArcadePortalRendersThroughTransition();
    cachedAutomaticArcadePortalRendersImmediateTransition();
    walkingOntoArcadePortalRendersThroughTransition();
    return 0;
}
