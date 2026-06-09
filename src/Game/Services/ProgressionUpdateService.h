#pragma once

#include <glm/vec2.hpp>

#include <functional>
#include <string>

class BuildingSystem;
class EmotionSystem;
class Inventory;
class Princess;
class QuestSystem;
class RegionManager;
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
