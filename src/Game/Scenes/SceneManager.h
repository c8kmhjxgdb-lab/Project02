#pragma once

#include "Game/Controllers/InputController.h"
#include "Game/GameState.h"
#include "Game/Scenes/IScene.h"
#include "Game/Scenes/WorldScene.h"

#include <memory>

struct GameState;
struct SDL_Window;
union SDL_Event;

namespace SceneManager {

struct State {
    std::unique_ptr<WorldScene::State> worldSceneState;
    std::unique_ptr<IScene> mainMenuScene;
    std::unique_ptr<IScene> worldScene;
    IScene* currentScene = nullptr;
    AppMode currentMode = AppMode::MainMenu;
};

State createState(GameState& gs);

bool handleEventCurrent(GameState& gs,
                        const SDL_Event& event,
                        const InputController::Callbacks& callbacks,
                        State& state);
void updateAndRenderCurrent(SDL_Window* window, GameState& gs, float dt, State& state);

}  // namespace SceneManager
