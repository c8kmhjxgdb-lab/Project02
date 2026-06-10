#include "App/GameLoop.h"

#include "App/WindowEvents.h"
#include "Game/Controllers/InputController.h"
#include "Game/GameState.h"
#include "Game/Scenes/SceneManager.h"
#include "Game/Services/GameSessionService.h"

#include <SDL2/SDL.h>

namespace GameLoop {

void run(SDL_Window* window, GameState& gs) {
    bool running = true;
    SceneManager::State sceneState = SceneManager::createState(gs);
    const float dt = 1.0f / 60.0f;

    InputController::Callbacks inputCallbacks;
    inputCallbacks.activateMainMenuSelection = [&sceneState](GameState& state) {
        GameSessionService::MenuActivationResult result =
            GameSessionService::activateMainMenuSelection(state);
        if (result.enterPlaying) {
            SceneManager::requestMode(sceneState, AppMode::Playing);
        }
        return result.quit;
    };

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_WINDOWEVENT) {
                WindowEvents::handleWindowEvent(gs, e.window);
            } else if (SceneManager::handleEventCurrent(gs, e, inputCallbacks, sceneState)) {
                running = false;
            }
        }

        SceneManager::updateAndRenderCurrent(window, gs, dt, sceneState);
    }
}

}  // namespace GameLoop
