#include "EmotionSystem.h"

#include "Game/Data/EmotionConfigLoader.h"

#include <algorithm>
#include <cmath>

EmotionSystem::EmotionSystem()
    : ventCooldown(0.0f)
    , atHome(false)
    , naturalRecoveryTimer(0.0f)
    , childlikeRecoveryTimer(0.0f)
    , childlikeHeartMin(0.0f)
    , childlikeHeartMax(1000.0f)
    , childlikeHeartLowThreshold(200.0f)
    , childlikeHeartLowSpeedMultiplier(0.7f)
    , grievanceDepressedThreshold(70.0f)
    , grievanceExtremeThreshold(90.0f)
    , homeGrievanceRecoveryPerMinute(1.0f)
    , ventCooldownDuration(5.0f)
{}

bool EmotionSystem::loadConfig(LuaVM& lua, const char* path) {
    EmotionConfig defaults;
    defaults.childlikeHeartMin = childlikeHeartMin;
    defaults.childlikeHeartMax = childlikeHeartMax;
    defaults.childlikeHeartLowThreshold = childlikeHeartLowThreshold;
    defaults.childlikeHeartLowSpeedMultiplier = childlikeHeartLowSpeedMultiplier;
    defaults.initialChildlikeHeart = state.childlikeHeart;
    defaults.grievanceDepressedThreshold = grievanceDepressedThreshold;
    defaults.grievanceExtremeThreshold = grievanceExtremeThreshold;
    defaults.homeGrievanceRecoveryPerMinute = homeGrievanceRecoveryPerMinute;
    defaults.ventCooldownDuration = ventCooldownDuration;

    EmotionConfig config;
    if (!EmotionConfigLoader::load(lua, path, defaults, config)) {
        return false;
    }

    childlikeHeartMin = config.childlikeHeartMin;
    childlikeHeartMax = config.childlikeHeartMax;
    childlikeHeartLowThreshold = config.childlikeHeartLowThreshold;
    childlikeHeartLowSpeedMultiplier = config.childlikeHeartLowSpeedMultiplier;
    if (config.hasChildlikeHeartConfig) {
        state.childlikeHeart = config.initialChildlikeHeart;
    }
    grievanceDepressedThreshold = config.grievanceDepressedThreshold;
    grievanceExtremeThreshold = config.grievanceExtremeThreshold;
    homeGrievanceRecoveryPerMinute = config.homeGrievanceRecoveryPerMinute;
    ventCooldownDuration = config.ventCooldownDuration;

    notifyChange();
    return true;
}

void EmotionSystem::addGrievance(float amount) {
    state.grievance = std::min(100.0f, state.grievance + amount);
    notifyChange();
}

void EmotionSystem::reduceGrievance(float amount) {
    state.grievance = std::max(0.0f, state.grievance - amount);
    notifyChange();
}

void EmotionSystem::setGrievance(float value) {
    state.grievance = std::clamp(value, 0.0f, 100.0f);
    notifyChange();
}

void EmotionSystem::setJoy(float value) {
    state.joy = std::clamp(value, 0.0f, 100.0f);
    notifyChange();
}

void EmotionSystem::setStress(float value) {
    state.stress = std::clamp(value, 0.0f, 100.0f);
    notifyChange();
}

void EmotionSystem::addChildlikeHeart(float amount) {
    setChildlikeHeart(state.childlikeHeart + amount);
}

void EmotionSystem::reduceChildlikeHeart(float amount) {
    setChildlikeHeart(state.childlikeHeart - amount);
}

void EmotionSystem::setChildlikeHeart(float value) {
    state.childlikeHeart = std::clamp(value, childlikeHeartMin, childlikeHeartMax);
    if (isLowChildlikeHeart()) {
        state.mood = CharacterMood::Worried;
    } else if (state.mood == CharacterMood::Worried) {
        state.mood = CharacterMood::Calm;
    }
    notifyChange();
}

float EmotionSystem::getChildlikeHeartPercent() const {
    if (childlikeHeartMax <= childlikeHeartMin) return 0.0f;
    return std::clamp((state.childlikeHeart - childlikeHeartMin) /
                      (childlikeHeartMax - childlikeHeartMin), 0.0f, 1.0f);
}

bool EmotionSystem::isLowChildlikeHeart() const {
    return state.childlikeHeart < childlikeHeartLowThreshold;
}

bool EmotionSystem::isChildlikeHeartEmpty() const {
    return state.childlikeHeart <= childlikeHeartMin;
}

ChildlikeHeartTier EmotionSystem::getChildlikeHeartTier() const {
    if (state.childlikeHeart >= 800.0f) {
        return ChildlikeHeartTier::Radiant;
    }
    if (state.childlikeHeart >= 500.0f) {
        return ChildlikeHeartTier::Vivid;
    }
    if (state.childlikeHeart >= 200.0f) {
        return ChildlikeHeartTier::Normal;
    }
    return ChildlikeHeartTier::Faded;
}

std::string EmotionSystem::getChildlikeHeartTierName() const {
    switch (getChildlikeHeartTier()) {
    case ChildlikeHeartTier::Faded:
        return "失色";
    case ChildlikeHeartTier::Normal:
        return "寻常";
    case ChildlikeHeartTier::Vivid:
        return "鲜活";
    case ChildlikeHeartTier::Radiant:
        return "绚烂";
    }
    return "寻常";
}

bool EmotionSystem::canSeeHiddenPickups() const {
    ChildlikeHeartTier tier = getChildlikeHeartTier();
    return tier == ChildlikeHeartTier::Vivid || tier == ChildlikeHeartTier::Radiant;
}

void EmotionSystem::setMood(CharacterMood newMood) {
    state.mood = newMood;
    notifyChange();
}

float EmotionSystem::getPostProcessIntensity() const {
    float grievanceRange = std::max(100.0f - grievanceDepressedThreshold, 1.0f);
    float grievanceIntensity = (state.grievance - grievanceDepressedThreshold) / grievanceRange;
    grievanceIntensity = std::clamp(grievanceIntensity, 0.0f, 1.0f);

    float childlikeIntensity = 0.0f;
    if (state.childlikeHeart < childlikeHeartLowThreshold) {
        childlikeIntensity = (childlikeHeartLowThreshold - state.childlikeHeart) /
                             std::max(childlikeHeartLowThreshold, 1.0f);
        childlikeIntensity = std::clamp(childlikeIntensity, 0.0f, 1.0f);
    }

    return std::max(grievanceIntensity, childlikeIntensity);
}

float EmotionSystem::getSpeedMultiplier() const {
    float multiplier = state.grievance >= grievanceDepressedThreshold ? 0.7f : 1.0f;
    if (state.grievance >= grievanceExtremeThreshold) {
        multiplier *= 0.9f;
    }
    if (isLowChildlikeHeart()) {
        multiplier *= childlikeHeartLowSpeedMultiplier;
    }
    return multiplier;
}

void EmotionSystem::vent() {
    if (ventCooldown > 0.0f) return; // 冷却中不可宣泄
    state.grievance = 0.0f;
    if (atHome) {
        state.childlikeHeart = std::min(childlikeHeartMax, state.childlikeHeart + 5.0f);
    }
    ventCooldown = ventCooldownDuration;
    notifyChange();
}

void EmotionSystem::update(float dt) {
    // 宣泄冷却
    if (ventCooldown > 0.0f) {
        ventCooldown -= dt;
        if (ventCooldown < 0.0f) ventCooldown = 0.0f;
    }

    // 自然恢复：在家时按配置每分钟降低委屈值
    if (atHome && state.grievance > 0.0f && homeGrievanceRecoveryPerMinute > 0.0f) {
        naturalRecoveryTimer += dt;
        if (naturalRecoveryTimer >= 60.0f) {
            naturalRecoveryTimer -= 60.0f;
            state.grievance = std::max(0.0f, state.grievance - homeGrievanceRecoveryPerMinute);
            notifyChange();
        }
    } else {
        naturalRecoveryTimer = 0.0f;
    }

    // 基地内缓慢恢复童心，后续由家具舒适度与任务奖励加成。
    if (atHome && state.childlikeHeart < childlikeHeartMax) {
        childlikeRecoveryTimer += dt;
        if (childlikeRecoveryTimer >= 60.0f) {
            childlikeRecoveryTimer -= 60.0f;
            state.childlikeHeart = std::min(childlikeHeartMax, state.childlikeHeart + 1.0f);
            notifyChange();
        }
    } else {
        childlikeRecoveryTimer = 0.0f;
    }
}

void EmotionSystem::notifyChange() {
    if (onEmotionChange) onEmotionChange(state);
}
