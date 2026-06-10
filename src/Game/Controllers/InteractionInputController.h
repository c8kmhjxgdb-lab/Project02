#pragma once

#include "Game/Services/DialogueService.h"

#include <box2d/box2d.h>
#include <SDL2/SDL.h>
#include <glm/vec2.hpp>

#include <functional>
#include <memory>
#include <string>

class BuildingSystem;
class EmotionSystem;
class Princess;
class RegionManager;
class TimeSystem;
class ToySystem;
class VentAnimation;
class WeatherSystem;
struct GameState;

namespace InteractionInputController {

struct Context {
    bool& isDead;
    BuildingSystem& buildingSystem;
    ToySystem& toySystem;
    DialogueTree& dialogueTree;
    DialogueUI& dialogueUI;
    RegionManager& regionManager;
    TimeSystem& timeSystem;
    EmotionSystem& emotionSystem;
    WeatherSystem& weatherSystem;
    VentAnimation& ventAnimation;
    std::unique_ptr<Princess>& princess;
    b2BodyId& playerBodyId;
    glm::vec2& homePosition;
    float& homeRadius;
    bool& isVenting;
    bool& talkedWithPrincessAtBaseThisFrame;
    DialogueService::Context dialogue;
};

struct Callbacks {
    std::function<bool(Context&, const glm::vec2&)> tryUseHomeBaseDoor;
    std::function<void(Context&, const std::string&)> showNotice;
};

Context makeContext(GameState& gs);
Callbacks makeCallbacks(GameState& gs);

void handleInteract(Context& context, const Callbacks& callbacks);
void handleDialogueNavigation(Context& context, SDL_Scancode scancode);

}  // namespace InteractionInputController
