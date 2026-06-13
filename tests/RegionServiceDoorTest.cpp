#include "Engine/Renderer/ParticleSystem.h"
#include "Engine/Renderer/MiniMap.h"
#include "Engine/Camera/Camera2D.h"
#include "Game/AI/Enemy.h"
#include "Game/Ability/BondTechnique.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/Projectile.h"
#include "Game/Ability/Shield.h"
#include "Game/Building/BuildingSystem.h"
#include "Game/Drop.h"
#include "Game/Services/RegionUpdateService.h"
#include "Game/Services/RegionService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/World/RegionManager.h"
#include "Game/World/TimeSystem.h"
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

glm::vec2 playerWorldPosition(b2BodyId playerBodyId) {
    b2Vec2 playerPos = b2Body_GetPosition(playerBodyId);
    return glm::vec2(playerPos.x, playerPos.y);
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

    MapRegion* region = fixture.regionManager.getCurrentRegion();
    glm::vec2 playerPos = playerWorldPosition(fixture.playerBodyId);
    TestSupport::require(
        region &&
            !WorldQuery::isNearPOI(
                region,
                "childhood_crack",
                playerPos,
                1.8f),
        "base exit landing tile is outside the manual prologue crack range");
    TestSupport::require(
        region && region->getConnectionAt(playerPos, 1.5f) == nullptr,
        "base exit landing tile is outside the automatic prologue crack range");
}

void enteringHomeBaseFromPrologueLandsOutsideBaseExit() {
    Fixture fixture;
    fixture.regionManager.transitionTo("real_street_prologue", glm::ivec2(8, 12), fixture.worldId);

    RegionService::DoorContext doorContext = fixture.makeDoorContext();
    bool usedCrack = RegionService::tryUseHomeBaseDoor(
        doorContext,
        poiWorldPosition(fixture.regionManager, "childhood_crack"));

    TestSupport::require(usedCrack, "prologue crack door is used");
    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "home_base"),
        "prologue crack switches current region to home base");

    glm::vec2 homeEntryPos = playerWorldPosition(fixture.playerBodyId);
    MapRegion* homeRegion = fixture.regionManager.getCurrentRegion();
    TestSupport::require(
        homeRegion &&
            !WorldQuery::isNearPOI(
                homeRegion,
                "base_exit",
                homeEntryPos,
                1.6f),
        "prologue crack landing tile is outside the manual base exit range");
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
    b2Vec2 arcadeEntryPos = b2Body_GetPosition(fixture.playerBodyId);
    MapRegion* arcadeRegion = fixture.regionManager.getCurrentRegion();
    TestSupport::require(
        arcadeRegion &&
            !WorldQuery::isNearPOI(
                arcadeRegion,
                "base_return_gate",
                glm::vec2(arcadeEntryPos.x, arcadeEntryPos.y),
                1.8f),
        "arcade entry landing tile is outside the manual return gate range");

    RegionService::DoorContext returnContext = fixture.makeDoorContext();
    bool usedReturnGate = RegionService::tryUseHomeBaseDoor(
        returnContext,
        poiWorldPosition(fixture.regionManager, "base_return_gate"));

    TestSupport::require(usedReturnGate, "popup arcade return gate is used");
    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "home_base"),
        "return gate switches current region to home base");
    glm::vec2 homeReturnPos = playerWorldPosition(fixture.playerBodyId);
    MapRegion* homeRegion = fixture.regionManager.getCurrentRegion();
    TestSupport::require(
        homeRegion &&
            !WorldQuery::isNearPOI(
                homeRegion,
                "arcade_gate",
                homeReturnPos,
                1.6f),
        "arcade return landing tile is outside the manual arcade gate range");
    TestSupport::require(
        homeRegion && homeRegion->getConnectionAt(homeReturnPos, 1.5f) == nullptr,
        "arcade return landing tile is outside the automatic arcade portal range");
}

void cachedRegionTransitionStartsCooldown() {
    Fixture fixture;

    fixture.regionManager.transitionTo("home_base", glm::ivec2(9, 11), fixture.worldId);
    fixture.regionManager.update(1.0f);
    fixture.regionManager.transitionTo("popup_arcade", glm::ivec2(30, 54), fixture.worldId);
    fixture.regionManager.update(1.0f);

    TestSupport::require(fixture.regionManager.hasRegion("home_base"), "home base is cached before return");
    bool returnedToCachedBase = fixture.regionManager.transitionTo(
        "home_base",
        glm::ivec2(17, 9),
        fixture.worldId);

    TestSupport::require(returnedToCachedBase, "cached home base transition completes immediately");
    TestSupport::require(
        fixture.regionManager.isOnCooldown(),
        "cached region transition starts portal cooldown");
}

void cachedRegionsReleasePhysicsBodiesUntilRestored() {
    Fixture fixture;

    fixture.regionManager.transitionTo("home_base", glm::ivec2(9, 11), fixture.worldId);
    fixture.regionManager.transitionTo("popup_arcade", glm::ivec2(30, 54), fixture.worldId);

    MapRegion* cachedHomeBase = fixture.regionManager.getRegion("home_base");
    TestSupport::require(cachedHomeBase != nullptr, "home base is cached after entering arcade");
    TestSupport::require(
        !b2Body_IsValid(cachedHomeBase->getTileManager().getBodyAt(7, 2)),
        "cached home base releases inactive tile physics bodies");

    fixture.regionManager.transitionTo("home_base", glm::ivec2(17, 9), fixture.worldId);
    MapRegion* restoredHomeBase = fixture.regionManager.getCurrentRegion();
    TestSupport::require(restoredHomeBase != nullptr, "home base is restored from cache");
    TestSupport::require(
        b2Body_IsValid(restoredHomeBase->getTileManager().getBodyAt(7, 2)),
        "restored home base rebuilds active tile physics bodies");
}

void cachedConnectionTransitionKeepsCopiedEntryTile() {
    Fixture fixture;

    fixture.regionManager.transitionTo("home_base", glm::ivec2(9, 11), fixture.worldId);
    fixture.regionManager.update(1.0f);
    TestSupport::require(fixture.regionManager.loadRegion("popup_arcade"),
                         "popup arcade is cached before connection transition");
    TestSupport::require(fixture.regionManager.loadRegion("dark_forest"),
                         "dark forest is cached before connection transition");
    TestSupport::require(fixture.regionManager.loadRegion("mountain_pass"),
                         "mountain pass is cached before connection transition");
    TestSupport::require(fixture.regionManager.loadRegion("coastal_town"),
                         "coastal town is cached before connection transition");
    TestSupport::require(fixture.regionManager.loadRegion("cached_extra_a"),
                         "extra region A is cached before connection transition");
    TestSupport::require(fixture.regionManager.loadRegion("cached_extra_b"),
                         "extra region B is cached before connection transition");

    MapRegion* baseRegion = fixture.regionManager.getCurrentRegion();
    TestSupport::require(baseRegion != nullptr, "home base exists before cached connection transition");
    MapConnection* arcadeConnection = baseRegion->getConnectionAt(
        baseRegion->getTileMap().tileToWorld(20, 9),
        1.5f);
    TestSupport::require(arcadeConnection != nullptr, "cached arcade connection exists");

    bool completedImmediately = fixture.regionManager.transitionTo(*arcadeConnection, fixture.worldId);
    TestSupport::require(completedImmediately, "cached connection transition completes immediately");
    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "popup_arcade"),
        "cached connection transition enters popup arcade");
    MapRegion* arcadeRegion = fixture.regionManager.getCurrentRegion();
    glm::vec2 expected = arcadeRegion->getTileMap().tileToWorld(30, 54);
    TestSupport::require(
        glm::distance(playerWorldPosition(fixture.playerBodyId), expected) < 0.01f,
        "cached connection transition lands on copied arcade entry tile");
}

void automaticArcadePortalCompletesFadeTransition() {
    Fixture fixture;

    fixture.regionManager.transitionTo("home_base", glm::ivec2(9, 11), fixture.worldId);
    fixture.regionManager.update(1.0f);
    fixture.regionManager.setTransitionEffectEnabled(true);
    fixture.regionManager.setTransitionDuration(0.2f);

    MapRegion* baseRegion = fixture.regionManager.getCurrentRegion();
    TestSupport::require(baseRegion != nullptr, "home base exists before automatic portal");
    MapConnection* arcadeConnection = baseRegion->getConnectionAt(
        baseRegion->getTileMap().tileToWorld(20, 9),
        1.5f);
    TestSupport::require(arcadeConnection != nullptr, "automatic arcade portal connection exists");

    bool completedImmediately = fixture.regionManager.transitionTo(*arcadeConnection, fixture.worldId);
    TestSupport::require(!completedImmediately, "automatic arcade portal starts fade transition");

    for (int i = 0; i < 8; ++i) {
        fixture.regionManager.update(0.1f);
    }

    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "popup_arcade"),
        "automatic arcade portal completes in popup arcade");
    MapRegion* arcadeRegion = fixture.regionManager.getCurrentRegion();
    glm::vec2 playerPos = playerWorldPosition(fixture.playerBodyId);
    TestSupport::require(
        arcadeRegion &&
            !WorldQuery::isNearPOI(
                arcadeRegion,
                "base_return_gate",
                playerPos,
                1.8f),
        "automatic arcade portal landing avoids return gate interaction range");
    TestSupport::require(
        arcadeRegion && arcadeRegion->getConnectionAt(playerPos, 1.5f) == nullptr,
        "automatic arcade portal landing avoids return portal auto range");
}

void regionUpdateServiceMovesPlayerThroughAutomaticArcadePortal() {
    Fixture fixture;
    TimeSystem timeSystem;
    Camera2D camera;
    MiniMap miniMap;
    float gameTime = 10.0f;
    RegionUpdateService::State regionState;

    timeSystem.init(10.0f);
    fixture.regionManager.transitionTo("home_base", glm::ivec2(9, 11), fixture.worldId);
    fixture.regionManager.update(1.0f);
    fixture.regionManager.setTransitionEffectEnabled(true);
    fixture.regionManager.setTransitionDuration(0.2f);

    MapRegion* baseRegion = fixture.regionManager.getCurrentRegion();
    TestSupport::require(baseRegion != nullptr, "home base exists before region update portal");
    glm::vec2 portalPos = baseRegion->getTileMap().tileToWorld(20, 9);
    b2Body_SetTransform(fixture.playerBodyId, b2Vec2{portalPos.x, portalPos.y}, b2Rot{0});

    RegionUpdateService::Context context{
        timeSystem,
        fixture.weatherSystem,
        camera,
        fixture.regionManager,
        miniMap,
        gameTime,
        fixture.worldId,
        fixture.playerBodyId,
        fixture.makeGameplayContext()
    };

    for (int i = 0; i < 8; ++i) {
        glm::vec2 playerPos = playerWorldPosition(fixture.playerBodyId);
        RegionUpdateService::update(
            context,
            0.1f,
            playerPos,
            fixture.regionManager.getCurrentRegion(),
            regionState);
    }

    TestSupport::require(
        WorldQuery::isCurrentRegion(fixture.regionManager, "popup_arcade"),
        "region update service sends automatic arcade portal to popup arcade");
    TestSupport::require(
        !fixture.regionManager.isTransitioning(),
        "region update service completes arcade portal transition");
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
    enteringHomeBaseFromPrologueLandsOutsideBaseExit();
    homeBaseArcadeGateRoundTripsToPopupArcade();
    cachedRegionTransitionStartsCooldown();
    cachedRegionsReleasePhysicsBodiesUntilRestored();
    cachedConnectionTransitionKeepsCopiedEntryTile();
    automaticArcadePortalCompletesFadeTransition();
    regionUpdateServiceMovesPlayerThroughAutomaticArcadePortal();
    enteringHomeBaseClearsTransientCombat();
    return 0;
}
