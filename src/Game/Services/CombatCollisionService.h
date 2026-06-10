#pragma once

#include <box2d/box2d.h>

class EnemyManager;
class HealthComponent;
class ParticleSystem;
class ProjectileManager;
class Shield;
struct GameState;

namespace CombatCollisionService {

struct Context {
    ProjectileManager& projectileManager;
    EnemyManager& enemyManager;
    ParticleSystem& particleSystem;
    Shield& shield;
    HealthComponent& playerHealth;
    b2BodyId playerBodyId;
    bool& isDead;
    bool& isFlying;
    float& deathTimer;
};

Context makeContext(GameState& gs);

void handleCollisions(Context& context);

}  // namespace CombatCollisionService
