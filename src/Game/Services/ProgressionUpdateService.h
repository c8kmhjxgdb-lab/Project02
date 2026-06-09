#pragma once

#include <glm/vec2.hpp>

struct GameState;

namespace ProgressionUpdateService {

struct State {
    float baseChildlikeBonusTimer = 0.0f;
};

void update(GameState& gs, float dt, const glm::vec2& playerPos, State& state);

}  // namespace ProgressionUpdateService
