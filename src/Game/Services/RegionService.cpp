#include "Game/Services/RegionService.h"

#include "Engine/Renderer/ParticleSystem.h"
#include "Game/Ability/BondTechnique.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/Projectile.h"
#include "Game/Ability/Shield.h"
#include "Game/AI/Enemy.h"
#include "Game/Building/BuildingSystem.h"
#include "Game/Drop.h"
#include "Game/GameState.h"
#include "Game/Services/AudioService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/World/RegionManager.h"
#include "Game/World/WeatherSystem.h"

#include <box2d/box2d.h>

namespace RegionService {

GameplayContext makeGameplayContext(GameState& gs) {
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
        gs.bondTechnique,
        &gs.audioSystem
    };
}

DoorContext makeDoorContext(GameState& gs) {
    return {
        makeGameplayContext(gs),
        gs.worldId
    };
}

void refreshWeatherContext(RegionManager& regionManager, WeatherSystem& weatherSystem) {
    const MapRegion* region = regionManager.getCurrentRegion();
    if (!region) {
        weatherSystem.setRegionContext("default", false, true);
        return;
    }

    bool indoor = region->getType() == RegionType::Indoor;
    weatherSystem.setRegionContext(region->getId(), indoor, !indoor);
}

void clearTransientCombat(GameplayContext& context) {
    context.projectileManager.clear();
    context.enemyManager.clear();
    context.dropManager.clear();
    context.particleSystem.clear();
    context.shield.reset();
    context.lightning.reset();
    context.bondTechnique.getCurrentTechnique().reset();
    context.bondTechnique.setCooldown(0.0f);
}

void refreshGameplayContext(GameplayContext& context) {
    refreshWeatherContext(context.regionManager, context.weatherSystem);
    bool inHomeBase = WorldQuery::isCurrentRegion(context.regionManager, "home_base");
    if (const MapRegion* region = context.regionManager.getCurrentRegion()) {
        context.buildingSystem.setTileSize(region->getTileMap().tileSize);
    }
    context.buildingSystem.setPhysicsEnabled(inHomeBase);
    if (!inHomeBase) {
        context.buildingSystem.setBuildMode(false);
    }
    if (inHomeBase) {
        clearTransientCombat(context);
    }
    if (context.audioSystem) {
        AudioService::playRegionBgm(*context.audioSystem, context.regionManager.getCurrentRegion());
    }
}

bool tryUseHomeBaseDoor(DoorContext& context, const glm::vec2& playerPos) {
    if (context.gameplay.regionManager.isTransitioning()) return false;

    MapRegion* region = context.gameplay.regionManager.getCurrentRegion();
    if (!region) return false;

    if (region->getId() == "starter_village" &&
        WorldQuery::isNearPOI(region, "player_home", playerPos, 1.8f)) {
        clearTransientCombat(context.gameplay);
        if (context.gameplay.regionManager.transitionTo(
                "home_base",
                glm::ivec2(9, 11),
                context.worldId)) {
            refreshGameplayContext(context.gameplay);
        }
        return true;
    }

    if (region->getId() == "real_street_prologue" &&
        WorldQuery::isNearPOI(region, "childhood_crack", playerPos, 1.8f)) {
        clearTransientCombat(context.gameplay);
        if (context.gameplay.regionManager.transitionTo(
                "home_base",
                glm::ivec2(12, 15),
                context.worldId)) {
            refreshGameplayContext(context.gameplay);
        }
        return true;
    }

    if (region->getId() == "home_base" &&
        WorldQuery::isNearPOI(region, "base_exit", playerPos, 1.6f)) {
        clearTransientCombat(context.gameplay);
        if (context.gameplay.regionManager.transitionTo(
                "real_street_prologue",
                glm::ivec2(18, 25),
                context.worldId)) {
            refreshGameplayContext(context.gameplay);
        }
        return true;
    }

    if (region->getId() == "home_base" &&
        WorldQuery::isNearPOI(region, "arcade_gate", playerPos, 1.6f)) {
        clearTransientCombat(context.gameplay);
        if (context.gameplay.regionManager.transitionTo(
                "popup_arcade",
                glm::ivec2(30, 56),
                context.worldId)) {
            refreshGameplayContext(context.gameplay);
        }
        return true;
    }

    if (region->getId() == "popup_arcade" &&
        WorldQuery::isNearPOI(region, "base_return_gate", playerPos, 1.8f)) {
        clearTransientCombat(context.gameplay);
        if (context.gameplay.regionManager.transitionTo(
                "home_base",
                glm::ivec2(19, 9),
                context.worldId)) {
            refreshGameplayContext(context.gameplay);
        }
        return true;
    }

    return false;
}

}  // namespace RegionService
