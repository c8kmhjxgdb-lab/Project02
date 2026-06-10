#pragma once

#include "Game/Emotion/EmotionSystem.h"

#include <string>

struct SkillTierProfile {
    std::string fireName;
    std::string iceName;
    std::string lightningName;
    std::string shieldName;
    std::string movementName;
    float fireDamageMultiplier = 1.0f;
    float iceDamageMultiplier = 1.0f;
    float projectileSpeedMultiplier = 1.0f;
    float projectileRadiusMultiplier = 1.0f;
    float iceSlowMultiplier = 0.4f;
    int lightningMaxChains = 3;
    float shieldDamageReduction = 0.4f;
    float movementDistance = 6.0f;
    float movementCooldown = 2.5f;
};

namespace ChildlikeSkillProfile {

SkillTierProfile forTier(ChildlikeHeartTier tier);

}  // namespace ChildlikeSkillProfile
