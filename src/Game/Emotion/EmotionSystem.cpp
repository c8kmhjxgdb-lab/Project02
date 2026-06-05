#include "EmotionSystem.h"
#include <algorithm>
#include <cmath>

EmotionSystem::EmotionSystem()
    : ventCooldown(0.0f)
    , atHome(false)
    , naturalRecoveryTimer(0.0f)
{}

void EmotionSystem::addGrievance(float amount) {
    state.grievance = std::min(100.0f, state.grievance + amount);
    if (onEmotionChange) onEmotionChange(state);
}

void EmotionSystem::reduceGrievance(float amount) {
    state.grievance = std::max(0.0f, state.grievance - amount);
    if (onEmotionChange) onEmotionChange(state);
}

void EmotionSystem::setGrievance(float value) {
    state.grievance = std::clamp(value, 0.0f, 100.0f);
    if (onEmotionChange) onEmotionChange(state);
}

void EmotionSystem::vent() {
    if (ventCooldown > 0.0f) return; // 冷却中不可宣泄
    state.grievance = 0.0f;
    ventCooldown = 5.0f; // 宣泄后5秒冷却
    if (onEmotionChange) onEmotionChange(state);
}

void EmotionSystem::update(float dt) {
    // 宣泄冷却
    if (ventCooldown > 0.0f) {
        ventCooldown -= dt;
        if (ventCooldown < 0.0f) ventCooldown = 0.0f;
    }

    // 自然恢复：在家时每60秒 -1 委屈值
    if (atHome && state.grievance > 0.0f) {
        naturalRecoveryTimer += dt;
        if (naturalRecoveryTimer >= 60.0f) {
            naturalRecoveryTimer -= 60.0f;
            state.grievance = std::max(0.0f, state.grievance - 1.0f);
            if (onEmotionChange) onEmotionChange(state);
        }
    }
}
