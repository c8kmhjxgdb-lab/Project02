#pragma once

#include "Engine/Renderer/ParticleSystem.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <vector>

namespace ParticleView {

struct VentTearsModel {
    bool visible = false;
    glm::vec2 position{0.0f, 0.0f};
    float shakeX = 0.0f;
    float animationTime = 0.0f;
};

void render(const std::vector<Particle>& particles,
            const VentTearsModel& ventTears,
            const glm::mat4& viewProj);

}  // namespace ParticleView
