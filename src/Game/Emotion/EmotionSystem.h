#pragma once

#include <glm/vec2.hpp>
#include <functional>
#include <algorithm>
#include <cstdint>
#include <string>

class LuaVM;

enum class CharacterMood : uint8_t {
    Calm,
    Happy,
    Worried,
    Inspired,
    Tired
};

enum class ChildlikeHeartTier : uint8_t {
    Faded,
    Normal,
    Vivid,
    Radiant
};

/**
 * 情感状态
 */
struct EmotionState {
    float childlikeHeart; // 童心值 0-1000，长期主线资源
    float grievance;      // 委屈值 0-100
    float joy;            // 快乐值 0-100（预留）
    float stress;         // 压力值 0-100（预留）
    CharacterMood mood;

    EmotionState()
        : childlikeHeart(950.0f)
        , grievance(0.0f)
        , joy(50.0f)
        , stress(0.0f)
        , mood(CharacterMood::Calm)
    {}

    // 委屈值超过阈值
    bool isDepressed() const { return grievance >= 70.0f; }
    bool isExtreme() const { return grievance >= 90.0f; }
};

/**
 * 情感系统
 */
class EmotionSystem {
public:
    EmotionSystem();

    // Load Stage 7 thresholds from assets/scripts/childhood_config.lua.
    bool loadConfig(LuaVM& lua, const char* path);

    // 更新委屈值
    void addGrievance(float amount);
    void reduceGrievance(float amount);
    void setGrievance(float value);
    void setJoy(float value);
    void setStress(float value);

    // 更新童心值
    void addChildlikeHeart(float amount);
    void reduceChildlikeHeart(float amount);
    void setChildlikeHeart(float value);
    float getChildlikeHeartPercent() const;
    bool isLowChildlikeHeart() const;
    bool isChildlikeHeartEmpty() const;
    ChildlikeHeartTier getChildlikeHeartTier() const;
    std::string getChildlikeHeartTierName() const;
    bool canSeeHiddenPickups() const;
    float getLowChildlikeHeartThreshold() const { return childlikeHeartLowThreshold; }

    // 设置/查询人物心情
    void setMood(CharacterMood newMood);
    CharacterMood getMood() const { return state.mood; }

    // 宣泄（蒙头哭）
    void vent();  // 清零委屈值

    // 设置是否在家（影响自然恢复）
    void setAtHome(bool atHomeFlag) { atHome = atHomeFlag; }
    bool isAtHome() const { return atHome; }

    // 获取当前状态
    const EmotionState& getState() const { return state; }

    // 获取后处理强度（暗角程度，clamp到[0,1]）
    float getPostProcessIntensity() const;

    // 移动速度修正（委屈时减速）
    float getSpeedMultiplier() const;

    // 回调
    using EmotionCallback = std::function<void(const EmotionState&)>;
    void setOnEmotionChange(EmotionCallback cb) { onEmotionChange = std::move(cb); }

    // 更新（dt为秒）
    void update(float dt);

private:
    EmotionState state;
    float ventCooldown;         // 宣泄后冷却时间
    bool atHome;                // 是否在家（影响自然恢复）
    float naturalRecoveryTimer; // 自然恢复计时器
    float childlikeRecoveryTimer;

    float childlikeHeartMin;
    float childlikeHeartMax;
    float childlikeHeartLowThreshold;
    float childlikeHeartLowSpeedMultiplier;
    float grievanceDepressedThreshold;
    float grievanceExtremeThreshold;
    float homeGrievanceRecoveryPerMinute;
    float ventCooldownDuration;

    EmotionCallback onEmotionChange;

    void notifyChange();
};
