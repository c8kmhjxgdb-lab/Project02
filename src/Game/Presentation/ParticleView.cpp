#include "Game/Presentation/ParticleView.h"

#include "Engine/Renderer/Draw2D.h"

#include <glm/vec3.hpp>

namespace ParticleView {

void render(const std::vector<Particle>& particles,
            const VentTearsModel& ventTears,
            const glm::mat4& viewProj) {
    Draw2D::beginFrame(viewProj);

    for (const auto& p : particles) {
        if (!p.active) continue;
        float alpha = p.maxLifetime > 0.0f ? p.lifetime / p.maxLifetime : 0.0f;
        Draw2D::drawCircleFilled(p.position.x, p.position.y,
            p.size * alpha, p.color * alpha);
    }

    if (ventTears.visible) {
        for (int i = 0; i < 3; ++i) {
            float tearY = ventTears.position.y - 0.5f - static_cast<float>(i) * 0.3f -
                ventTears.animationTime * 2.0f;
            if (tearY > ventTears.position.y - 2.0f) {
                Draw2D::drawCircleFilled(
                    ventTears.position.x + ventTears.shakeX + static_cast<float>(i - 1) * 0.15f,
                    tearY,
                    0.03f,
                    glm::vec3(0.3f, 0.5f, 0.9f));
            }
        }
    }

    Draw2D::endFrame();
}

}  // namespace ParticleView
