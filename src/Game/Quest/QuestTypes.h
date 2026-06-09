#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct QuestSnapshot {
    bool inHomeBase = false;
    int placedBedCount = 0;
    int placedDeskCount = 0;
    int placedLampCount = 0;
    int placedFlowerPotCount = 0;
    int placedToyShelfCount = 0;
    bool miniCarCollected = false;
    bool isRainy = false;
    bool isNight = false;
    bool talkedWithPrincessAtBase = false;
    float childlikeHeart = 950.0f;
    float lowChildlikeHeartThreshold = 200.0f;
};

struct QuestReward {
    std::string questId;
    int coins = 0;
    float childlikeHeart = 0.0f;
    float reduceGrievance = 0.0f;
    float affection = 0.0f;
    std::string unlockFurniture;
    bool completed = false;
};

enum class QuestState : uint8_t {
    Hidden,
    Available,
    Active,
    Completed,
    Rewarded
};

struct QuestObjectiveProgress {
    std::string type;
    std::string targetId;
    int required = 1;
    int current = 0;
};

struct QuestSaveEntry {
    std::string id;
    QuestState state = QuestState::Available;
    std::vector<QuestObjectiveProgress> objectives;
    bool rewardClaimed = false;
};

struct QuestDef {
    std::string id;
    std::string name;
    std::vector<std::string> requiredFurniture;
    bool requiresMiniCar = false;
    bool requiresRainy = false;
    bool requiresNight = false;
    bool requiresLowChildlikeHeart = false;
    bool requiresTalk = false;
    QuestReward reward;
};
