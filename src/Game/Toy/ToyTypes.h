#pragma once

#include <string>
#include <vector>

struct ToyReward {
    int coins = 0;
    float childlikeHeart = 0.0f;
    float affection = 0.0f;
    bool granted = false;
};

struct ToySaveData {
    std::vector<std::string> collectedToys;
    int miniCarLastRewardDay = 0;
    float miniCarBestTime = 0.0f;
};

struct ToyDef {
    std::string id;
    std::string name;
    std::string displayFurniture;
    int rewardCoins = 8;
    float rewardChildlikeHeart = 12.0f;
    float rewardAffection = 5.0f;
    bool dailyReward = true;
};
