#include "Game/Services/AbilityUpdateService.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

namespace AbilityUpdateService {

Context makeContext(GameState& gs) {
    return {
        gs.shield,
        gs.lightning,
        gs.bondTechnique,
        gs.enemyManager,
        gs.playerBodyId
    };
}

void update(Context& context, float dt) {
    context.shield.update(dt);
    context.lightning.update(dt);
    context.lightning.updateCooldown(dt);
    context.bondTechnique.update(dt);
    context.bondTechnique.updateCooldown(dt);

    auto aliveEnemies = context.enemyManager.getAlive();

    if (context.bondTechnique.isActive()) {
        BondTechnique& tech = context.bondTechnique.getCurrentTechnique();
        if (!tech.hasDealtDamage() && tech.radius > tech.maxRadius * 0.3f && !tech.waveFronts.empty()) {
            for (const Enemy* enemy : aliveEnemies) {
                if (!enemy || !b2Body_IsValid(enemy->bodyId)) continue;
                b2Vec2 ePos = b2Body_GetPosition(enemy->bodyId);
                glm::vec2 enemyPos(ePos.x, ePos.y);
                if (glm::distance(tech.waveFronts[0], enemyPos) < tech.radius) {
                    context.enemyManager.damage(enemy->id, tech.damage);
                }
            }
            tech.markDamaged();
        }
    }

    if (context.shield.isActive()) {
        b2Vec2 pPos = b2Body_GetPosition(context.playerBodyId);
        glm::vec2 playerPos(pPos.x, pPos.y);

        for (const Enemy* enemy : aliveEnemies) {
            if (enemy && b2Body_IsValid(enemy->bodyId)) {
                context.shield.checkAndRepelEnemy(enemy->bodyId, playerPos, 15.0f);
            }
        }
    }
}

}  // namespace AbilityUpdateService
