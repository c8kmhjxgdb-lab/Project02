#pragma once

#include "Game/World/MapRegion.h"
#include "Game/World/RegionManager.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * 玩家进度数据
 */
struct PlayerProgress {
    std::vector<std::string> discoveredRegions;
    std::vector<std::string> completedQuests;
    std::vector<std::string> collectedItems;
    float totalPlayTime;
    int maxHealth;
    int maxMana;
};

/**
 * 存档数据
 */
struct SaveData {
    int version = 2;
    std::string timestamp;

    // 玩家数据
    struct PlayerData {
        glm::vec2 position;
        float health;
        float maxHealth;
        float mana;
        float maxMana;
        int coins;
        PlayerProgress progress;
    } player;

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
    float grievance;
    float joy;

    SaveData() : grievance(0.0f), joy(0.0f) {}
};

/**
 * SaveSystem — 存档/读档系统
 *
 * 使用种子+差异策略保存游戏状态，确保存档文件小巧高效。
 * 区域地形基于种子程序生成，只保存玩家修改的差异。
 */
class SaveSystem {
public:
    // 保存游戏
    bool saveGame(const std::string& slot,
                 const glm::vec2& playerPos,
                 float playerHealth,
                 float playerMaxHealth,
                 float playerMana,
                 float playerMaxMana,
                 int playerCoins,
                 const PlayerProgress& progress,
                 const RegionManager& regionManager,
                 float grievance);

    // 加载游戏
    bool loadGame(const std::string& slot, SaveData& outData);

    // 删除存档
    bool deleteSave(const std::string& slot);

    // 检查存档是否存在
    bool hasSave(const std::string& slot) const;

    // 获取存档列表
    std::vector<std::string> getSaveSlots() const;

    // 获取存档元数据（不加载完整数据）
    struct SaveMeta {
        std::string slot;
        std::string timestamp;
        std::string regionName;
        float playTime;
    };
    SaveMeta getSaveMeta(const std::string& slot) const;

private:
    std::string getSavePath(const std::string& slot) const;

    // JSON 序列化/反序列化
    json saveDataToJson(const SaveData& data) const;
    SaveData jsonToSaveData(const json& data) const;

    // 获取当前时间戳
    static std::string getCurrentTimestamp();
};
