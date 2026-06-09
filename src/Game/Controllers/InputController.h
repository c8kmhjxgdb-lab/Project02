#pragma once

#include <SDL2/SDL.h>

#include <functional>

struct GameState;

namespace InputController {

struct Callbacks {
    std::function<bool(GameState&)> activateMainMenuSelection;
};

// Returns true when the event requests application shutdown.
bool handleEvent(GameState& gs, const SDL_Event& e, const Callbacks& callbacks);

}  // namespace InputController
