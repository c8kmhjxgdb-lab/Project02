#include "BondTechnique.h"

BondTechniqueSystem::BondTechniqueSystem() {}

bool BondTechniqueSystem::activate(const glm::vec2& centerPos) {
    if (!canActivate()) return false;

    technique.activate(centerPos);
    return true;
}

void BondTechniqueSystem::update(float dt) {
    if (!technique.active) return;

    technique.remainingTime -= dt;

    // Expand radius
    float progress = 1.0f - (technique.remainingTime / technique.lifetime);
    technique.radius = technique.maxRadius * progress;

    if (technique.remainingTime <= 0.0f) {
        technique.reset();
    }
}
