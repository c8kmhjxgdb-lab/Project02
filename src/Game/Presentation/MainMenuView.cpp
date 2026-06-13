#include "Game/Presentation/MainMenuView.h"

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/TextRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {

struct MenuLayout {
    float menuW = 260.0f;
    float menuX = 0.0f;
    float menuY = 154.0f;
    float itemH = 46.0f;
    float itemGap = 12.0f;
};

MenuLayout getMenuLayout(float screenW) {
    MenuLayout layout;
    layout.menuX = screenW - layout.menuW - 56.0f;
    return layout;
}

void drawPixelBlock(float x,
                    float y,
                    float unit,
                    int bx,
                    int by,
                    int bw,
                    int bh,
                    const glm::vec3& color,
                    float alpha = 1.0f) {
    Draw2D::drawRectFilled(
        x + static_cast<float>(bx) * unit,
        y + static_cast<float>(by) * unit,
        static_cast<float>(bw) * unit,
        static_cast<float>(bh) * unit,
        color,
        alpha);
}

void drawMenuHero(float animationTime, const glm::mat4& uiProj, float screenW, float screenH) {
    float unit = std::max(4.0f, std::min(screenW, screenH) / 120.0f);
    float baseX = 64.0f;
    float baseY = 86.0f;
    float t = animationTime;

    Draw2D::beginFrame(uiProj);

    Draw2D::drawRectGradient(0.0f, 0.0f, screenW, screenH,
        glm::vec3(0.08f, 0.13f, 0.21f),
        glm::vec3(0.22f, 0.13f, 0.23f),
        glm::vec3(0.16f, 0.28f, 0.26f),
        glm::vec3(0.34f, 0.18f, 0.20f));

    for (int i = 0; i < 54; ++i) {
        float sx = 24.0f + std::fmod(static_cast<float>(i * 97), screenW - 48.0f);
        float sy = 86.0f + std::fmod(static_cast<float>(i * 53), screenH - 120.0f);
        float twinkle = 0.35f + 0.35f * std::sin(t * 1.8f + static_cast<float>(i));
        float size = (i % 3 == 0) ? 3.0f : 2.0f;
        Draw2D::drawRectFilled(sx, sy, size, size,
            i % 2 == 0 ? glm::vec3(1.0f, 0.90f, 0.54f) : glm::vec3(0.65f, 0.88f, 1.0f),
            twinkle);
    }

    Draw2D::drawCircleFilled(screenW * 0.50f, screenH * 0.30f, screenW * 0.32f,
        glm::vec3(1.0f, 0.62f, 0.28f), 0.08f);
    Draw2D::drawLine(screenW * 0.42f, screenH * 0.24f,
                     screenW * 0.64f, screenH * 0.58f,
                     glm::vec3(1.0f, 0.70f, 0.30f), 8.0f, 0.34f);
    Draw2D::drawLine(screenW * 0.48f, screenH * 0.18f,
                     screenW * 0.74f, screenH * 0.52f,
                     glm::vec3(0.70f, 0.90f, 1.0f), 5.0f, 0.25f);

    float groundY = 64.0f;
    Draw2D::drawRectFilled(0.0f, 0.0f, screenW, groundY + 12.0f, glm::vec3(0.06f, 0.10f, 0.10f), 0.96f);
    Draw2D::drawRectFilled(0.0f, groundY + 12.0f, screenW, 6.0f, glm::vec3(0.78f, 0.50f, 0.22f), 0.82f);

    auto drawHero = [&](float x, float y, bool princess) {
        glm::vec3 outline(0.04f, 0.04f, 0.05f);
        glm::vec3 skin(0.95f, 0.74f, 0.57f);
        glm::vec3 hair = princess ? glm::vec3(0.45f, 0.23f, 0.16f) : glm::vec3(0.15f, 0.17f, 0.18f);
        glm::vec3 cloth = princess ? glm::vec3(0.92f, 0.48f, 0.70f) : glm::vec3(0.20f, 0.46f, 0.82f);
        glm::vec3 accent = princess ? glm::vec3(1.0f, 0.84f, 0.24f) : glm::vec3(1.0f, 0.72f, 0.26f);

        drawPixelBlock(x, y, unit, 4, 0, 3, 2, outline);
        drawPixelBlock(x, y, unit, 10, 0, 3, 2, outline);
        drawPixelBlock(x, y, unit, 5, 2, 7, 5, cloth);
        drawPixelBlock(x, y, unit, 4, 6, 9, 2, cloth * 1.08f);
        drawPixelBlock(x, y, unit, 3, 7, 2, 4, skin);
        drawPixelBlock(x, y, unit, 12, 7, 2, 4, skin);
        drawPixelBlock(x, y, unit, 5, 8, 7, 6, skin);
        drawPixelBlock(x, y, unit, 4, 11, 9, 4, hair);
        drawPixelBlock(x, y, unit, 5, 9, 2, 2, hair);
        drawPixelBlock(x, y, unit, 10, 9, 2, 2, hair);
        drawPixelBlock(x, y, unit, 6, 10, 1, 1, outline);
        drawPixelBlock(x, y, unit, 10, 10, 1, 1, outline);
        if (princess) {
            drawPixelBlock(x, y, unit, 6, 15, 1, 2, accent);
            drawPixelBlock(x, y, unit, 8, 16, 1, 2, accent);
            drawPixelBlock(x, y, unit, 10, 15, 1, 2, accent);
        } else {
            drawPixelBlock(x, y, unit, 12, 5, 5, 1, accent);
            drawPixelBlock(x, y, unit, 16, 6, 1, 4, accent);
        }
    };

    auto drawVillain = [&](float x, float y) {
        glm::vec3 cloak(0.20f, 0.08f, 0.13f);
        glm::vec3 shadow(0.04f, 0.03f, 0.04f);
        glm::vec3 horn(0.62f, 0.64f, 0.70f);
        glm::vec3 eye(1.0f, 0.18f, 0.16f);

        drawPixelBlock(x, y, unit, 2, 0, 12, 3, shadow);
        drawPixelBlock(x, y, unit, 1, 3, 14, 9, cloak);
        drawPixelBlock(x, y, unit, 4, 9, 8, 6, shadow);
        drawPixelBlock(x, y, unit, 2, 13, 3, 4, horn);
        drawPixelBlock(x, y, unit, 11, 13, 3, 4, horn);
        drawPixelBlock(x, y, unit, 5, 11, 2, 1, eye);
        drawPixelBlock(x, y, unit, 9, 11, 2, 1, eye);
        drawPixelBlock(x, y, unit, 4, 6, 8, 1, glm::vec3(0.58f, 0.16f, 0.22f));
    };

    drawHero(baseX, baseY, false);
    drawHero(baseX + unit * 18.0f, baseY + unit * 2.0f, true);
    drawVillain(screenW - 220.0f, baseY + unit * 4.0f);

    Draw2D::drawRectFilled(screenW * 0.50f - 32.0f, baseY + unit * 13.0f,
                           64.0f, 10.0f, glm::vec3(1.0f, 0.82f, 0.30f), 0.92f);
    Draw2D::drawRectFilled(screenW * 0.50f - 5.0f, baseY + unit * 9.0f,
                           10.0f, 64.0f, glm::vec3(0.75f, 0.92f, 1.0f), 0.84f);

    Draw2D::endFrame();
}

}  // namespace

namespace MainMenuView {

void render(const Model& model, const glm::mat4& uiProj, int screenW, int screenH) {
    float sw = static_cast<float>(std::max(screenW, 800));
    float sh = static_cast<float>(std::max(screenH, 600));
    drawMenuHero(model.animationTime, uiProj, sw, sh);

    TextRenderer::drawText(uiProj,
        56.0f,
        sh - 112.0f,
        "星愿之子",
        44,
        glm::vec3(1.0f, 0.88f, 0.44f),
        0.98f);
    TextRenderer::drawText(uiProj,
        60.0f,
        sh - 148.0f,
        "我会长大，但不交出童心",
        22,
        glm::vec3(0.78f, 0.92f, 1.0f),
        0.92f);

    const std::array<std::string, kMenuItemCount> labels = {
        "新游戏  New Game",
        model.hasSave ? "读取存档  Load Save" : "读取存档  No Save",
        "退出  Exit"
    };

    MenuLayout layout = getMenuLayout(sw);

    Draw2D::beginFrame(uiProj);
    Draw2D::drawRectFilled(layout.menuX - 18.0f, layout.menuY - 22.0f,
        layout.menuW + 36.0f, layout.itemH * 3.0f + layout.itemGap * 2.0f + 44.0f,
        glm::vec3(0.04f, 0.05f, 0.07f), 0.72f);
    Draw2D::drawRect(layout.menuX - 18.0f, layout.menuY - 22.0f,
        layout.menuW + 36.0f, layout.itemH * 3.0f + layout.itemGap * 2.0f + 44.0f,
        glm::vec3(0.78f, 0.60f, 0.30f), 2.0f, 0.72f);

    for (int i = 0; i < kMenuItemCount; ++i) {
        float y = layout.menuY + static_cast<float>(2 - i) * (layout.itemH + layout.itemGap);
        bool selected = model.selectedIndex == i;
        bool disabled = (i == 1 && !model.hasSave);
        glm::vec3 fill = selected ? glm::vec3(0.24f, 0.22f, 0.16f) : glm::vec3(0.08f, 0.09f, 0.10f);
        if (disabled) fill = glm::vec3(0.06f, 0.06f, 0.065f);
        Draw2D::drawRectFilled(layout.menuX, y, layout.menuW, layout.itemH, fill, selected ? 0.94f : 0.82f);
        Draw2D::drawRect(layout.menuX, y, layout.menuW, layout.itemH,
            selected ? glm::vec3(1.0f, 0.74f, 0.30f) : glm::vec3(0.32f, 0.36f, 0.38f),
            2.0f,
            disabled ? 0.35f : 0.82f);
        if (selected) {
            Draw2D::drawRectFilled(layout.menuX + 10.0f, y + 10.0f, 5.0f, layout.itemH - 20.0f,
                glm::vec3(1.0f, 0.78f, 0.30f), disabled ? 0.40f : 0.95f);
        }
    }
    Draw2D::endFrame();

    for (int i = 0; i < kMenuItemCount; ++i) {
        float y = layout.menuY + static_cast<float>(2 - i) * (layout.itemH + layout.itemGap);
        bool disabled = (i == 1 && !model.hasSave);
        TextRenderer::drawText(uiProj,
            layout.menuX + 28.0f,
            y + 13.0f,
            labels[static_cast<size_t>(i)],
            18,
            disabled ? glm::vec3(0.44f, 0.47f, 0.50f) : glm::vec3(0.94f, 0.96f, 0.92f),
            disabled ? 0.70f : 0.98f);
    }

    std::string hint = "W/S 或 方向键选择，Enter 确认";
    if (!model.saveTimestamp.empty()) {
        hint += "\n继续旅程 Continue";
        hint += "\n时间: " + model.saveTimestamp;
        if (!model.saveRegionName.empty()) {
            hint += "\n区域: " + model.saveRegionName;
        }
        hint += "\n童心: " + std::to_string(static_cast<int>(model.childlikeHeart));
        hint += "\n伙伴: " + std::to_string(model.rescuedPartners);
    }
    TextRenderer::drawText(uiProj,
        layout.menuX - 8.0f,
        layout.menuY - 74.0f,
        hint,
        14,
        glm::vec3(0.72f, 0.82f, 0.86f),
        0.88f,
        300);

    if (model.messageTimer > 0.0f && !model.message.empty()) {
        TextRenderer::drawTextCentered(uiProj,
            sw * 0.5f,
            112.0f,
            model.message,
            18,
            glm::vec3(1.0f, 0.72f, 0.36f),
            std::min(0.98f, model.messageTimer * 0.5f),
            420);
    }
}

int hitTest(float screenX, float screenYFromTop, int screenW, int screenH) {
    float sw = static_cast<float>(std::max(screenW, 800));
    float sh = static_cast<float>(std::max(screenH, 600));
    MenuLayout layout = getMenuLayout(sw);
    float uiY = sh - screenYFromTop;

    for (int i = 0; i < kMenuItemCount; ++i) {
        float y = layout.menuY + static_cast<float>(2 - i) * (layout.itemH + layout.itemGap);
        if (screenX >= layout.menuX && screenX <= layout.menuX + layout.menuW &&
            uiY >= y && uiY <= y + layout.itemH) {
            return i;
        }
    }

    return -1;
}

}  // namespace MainMenuView
