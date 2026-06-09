#pragma once

#include "Game/Controllers/InputController.h"

#include <SDL2/SDL.h>

struct GameState;

namespace MainMenuInputController {

bool handleKeyDown(GameState& gs,
                   SDL_Scancode scancode,
                   const InputController::Callbacks& callbacks);

bool handleMouseButtonDown(GameState& gs,
                           const SDL_MouseButtonEvent& buttonEvent,
                           const InputController::Callbacks& callbacks);

}  // namespace MainMenuInputController
