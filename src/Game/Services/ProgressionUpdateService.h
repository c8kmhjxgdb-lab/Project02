#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <functional>
#include <string>

class BuildingSystem;
class DropManager;
class EmotionSystem;
class Inventory;
class PopupCrownBoss;
class Princess;
class QuestSystem;
class RegionManager;
class StoryProgress;
class TimeSystem;
class ToySystem;
class WeatherSystem;
struct InputState;
struct GameState;

namespace ProgressionUpdateService {

struct Context {
    RegionManager& regionManager;
    EmotionSystem& emotionSystem;
    const glm::vec2& homePosition;
    float homeRadius;
    BuildingSystem& buildingSystem;
    ToySystem& toySystem;
    InputState& input;
    TimeSystem& timeSystem;
    Inventory& inventory;
    StoryProgress& storyProgress;
    PopupCrownBoss& popupCrownBoss;
    DropManager& dropManager;
    b2WorldId worldId;
    Princess* princess;
    WeatherSystem& weatherSystem;
    QuestSystem& questSystem;
    bool& talkedWithPrincessAtBaseThisFrame;
    std::function<void(const std::string&)> showNotice;
};

struct State {
    float baseChildlikeBonusTimer = 0.0f;
};

Context makeContext(GameState& gs);

void update(Context& context, float dt, const glm::vec2& playerPos, State& state);

}  // namespace ProgressionUpdateService
