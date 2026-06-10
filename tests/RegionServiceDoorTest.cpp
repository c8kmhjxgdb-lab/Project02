#include "Engine/Renderer/ParticleSystem.h"
#include "Game/AI/Enemy.h"
#include "Game/Ability/BondTechnique.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/Projectile.h"
#include "Game/Ability/Shield.h"
#include "Game/Building/BuildingSystem.h"
#include "Game/Drop.h"
#include "Game/Services/RegionService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/World/RegionManager.h"
#include "Game/World/WeatherSystem.h"
#include "TestSupport.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>

namespace {

struct Fixture {
    b2WorldId worldId = b2_nullWorldId;
    b2BodyId playerBodyId = b2_nullBodyId;
    RegionManager regionManager;
    WeatherSystem weatherSystem;
    BuildingSystem buildingSystem;
    ProjectileManager projectileManager;
    EnemyManager enemyManager;
    DropManager dropManager;
    ParticleSystem particleSystem{128};
    Shield shield;
    Lightning lightning;
    BondTechniqueSystem bondTechnique;

    Fixture() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, 0.0f};
        worldId = b2CreateWorld(&worldDef);

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = b2Vec2{0.0f, 0.0f};
        playerBodyId = b2CreateBody(worldId, &bodyDef);

        regionManager.setWorldId(worldId);
        regionManager.setPlayerBody(playerBodyId);
        regionManager.setTransitionEffectEnabled(false);
        regionManager.init();
        if (MapRegion* region = regionManager.getCurrentRegion()) {
            region->buildPhysics(worldId);
        }

        buildingSystem.init(worldId);
        projectileManager.init();
        enemyManager.init();
        dropManager.init();
        particleSystem.init();
    }

    ~Fixture() {
        regionManager.shutdown();
        buildingSystem.shutdown();
        projectileManager.clear();
        enemyManager.clear();
        dropManager.clear();
        if (b2World_IsValid(worldId)) {
            b2DestroyWorld(worldId);
        }
    }

    RegionService::GameplayContext makeGameplayContext() {
        return {
            regionManager,
            weatherSystem,
            buildingSystem,
            projectileManager,
            enemyManager,
            dropManager,
            particleSystem,
            shield,
            lightning,
            bondTechnique,
            nullptr
        };
    }

    RegionService::DoorContext makeDoorContext() {
        return {
            makeGameplayContext(),
            worldId
        };
    }
};

glm::vec2 poiWorldPosition(const RegionManager& regionManager, const char* poiId) {
    const MapRegion* region = regionManager.getCurrentRegion();
    TestSupport::require(region != nullptr, "current region exists");
    const PointOfInterest* poi = WorldQuery::findPOI(region, poiId);
    TestSupport::require(poi != nullptr, "poi exists");
    return region->getTileMap().tileToWorld(poi->tilePos.x, poi->tilePos.y);
}

void enteringHomeBaseSwitchesRegionAndRefreshesContext() {
    Fixture fixture;

    RegionService::DoorContext doorContext = fixture.makeDoorContext();
    bool usedDoor = RegionService::tryUseHomeBaseDoor(
        doorContext,
        poiWorldPosition(fixture.regionManager, "player_home"));

    TestSupport::require(usedDoor, "starter village home door is used");
    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "home_base"),
        "door switches current region to home base");
    TestSupport::require(
        fixture.buildingSystem.isBuildableHere("home_base"),
        "home base remains buildable after region refresh");
    TestSupport::require(
        fixture.weatherSystem.isIndoorContext(),
        "home base sets indoor weather context");
    TestSupport::require(
        !fixture.weatherSystem.shouldEmitParticles(),
        "home base disables weather particles");
    TestSupport::require(
        fixture.regionManager.hasRegion("starter_village"),
        "starter village is cached after entering home base");
}

void leavingHomeBaseReturnsToPrologueAndTeleportsPlayer() {
    Fixture fixture;

    RegionService::DoorContext enterContext = fixture.makeDoorContext();
    RegionService::tryUseHomeBaseDoor(
        enterContext,
        poiWorldPosition(fixture.regionManager, "player_home"));

    RegionService::DoorContext exitContext = fixture.makeDoorContext();
    bool usedExit = RegionService::tryUseHomeBaseDoor(
        exitContext,
        poiWorldPosition(fixture.regionManager, "base_exit"));

    TestSupport::require(usedExit, "home base exit door is used");
    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "real_street_prologue"),
        "exit switches current region to prologue street");
    TestSupport::require(
        fixture.regionManager.hasRegion("home_base"),
        "home base is cached after leaving");

    const MapRegion* region = fixture.regionManager.getCurrentRegion();
    glm::vec2 expected = region->getTileMap().tileToWorld(18, 25);
    b2Vec2 playerPos = b2Body_GetPosition(fixture.playerBodyId);
    TestSupport::require(
        glm::length(glm::vec2(playerPos.x, playerPos.y) - expected) < 0.001f,
        "exit teleports player to prologue crack tile");
}

void homeBaseArcadeGateRoundTripsToPopupArcade() {
    Fixture fixture;

    RegionService::DoorContext enterContext = fixture.makeDoorContext();
    RegionService::tryUseHomeBaseDoor(
        enterContext,
        poiWorldPosition(fixture.regionManager, "player_home"));

    RegionService::DoorContext arcadeContext = fixture.makeDoorContext();
    bool usedArcadeGate = RegionService::tryUseHomeBaseDoor(
        arcadeContext,
        poiWorldPosition(fixture.regionManager, "arcade_gate"));

    TestSupport::require(usedArcadeGate, "home base arcade gate is used");
    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "popup_arcade"),
        "arcade gate switches current region to popup arcade");

    RegionService::DoorContext returnContext = fixture.makeDoorContext();
    bool usedReturnGate = RegionService::tryUseHomeBaseDoor(
        returnContext,
        poiWorldPosition(fixture.regionManager, "base_return_gate"));

    TestSupport::require(usedReturnGate, "popup arcade return gate is used");
    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "home_base"),
        "return gate switches current region to home base");
}

void enteringHomeBaseClearsTransientCombat() {
    Fixture fixture;

    fixture.projectileManager.fire(
        fixture.worldId,
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        ProjectileType::Fireball,
        25.0f,
        18.0f,
        fixture.playerBodyId);
    fixture.enemyManager.spawn(fixture.worldId, glm::vec2(2.0f, 0.0f), EnemyType::Chaser);
    fixture.dropManager.spawn(fixture.worldId, glm::vec2(1.0f, 1.0f), DropType::Coin, 3);
    fixture.particleSystem.emit(
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        1.0f,
        0.1f);
    fixture.shield.activate();
    fixture.lightning.begin(glm::vec2(0.0f, 0.0f));
    fixture.bondTechnique.activate(glm::vec2(0.0f, 0.0f));
    fixture.bondTechnique.setCooldown(5.0f);

    RegionService::DoorContext doorContext = fixture.makeDoorContext();
    RegionService::tryUseHomeBaseDoor(
        doorContext,
        poiWorldPosition(fixture.regionManager, "player_home"));

    TestSupport::require(fixture.projectileManager.getActive().empty(), "home entry clears projectiles");
    TestSupport::require(fixture.enemyManager.getActive().empty(), "home entry clears enemies");
    TestSupport::require(fixture.dropManager.getActive().empty(), "home entry clears drops");
    TestSupport::require(fixture.particleSystem.getActiveCount() == 0, "home entry clears particles");
    TestSupport::require(!fixture.shield.isActive(), "home entry resets shield");
    TestSupport::require(!fixture.lightning.isActive(), "home entry resets lightning");
    TestSupport::require(!fixture.bondTechnique.isActive(), "home entry resets bond technique");
    TestSupport::require(fixture.bondTechnique.getCooldown() == 0.0f, "home entry resets bond cooldown");
}

}  // namespace

int main() {
    enteringHomeBaseSwitchesRegionAndRefreshesContext();
    leavingHomeBaseReturnsToPrologueAndTeleportsPlayer();
    homeBaseArcadeGateRoundTripsToPopupArcade();
    enteringHomeBaseClearsTransientCombat();
    return 0;
}
