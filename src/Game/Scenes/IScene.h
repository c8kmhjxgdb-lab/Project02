#pragma once

#include "Game/Controllers/InputController.h"

struct GameState;
struct SDL_Window;
union SDL_Event;

class IScene {
public:
    virtual ~IScene() = default;

    virtual void enter(GameState& gs) { (void)gs; }
    virtual bool handleEvent(GameState& gs,
                             const SDL_Event& event,
                             const InputController::Callbacks& callbacks) = 0;
    virtual void update(GameState& gs, float dt) = 0;
    virtual void render(SDL_Window* window, GameState& gs, float dt) = 0;
    virtual void exit(GameState& gs) { (void)gs; }
};
