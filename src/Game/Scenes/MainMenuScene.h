#pragma once

#include "Game/Controllers/InputController.h"

struct GameState;
struct SDL_Window;
union SDL_Event;

namespace MainMenuScene {

bool handleEvent(GameState& gs,
                 const SDL_Event& event,
                 const InputController::Callbacks& callbacks);
void update(GameState& gs, float dt);
void render(SDL_Window* window, GameState& gs);
void updateAndRender(SDL_Window* window, GameState& gs, float dt);

}  // namespace MainMenuScene
