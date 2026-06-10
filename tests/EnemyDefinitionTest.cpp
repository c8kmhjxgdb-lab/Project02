#include "Game/AI/EnemyDefinition.h"
#include "Game/GameState.h"
#include "Game/Services/EnemySpawnService.h"
#include "Game/Services/WorldQuery.h"
#include "TestSupport.h"

#include <box2d/box2d.h>

namespace {

struct PhysicsFixture {
    b2WorldId worldId = b2_nullWorldId;

    PhysicsFixture() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, 0.0f};
        worldId = b2CreateWorld(&worldDef);
    }

    ~PhysicsFixture() {
        if (b2World_IsValid(worldId)) {
            b2DestroyWorld(worldId);
        }
    }
};

void definitionsExposeFirstChapterEnemies() {
    const EnemyDef* bubble = EnemyDefinitions::find("popup_bubble");
    const EnemyDef* button = EnemyDefinitions::find("payment_button");
    const EnemyDef* closeX = EnemyDefinitions::find("close_x_bug");
    const EnemyDef* scrap = EnemyDefinitions::find("scrap_soldier_elite");

    TestSupport::require(bubble != nullptr, "popup bubble exists");
    TestSupport::require(bubble->baseType == EnemyType::Chaser, "popup bubble is chaser");
    TestSupport::require(bubble->special == EnemySpecialEffect::PopupCover, "popup bubble covers");
    TestSupport::require(button != nullptr && button->baseType == EnemyType::Shooter, "payment button is shooter");
    TestSupport::require(closeX != nullptr && closeX->baseType == EnemyType::Exploder, "close x is exploder");
    TestSupport::require(scrap != nullptr && scrap->maxHealth > bubble->maxHealth, "elite is tougher");
}

void spawnByDefinitionAppliesDefinitionStats() {
    PhysicsFixture fixture;
    EnemyManager enemyManager;
    enemyManager.init();

    EnemyId enemyId = enemyManager.spawnByDefinition(
        fixture.worldId,
        glm::vec2(1.0f, 2.0f),
        "scrap_soldier_elite");

    const EnemyDef* def = EnemyDefinitions::find("scrap_soldier_elite");
    const Enemy* enemy = enemyManager.find(enemyId);
    TestSupport::require(def != nullptr, "elite definition exists for spawn check");
    TestSupport::require(enemy != nullptr, "spawn by definition creates enemy");
    TestSupport::require(enemy->definitionId == "scrap_soldier_elite", "enemy stores definition id");
    TestSupport::require(enemy->type == def->baseType, "enemy uses definition base type");
    TestSupport::require(enemy->maxHealth == def->maxHealth, "enemy uses definition max health");
    TestSupport::require(enemy->health == def->maxHealth, "enemy current health matches definition");
    TestSupport::require(enemy->damage == def->damage, "enemy uses definition damage");
    TestSupport::require(enemy->speed == def->speed, "enemy uses definition speed");
    TestSupport::require(enemy->radius == def->radius, "enemy uses definition radius");
    TestSupport::require(enemy->coinDropMin == def->coinDropMin, "enemy uses definition minimum coin drop");
    TestSupport::require(enemy->coinDropMax == def->coinDropMax, "enemy uses definition maximum coin drop");
    TestSupport::require(
        enemy->specialEffect == static_cast<uint8_t>(def->special),
        "enemy stores special effect");
}

void popupArcadeCreatesFixedSpawnsOnce() {
    GameState gs{};

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{0.0f, 0.0f};
    gs.worldId = b2CreateWorld(&worldDef);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{30.0f, 56.0f};
    gs.playerBodyId = b2CreateBody(gs.worldId, &bodyDef);

    gs.regionManager.setWorldId(gs.worldId);
    gs.regionManager.setPlayerBody(gs.playerBodyId);
    gs.regionManager.setTransitionEffectEnabled(false);
    gs.regionManager.init();
    gs.regionManager.transitionTo("popup_arcade", glm::ivec2(30, 56), gs.worldId);

    gs.enemyManager.init();
    gs.enemySpawnTimer = 99.0f;
    gs.enemySpawnInterval = 5.0f;
    gs.maxEnemies = 12;

    EnemySpawnService::Context context = EnemySpawnService::makeContext(gs);
    EnemySpawnService::update(context, 0.1f);

    TestSupport::require(
        WorldQuery::isCurrentRegion(gs.regionManager, "popup_arcade"),
        "fixture is in popup arcade");
    TestSupport::require(
        gs.storyProgress.getFlag("popup_arcade_spawns_created"),
        "fixed spawn flag is set");
    TestSupport::require(
        gs.enemyManager.getAlive().size() == 7,
        "popup arcade creates deterministic first chapter enemies");

    int bubbleCount = 0;
    int buttonCount = 0;
    int closeXCount = 0;
    int scrapCount = 0;
    int eliteCount = 0;
    for (const Enemy* enemy : gs.enemyManager.getAlive()) {
        if (enemy->definitionId == "popup_bubble") ++bubbleCount;
        if (enemy->definitionId == "payment_button") ++buttonCount;
        if (enemy->definitionId == "close_x_bug") ++closeXCount;
        if (enemy->definitionId == "scrap_soldier") ++scrapCount;
        if (enemy->definitionId == "scrap_soldier_elite") ++eliteCount;
    }
    TestSupport::require(bubbleCount == 2, "popup arcade spawns two popup bubbles");
    TestSupport::require(buttonCount == 2, "popup arcade spawns two payment buttons");
    TestSupport::require(closeXCount == 1, "popup arcade spawns one close x bug");
    TestSupport::require(scrapCount == 1, "popup arcade spawns one scrap soldier");
    TestSupport::require(eliteCount == 1, "popup arcade spawns one scrap elite");

    EnemySpawnService::update(context, 99.0f);
    TestSupport::require(
        gs.enemyManager.getAlive().size() == 7,
        "fixed popup arcade spawns do not repeat");

    gs.enemyManager.clear();
    gs.regionManager.shutdown();
    if (b2World_IsValid(gs.worldId)) {
        b2DestroyWorld(gs.worldId);
    }
}

}  // namespace

int main() {
    definitionsExposeFirstChapterEnemies();
    spawnByDefinitionAppliesDefinitionStats();
    popupArcadeCreatesFixedSpawnsOnce();
    return 0;
}
