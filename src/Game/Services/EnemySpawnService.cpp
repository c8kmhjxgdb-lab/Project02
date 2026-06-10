#include "Game/Services/EnemySpawnService.h"

#include "Game/GameState.h"
#include "Game/Progress/StoryProgress.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/World/MapRegion.h"
#include "Utils/Math.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <cmath>

namespace EnemySpawnService {

namespace {

glm::vec2 poiPosition(const MapRegion* region, const char* poiId) {
    const PointOfInterest* poi = WorldQuery::findPOI(region, poiId);
    if (!region || !poi) {
        return glm::vec2(0.0f, 0.0f);
    }
    return region->getTileMap().tileToWorld(poi->tilePos.x, poi->tilePos.y);
}

void spawnPopupArcadeFixedEnemies(Context& context) {
    if (context.storyProgress.getFlag("popup_arcade_spawns_created")) {
        return;
    }

    const MapRegion* region = context.regionManager.getCurrentRegion();
    auto spawn = [&context](const glm::vec2& pos, const char* enemyDefId) {
        context.enemyManager.spawnByDefinition(context.combatSpawn.worldId, pos, enemyDefId);
    };

    spawn(glm::vec2(30.0f, 50.0f), "popup_bubble");
    spawn(poiPosition(region, "trial_token_1") + glm::vec2(1.5f, 0.0f), "popup_bubble");
    spawn(poiPosition(region, "trial_token_2") + glm::vec2(-1.5f, 0.0f), "payment_button");
    spawn(glm::vec2(32.0f, 40.0f), "payment_button");
    spawn(glm::vec2(30.0f, 35.0f), "close_x_bug");
    spawn(poiPosition(region, "tieyi_cage") + glm::vec2(-2.0f, 0.0f), "scrap_soldier");
    spawn(poiPosition(region, "tieyi_cage") + glm::vec2(2.0f, 0.0f), "scrap_soldier_elite");

    context.storyProgress.setFlag("popup_arcade_spawns_created", true);
    context.enemySpawnTimer = 0.0f;
}

}  // namespace

Context makeContext(GameState& gs) {
    return {
        gs.regionManager,
        gs.enemyManager,
        gs.playerBodyId,
        gs.enemySpawnTimer,
        gs.enemySpawnInterval,
        gs.maxEnemies,
        gs.charTime,
        CombatService::makeSpawnContext(gs),
        gs.storyProgress
    };
}

void update(Context& context, float dt) {
    bool inHomeBase = WorldQuery::isCurrentRegion(context.regionManager, "home_base");
    bool inPopupArcade = WorldQuery::isCurrentRegion(context.regionManager, "popup_arcade");
    context.enemySpawnTimer += dt;
    if (inHomeBase) {
        context.enemySpawnTimer = 0.0f;
        return;
    }
    if (inPopupArcade) {
        spawnPopupArcadeFixedEnemies(context);
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
