#include "Game/Boss/PopupCrownBoss.h"
#include "TestSupport.h"

void phasesFollowHealthThresholds() {
    PopupCrownBoss boss;
    boss.start();

    TestSupport::require(boss.getPhase() == PopupCrownPhase::WelcomePopup, "starts in welcome phase");
    boss.applyDamage(31.0f);
    TestSupport::require(boss.getPhase() == PopupCrownPhase::LimitedOffer, "enters limited offer below 70");
    boss.applyDamage(36.0f);
    TestSupport::require(boss.getPhase() == PopupCrownPhase::SmallWindowWorld, "enters small window below 35");
}

void rewardsReflectTemptationAndHeart() {
    PopupCrownBoss cleanBoss;
    cleanBoss.start();
    cleanBoss.applyDamage(1000.0f);
    BossReward cleanReward = cleanBoss.buildReward(650.0f);

    TestSupport::require(cleanReward.defeated, "clean boss defeated");
    TestSupport::require(cleanReward.itemRewards.size() >= 1, "clean boss grants item rewards");
    TestSupport::require(cleanReward.hiddenSticker, "clean high heart grants sticker");

    PopupCrownBoss temptedBoss;
    temptedBoss.start();
    temptedBoss.recordTemptationUse();
    temptedBoss.applyDamage(1000.0f);
    BossReward temptedReward = temptedBoss.buildReward(650.0f);

    TestSupport::require(!temptedReward.hiddenSticker, "temptation blocks sticker");
}

int main() {
    phasesFollowHealthThresholds();
    rewardsReflectTemptationAndHeart();
    return 0;
}
