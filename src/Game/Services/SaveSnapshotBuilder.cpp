#include "Game/Services/SaveSnapshotBuilder.h"

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

    return gs.saveSystem.saveGame(
        slot,
        saveRegion ? saveRegion->getId() : std::string("starter_village"),
        glm::vec2(pPos.x, pPos.y),
        gs.playerHealth.getCurrentHealth(),
        gs.playerHealth.getMaxHealth(),
        gs.playerMana,
        gs.playerMaxMana,
        gs.inventory.getCoins(),
        collectPlayerProgress(gs),
        gs.regionManager,
        emotion.childlikeHeart,
        emotion.grievance,
        emotion.joy,
        emotion.stress,
        gs.timeSystem.getDay(),
        gs.timeSystem.getHour(),
        WeatherSystem::getWeatherId(gs.weatherSystem.getCurrentWeather()),
        gs.weatherSystem.getIntensity(),
        gs.weatherSystem.getCurrentSpecialTag(),
        collectPrincessSaveData(gs),
        gs.buildingSystem.getInstances(),
        gs.inventory.getFurnitureStock(),
        gs.inventory.getUnlockedFurniture(),
        gs.toySystem.getSaveData(),
        gs.questSystem.getSaveData()
    );
}

}  // namespace SaveSnapshotBuilder
