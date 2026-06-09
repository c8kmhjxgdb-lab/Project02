#pragma once

#include "Game/Ability/Projectile.h"

#include <glm/vec2.hpp>

struct GameState;

namespace CombatService {

bool tryCastProjectile(GameState& gs, ProjectileType type);

bool tryCastLightning(GameState& gs);

void spawnEnemy(GameState& gs, const glm::vec2& pos);

void handleCollisions(GameState& gs);

}  // namespace CombatService
