#include "Game/Presentation/WorldSignView.h"

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/TextRenderer.h"

#include <glm/vec3.hpp>
#include <string>

namespace {

bool isGuidancePOI(const PointOfInterest& poi) {
    return poi.id == "player_home" ||
           poi.id == "starter_village_sign" ||
           poi.id == "base_exit" ||
           poi.id == "star_candy" ||
           poi.id == "childhood_crack" ||
           poi.id == "base_map_table" ||
           poi.id == "save_bed" ||
           poi.id == "pixel_controller_spot" ||
           poi.id == "arcade_gate" ||
           poi.id == "base_return_gate" ||
           poi.id == "tieyi_cage" ||
           poi.id == "arcade_boss_door";
}

std::string getGuidanceText(const PointOfInterest& poi) {
    if (poi.id == "player_home") {
        return "玩家之家\nPlayer Home\n按 E 进入 / Press E";
    }
    if (poi.id == "starter_village_sign") {
        return "新手村\nStarter Village";
    }
    if (poi.id == "base_exit") {
        return "返回小卖部\nBack to Street\n按 E 返回 / Press E";
    }
    if (poi.id == "star_candy") {
        return "星星糖\nStar Candy\n按 E 调查 / Press E";
    }
    if (poi.id == "childhood_crack") {
        return "童年裂缝\nChildhood Crack\n按 E 回基地 / Press E";
    }
    if (poi.id == "base_map_table") {
        return "地图桌\nMap Table\n第一章入口在右侧";
    }
    if (poi.id == "save_bed") {
        return "存档床\nSave Bed\nF5 保存 / F9 读取";
    }
    if (poi.id == "pixel_controller_spot") {
        return "像素手柄座\nController Spot\n通关后按 E 安放";
    }
    if (poi.id == "arcade_gate") {
        return "弹窗游乐厅\nPopup Arcade\n按 E 进入 / Press E";
    }
    if (poi.id == "base_return_gate") {
        return "返回秘密基地\nReturn Base\n按 E 返回 / Press E";
    }
    if (poi.id == "tieyi_cage") {
        return "铁翼牢笼\nTieyi Cage\n击败精英怪后按 E";
    }
    if (poi.id == "arcade_boss_door") {
        return "六元冠冕门\nCrown Door\n3 试玩币 + 铁翼";
    }
    return poi.displayName;
}

glm::vec3 boardFill(const PointOfInterest& poi) {
    if (poi.id == "star_candy" || poi.id == "pixel_controller_spot") {
        return glm::vec3(0.30f, 0.45f, 0.66f);
    }
    if (poi.id == "arcade_gate" || poi.id == "base_return_gate" ||
        poi.id == "childhood_crack" || poi.id == "arcade_boss_door") {
        return glm::vec3(0.24f, 0.22f, 0.42f);
    }
    if (poi.id == "tieyi_cage") {
        return glm::vec3(0.45f, 0.18f, 0.20f);
    }
    return glm::vec3(0.48f, 0.30f, 0.15f);
}

glm::vec3 boardInner(const PointOfInterest& poi) {
    if (poi.id == "star_candy" || poi.id == "pixel_controller_spot") {
        return glm::vec3(0.58f, 0.78f, 0.94f);
    }
    if (poi.id == "arcade_gate" || poi.id == "base_return_gate" ||
        poi.id == "childhood_crack" || poi.id == "arcade_boss_door") {
        return glm::vec3(0.58f, 0.52f, 0.86f);
    }
    if (poi.id == "tieyi_cage") {
        return glm::vec3(0.88f, 0.42f, 0.32f);
    }
    return glm::vec3(0.74f, 0.55f, 0.30f);
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
        float boardW = (poi.id == "starter_village_sign") ? 2.15f : 2.95f;
        float boardH = 0.78f;
        float boardX = pos.x - boardW * 0.5f;
        float boardY = pos.y + 0.60f;

        Draw2D::drawLine(pos.x - 0.42f, pos.y - 0.25f, pos.x - 0.42f, boardY,
                         glm::vec3(0.26f, 0.16f, 0.08f), 0.08f, 0.95f);
        Draw2D::drawLine(pos.x + 0.42f, pos.y - 0.25f, pos.x + 0.42f, boardY,
                         glm::vec3(0.26f, 0.16f, 0.08f), 0.08f, 0.95f);
        Draw2D::drawRectFilled(boardX, boardY, boardW, boardH,
                               boardFill(poi), 0.96f);
        Draw2D::drawRectFilled(boardX + 0.08f, boardY + 0.10f, boardW - 0.16f, boardH - 0.20f,
                               boardInner(poi), 0.95f);
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
