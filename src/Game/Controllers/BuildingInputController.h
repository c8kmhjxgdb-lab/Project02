#pragma once

#include <SDL2/SDL.h>
#include <glm/vec2.hpp>

#include <functional>
#include <string>

class BuildingSystem;
class Inventory;
class RegionManager;
class TimeSystem;
class ToySystem;
struct GameState;

namespace BuildingInputController {

struct Context {
    bool& isDead;
    BuildingSystem& buildingSystem;
    RegionManager& regionManager;
    ToySystem& toySystem;
    TimeSystem& timeSystem;
    Inventory& inventory;
};

struct Callbacks {
    std::function<glm::vec2(const Context&)> getMouseWorldPoint;
    std::function<void(Context&, const std::string&)> showNotice;
};

Context makeContext(GameState& gs);
Callbacks makeCallbacks(GameState& gs);

bool canBuildHere(Context& context);
void handleToggleKey(Context& context, SDL_Scancode scancode);
void handleKeyDown(Context& context, SDL_Scancode scancode, const Callbacks& callbacks);
void handleMouseButtonDown(Context& context, Uint8 button, const Callbacks& callbacks);
bool handleMouseWheel(Context& context, int wheelY);

}  // namespace BuildingInputController
