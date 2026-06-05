#pragma once

#include <glm/vec2.hpp>
#include <functional>
#include <algorithm>

/**
 * 情感状态
 */
struct EmotionState {
    float grievance;      // 委屈值 0-100
    float joy;            // 快乐值 0-100（预留）
    float stress;         // 压力值 0-100（预留）

    EmotionState() : grievance(0), joy(50), stress(0) {}

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

    // 更新委屈值
    void addGrievance(float amount);
    void reduceGrievance(float amount);
    void setGrievance(float value);

    // 宣泄（蒙头哭）
    void vent();  // 清零委屈值

    // 设置是否在家（影响自然恢复）
    void setAtHome(bool atHomeFlag) { atHome = atHomeFlag; }
    bool isAtHome() const { return atHome; }

    // 获取当前状态
    const EmotionState& getState() const { return state; }

    // 获取后处理强度（暗角程度，clamp到[0,1]）
    float getPostProcessIntensity() const {
        float v = (state.grievance - 70.0f) / 30.0f;
        return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    }

    // 移动速度修正（委屈时减速）
    float getSpeedMultiplier() const {
        return state.isDepressed() ? 0.7f : 1.0f;
    }

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

    EmotionCallback onEmotionChange;
};
