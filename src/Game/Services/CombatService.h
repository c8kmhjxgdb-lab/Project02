#pragma once

#include "Game/Ability/Projectile.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

namespace CombatCollisionService {
struct Context;
}

class EnemyManager;
struct GameState;

namespace CombatService {

struct SpawnContext {
    b2BodyId playerBodyId;
    float& charTime;
    int& enemiesKilled;
    EnemyManager& enemyManager;
    b2WorldId worldId;
};

bool tryCastProjectile(GameState& gs, ProjectileType type);

bool tryCastLightning(GameState& gs);

SpawnContext makeSpawnContext(GameState& gs);

void spawnEnemy(SpawnContext& context, const glm::vec2& pos);

void spawnEnemy(GameState& gs, const glm::vec2& pos);

void handleCollisions(CombatCollisionService::Context& context);

void handleCollisions(GameState& gs);

}  // namespace CombatService
