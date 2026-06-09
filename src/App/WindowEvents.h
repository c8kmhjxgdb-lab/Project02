#pragma once

#include <SDL2/SDL.h>

struct GameState;

namespace WindowEvents {

void handleWindowEvent(GameState& gs, const SDL_WindowEvent& event);

}  // namespace WindowEvents
