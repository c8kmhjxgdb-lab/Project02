#include "BondTechnique.h"

BondTechniqueSystem::BondTechniqueSystem() {}

bool BondTechniqueSystem::activate(const glm::vec2& centerPos) {
    if (!canActivate()) return false;

    technique.activate(centerPos);
    return true;
}

void BondTechniqueSystem::update(float dt) {
    if (!technique.isActive()) return;

    technique.remainingTime -= dt;
    if (technique.remainingTime < 0.0f) technique.remainingTime = 0.0f;

    // Expand radius (guard against division by zero)
    if (technique.lifetime > 0.0f) {
        float progress = 1.0f - (technique.remainingTime / technique.lifetime);
        technique.radius = technique.maxRadius * progress;
    } else {
        technique.radius = technique.maxRadius;
    }

    if (technique.remainingTime <= 0.0f) {
        technique.reset();
    }
}
