#include "Game/Presentation/GameMenuView.h"

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/TextRenderer.h"

#include <algorithm>
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

namespace {

const char* pageName(GameMenuPage page) {
    switch (page) {
        case GameMenuPage::Quest: return "任务";
        case GameMenuPage::Character: return "属性";
        case GameMenuPage::Inventory: return "背包";
        case GameMenuPage::Partners: return "伙伴";
        case GameMenuPage::System: return "系统";
    }
    return "?";
}

const std::vector<std::string>& activeLines(const GameMenuView::Model& model) {
    switch (model.page) {
        case GameMenuPage::Quest: return model.questLines;
        case GameMenuPage::Character: return model.characterLines;
        case GameMenuPage::Inventory: return model.inventoryLines;
        case GameMenuPage::Partners: return model.partnerLines;
        case GameMenuPage::System: return model.systemLines;
    }
    return model.systemLines;
}

}  // namespace

namespace GameMenuView {

void render(const Model& model, int screenWidth, int screenHeight) {
    if (!model.open) return;

    float w = static_cast<float>(std::max(screenWidth, 800));
    float h = static_cast<float>(std::max(screenHeight, 600));
    glm::mat4 uiProj = glm::ortho(0.0f, w, 0.0f, h);

    Draw2D::beginFrame(uiProj);
    Draw2D::drawRectFilled(0.0f, 0.0f, w, h, glm::vec3(0.02f, 0.025f, 0.035f), 0.78f);

    float panelX = 72.0f;
    float panelY = 64.0f;
    float panelW = w - 144.0f;
    float panelH = h - 128.0f;
    Draw2D::drawRectFilled(panelX, panelY, panelW, panelH, glm::vec3(0.035f, 0.045f, 0.060f), 0.94f);
    Draw2D::drawRect(panelX, panelY, panelW, panelH, glm::vec3(0.28f, 0.55f, 0.88f), 2.0f, 0.88f);
    Draw2D::drawRectFilled(panelX, panelY + panelH - 5.0f, panelW, 5.0f, glm::vec3(1.0f, 0.76f, 0.28f), 0.9f);

    std::array<GameMenuPage, 5> pages = {
        GameMenuPage::Quest,
        GameMenuPage::Character,
        GameMenuPage::Inventory,
        GameMenuPage::Partners,
        GameMenuPage::System
    };

    float tabX = panelX + 24.0f;
    float tabY = panelY + panelH - 70.0f;
    for (GameMenuPage page : pages) {
        bool selected = page == model.page;
        glm::vec3 color = selected ? glm::vec3(0.20f, 0.45f, 0.78f) : glm::vec3(0.07f, 0.09f, 0.12f);
        Draw2D::drawRectFilled(tabX, tabY, 112.0f, 34.0f, color, selected ? 0.98f : 0.74f);
        Draw2D::drawRect(tabX, tabY, 112.0f, 34.0f,
            selected ? glm::vec3(0.82f, 0.92f, 1.0f) : glm::vec3(0.24f, 0.30f, 0.36f),
            1.5f, 0.85f);
        tabX += 124.0f;
    }
    Draw2D::endFrame();

    TextRenderer::drawText(uiProj, panelX + 26.0f, panelY + panelH - 42.0f,
        "星愿菜单", 24, glm::vec3(1.0f, 0.88f, 0.48f), 0.98f);

    tabX = panelX + 36.0f;
    for (GameMenuPage page : pages) {
        TextRenderer::drawText(uiProj, tabX, tabY + 9.0f,
            pageName(page), 16,
            page == model.page ? glm::vec3(1.0f) : glm::vec3(0.74f, 0.82f, 0.88f),
            0.98f);
        tabX += 124.0f;
    }

    float lineY = panelY + panelH - 118.0f;
    const std::vector<std::string>& lines = activeLines(model);
    for (const std::string& line : lines) {
        TextRenderer::drawText(uiProj, panelX + 36.0f, lineY,
            line, 17, glm::vec3(0.90f, 0.96f, 1.0f), 0.96f);
        lineY -= 26.0f;
        if (lineY < panelY + 34.0f) break;
    }
}

}  // namespace GameMenuView
