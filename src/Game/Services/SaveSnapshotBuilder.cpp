#include "Game/Services/SaveSnapshotBuilder.h"

#include "Game/Data/SaveMigration.h"
#include "Game/Data/SaveRepository.h"
#include "Game/Data/SaveSerializer.h"
#include "Game/GameState.h"

#include <box2d/box2d.h>

namespace {

SaveData::PrincessData collectPrincessSaveData(const GameState& gs) {
    SaveData::PrincessData data;
    if (!gs.princess) return data;

    data.name = gs.princess->getName();
    data.position = gs.princess->getPosition();
    data.affection = gs.princess->getAffection();
    data.following = gs.princess->isFollowing();
    data.ultimateCharge = gs.princess->ultimateCharge;
    data.hasBody = gs.princess->hasBody();
    return data;
}

PlayerProgress collectPlayerProgress(const GameState& gs) {
    PlayerProgress progress;
    progress.discoveredRegions = gs.regionManager.getDiscoveredRegions();
    progress.completedQuests = gs.questSystem.getCompletedQuests();
    progress.collectedItems = gs.toySystem.getCollectedToys();
    progress.totalPlayTime = gs.totalPlayTimeSeconds;
    progress.maxHealth = static_cast<int>(gs.playerHealth.getMaxHealth());
    progress.maxMana = static_cast<int>(gs.playerMaxMana);
    return progress;
}

}  // namespace

namespace SaveSnapshotBuilder {

bool saveCurrentGame(GameState& gs, const std::string& slot) {
    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
    const EmotionState& emotion = gs.emotionSystem.getState();
    MapRegion* saveRegion = gs.regionManager.getCurrentRegion();

    SaveData data;
    data.version = SaveMigration::kCurrentVersion;
    data.timestamp = SaveSerializer::currentTimestamp();
    data.player.regionId = saveRegion ? saveRegion->getId() : std::string("starter_village");
    data.player.position = glm::vec2(pPos.x, pPos.y);
    data.player.health = gs.playerHealth.getCurrentHealth();
    data.player.maxHealth = gs.playerHealth.getMaxHealth();
    data.player.mana = gs.playerMana;
    data.player.maxMana = gs.playerMaxMana;
    data.player.coins = gs.inventory.getCoins();
    data.player.progress = collectPlayerProgress(gs);
    data.princess = collectPrincessSaveData(gs);
    data.childlikeHeart = emotion.childlikeHeart;
    data.grievance = emotion.grievance;
    data.joy = emotion.joy;
    data.stress = emotion.stress;
    data.environment.day = gs.timeSystem.getDay();
    data.environment.hour = gs.timeSystem.getHour();
    data.environment.weather = WeatherSystem::getWeatherId(gs.weatherSystem.getCurrentWeather());
    data.environment.weatherIntensity = gs.weatherSystem.getIntensity();
    data.environment.storyWeatherTag = gs.weatherSystem.getCurrentSpecialTag();
    data.homeFurniture = gs.buildingSystem.getInstances();
    data.furnitureStock = gs.inventory.getFurnitureStock();
    data.unlockedFurniture = gs.inventory.getUnlockedFurniture();
    data.toyData = gs.toySystem.getSaveData();
    data.quests = gs.questSystem.getSaveData();

    for (const auto& regionId : gs.regionManager.getDiscoveredRegions()) {
        const MapRegion* region = gs.regionManager.getRegion(regionId);
        if (!region) continue;

        SaveData::RegionData regionData;
        regionData.id = region->getId();
        regionData.name = region->getName();
        regionData.seed = region->getSeed();
        regionData.size = {region->getWidth(), region->getHeight()};
        regionData.tileSize = region->getTileMap().tileSize;
        regionData.modifications = region->getTileManager().getModifications();
        regionData.decorModifications = region->getDecorations();
        regionData.pois = region->getPOIs();
        data.regions.push_back(regionData);
    }

    return SaveRepository::writeSave(slot, SaveSerializer::toJson(data));
}

}  // namespace SaveSnapshotBuilder
