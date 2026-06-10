#include "Game/Ability/ChildlikeSkillProfile.h"

namespace {

SkillTierProfile normalProfile() {
    return {
        "火球术", "冰锥刺", "闪电链", "护盾", "飞行",
        1.0f, 1.0f, 1.0f, 1.0f, 0.55f, 3, 0.40f, 6.0f, 2.5f
    };
}

}  // namespace

SkillTierProfile ChildlikeSkillProfile::forTier(ChildlikeHeartTier tier) {
    switch (tier) {
    case ChildlikeHeartTier::Faded:
        return {
            "打火机", "冷风", "静电", "抱头", "快步",
            0.6f, 0.5f, 0.75f, 0.85f, 0.75f, 1, 0.20f, 3.0f, 3.0f
        };
    case ChildlikeHeartTier::Normal:
        return normalProfile();
    case ChildlikeHeartTier::Vivid:
        return {
            "流星坠", "霜花爆裂", "雷暴交响", "星辉屏障", "星翼滑翔",
            1.4f, 1.5f, 1.18f, 1.2f, 0.35f, 5, 0.60f, 10.0f, 2.0f
        };
    case ChildlikeHeartTier::Radiant:
        return {
            "星愿焰火", "童心冻结", "童心共鸣雷", "童心圣域", "星愿翱翔",
            2.0f, 2.2f, 1.35f, 1.45f, 0.20f, 8, 0.80f, 15.0f, 1.5f
        };
    }

    return normalProfile();
}
