#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace AimReticleView {

struct Model {
    bool visible = false;
    glm::vec2 playerPosition{0.0f, 0.0f};
    glm::vec2 targetPosition{0.0f, 0.0f};
    glm::vec2 aimDirection{1.0f, 0.0f};
};

void render(const Model& model, const glm::mat4& viewProj);

}  // namespace AimReticleView
