#include "Game/Scenes/SceneManager.h"

#include "Game/Scenes/MainMenuScene.h"
#include "Game/Scenes/WorldScene.h"

namespace {

IScene* selectScene(SceneManager::State& state, AppMode mode) {
    if (mode == AppMode::MainMenu) {
        return state.mainMenuScene.get();
    }
    return state.worldScene.get();
}

void syncCurrentScene(GameState& gs, SceneManager::State& state) {
    if (state.currentScene && state.currentMode == state.requestedMode) {
        return;
    }

    if (state.currentScene) {
        state.currentScene->exit(gs);
    }

    state.currentMode = state.requestedMode;
    state.currentScene = selectScene(state, state.requestedMode);

    if (state.currentScene) {
        state.currentScene->enter(gs);
    }
}

}  // namespace

namespace SceneManager {

State createState(GameState& gs) {
    State state;
    state.mainMenuScene = MainMenuScene::create();
    state.worldScene = WorldScene::create();
    state.currentMode = state.requestedMode;
    state.currentScene = selectScene(state, state.requestedMode);
    if (state.currentScene) {
        state.currentScene->enter(gs);
    }
    return state;
}

bool handleEventCurrent(GameState& gs,
                        const SDL_Event& event,
                        const InputController::Callbacks& callbacks,
                        State& state) {
    syncCurrentScene(gs, state);
    if (!state.currentScene) return false;
    return state.currentScene->handleEvent(gs, event, callbacks);
}

void updateAndRenderCurrent(SDL_Window* window, GameState& gs, float dt, State& state) {
    syncCurrentScene(gs, state);
    if (!state.currentScene) return;

    state.currentScene->update(gs, dt);
    syncCurrentScene(gs, state);
    if (!state.currentScene) return;

    state.currentScene->render(window, gs, dt);
}

void requestMode(State& state, AppMode mode) {
    state.requestedMode = mode;
}

}  // namespace SceneManager
