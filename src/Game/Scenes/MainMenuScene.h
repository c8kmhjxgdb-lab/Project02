#pragma once

#include "Game/Controllers/InputController.h"
#include "Game/Scenes/IScene.h"

#include <memory>

struct GameState;
struct SDL_Window;
union SDL_Event;

namespace MainMenuScene {

std::unique_ptr<IScene> create();

bool handleEvent(GameState& gs,
                 const SDL_Event& event,
                 const InputController::Callbacks& callbacks);
void update(GameState& gs, float dt);
void render(SDL_Window* window, GameState& gs);
void updateAndRender(SDL_Window* window, GameState& gs, float dt);

}  // namespace MainMenuScene
