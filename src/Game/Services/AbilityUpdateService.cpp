#include "Game/Services/AbilityUpdateService.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

namespace AbilityUpdateService {

void update(GameState& gs, float dt) {
    gs.shield.update(dt);
    gs.lightning.update(dt);
    gs.lightning.updateCooldown(dt);
    gs.bondTechnique.update(dt);
    gs.bondTechnique.updateCooldown(dt);

    auto aliveEnemies = gs.enemyManager.getAlive();

    if (gs.bondTechnique.isActive()) {
        BondTechnique& tech = gs.bondTechnique.getCurrentTechnique();
        if (!tech.hasDealtDamage() && tech.radius > tech.maxRadius * 0.3f && !tech.waveFronts.empty()) {
            for (const Enemy* enemy : aliveEnemies) {
                if (!enemy || !b2Body_IsValid(enemy->bodyId)) continue;
                b2Vec2 ePos = b2Body_GetPosition(enemy->bodyId);
                glm::vec2 enemyPos(ePos.x, ePos.y);
                if (glm::distance(tech.waveFronts[0], enemyPos) < tech.radius) {
                    gs.enemyManager.damage(enemy->id, tech.damage);
                }
            }
            tech.markDamaged();
        }
    }

    if (gs.shield.isActive()) {
        b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
        glm::vec2 playerPos(pPos.x, pPos.y);

        for (const Enemy* enemy : aliveEnemies) {
            if (enemy && b2Body_IsValid(enemy->bodyId)) {
                gs.shield.checkAndRepelEnemy(enemy->bodyId, playerPos, 15.0f);
            }
        }
    }
}

}  // namespace AbilityUpdateService
