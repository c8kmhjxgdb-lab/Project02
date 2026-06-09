#include "Game/Presentation/AbilityView.h"

#include "Engine/Renderer/Draw2D.h"

#include <cmath>
#include <cstddef>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

namespace {

void renderShield(const Shield& shield, const glm::vec2& playerPos, float animationTime) {
    if (!shield.isActive()) return;

    float intensity = shield.getIntensity();
    if (intensity <= 0.01f) return;

    float radius = shield.getRadius();
    float rotation = shield.getRotationAngle();

    constexpr int dashCount = 10;
    float spacingAngle = glm::two_pi<float>() / static_cast<float>(dashCount);
    float dashAngle = spacingAngle * 0.56f;

    glm::vec3 shieldColor(0.28f, 0.72f, 1.0f);
    glm::vec3 coreColor(0.82f, 0.95f, 1.0f);
    float pulse = 0.75f + 0.25f * std::sin(animationTime * 8.0f);
    float lineWidth = 0.045f;

    Draw2D::drawCircleFilled(playerPos.x, playerPos.y, radius * 1.04f,
        shieldColor, 0.035f * intensity);
    Draw2D::drawCircle(playerPos.x, playerPos.y, radius * 1.10f,
        shieldColor, 0.030f, 48, 0.28f * intensity);
    Draw2D::drawCircle(playerPos.x, playerPos.y, radius * 0.78f,
        coreColor, 0.020f, 48, 0.18f * intensity);

    for (int d = 0; d < dashCount; ++d) {
        float startAngle = rotation + static_cast<float>(d) * spacingAngle;
        float endAngle = startAngle + dashAngle;

        constexpr int arcSegments = 7;
        for (int i = 0; i < arcSegments; ++i) {
            float a0 = startAngle + (endAngle - startAngle) * (i / static_cast<float>(arcSegments));
            float a1 = startAngle + (endAngle - startAngle) * ((i + 1) / static_cast<float>(arcSegments));

            glm::vec2 p0 = playerPos + glm::vec2(std::cos(a0), std::sin(a0)) * radius;
            glm::vec2 p1 = playerPos + glm::vec2(std::cos(a1), std::sin(a1)) * radius;

            Draw2D::drawLine(p0.x, p0.y, p1.x, p1.y, shieldColor, lineWidth * 2.0f, 0.18f * intensity);
            Draw2D::drawLine(p0.x, p0.y, p1.x, p1.y, coreColor, lineWidth, 0.78f * intensity);
        }

        float runeAngle = startAngle + dashAngle * 0.5f;
        glm::vec2 rune = playerPos + glm::vec2(std::cos(runeAngle), std::sin(runeAngle)) * (radius * 1.05f);
        float runeSize = 0.055f + pulse * 0.018f;
        Draw2D::drawCircleFilled(rune.x, rune.y, runeSize, coreColor, 0.62f * intensity);
        Draw2D::drawCircle(rune.x, rune.y, runeSize * 1.8f, shieldColor, 0.012f, 16, 0.42f * intensity);
    }

    for (int i = 0; i < 4; ++i) {
        float a = -rotation * 0.75f + static_cast<float>(i) * glm::half_pi<float>();
        glm::vec2 p = playerPos + glm::vec2(std::cos(a), std::sin(a)) * (radius * 0.58f);
        Draw2D::drawLine(playerPos.x, playerPos.y, p.x, p.y, coreColor, 0.014f, 0.20f * intensity);
    }
}

void renderFlightFog(const AbilityView::WorldEffectsModel& model) {
    if (!model.isFlying || model.flightHeight <= 2.0f || model.flightMaxHeight <= 0.0f) return;

    float fogAlpha = (model.flightHeight / model.flightMaxHeight) * 0.15f;
    float fogRadius = 3.0f + model.flightHeight;

    Draw2D::drawCircleFilled(model.playerPosition.x, model.playerPosition.y, fogRadius,
        glm::vec3(0.8f, 0.85f, 0.9f), fogAlpha);
}

void renderLightning(const Lightning& lightning, float animationTime) {
    if (!lightning.isActive()) return;

    const LightningChain& chain = lightning.getCurrentChain();
    if (chain.points.size() < 2) return;

    float flashIntensity = chain.lifetime > 0.0f ? chain.remainingTime / chain.lifetime : 0.0f;

    glm::vec3 glowColor(0.24f, 0.62f, 1.0f);
    glm::vec3 lightningColor(0.70f, 0.90f, 1.0f);
    glm::vec3 coreColor(1.0f, 1.0f, 0.92f);
    float lineWidth = 0.045f + 0.035f * flashIntensity;

    for (std::size_t i = 0; i < chain.points.size() - 1; ++i) {
        glm::vec2 start = chain.points[i];
        glm::vec2 end = chain.points[i + 1];
        glm::vec2 delta = end - start;
        if (glm::length(delta) < 0.001f) continue;

        glm::vec2 direction = glm::normalize(delta);
        glm::vec2 perpendicular(-direction.y, direction.x);

        constexpr int segments = 7;
        glm::vec2 prevPoint = start;
        for (int s = 0; s < segments; ++s) {
            float t = (s + 1) / static_cast<float>(segments);
            glm::vec2 targetPoint = start + (end - start) * t;

            float phase = std::sin(animationTime * 21.0f + static_cast<float>(i) * 3.7f + s * 1.9f);
            float offset = ((s % 2 == 0) ? 1.0f : -1.0f) *
                (0.10f + 0.08f * std::abs(phase)) * flashIntensity;
            glm::vec2 zigzagPoint = (prevPoint + targetPoint) * 0.5f + perpendicular * offset;

            Draw2D::drawLine(prevPoint.x, prevPoint.y, zigzagPoint.x, zigzagPoint.y,
                glowColor, lineWidth * 3.0f, 0.18f * flashIntensity);
            Draw2D::drawLine(zigzagPoint.x, zigzagPoint.y, targetPoint.x, targetPoint.y,
                glowColor, lineWidth * 3.0f, 0.18f * flashIntensity);
            Draw2D::drawLine(prevPoint.x, prevPoint.y, zigzagPoint.x, zigzagPoint.y,
                lightningColor, lineWidth * 1.5f, 0.58f * flashIntensity);
            Draw2D::drawLine(zigzagPoint.x, zigzagPoint.y, targetPoint.x, targetPoint.y,
                lightningColor, lineWidth * 1.5f, 0.58f * flashIntensity);
            Draw2D::drawLine(prevPoint.x, prevPoint.y, zigzagPoint.x, zigzagPoint.y,
                coreColor, lineWidth * 0.48f, 0.92f * flashIntensity);
            Draw2D::drawLine(zigzagPoint.x, zigzagPoint.y, targetPoint.x, targetPoint.y,
                coreColor, lineWidth * 0.48f, 0.92f * flashIntensity);

            if (s == 2 || s == 5) {
                float branchSide = (s == 2) ? 1.0f : -1.0f;
                glm::vec2 branchEnd = zigzagPoint + (perpendicular * branchSide + direction * 0.25f) *
                    (0.35f + 0.10f * flashIntensity);
                Draw2D::drawLine(zigzagPoint.x, zigzagPoint.y, branchEnd.x, branchEnd.y,
                    lightningColor, lineWidth * 0.70f, 0.42f * flashIntensity);
            }

            prevPoint = targetPoint;
        }
    }

    for (const auto& point : chain.points) {
        Draw2D::drawCircleFilled(point.x, point.y, 0.32f * flashIntensity,
            glowColor, flashIntensity * 0.20f);
        Draw2D::drawCircleFilled(point.x, point.y, 0.13f * flashIntensity,
            coreColor, flashIntensity * 0.72f);
        Draw2D::drawCircle(point.x, point.y, 0.30f * flashIntensity,
            lightningColor, 0.020f, 28, flashIntensity * 0.55f);
    }
}

void renderBondTechnique(const BondTechniqueSystem& bondTechnique, float animationTime) {
    if (!bondTechnique.isActive()) return;

    const BondTechnique& tech = bondTechnique.getCurrentTechnique();
    if (tech.waveFronts.empty()) return;

    float progress = tech.lifetime > 0.0f ? 1.0f - (tech.remainingTime / tech.lifetime) : 1.0f;
    float alpha = 0.42f * (1.0f - progress);
    glm::vec2 center = tech.waveFronts[0];
    glm::vec3 goldCore(1.0f, 0.92f, 0.55f);
    glm::vec3 roseGold(1.0f, 0.58f, 0.30f);

    Draw2D::drawCircle(center.x, center.y, tech.radius,
        goldCore, 0.080f * (1.0f - progress), 64, alpha);
    Draw2D::drawCircle(center.x, center.y, tech.radius * 0.78f,
        roseGold, 0.040f * (1.0f - progress), 64, alpha * 0.55f);
    Draw2D::drawCircle(center.x, center.y, tech.radius * 1.08f,
        glm::vec3(1.0f, 0.78f, 0.35f), 0.035f, 64, alpha * 0.35f);

    constexpr int rays = 16;
    for (int i = 0; i < rays; ++i) {
        float a = (static_cast<float>(i) / static_cast<float>(rays)) * glm::two_pi<float>() +
            animationTime * 0.9f;
        glm::vec2 p0 = center + glm::vec2(std::cos(a), std::sin(a)) * (tech.radius * 0.22f);
        glm::vec2 p1 = center + glm::vec2(std::cos(a), std::sin(a)) * (tech.radius * 0.96f);
        Draw2D::drawLine(p0.x, p0.y, p1.x, p1.y,
            goldCore, 0.018f, alpha * 0.35f);
    }

    if (progress < 0.5f) {
        Draw2D::drawCircleFilled(center.x, center.y, tech.radius * 0.50f,
            goldCore, alpha * 0.32f);
        Draw2D::drawCircleFilled(center.x, center.y, tech.radius * 0.22f,
            glm::vec3(1.0f, 1.0f, 0.82f), alpha * 0.45f);
    }
}

}  // namespace

namespace AbilityView {

void renderWorldEffects(const WorldEffectsModel& model) {
    if (model.shield) {
        renderShield(*model.shield, model.playerPosition, model.animationTime);
    }
    renderFlightFog(model);
    if (model.lightning) {
        renderLightning(*model.lightning, model.animationTime);
    }
    if (model.bondTechnique) {
        renderBondTechnique(*model.bondTechnique, model.animationTime);
    }
}

}  // namespace AbilityView
