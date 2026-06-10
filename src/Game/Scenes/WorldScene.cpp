#include "Game/Scenes/WorldScene.h"

#include "Game/Controllers/InputController.h"
#include "Game/Presentation/GameRenderer.h"
#include "Game/Presentation/MiniMapPresenter.h"
#include "Game/Presentation/WorldPresentationBuilder.h"

#include <SDL2/SDL.h>

namespace WorldScene {

namespace {

class Scene final : public IScene {
public:
    Scene() : state(createState()) {}

    bool handleEvent(GameState& gs,
                     const SDL_Event& event,
                     const InputController::Callbacks& callbacks) override {
        return WorldScene::handleEvent(gs, event, callbacks);
    }

    void update(GameState& gs, float dt) override {
        WorldScene::update(nullptr, gs, dt, state);
    }

    void render(SDL_Window* window, GameState& gs, float dt) override {
        WorldScene::render(window, gs, dt, state);
    }

private:
    State state;
};

}  // namespace

State createState() {
    State state;
    state.titleState = WindowTitlePresenter::State{SDL_GetTicks(), 0};
    return state;
}

std::unique_ptr<IScene> create() {
    return std::make_unique<Scene>();
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
    GameRenderer::WorldRenderContext renderContext = WorldPresentationBuilder::buildRenderContext(gs);
    GameRenderer::renderWorld(renderContext, state.tileColors, dt);
    SDL_GL_SwapWindow(window);
    WindowTitlePresenter::update(window, gs, state.titleState);
}

void updateAndRender(SDL_Window* window, GameState& gs, float dt, State& state) {
    update(window, gs, dt, state);
    render(window, gs, dt, state);
}

}  // namespace WorldScene
