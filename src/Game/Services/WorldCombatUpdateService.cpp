#include "Game/Services/WorldCombatUpdateService.h"

#include "Game/GameState.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/ProjectileTrailService.h"

namespace WorldCombatUpdateService {

Context makeContext(GameState& gs) {
    return {
        gs.projectileManager,
        gs.particleSystem,
        gs.enemyManager,
        gs.dropManager,
        gs.playerHealth,
        gs.fireballCooldown,
        gs.worldId,
        ProjectileTrailService::makeContext(gs),
        CombatCollisionService::makeContext(gs)
    };
}

void updateAlive(Context& context, float dt, const glm::vec2& playerPos) {
    context.projectileManager.update(dt, context.worldId);

    ProjectileTrailService::emitTrails(context.projectileTrail);

    context.particleSystem.update(dt);

    context.enemyManager.update(dt, context.worldId, playerPos);
    context.dropManager.update(dt, playerPos);

    if (context.fireballCooldown > 0.0f) {
        context.fireballCooldown -= dt;
    }

    CombatService::handleCollisions(context.collision);

    context.enemyManager.cleanup();

    context.playerHealth.update(dt);
}

void updateWhileDead(Context& context, float dt) {
    context.projectileManager.update(dt, context.worldId);
    context.particleSystem.update(dt);
}

}  // namespace WorldCombatUpdateService
