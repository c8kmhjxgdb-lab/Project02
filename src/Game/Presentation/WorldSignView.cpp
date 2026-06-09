#include "Game/Presentation/WorldSignView.h"

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/TextRenderer.h"

#include <glm/vec3.hpp>
#include <string>

namespace {

bool isGuidancePOI(const PointOfInterest& poi) {
    return poi.id == "player_home" ||
           poi.id == "starter_village_sign" ||
           poi.id == "base_exit";
}

std::string getGuidanceText(const PointOfInterest& poi) {
    if (poi.id == "player_home") {
        return "玩家之家\nPlayer Home\n按 E 进入 / Press E";
    }
    if (poi.id == "starter_village_sign") {
        return "新手村\nStarter Village";
    }
    if (poi.id == "base_exit") {
        return "返回新手村\nBack to Village\n按 E 返回 / Press E";
    }
    return poi.displayName;
}

}  // namespace

namespace WorldSignView {

void renderBoards(const TileMap& map,
                  const std::vector<PointOfInterest>& pois,
                  const glm::mat4& viewProj) {
    Draw2D::beginFrame(viewProj);
    for (const auto& poi : pois) {
        if (!isGuidancePOI(poi)) continue;

        glm::vec2 pos = map.tileToWorld(poi.tilePos.x, poi.tilePos.y);
        float boardW = (poi.id == "starter_village_sign") ? 2.15f : 2.65f;
        float boardH = 0.78f;
        float boardX = pos.x - boardW * 0.5f;
        float boardY = pos.y + 0.60f;

        Draw2D::drawLine(pos.x - 0.42f, pos.y - 0.25f, pos.x - 0.42f, boardY,
                         glm::vec3(0.26f, 0.16f, 0.08f), 0.08f, 0.95f);
        Draw2D::drawLine(pos.x + 0.42f, pos.y - 0.25f, pos.x + 0.42f, boardY,
                         glm::vec3(0.26f, 0.16f, 0.08f), 0.08f, 0.95f);
        Draw2D::drawRectFilled(boardX, boardY, boardW, boardH,
                               glm::vec3(0.48f, 0.30f, 0.15f), 0.96f);
        Draw2D::drawRectFilled(boardX + 0.08f, boardY + 0.10f, boardW - 0.16f, boardH - 0.20f,
                               glm::vec3(0.74f, 0.55f, 0.30f), 0.95f);
        Draw2D::drawRect(boardX, boardY, boardW, boardH,
                         glm::vec3(0.22f, 0.12f, 0.06f), 0.035f, 0.88f);
    }
    Draw2D::endFrame();
}

void renderText(const TileMap& map,
                const std::vector<PointOfInterest>& pois,
                const Camera2D& camera,
                const glm::mat4& screenProj,
                int screenW,
                int screenH) {
    for (const auto& poi : pois) {
        if (!isGuidancePOI(poi)) continue;

        glm::vec2 pos = map.tileToWorld(poi.tilePos.x, poi.tilePos.y);
        glm::vec2 screen = camera.worldToScreen(pos.x, pos.y + 1.04f,
            static_cast<float>(screenW), static_cast<float>(screenH));
        if (screen.x < -120.0f || screen.x > screenW + 120.0f ||
            screen.y < -80.0f || screen.y > screenH + 80.0f) {
            continue;
        }

        float uiX = screen.x;
        float uiY = static_cast<float>(screenH) - screen.y;
        TextRenderer::drawTextCentered(screenProj,
            uiX,
            uiY,
            getGuidanceText(poi),
            13,
            glm::vec3(0.16f, 0.09f, 0.04f),
            0.96f,
            168);
    }
}

}  // namespace WorldSignView
