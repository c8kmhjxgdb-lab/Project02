#pragma once

struct GameState;
struct SDL_Window;

namespace GameLoop {

void run(SDL_Window* window, GameState& gs);

}  // namespace GameLoop
