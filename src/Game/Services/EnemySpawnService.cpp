#include "Game/Services/EnemySpawnService.h"

#include "Game/GameState.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/WorldQuery.h"
#include "Utils/Math.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <cmath>

namespace EnemySpawnService {

Context makeContext(GameState& gs) {
    return {
        gs.regionManager,
        gs.enemyManager,
        gs.playerBodyId,
        gs.enemySpawnTimer,
        gs.enemySpawnInterval,
        gs.maxEnemies,
        gs.charTime,
        CombatService::makeSpawnContext(gs)
    };
}

void update(Context& context, float dt) {
    bool inHomeBase = WorldQuery::isCurrentRegion(context.regionManager, "home_base");
    context.enemySpawnTimer += dt;
    if (inHomeBase) {
        context.enemySpawnTimer = 0.0f;
        return;
    }

    if (context.enemySpawnTimer < context.enemySpawnInterval) {
        return;
    }

    context.enemySpawnTimer = 0.0f;

    auto spawnAliveEnemies = context.enemyManager.getAlive();
    if (static_cast<int>(spawnAliveEnemies.size()) >= context.maxEnemies) {
        return;
    }

    b2Vec2 pPosSpawn = b2Body_GetPosition(context.playerBodyId);
    glm::vec2 playerPosSpawn(pPosSpawn.x, pPosSpawn.y);
    float angle = Math::hashRandom(static_cast<unsigned int>(context.charTime * 777)) * 6.28318f;
    float dist = 8.0f + Math::hashRandom(static_cast<unsigned int>(context.charTime * 333)) * 5.0f;
    glm::vec2 spawnPos = playerPosSpawn + glm::vec2(std::cos(angle), std::sin(angle)) * dist;
    CombatService::spawnEnemy(context.combatSpawn, spawnPos);
}

}  // namespace EnemySpawnService
