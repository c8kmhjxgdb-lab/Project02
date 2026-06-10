#pragma once

#include "Game/Services/CombatService.h"

class EnemyManager;
class RegionManager;
class StoryProgress;
struct GameState;

namespace EnemySpawnService {

struct Context {
    RegionManager& regionManager;
    EnemyManager& enemyManager;
    b2BodyId playerBodyId;
    float& enemySpawnTimer;
    float& enemySpawnInterval;
    int& maxEnemies;
    float& charTime;
    CombatService::SpawnContext combatSpawn;
    StoryProgress& storyProgress;
};

Context makeContext(GameState& gs);

void update(Context& context, float dt);

}  // namespace EnemySpawnService
