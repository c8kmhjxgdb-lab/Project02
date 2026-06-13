#include "Game/Services/WorldCombatUpdateService.h"

#include "Game/GameState.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/ProjectileTrailService.h"

namespace WorldCombatUpdateService {

namespace {

void updateCombatStoryFlags(Context& context) {
    if (context.storyProgress.getFlag("scrap_elite_defeated")) {
        return;
    }

    for (const Enemy& enemy : context.enemyManager.getActive()) {
        if (enemy.state == Enemy::State::Dead &&
            enemy.definitionId == "scrap_soldier_elite") {
            context.storyProgress.setFlag("scrap_elite_defeated", true);
            return;
        }
    }
}

}  // namespace

Context makeContext(GameState& gs) {
    return {
        gs.projectileManager,
        gs.particleSystem,
        gs.enemyManager,
        gs.dropManager,
        gs.playerHealth,
        gs.fireballCooldown,
        gs.storyProgress,
        gs.popupCrownBoss,
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
    context.popupCrownBoss.update(dt);

    if (context.fireballCooldown > 0.0f) {
        context.fireballCooldown -= dt;
    }

    CombatService::handleCollisions(context.collision);
    updateCombatStoryFlags(context);

    context.enemyManager.cleanup();

    context.playerHealth.update(dt);
}

void updateWhileDead(Context& context, float dt) {
    context.projectileManager.update(dt, context.worldId);
    context.particleSystem.update(dt);
}

}  // namespace WorldCombatUpdateService
