#include "Game/Scenes/WorldScene.h"

#include "Game/Controllers/InputController.h"
#include "Game/GameState.h"
#include "Game/Presentation/GameRenderer.h"
#include "Game/Presentation/MiniMapPresenter.h"
#include "Game/Presentation/PresentationModelBuilder.h"
#include "Game/Services/PlayerInputQuery.h"

#include <SDL2/SDL.h>

namespace WorldScene {

namespace {

GameRenderer::WorldRenderContext makeRenderContext(GameState& gs) {
    return {
        gs.camera,
        gs.regionManager,
        gs.decorRenderer,
        gs.buildingSystem,
        gs.toySystem,
        gs.dropManager,
        gs.enemyManager,
        gs.projectileManager,
        gs.particleSystem,
        gs.postProcess,
        gs.miniMap,
        gs.dialogueUI,
        gs.emotionSystem,
        gs.timeSystem,
        gs.princess.get(),
        gs.playerBodyId,
        gs.isDead,
        gs.screenWidth,
        gs.screenHeight,
        gs.charTime,
        gs.postProcessShader,
        PlayerInputQuery::getMouseWorldPoint(gs),
        PresentationModelBuilder::buildEnemyRenderResources(gs),
        PresentationModelBuilder::buildCharacterRenderResources(gs),
        PresentationModelBuilder::buildCharacterModel(gs),
        PresentationModelBuilder::buildProjectileRenderResources(gs),
        PresentationModelBuilder::buildAbilityEffectsModel(gs),
        PresentationModelBuilder::buildVentTearsModel(gs),
        PresentationModelBuilder::buildAimReticleModel(gs),
        PresentationModelBuilder::buildHudModel(gs)
    };
}

}  // namespace

State createState() {
    State state;
    state.titleState = WindowTitlePresenter::State{SDL_GetTicks(), 0};
    return state;
}

bool handleEvent(GameState& gs,
                 const SDL_Event& event,
                 const InputController::Callbacks& callbacks) {
    return InputController::handleEvent(gs, event, callbacks);
}

void update(SDL_Window* /*window*/, GameState& gs, float dt, State& state) {
    WorldUpdateService::update(gs, dt, state.worldUpdateState);
    MiniMapPresenter::update(gs, dt);
}

void render(SDL_Window* window, GameState& gs, float dt, State& state) {
    GameRenderer::WorldRenderContext renderContext = makeRenderContext(gs);
    GameRenderer::renderWorld(renderContext, state.tileColors, dt);
    SDL_GL_SwapWindow(window);
    WindowTitlePresenter::update(window, gs, state.titleState);
}

void updateAndRender(SDL_Window* window, GameState& gs, float dt, State& state) {
    update(window, gs, dt, state);
    render(window, gs, dt, state);
}

}  // namespace WorldScene
