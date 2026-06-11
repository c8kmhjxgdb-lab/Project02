#pragma once

#include "Game/Boss/BossController.h"

enum class PopupCrownPhase {
    Inactive,
    WelcomePopup,
    LimitedOffer,
    SmallWindowWorld,
    Defeated
};

class PopupCrownBoss final : public BossController {
public:
    void start() override;
    void update(float dt) override;
    void applyDamage(float amount) override;
    bool isActive() const override;
    bool isDefeated() const override;

    PopupCrownPhase getPhase() const { return phase; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    void recordTemptationUse();
    BossReward buildReward(float childlikeHeartAtClear) const;

private:
    float maxHealth = 100.0f;
    float health = 0.0f;
    float phaseTimer = 0.0f;
    int temptationUses = 0;
    PopupCrownPhase phase = PopupCrownPhase::Inactive;

    void refreshPhase();
};
