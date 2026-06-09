#pragma once

struct GameState;
struct SDL_Window;

namespace GameBootstrap {

bool initialize(GameState& gs, SDL_Window* window);

void shutdown(GameState& gs);

}  // namespace GameBootstrap
