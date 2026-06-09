#pragma once

#include <SDL2/SDL.h>

struct GameState;

namespace WindowTitlePresenter {

struct State {
    Uint32 lastUpdateTicks = 0;
    int frameCount = 0;
};

void update(SDL_Window* window, const GameState& gs, State& state);

}  // namespace WindowTitlePresenter
