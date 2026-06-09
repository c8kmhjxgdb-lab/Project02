#pragma once

#include <SDL2/SDL.h>

struct GameState;

namespace InteractionInputController {

void handleInteract(GameState& gs);
void handleDialogueNavigation(GameState& gs, SDL_Scancode scancode);

}  // namespace InteractionInputController
