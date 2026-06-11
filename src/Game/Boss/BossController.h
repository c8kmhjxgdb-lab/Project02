#pragma once

#include "Game/Quest/QuestTypes.h"

#include <string>
#include <vector>

struct BossReward {
    bool defeated = false;
    int coins = 0;
    float childlikeHeart = 0.0f;
    std::vector<QuestItemReward> itemRewards;
    bool hiddenSticker = false;
    std::string storyFlag;
};

class BossController {
public:
    virtual ~BossController() = default;
    virtual void start() = 0;
    virtual void update(float dt) = 0;
    virtual void applyDamage(float amount) = 0;
    virtual bool isActive() const = 0;
    virtual bool isDefeated() const = 0;
};
