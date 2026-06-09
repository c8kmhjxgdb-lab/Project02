#include "Game/Services/SessionService.h"

#include "Game/Services/WorldQuery.h"

#include <box2d/box2d.h>

namespace SessionService {

void showNotice(GameState& gs, const std::string& notice) {
    gs.stage7Notice = notice;
    gs.stage7NoticeTimer = 4.0f;
}

void refreshWeatherRegionContext(GameState& gs) {
    const MapRegion* region = gs.regionManager.getCurrentRegion();
    if (!region) {
        gs.weatherSystem.setRegionContext("default", false, true);
        return;
    }

    bool indoor = region->getType() == RegionType::Indoor;
    gs.weatherSystem.setRegionContext(region->getId(), indoor, !indoor);
}

void clearTransientCombat(GameState& gs) {
    gs.projectileManager.clear();
    gs.enemyManager.clear();
    gs.dropManager.clear();
    gs.particleSystem.clear();
    gs.shield.reset();
    gs.lightning.reset();
    gs.bondTechnique.getCurrentTechnique().reset();
    gs.bondTechnique.setCooldown(0.0f);
}

void refreshRegionGameplayContext(GameState& gs) {
    refreshWeatherRegionContext(gs);
    bool inHomeBase = WorldQuery::isCurrentRegion(gs.regionManager, "home_base");
    if (const MapRegion* region = gs.regionManager.getCurrentRegion()) {
        gs.buildingSystem.setTileSize(region->getTileMap().tileSize);
    }
    gs.buildingSystem.setPhysicsEnabled(inHomeBase);
    if (!inHomeBase) {
        gs.buildingSystem.setBuildMode(false);
    }
    if (inHomeBase) {
        clearTransientCombat(gs);
    }
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
