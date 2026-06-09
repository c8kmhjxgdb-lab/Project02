#include "Game/Services/EnemySpawnService.h"

#include "Game/GameState.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/WorldQuery.h"
#include "Utils/Math.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <cmath>

namespace EnemySpawnService {

void update(GameState& gs, float dt) {
    bool inHomeBase = WorldQuery::isCurrentRegion(gs.regionManager, "home_base");
    gs.enemySpawnTimer += dt;
    if (inHomeBase) {
        gs.enemySpawnTimer = 0.0f;
        return;
    }

    if (gs.enemySpawnTimer < gs.enemySpawnInterval) {
        return;
    }

    gs.enemySpawnTimer = 0.0f;

    auto spawnAliveEnemies = gs.enemyManager.getAlive();
    if (static_cast<int>(spawnAliveEnemies.size()) >= gs.maxEnemies) {
        return;
    }

    b2Vec2 pPosSpawn = b2Body_GetPosition(gs.playerBodyId);
    glm::vec2 playerPosSpawn(pPosSpawn.x, pPosSpawn.y);
    float angle = Math::hashRandom(static_cast<unsigned int>(gs.charTime * 777)) * 6.28318f;
    float dist = 8.0f + Math::hashRandom(static_cast<unsigned int>(gs.charTime * 333)) * 5.0f;
    glm::vec2 spawnPos = playerPosSpawn + glm::vec2(std::cos(angle), std::sin(angle)) * dist;
    CombatService::spawnEnemy(gs, spawnPos);
}

}  // namespace EnemySpawnService
