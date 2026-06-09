#pragma once

#include "Game/Data/SaveData.h"

#include <string>
#include <vector>

class RegionManager;

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
                 const std::vector<QuestSaveEntry>& quests);

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
    // 获取当前时间戳
    static std::string getCurrentTimestamp();
};
