#include "Game/Ability/ChildlikeSkillProfile.h"
#include "TestSupport.h"

#include <cmath>
#include <string>

namespace {

bool nearlyEqual(float lhs, float rhs) {
    return std::fabs(lhs - rhs) <= 0.0001f;
}

void requireProfile(ChildlikeHeartTier tier,
                    const std::string& fireName,
                    const std::string& iceName,
                    const std::string& lightningName,
                    const std::string& shieldName,
                    const std::string& movementName,
                    float fireDamageMultiplier,
                    float iceDamageMultiplier,
                    float projectileSpeedMultiplier,
                    float projectileRadiusMultiplier,
                    float iceSlowMultiplier,
                    int lightningMaxChains,
                    float shieldDamageReduction,
                    float movementDistance,
                    float movementCooldown) {
    const SkillTierProfile profile = ChildlikeSkillProfile::forTier(tier);

    TestSupport::require(profile.fireName == fireName, "fire skill name matches tier");
    TestSupport::require(profile.iceName == iceName, "ice skill name matches tier");
    TestSupport::require(profile.lightningName == lightningName, "lightning skill name matches tier");
    TestSupport::require(profile.shieldName == shieldName, "shield skill name matches tier");
    TestSupport::require(profile.movementName == movementName, "movement skill name matches tier");
    TestSupport::require(nearlyEqual(profile.fireDamageMultiplier, fireDamageMultiplier), "fire multiplier matches tier");
    TestSupport::require(nearlyEqual(profile.iceDamageMultiplier, iceDamageMultiplier), "ice multiplier matches tier");
    TestSupport::require(nearlyEqual(profile.projectileSpeedMultiplier, projectileSpeedMultiplier),
        "projectile speed multiplier matches tier");
    TestSupport::require(nearlyEqual(profile.projectileRadiusMultiplier, projectileRadiusMultiplier),
        "projectile radius multiplier matches tier");
    TestSupport::require(nearlyEqual(profile.iceSlowMultiplier, iceSlowMultiplier), "ice slow multiplier matches tier");
    TestSupport::require(profile.lightningMaxChains == lightningMaxChains, "lightning chain count matches tier");
    TestSupport::require(nearlyEqual(profile.shieldDamageReduction, shieldDamageReduction),
        "shield reduction matches tier");
    TestSupport::require(nearlyEqual(profile.movementDistance, movementDistance), "movement distance matches tier");
    TestSupport::require(nearlyEqual(profile.movementCooldown, movementCooldown), "movement cooldown matches tier");
}

void profilesMatchTierTable() {
    requireProfile(ChildlikeHeartTier::Faded,
        "打火机", "冷风", "静电", "抱头", "快步",
        0.6f, 0.5f, 0.75f, 0.85f, 0.75f, 1, 0.20f, 3.0f, 3.0f);
    requireProfile(ChildlikeHeartTier::Normal,
        "火球术", "冰锥刺", "闪电链", "护盾", "飞行",
        1.0f, 1.0f, 1.0f, 1.0f, 0.55f, 3, 0.40f, 6.0f, 2.5f);
    requireProfile(ChildlikeHeartTier::Vivid,
        "流星坠", "霜花爆裂", "雷暴交响", "星辉屏障", "星翼滑翔",
        1.4f, 1.5f, 1.18f, 1.2f, 0.35f, 5, 0.60f, 10.0f, 2.0f);
    requireProfile(ChildlikeHeartTier::Radiant,
        "星愿焰火", "童心冻结", "童心共鸣雷", "童心圣域", "星愿翱翔",
        2.0f, 2.2f, 1.35f, 1.45f, 0.20f, 8, 0.80f, 15.0f, 1.5f);
}

void unknownTierFallsBackToNormalProfile() {
    const SkillTierProfile unknown = ChildlikeSkillProfile::forTier(static_cast<ChildlikeHeartTier>(255));
    const SkillTierProfile normal = ChildlikeSkillProfile::forTier(ChildlikeHeartTier::Normal);

    TestSupport::require(unknown.fireName == normal.fireName, "unknown tier uses normal fire name");
    TestSupport::require(unknown.iceName == normal.iceName, "unknown tier uses normal ice name");
    TestSupport::require(unknown.lightningName == normal.lightningName, "unknown tier uses normal lightning name");
    TestSupport::require(unknown.shieldName == normal.shieldName, "unknown tier uses normal shield name");
    TestSupport::require(unknown.movementName == normal.movementName, "unknown tier uses normal movement name");
    TestSupport::require(nearlyEqual(unknown.fireDamageMultiplier, normal.fireDamageMultiplier),
        "unknown tier uses normal fire multiplier");
    TestSupport::require(nearlyEqual(unknown.iceDamageMultiplier, normal.iceDamageMultiplier),
        "unknown tier uses normal ice multiplier");
    TestSupport::require(nearlyEqual(unknown.projectileSpeedMultiplier, normal.projectileSpeedMultiplier),
        "unknown tier uses normal projectile speed multiplier");
    TestSupport::require(nearlyEqual(unknown.projectileRadiusMultiplier, normal.projectileRadiusMultiplier),
        "unknown tier uses normal projectile radius multiplier");
    TestSupport::require(nearlyEqual(unknown.iceSlowMultiplier, normal.iceSlowMultiplier),
        "unknown tier uses normal ice slow multiplier");
    TestSupport::require(unknown.lightningMaxChains == normal.lightningMaxChains,
        "unknown tier uses normal lightning chain count");
    TestSupport::require(nearlyEqual(unknown.shieldDamageReduction, normal.shieldDamageReduction),
        "unknown tier uses normal shield reduction");
    TestSupport::require(nearlyEqual(unknown.movementDistance, normal.movementDistance),
        "unknown tier uses normal movement distance");
    TestSupport::require(nearlyEqual(unknown.movementCooldown, normal.movementCooldown),
        "unknown tier uses normal movement cooldown");
}

}  // namespace

int main() {
    profilesMatchTierTable();
    unknownTierFallsBackToNormalProfile();
    return 0;
}
