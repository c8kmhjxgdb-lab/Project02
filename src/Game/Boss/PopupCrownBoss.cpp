#include "Game/Boss/PopupCrownBoss.h"

#include <algorithm>

void PopupCrownBoss::start() {
    health = maxHealth;
    phaseTimer = 0.0f;
    temptationUses = 0;
    phase = PopupCrownPhase::WelcomePopup;
}

void PopupCrownBoss::update(float dt) {
    if (!isActive()) return;
    phaseTimer += dt;
}

void PopupCrownBoss::applyDamage(float amount) {
    if (!isActive() || amount <= 0.0f) return;
    health = std::max(0.0f, health - amount);
    refreshPhase();
}

bool PopupCrownBoss::isActive() const {
    return phase != PopupCrownPhase::Inactive && phase != PopupCrownPhase::Defeated;
}

bool PopupCrownBoss::isDefeated() const {
    return phase == PopupCrownPhase::Defeated;
}

void PopupCrownBoss::recordTemptationUse() {
    if (isActive()) {
        ++temptationUses;
    }
}

BossReward PopupCrownBoss::buildReward(float childlikeHeartAtClear) const {
    BossReward reward;
    reward.defeated = isDefeated();
    if (!reward.defeated) return reward;

    reward.coins = 60;
    reward.childlikeHeart = 120.0f;
    reward.itemRewards.push_back({"pixel_controller", 1});
    reward.itemRewards.push_back({"pixel_screw", 4});
    reward.itemRewards.push_back({"old_button", 2});
    reward.hiddenSticker = temptationUses == 0 && childlikeHeartAtClear >= 500.0f;
    if (reward.hiddenSticker) {
        reward.itemRewards.push_back({"no_pay_victory_sticker", 1});
    }
    reward.storyFlag = "popup_crown_defeated";
    return reward;
}

void PopupCrownBoss::refreshPhase() {
    if (health <= 0.0f) {
        phase = PopupCrownPhase::Defeated;
        return;
    }
    float pct = maxHealth > 0.0f ? health / maxHealth : 0.0f;
    if (pct <= 0.35f) {
        phase = PopupCrownPhase::SmallWindowWorld;
    } else if (pct <= 0.70f) {
        phase = PopupCrownPhase::LimitedOffer;
    } else {
        phase = PopupCrownPhase::WelcomePopup;
    }
}
