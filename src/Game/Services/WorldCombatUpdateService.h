#pragma once

#include "Game/Services/CombatCollisionService.h"
#include "Game/Services/ProjectileTrailService.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

class DropManager;
class EnemyManager;
class HealthComponent;
class ParticleSystem;
class PopupCrownBoss;
class ProjectileManager;
struct GameState;

namespace WorldCombatUpdateService {

struct Context {
    ProjectileManager& projectileManager;
    ParticleSystem& particleSystem;
    EnemyManager& enemyManager;
    DropManager& dropManager;
    HealthComponent& playerHealth;
    float& fireballCooldown;
    PopupCrownBoss& popupCrownBoss;
    b2WorldId worldId;
    ProjectileTrailService::Context projectileTrail;
    CombatCollisionService::Context collision;
};

Context makeContext(GameState& gs);

void updateAlive(Context& context, float dt, const glm::vec2& playerPos);
void updateWhileDead(Context& context, float dt);

}  // namespace WorldCombatUpdateService
