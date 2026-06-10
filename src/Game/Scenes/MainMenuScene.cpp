#include "Game/Scenes/MainMenuScene.h"

#include "Game/Controllers/InputState.h"
#include "Game/Controllers/MainMenuInputController.h"
#include "Game/GameState.h"
#include "Game/Presentation/GameRenderer.h"
#include "Game/Presentation/PresentationModelBuilder.h"

#include <SDL2/SDL.h>
#include <glm/vec2.hpp>

#include <algorithm>

namespace MainMenuScene {

namespace {

class Scene final : public IScene {
public:
    bool handleEvent(GameState& gs,
                     const SDL_Event& event,
                     const InputController::Callbacks& callbacks) override {
        return MainMenuScene::handleEvent(gs, event, callbacks);
    }

    void update(GameState& gs, float dt) override {
        MainMenuScene::update(gs, dt);
    }

    void render(SDL_Window* window, GameState& gs, float /*dt*/) override {
        MainMenuScene::render(window, gs);
    }
};

GameRenderer::MainMenuRenderContext makeRenderContext(GameState& gs) {
    return {
        PresentationModelBuilder::buildMainMenuModel(gs),
        gs.screenWidth,
        gs.screenHeight
    };
}

}  // namespace

std::unique_ptr<IScene> create() {
    return std::make_unique<Scene>();
}

bool handleEvent(GameState& gs,
                 const SDL_Event& event,
                 const InputController::Callbacks& callbacks) {
    if (event.type == SDL_KEYDOWN) {
        SDL_Scancode scancode = event.key.keysym.scancode;
        gs.input.setKey(scancode, true);
        return MainMenuInputController::handleKeyDown(gs, scancode, callbacks);
    }

    if (event.type == SDL_KEYUP) {
        gs.input.setKey(event.key.keysym.scancode, false);
    } else if (event.type == SDL_MOUSEMOTION) {
        gs.input.mousePos = glm::vec2(static_cast<float>(event.motion.x),
                                      static_cast<float>(event.motion.y));
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        gs.input.mousePos = glm::vec2(static_cast<float>(event.button.x),
                                      static_cast<float>(event.button.y));
        return MainMenuInputController::handleMouseButtonDown(gs, event.button, callbacks);
    }

    return false;
}

void update(GameState& gs, float dt) {
    gs.charTime += dt;
    if (gs.ui.menuMessageTimer > 0.0f) {
        gs.ui.menuMessageTimer = std::max(0.0f, gs.ui.menuMessageTimer - dt);
    }
}

void render(SDL_Window* window, GameState& gs) {
    GameRenderer::MainMenuRenderContext renderContext = makeRenderContext(gs);
    GameRenderer::renderMainMenu(renderContext);
    SDL_GL_SwapWindow(window);
}

void updateAndRender(SDL_Window* window, GameState& gs, float dt) {
    update(gs, dt);
    render(window, gs);
}

}  // namespace MainMenuScene
