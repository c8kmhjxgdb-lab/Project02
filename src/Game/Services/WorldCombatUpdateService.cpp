#include "Game/Services/WorldCombatUpdateService.h"

#include "Game/GameState.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/ProjectileTrailService.h"

namespace WorldCombatUpdateService {

void updateAlive(GameState& gs, float dt, const glm::vec2& playerPos) {
    gs.projectileManager.update(dt, gs.worldId);

    ProjectileTrailService::emitTrails(gs);

    gs.particleSystem.update(dt);

    gs.enemyManager.update(dt, gs.worldId, playerPos);
    gs.dropManager.update(dt, playerPos);

    if (gs.fireballCooldown > 0.0f) {
        gs.fireballCooldown -= dt;
    }

    CombatService::handleCollisions(gs);

    gs.enemyManager.cleanup();

    gs.playerHealth.update(dt);
}

void updateWhileDead(GameState& gs, float dt) {
    gs.projectileManager.update(dt, gs.worldId);
    gs.particleSystem.update(dt);
}

}  // namespace WorldCombatUpdateService
