#pragma once

#include <SDL2/SDL.h>

struct GameState;

namespace AbilityInputController {

void handleKeyDown(GameState& gs, SDL_Scancode scancode);
void handleMouseButtonDown(GameState& gs, Uint8 button);

}  // namespace AbilityInputController
