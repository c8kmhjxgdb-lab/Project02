#include "Game/Presentation/HudView.h"

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/TextRenderer.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

namespace {

float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

glm::vec3 tintColor(const glm::vec3& color, float amount) {
    return color + (glm::vec3(1.0f) - color) * amount;
}

glm::vec3 shadeColor(const glm::vec3& color, float amount) {
    return color * (1.0f - amount);
}

void drawBar(float x,
             float y,
             float w,
             float h,
             float pct,
             const glm::vec3& fill,
             const glm::vec3& icon) {
    pct = clamp01(pct);
    Draw2D::drawRectFilled(x, y, w, h, glm::vec3(0.05f, 0.06f, 0.07f), 0.72f);
    Draw2D::drawRectFilled(x, y, w * pct, h, fill, 0.92f);
    Draw2D::drawRectFilled(x, y + h * 0.55f, w * pct, h * 0.35f, tintColor(fill, 0.35f), 0.26f);
    Draw2D::drawRect(x, y, w, h, tintColor(fill, 0.28f), 2.0f, 0.85f);
    Draw2D::drawCircleFilled(x - 13.0f, y + h * 0.5f, 7.0f, icon, 0.94f);
    Draw2D::drawCircle(x - 13.0f, y + h * 0.5f, 10.0f, tintColor(icon, 0.22f), 2.0f, 24, 0.7f);
}

void drawAbilitySlot(float x,
                     float y,
                     float size,
                     float ready,
                     bool enabled,
                     const glm::vec3& accent,
                     int icon) {
    ready = clamp01(ready);
    glm::vec3 frame = enabled ? tintColor(accent, 0.18f) : glm::vec3(0.22f, 0.24f, 0.25f);
    glm::vec3 fill = enabled ? shadeColor(accent, 0.55f) : glm::vec3(0.08f, 0.09f, 0.10f);

    Draw2D::drawRectFilled(x, y, size, size, glm::vec3(0.03f, 0.04f, 0.05f), 0.74f);
    Draw2D::drawRectFilled(x + 4.0f, y + 4.0f, size - 8.0f, size - 8.0f, fill, enabled ? 0.82f : 0.46f);
    Draw2D::drawRect(x, y, size, size, frame, 2.0f, enabled ? 0.96f : 0.55f);

    float cx = x + size * 0.5f;
    float cy = y + size * 0.5f;
    glm::vec3 iconColor = enabled ? tintColor(accent, 0.55f) : glm::vec3(0.36f);

    if (icon == 0) {
        Draw2D::drawCircleFilled(cx, cy, size * 0.17f, iconColor, 0.95f);
        Draw2D::drawCircle(cx, cy, size * 0.25f, accent, 2.0f, 20, 0.5f);
    } else if (icon == 1) {
        Draw2D::drawLine(cx, y + size * 0.22f, cx, y + size * 0.78f, iconColor, 3.0f);
        Draw2D::drawLine(x + size * 0.25f, cy, x + size * 0.75f, cy, iconColor, 3.0f);
        Draw2D::drawLine(x + size * 0.31f, y + size * 0.31f,
                         x + size * 0.69f, y + size * 0.69f, iconColor, 2.0f);
        Draw2D::drawLine(x + size * 0.69f, y + size * 0.31f,
                         x + size * 0.31f, y + size * 0.69f, iconColor, 2.0f);
    } else if (icon == 2) {
        Draw2D::drawCircle(cx, cy, size * 0.24f, iconColor, 3.0f, 28, 0.92f);
        Draw2D::drawCircleFilled(cx, cy, size * 0.08f, iconColor, 0.65f);
    } else if (icon == 3) {
        Draw2D::drawLine(x + size * 0.34f, y + size * 0.76f,
                         x + size * 0.50f, y + size * 0.52f, iconColor, 4.0f);
        Draw2D::drawLine(x + size * 0.50f, y + size * 0.52f,
                         x + size * 0.42f, y + size * 0.52f, iconColor, 4.0f);
        Draw2D::drawLine(x + size * 0.42f, y + size * 0.52f,
                         x + size * 0.62f, y + size * 0.24f, iconColor, 4.0f);
    } else if (icon == 4) {
        Draw2D::drawCircleFilled(cx - size * 0.10f, cy, size * 0.11f, iconColor, 0.85f);
        Draw2D::drawCircleFilled(cx + size * 0.10f, cy, size * 0.11f, iconColor, 0.85f);
        Draw2D::drawCircle(cx, cy, size * 0.30f, iconColor, 2.0f, 28, 0.65f);
    } else {
        Draw2D::drawLine(cx, y + size * 0.20f, cx, y + size * 0.72f, iconColor, 2.0f);
        Draw2D::drawLine(cx, y + size * 0.58f, x + size * 0.24f, y + size * 0.42f, iconColor, 3.0f);
        Draw2D::drawLine(cx, y + size * 0.58f, x + size * 0.76f, y + size * 0.42f, iconColor, 3.0f);
    }

    if (ready < 0.999f) {
        Draw2D::drawRectFilled(x + 4.0f, y + 4.0f, size - 8.0f, (size - 8.0f) * (1.0f - ready),
                               glm::vec3(0.0f), 0.56f);
        Draw2D::drawRectFilled(x + 5.0f, y + 5.0f, (size - 10.0f) * ready, 4.0f,
                               accent, 0.82f);
    }
}

}  // namespace

namespace HudView {

void render(const Model& model, int screenWidth, int screenHeight) {
    float screenW = static_cast<float>(std::max(screenWidth, 800));
    float screenH = static_cast<float>(std::max(screenHeight, 600));
    glm::mat4 uiProj = glm::ortho(0.0f, screenW, 0.0f, screenH);

    Draw2D::beginFrame(uiProj);

    float panelX = 16.0f;
    float panelY = screenH - 132.0f;
    float panelW = 268.0f;
    float panelH = 116.0f;
    Draw2D::drawRectFilled(panelX, panelY, panelW, panelH, glm::vec3(0.035f, 0.045f, 0.055f), 0.68f);
    Draw2D::drawRect(panelX, panelY, panelW, panelH, glm::vec3(0.24f, 0.30f, 0.34f), 2.0f, 0.7f);
    Draw2D::drawRectFilled(panelX, panelY + panelH - 4.0f, panelW, 4.0f, glm::vec3(0.95f, 0.70f, 0.30f), 0.75f);

    float hpPercent = clamp01(model.healthPercent);
    glm::vec3 hpColor = hpPercent > 0.5f ? glm::vec3(0.20f, 0.76f, 0.34f) :
                       hpPercent > 0.25f ? glm::vec3(0.90f, 0.74f, 0.20f) :
                       glm::vec3(0.88f, 0.20f, 0.18f);
    drawBar(panelX + 34.0f, panelY + 78.0f, 216.0f, 18.0f, hpPercent, hpColor, glm::vec3(0.95f, 0.24f, 0.24f));
    drawBar(panelX + 34.0f, panelY + 52.0f, 216.0f, 14.0f, model.manaPercent, glm::vec3(0.20f, 0.42f, 0.92f), glm::vec3(0.28f, 0.54f, 1.0f));
    drawBar(panelX + 34.0f, panelY + 30.0f, 150.0f, 10.0f, model.flightPercent, glm::vec3(0.52f, 0.78f, 1.0f), glm::vec3(0.70f, 0.88f, 1.0f));
    drawBar(panelX + 34.0f, panelY + 12.0f, 150.0f, 8.0f, model.ultimatePercent, glm::vec3(1.0f, 0.74f, 0.24f), glm::vec3(1.0f, 0.86f, 0.36f));

    if (model.shieldActivePercent > 0.0f) {
        float shieldActive = clamp01(model.shieldActivePercent);
        Draw2D::drawCircle(panelX + 232.0f, panelY + 28.0f, 20.0f, glm::vec3(0.38f, 0.74f, 1.0f), 3.0f, 32, 0.72f);
        Draw2D::drawCircleFilled(panelX + 232.0f, panelY + 28.0f, 17.0f, glm::vec3(0.20f, 0.48f, 0.82f), 0.18f * shieldActive);
    }

    float coinX = panelX + panelW + 14.0f;
    float coinY = screenH - 58.0f;
    float coinW = 176.0f;
    float coinH = 40.0f;
    Draw2D::drawRectFilled(coinX, coinY, coinW, coinH, glm::vec3(0.045f, 0.045f, 0.040f), 0.74f);
    Draw2D::drawRect(coinX, coinY, coinW, coinH, glm::vec3(0.86f, 0.66f, 0.22f), 2.0f, 0.70f);
    Draw2D::drawCircleFilled(coinX + 22.0f, coinY + coinH * 0.5f, 11.0f, glm::vec3(1.0f, 0.78f, 0.22f), 0.95f);
    Draw2D::drawCircle(coinX + 22.0f, coinY + coinH * 0.5f, 14.0f, glm::vec3(1.0f, 0.90f, 0.44f), 2.0f, 24, 0.72f);

    const float slotSize = 46.0f;
    const float slotGap = 8.0f;
    const float slotCount = 6.0f;
    float slotsW = slotCount * slotSize + (slotCount - 1.0f) * slotGap;
    float slotsX = (screenW - slotsW) * 0.5f;
    float slotsY = 18.0f;
    Draw2D::drawRectFilled(slotsX - 10.0f, slotsY - 10.0f, slotsW + 20.0f, slotSize + 20.0f,
                           glm::vec3(0.025f, 0.030f, 0.035f), 0.62f);

    drawAbilitySlot(slotsX + (slotSize + slotGap) * 0.0f, slotsY, slotSize, model.fireReady,
                    model.canUseFireball, glm::vec3(1.0f, 0.44f, 0.12f), 0);
    drawAbilitySlot(slotsX + (slotSize + slotGap) * 1.0f, slotsY, slotSize, model.fireReady,
                    model.canUseIceSpike, glm::vec3(0.48f, 0.82f, 1.0f), 1);
    drawAbilitySlot(slotsX + (slotSize + slotGap) * 2.0f, slotsY, slotSize, model.shieldReady,
                    model.canUseShield, glm::vec3(0.32f, 0.68f, 1.0f), 2);
    drawAbilitySlot(slotsX + (slotSize + slotGap) * 3.0f, slotsY, slotSize, model.lightningReady,
                    model.canUseLightning, glm::vec3(0.76f, 0.90f, 1.0f), 3);
    drawAbilitySlot(slotsX + (slotSize + slotGap) * 4.0f, slotsY, slotSize, model.bondReady,
                    model.canUseBond, glm::vec3(1.0f, 0.76f, 0.22f), 4);
    drawAbilitySlot(slotsX + (slotSize + slotGap) * 5.0f, slotsY, slotSize, model.flightReady,
                    model.canUseFlight, glm::vec3(0.62f, 0.82f, 1.0f), 5);

    Draw2D::endFrame();

    TextRenderer::drawText(uiProj,
        coinX + 42.0f,
        coinY + 9.0f,
        "金币 Coins: " + std::to_string(model.coins),
        18,
        glm::vec3(1.0f, 0.90f, 0.56f),
        0.98f);

    TextRenderer::drawText(uiProj,
        screenW - 250.0f,
        screenH - 118.0f,
        model.statusText,
        14,
        glm::vec3(0.92f, 0.96f, 0.98f),
        0.94f);

    if (model.noticeTimer > 0.0f && !model.noticeText.empty()) {
        TextRenderer::drawText(uiProj,
            screenW - 330.0f,
            116.0f,
            model.noticeText,
            15,
            glm::vec3(1.0f, 0.86f, 0.46f),
            std::min(0.98f, model.noticeTimer * 0.35f));
    }
}

}  // namespace HudView
