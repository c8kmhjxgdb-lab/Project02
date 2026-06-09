#include "Game/Scenes/MainMenuScene.h"

#include "Game/Controllers/InputController.h"
#include "Game/GameState.h"
#include "Game/Presentation/GameRenderer.h"

#include <SDL2/SDL.h>

#include <algorithm>

namespace MainMenuScene {

bool handleEvent(GameState& gs,
                 const SDL_Event& event,
                 const InputController::Callbacks& callbacks) {
    return InputController::handleEvent(gs, event, callbacks);
}

void update(GameState& gs, float dt) {
    gs.charTime += dt;
    if (gs.menuMessageTimer > 0.0f) {
        gs.menuMessageTimer = std::max(0.0f, gs.menuMessageTimer - dt);
    }
}

void render(SDL_Window* window, GameState& gs) {
    GameRenderer::renderMainMenu(gs);
    SDL_GL_SwapWindow(window);
}

void updateAndRender(SDL_Window* window, GameState& gs, float dt) {
    update(gs, dt);
    render(window, gs);
}

}  // namespace MainMenuScene
