#pragma once

#include "Game/Ability/BondTechnique.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/Shield.h"

#include <glm/vec2.hpp>

namespace AbilityView {

struct WorldEffectsModel {
    const Shield* shield = nullptr;
    const Lightning* lightning = nullptr;
    const BondTechniqueSystem* bondTechnique = nullptr;
    glm::vec2 playerPosition{0.0f, 0.0f};
    bool isFlying = false;
    float flightHeight = 0.0f;
    float flightMaxHeight = 1.0f;
    float animationTime = 0.0f;
};

void renderWorldEffects(const WorldEffectsModel& model);

}  // namespace AbilityView
