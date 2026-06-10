#pragma once

#include "Game/Ability/Projectile.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

namespace CombatCollisionService {
struct Context;
}

class EnemyManager;
class Lightning;
class ParticleSystem;
class ProjectileManager;
struct GameState;
namespace Engine { namespace Audio { class AudioSystem; } }

namespace CombatService {

struct CastContext {
    bool& isDead;
    b2WorldId worldId;
    b2BodyId playerBodyId;
    glm::vec2& facingDir;
    ProjectileManager& projectileManager;
    ParticleSystem& particleSystem;
    float& fireballCooldown;
    float& fireballCooldownMax;
    Lightning& lightning;
    float& playerMana;
    EnemyManager& enemyManager;
    Engine::Audio::AudioSystem* audioSystem = nullptr;
};

struct SpawnContext {
    b2BodyId playerBodyId;
    float& charTime;
    int& enemiesKilled;
    EnemyManager& enemyManager;
    b2WorldId worldId;
};

CastContext makeCastContext(GameState& gs);

bool tryCastProjectile(CastContext& context,
                       ProjectileType type,
                       const glm::vec2& playerPos,
                       const glm::vec2& aimDir);

bool tryCastLightning(CastContext& context,
                      const glm::vec2& playerPos,
                      const glm::vec2& aimDir);

SpawnContext makeSpawnContext(GameState& gs);

void spawnEnemy(SpawnContext& context, const glm::vec2& pos);

void handleCollisions(CombatCollisionService::Context& context);

}  // namespace CombatService
