#include "VentAnimation.h"
#include <cmath>

void VentAnimation::start(const glm::vec2& pos) {
    position = pos;
    active = true;
    timer = 0.0f;
    duration = 3.0f;
}

void VentAnimation::update(float dt) {
    if (!active) return;

    timer += dt;
    if (timer >= duration) {
        stop();
    }
}

float VentAnimation::getShakeAmount() const {
    if (!active) return 0.0f;

    // 颤抖幅度：先增强后减弱
    float progress = timer / duration;
    float intensity = sinf(timer * 15.0f) * 0.05f;
    // 在中间时段最强
    float envelope = 1.0f - fabs(progress - 0.5f) * 2.0f;
    return intensity * envelope;
}

void VentAnimation::stop() {
    active = false;
    timer = 0.0f;
}
