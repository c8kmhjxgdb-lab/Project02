#pragma once

#include "Game/Controllers/InputController.h"
#include "Game/Presentation/WindowTitlePresenter.h"
#include "Game/Scenes/IScene.h"
#include "Game/Services/WorldUpdateService.h"
#include "Game/World/TileMap.h"

#include <memory>

struct GameState;
struct SDL_Window;
union SDL_Event;

namespace WorldScene {

struct State {
    WindowTitlePresenter::State titleState;
    WorldUpdateService::State worldUpdateState;
    TileColors tileColors;
};

State createState();
std::unique_ptr<IScene> create();

bool handleEvent(GameState& gs,
                 const SDL_Event& event,
                 const InputController::Callbacks& callbacks);
void update(SDL_Window* window, GameState& gs, float dt, State& state);
void render(SDL_Window* window, GameState& gs, float dt, State& state);
void updateAndRender(SDL_Window* window, GameState& gs, float dt, State& state);

}  // namespace WorldScene
