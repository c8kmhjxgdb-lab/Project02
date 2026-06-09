#include "SaveSystem.h"

#include "Game/Data/SaveRepository.h"
#include "Game/Data/SaveSerializer.h"
#include "Game/World/RegionManager.h"

#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstdio>

std::string SaveSystem::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}

bool SaveSystem::saveGame(const std::string& slot,
                         const std::string& playerRegionId,
                         const glm::vec2& playerPos,
                         float playerHealth,
                         float playerMaxHealth,
                         float playerMana,
                         float playerMaxMana,
                         int playerCoins,
                         const PlayerProgress& progress,
                         const RegionManager& regionManager,
                         float childlikeHeart,
                         float grievance,
                         float joy,
                         float stress,
                         int environmentDay,
                         float environmentHour,
                         const std::string& weather,
                         float weatherIntensity,
                         const std::string& storyWeatherTag,
                         const SaveData::PrincessData& princessData,
                         const std::vector<FurnitureInstance>& homeFurniture,
                         const std::vector<FurnitureStock>& furnitureStock,
                         const std::vector<std::string>& unlockedFurniture,
                         const ToySaveData& toyData,
                         const std::vector<QuestSaveEntry>& quests) {
    SaveData data;
    data.version = 3;
    data.timestamp = getCurrentTimestamp();

    // 玩家数据
    data.player.regionId = playerRegionId;
    data.player.position = playerPos;
    data.player.health = playerHealth;
    data.player.maxHealth = playerMaxHealth;
    data.player.mana = playerMana;
    data.player.maxMana = playerMaxMana;
    data.player.coins = playerCoins;
    data.player.progress = progress;
    data.princess = princessData;

    // 情感状态
    data.childlikeHeart = childlikeHeart;
    data.grievance = grievance;
    data.joy = joy;
    data.stress = stress;

    // 环境状态
    data.environment.day = environmentDay > 0 ? environmentDay : 1;
    data.environment.hour = environmentHour;
    data.environment.weather = weather;
    data.environment.weatherIntensity = weatherIntensity;
    data.environment.storyWeatherTag = storyWeatherTag;
    data.homeFurniture = homeFurniture;
    data.furnitureStock = furnitureStock;
    data.unlockedFurniture = unlockedFurniture;
    data.toyData = toyData;
    data.quests = quests;

    // 区域数据
    for (const auto& regionId : regionManager.getDiscoveredRegions()) {
        const MapRegion* region = regionManager.getRegion(regionId);
        if (!region) continue;

        SaveData::RegionData regionData;
        regionData.id = region->getId();
        regionData.name = region->getName();
        regionData.seed = region->getSeed();
        regionData.size = {region->getWidth(), region->getHeight()};
        regionData.tileSize = region->getTileMap().tileSize;

        // 获取修改记录（从 MapTileManager）
        regionData.modifications = region->getTileManager().getModifications();
        regionData.decorModifications = region->getDecorations();
        regionData.pois = region->getPOIs();

        data.regions.push_back(regionData);
    }

    SaveSerializer::Json jsonData = SaveSerializer::toJson(data);
    return SaveRepository::writeSave(slot, jsonData);
}

bool SaveSystem::loadGame(const std::string& slot, SaveData& outData) {
    SaveSerializer::Json jsonData;
    std::string errorMessage;
    if (!SaveRepository::readSave(slot, jsonData, &errorMessage)) {
        if (!errorMessage.empty()) {
            fprintf(stderr, "[SaveSystem] Failed to parse %s: %s\n",
                SaveRepository::getSavePath(slot).c_str(),
                errorMessage.c_str());
        }
        outData = SaveData{};
        return false;
    }

    outData = SaveSerializer::fromJson(jsonData);
    return true;
}

bool SaveSystem::deleteSave(const std::string& slot) {
    return SaveRepository::deleteSave(slot);
}

bool SaveSystem::hasSave(const std::string& slot) const {
    return SaveRepository::hasSave(slot);
}

std::vector<std::string> SaveSystem::getSaveSlots() const {
    return SaveRepository::getSaveSlots();
}

SaveSystem::SaveMeta SaveSystem::getSaveMeta(const std::string& slot) const {
    SaveMeta meta;
    meta.slot = slot;
    meta.timestamp = "";
    meta.regionName = "";
    meta.playTime = 0.0f;

    SaveSerializer::Json jsonData;
    if (SaveRepository::readSave(slot, jsonData)) {

        meta.timestamp = jsonData.value("timestamp", "");
        // playTime is stored at player.progress.totalPlayTime (see SaveSerializer::toJson),
        // not at the (non-existent) player.playTime key.
        if (jsonData.contains("player") && jsonData["player"].is_object() &&
            jsonData["player"].contains("progress")) {
            meta.playTime = jsonData["player"]["progress"].value("totalPlayTime", 0.0f);
        } else {
            meta.playTime = 0.0f;
        }

        if (jsonData.contains("regions") && !jsonData["regions"].empty()) {
            meta.regionName = jsonData["regions"][0].value("name", "");
        }
    }

    return meta;
}
