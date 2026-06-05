#include "Lightning.h"

Lightning::Lightning() {}

void Lightning::begin(const glm::vec2& startPos) {
    currentChain.start(startPos);
}

void Lightning::addHit(const glm::vec2& hitPos, float damage) {
    currentChain.addHit(hitPos, damage);
}

void Lightning::end() {
    // 雷电链完成，视觉效果由 isActive() 和 lifetime 控制
}

void Lightning::update(float dt) {
    if (!currentChain.active) return;

    currentChain.remainingTime -= dt;
    if (currentChain.remainingTime <= 0.0f) {
        currentChain.active = false;
        currentChain.remainingTime = 0.0f;
    }
}
