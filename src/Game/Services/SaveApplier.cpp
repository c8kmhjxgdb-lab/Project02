#include "Game/Services/SaveApplier.h"

#include "Game/GameState.h"
#include "Game/Services/NoticeService.h"
#include "Game/Services/RegionService.h"

#include <algorithm>
#include <box2d/box2d.h>

namespace {

void applySavedRegions(GameState& gs, const SaveData& saveData) {
    for (const SaveData::RegionData& savedRegion : saveData.regions) {
        if (savedRegion.id.empty()) continue;
        gs.regionManager.loadRegion(savedRegion.id);
        MapRegion* region = gs.regionManager.getRegion(savedRegion.id);
        if (!region) continue;

        region->getTileManager().replayModifications(savedRegion.modifications);
        if (!savedRegion.decorModifications.empty()) {
            region->setDecorations(savedRegion.decorModifications);
        }
        if (!savedRegion.pois.empty()) {
            region->setPOIs(savedRegion.pois);
        }
    }
}

}  // namespace

namespace SaveApplier {

void resetSessionState(GameState& gs) {
    RegionService::GameplayContext regionGameplay = RegionService::makeGameplayContext(gs);
    RegionService::clearTransientCombat(regionGameplay);
    gs.input.clear();
    gs.buildingSystem.setBuildMode(false);
    gs.toySystem.stopMiniCar();
    gs.dialogueTree.end();
    gs.dialogueUI.hide();
    gs.isVenting = false;
    gs.ui.talkedWithPrincessAtBaseThisFrame = false;
    gs.ui.stage7Notice.clear();
    gs.ui.stage7NoticeTimer = 0.0f;
    gs.isDead = false;
    gs.deathTimer = 0.0f;
    gs.isFlying = false;
    gs.flightHeight = 0.0f;
    gs.flightHeightTarget = 0.0f;
    gs.flightCooldown = 0.0f;
    gs.shieldCooldown = 0.0f;
    gs.fireballCooldown = 0.0f;
    gs.enemySpawnTimer = 0.0f;
    gs.score = 0;
    gs.enemiesKilled = 0;
}

bool applySaveData(GameState& gs, const SaveData& saveData) {
    resetSessionState(gs);
    gs.regionManager.resetLoadedRegions();

    for (const auto& regionId : saveData.player.progress.discoveredRegions) {
        if (!regionId.empty()) {
            gs.regionManager.loadRegion(regionId);
        }
    }
    for (const SaveData::RegionData& region : saveData.regions) {
        if (!region.id.empty()) {
            gs.regionManager.loadRegion(region.id);
        }
    }

    applySavedRegions(gs, saveData);

    std::string savedRegionId = saveData.player.regionId.empty()
        ? std::string("starter_village")
        : saveData.player.regionId;
    bool previousTransitionEffect = gs.regionManager.isTransitionEffectEnabled();
    gs.regionManager.setTransitionEffectEnabled(false);
    MapRegion* currentSaveRegion = gs.regionManager.getCurrentRegion();
    if (!currentSaveRegion || currentSaveRegion->getId() != savedRegionId) {
        gs.regionManager.transitionTo(savedRegionId, glm::ivec2(0, 0), gs.worldId);
    }
    gs.regionManager.setTransitionEffectEnabled(previousTransitionEffect);

    b2Body_SetTransform(gs.playerBodyId,
        b2Vec2{saveData.player.position.x, saveData.player.position.y},
        b2Rot{0});
    b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);
    gs.playerHealth.restore(saveData.player.health, saveData.player.maxHealth);
    gs.playerMaxMana = std::max(saveData.player.maxMana, 1.0f);
    gs.playerMana = std::clamp(saveData.player.mana, 0.0f, gs.playerMaxMana);
    gs.inventory.setCoins(saveData.player.coins);
    gs.inventory.loadFurnitureStock(saveData.furnitureStock);
    gs.inventory.loadItemStacks(saveData.itemStacks);
    gs.inventory.loadUnlockedFurniture(saveData.unlockedFurniture);
    gs.storyProgress.loadSnapshot(saveData.storyProgress);

    gs.camera.position = saveData.player.position;
    gs.spawnPoint = saveData.player.position;

    gs.emotionSystem.setChildlikeHeart(saveData.childlikeHeart);
    gs.emotionSystem.setGrievance(saveData.grievance);
    gs.emotionSystem.setJoy(saveData.joy);
    gs.emotionSystem.setStress(saveData.stress);
    gs.timeSystem.setDay(saveData.environment.day);
    gs.timeSystem.setHour(saveData.environment.hour);
    gs.gameTime = gs.timeSystem.getHour();
    gs.weatherSystem.setWeatherImmediate(
        WeatherSystem::weatherFromId(saveData.environment.weather),
        saveData.environment.weatherIntensity);

    gs.buildingSystem.loadInstances(saveData.homeFurniture);
    gs.toySystem.loadSaveData(saveData.toyData);
    if (!saveData.quests.empty()) {
        gs.questSystem.loadSaveData(saveData.quests);
    } else {
        gs.questSystem.loadCompletedQuests(saveData.player.progress.completedQuests);
    }

    if (gs.princess) {
        gs.princess->affection = std::clamp(saveData.princess.affection, 0.0f, 1000.0f);
        gs.princess->setFollowing(saveData.princess.following);
        gs.princess->ultimateCharge = std::clamp(saveData.princess.ultimateCharge, 0.0f, 100.0f);
        if (gs.princess->hasBody()) {
            b2Body_SetTransform(gs.princess->getBodyId(),
                b2Vec2{saveData.princess.position.x, saveData.princess.position.y},
                b2Rot{0});
            b2Body_SetLinearVelocity(gs.princess->getBodyId(), b2Vec2_zero);
        }
    }

    gs.totalPlayTimeSeconds = std::max(0.0f, saveData.player.progress.totalPlayTime);
    RegionService::GameplayContext regionGameplay = RegionService::makeGameplayContext(gs);
    RegionService::refreshGameplayContext(regionGameplay);
    MapRegion* region = gs.regionManager.getCurrentRegion();
    if (region) {
        gs.miniMap.setMapDimensions(region->getWidth(), region->getHeight(),
            region->getTileMap().tileSize);
        gs.miniMap.forceUpdate(saveData.player.position);
    }
    NoticeService::Context noticeContext = NoticeService::makeContext(gs);
    NoticeService::showNotice(noticeContext, "读档完成 Loaded");
    return true;
}

}  // namespace SaveApplier
