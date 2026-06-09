#include "Game/Presentation/AimReticleView.h"

#include "Engine/Renderer/Draw2D.h"

#include <glm/vec3.hpp>

namespace AimReticleView {

void render(const Model& model, const glm::mat4& viewProj) {
    if (!model.visible) return;

    glm::vec2 guideStart = model.playerPosition + model.aimDirection * 0.55f;
    glm::vec3 color(0.92f, 0.78f, 0.34f);
    const glm::vec2& target = model.targetPosition;

    Draw2D::beginFrame(viewProj);
    Draw2D::drawLine(guideStart.x, guideStart.y, target.x, target.y, color, 0.018f, 0.22f);
    Draw2D::drawCircle(target.x, target.y, 0.22f, color, 0.025f, 28, 0.78f);
    Draw2D::drawLine(target.x - 0.32f, target.y, target.x - 0.10f, target.y, color, 0.018f, 0.78f);
    Draw2D::drawLine(target.x + 0.10f, target.y, target.x + 0.32f, target.y, color, 0.018f, 0.78f);
    Draw2D::drawLine(target.x, target.y - 0.32f, target.x, target.y - 0.10f, color, 0.018f, 0.78f);
    Draw2D::drawLine(target.x, target.y + 0.10f, target.x, target.y + 0.32f, color, 0.018f, 0.78f);
    Draw2D::endFrame();
}

}  // namespace AimReticleView
