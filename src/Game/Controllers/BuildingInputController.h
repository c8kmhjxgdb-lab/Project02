#pragma once

#include <SDL2/SDL.h>

struct GameState;

namespace BuildingInputController {

bool canBuildHere(GameState& gs);
void handleToggleKey(GameState& gs, SDL_Scancode scancode);
void handleKeyDown(GameState& gs, SDL_Scancode scancode);
void handleMouseButtonDown(GameState& gs, Uint8 button);
bool handleMouseWheel(GameState& gs, int wheelY);

}  // namespace BuildingInputController
