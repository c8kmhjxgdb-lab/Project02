#pragma once

#include <glm/vec2.hpp>

struct GameState;

namespace WorldCombatUpdateService {

void updateAlive(GameState& gs, float dt, const glm::vec2& playerPos);
void updateWhileDead(GameState& gs, float dt);

}  // namespace WorldCombatUpdateService
