#include "Lightning.h"
#include <box2d/box2d.h>

Lightning::Lightning() {}

void Lightning::begin(const glm::vec2& startPos) {
    currentChain.start(startPos);
}

void Lightning::addHit(const glm::vec2& hitPos, float damage, const b2BodyId& bodyId) {
    currentChain.addHit(hitPos, damage, bodyId);
}

void Lightning::update(float dt) {
    if (!currentChain.active) return;

    currentChain.remainingTime -= dt;
    if (currentChain.remainingTime <= 0.0f) {
        currentChain.active = false;
        currentChain.remainingTime = 0.0f;
    }
}

// --- LightningChain implementation ---

void LightningChain::addHit(const glm::vec2& hitPos, float damage, const b2BodyId& bodyId) {
    points.push_back(hitPos);
    damageValues.push_back(damage);
    hitBodies.push_back(bodyId);
}

bool LightningChain::hasHit(const b2BodyId& bodyId) const {
    // b2BodyId doesn't have operator==, compare fields manually
    for (const auto& hit : hitBodies) {
        if (hit.index1 == bodyId.index1 &&
            hit.world0 == bodyId.world0 &&
            hit.generation == bodyId.generation) {
            return true;
        }
    }
    return false;
}
