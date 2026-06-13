#pragma once

#include "Game/Building/Furniture.h"
#include "Game/Inventory/Inventory.h"
#include "Game/Progress/StoryProgress.h"
#include "Game/Quest/QuestTypes.h"
#include "Game/Toy/ToyTypes.h"
#include "Game/World/MapRegion.h"

#include <glm/vec2.hpp>
#include <string>
#include <vector>

/**
 * 玩家进度数据
 */
struct PlayerProgress {
    std::vector<std::string> discoveredRegions;
    std::vector<std::string> completedQuests;
    std::vector<std::string> collectedItems;
    float totalPlayTime = 0.0f;
    int maxHealth = 100;
    int maxMana = 100;
};

/**
 * 存档数据
 */
struct SaveData {
    int version = 4;
    std::string timestamp;

    // 玩家数据
    struct PlayerData {
        std::string regionId = "real_street_prologue";
        glm::vec2 position = glm::vec2(8.0f, 12.0f);
        float health = 100.0f;
        float maxHealth = 100.0f;
        float mana = 100.0f;
        float maxMana = 100.0f;
        int coins = 0;
        PlayerProgress progress;
    } player;

    struct PrincessData {
        std::string name = "小夏";
        glm::vec2 position = glm::vec2(5.0f, 3.0f);
        float affection = 0.0f;
        bool following = false;
        float ultimateCharge = 0.0f;
        bool hasBody = false;
    } princess;

    // 区域数据（种子+差异）
    struct RegionData {
        std::string id;
        std::string name;
        int seed;
        glm::ivec2 size;
        float tileSize;
        std::vector<TileModification> modifications;
        std::vector<Decoration> decorModifications;
        std::vector<PointOfInterest> pois;
    };

    std::vector<RegionData> regions;

    // 情感状态
    float childlikeHeart;
    float grievance;
    float joy;
    float stress;

    // 环境状态
    struct EnvironmentData {
        int day = 1;
        float hour = 10.0f;
        std::string weather = "Clear";
        float weatherIntensity = 0.0f;
        std::string storyWeatherTag;
    } environment;

    // 阶段7：秘密基地家具布局
    std::vector<FurnitureInstance> homeFurniture;
    std::vector<FurnitureStock> furnitureStock;
    std::vector<ItemStack> itemStacks;
    std::vector<std::string> unlockedFurniture;
    ToySaveData toyData;
    std::vector<QuestSaveEntry> quests;
    StoryProgressSnapshot storyProgress;

    SaveData()
        : childlikeHeart(950.0f)
        , grievance(0.0f)
        , joy(50.0f)
        , stress(0.0f)
    {}
};
