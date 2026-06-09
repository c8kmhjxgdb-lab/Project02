#include "Game/Services/SessionService.h"

#include "Game/Ability/BondTechnique.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/Projectile.h"
#include "Game/Ability/Shield.h"
#include "Game/AI/Enemy.h"
#include "Game/Building/BuildingSystem.h"
#include "Game/Drop.h"
#include "Game/GameState.h"
#include "Game/World/RegionManager.h"
#include "Game/World/WeatherSystem.h"
#include "Engine/Renderer/ParticleSystem.h"
#include "Game/Services/WorldQuery.h"

#include <box2d/box2d.h>

namespace {

void clearTransientCombat(SessionService::RegionGameplayContext& context) {
    context.projectileManager.clear();
    context.enemyManager.clear();
    context.dropManager.clear();
    context.particleSystem.clear();
    context.shield.reset();
    context.lightning.reset();
    context.bondTechnique.getCurrentTechnique().reset();
    context.bondTechnique.setCooldown(0.0f);
}

}  // namespace

namespace SessionService {

RegionGameplayContext makeRegionGameplayContext(GameState& gs) {
    return {
        gs.regionManager,
        gs.weatherSystem,
        gs.buildingSystem,
        gs.projectileManager,
        gs.enemyManager,
        gs.dropManager,
        gs.particleSystem,
        gs.shield,
        gs.lightning,
        gs.bondTechnique
    };
}

void showNotice(GameState& gs, const std::string& notice) {
    gs.stage7Notice = notice;
    gs.stage7NoticeTimer = 4.0f;
}

void refreshWeatherRegionContext(RegionManager& regionManager,
                                 WeatherSystem& weatherSystem) {
    const MapRegion* region = regionManager.getCurrentRegion();
    if (!region) {
        weatherSystem.setRegionContext("default", false, true);
        return;
    }

    bool indoor = region->getType() == RegionType::Indoor;
    weatherSystem.setRegionContext(region->getId(), indoor, !indoor);
}

void refreshWeatherRegionContext(GameState& gs) {
    refreshWeatherRegionContext(gs.regionManager, gs.weatherSystem);
}

void clearTransientCombat(GameState& gs) {
    RegionGameplayContext context = makeRegionGameplayContext(gs);
    ::clearTransientCombat(context);
}

void refreshRegionGameplayContext(RegionGameplayContext& context) {
    refreshWeatherRegionContext(context.regionManager, context.weatherSystem);
    bool inHomeBase = WorldQuery::isCurrentRegion(context.regionManager, "home_base");
    if (const MapRegion* region = context.regionManager.getCurrentRegion()) {
        context.buildingSystem.setTileSize(region->getTileMap().tileSize);
    }
    context.buildingSystem.setPhysicsEnabled(inHomeBase);
    if (!inHomeBase) {
        context.buildingSystem.setBuildMode(false);
    }
    if (inHomeBase) {
        ::clearTransientCombat(context);
    }
}

void refreshRegionGameplayContext(GameState& gs) {
    RegionGameplayContext context = makeRegionGameplayContext(gs);
    refreshRegionGameplayContext(context);
}

bool tryUseHomeBaseDoor(GameState& gs, const glm::vec2& playerPos) {
    if (gs.regionManager.isTransitioning()) return false;

    MapRegion* region = gs.regionManager.getCurrentRegion();
    if (!region) return false;

    if (region->getId() == "starter_village" &&
        WorldQuery::isNearPOI(region, "player_home", playerPos, 1.8f)) {
        clearTransientCombat(gs);
        if (gs.regionManager.transitionTo("home_base", glm::ivec2(9, 11), gs.worldId)) {
            refreshRegionGameplayContext(gs);
        }
        return true;
    }

    if (region->getId() == "home_base" &&
        WorldQuery::isNearPOI(region, "base_exit", playerPos, 1.6f)) {
        clearTransientCombat(gs);
        if (gs.regionManager.transitionTo("starter_village", glm::ivec2(5, 7), gs.worldId)) {
            refreshRegionGameplayContext(gs);
        }
        return true;
    }

    return false;
}

}  // namespace SessionService
