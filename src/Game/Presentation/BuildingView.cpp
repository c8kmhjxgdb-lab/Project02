#include "Game/Presentation/BuildingView.h"

#include "Engine/Renderer/Draw2D.h"
#include "Game/Building/BuildingSystem.h"

#include <algorithm>

namespace {

glm::ivec2 rotatedSize(const FurnitureDef& def, uint8_t rotation) {
    if ((rotation % 2) == 1) {
        return {def.size.y, def.size.x};
    }
    return def.size;
}

const FurnitureDef* findDef(const BuildingSystem& buildingSystem, const std::string& defId) {
    const auto& definitions = buildingSystem.getDefinitions();
    auto it = std::find_if(definitions.begin(), definitions.end(),
        [&defId](const FurnitureDef& def) { return def.id == defId; });
    return it == definitions.end() ? nullptr : &*it;
}

void drawFurnitureShape(const FurnitureDef& def,
                        const glm::vec2& pos,
                        const glm::vec2& size,
                        float alpha) {
    Draw2D::drawRectFilled(pos.x, pos.y, size.x, size.y, def.color, alpha);
    Draw2D::drawRect(pos.x, pos.y, size.x, size.y, def.accentColor, 0.035f, alpha);

    if (def.category == FurnitureCategory::Bed) {
        Draw2D::drawRectFilled(pos.x + 0.18f, pos.y + size.y * 0.54f,
                               size.x - 0.36f, size.y * 0.32f,
                               def.accentColor, alpha * 0.78f);
    } else if (def.category == FurnitureCategory::Light) {
        Draw2D::drawCircleFilled(pos.x + size.x * 0.5f, pos.y + size.y * 0.58f,
                                 std::min(size.x, size.y) * 0.26f,
                                 def.accentColor, alpha * 0.92f);
        Draw2D::drawCircle(pos.x + size.x * 0.5f, pos.y + size.y * 0.58f,
                           std::min(size.x, size.y) * 0.38f,
                           def.accentColor, 0.025f, 24, alpha * 0.5f);
    } else if (def.category == FurnitureCategory::Storage) {
        for (int i = 1; i < 3; ++i) {
            float x = pos.x + size.x * (static_cast<float>(i) / 3.0f);
            Draw2D::drawLine(x, pos.y + 0.12f, x, pos.y + size.y - 0.12f,
                             def.accentColor, 0.025f, alpha * 0.7f);
        }
    } else if (def.category == FurnitureCategory::Poster) {
        Draw2D::drawLine(pos.x + 0.15f, pos.y + 0.18f,
                         pos.x + size.x - 0.15f, pos.y + size.y - 0.18f,
                         def.accentColor, 0.03f, alpha * 0.8f);
    }
}

}  // namespace

namespace BuildingView {

void render(const BuildingSystem& buildingSystem, const TileMap& map, const glm::mat4& viewProj) {
    Draw2D::beginFrame(viewProj);
    for (const FurnitureInstance& instance : buildingSystem.getInstances()) {
        const FurnitureDef* def = findDef(buildingSystem, instance.defId);
        if (!def) continue;
        glm::ivec2 sizeTiles = rotatedSize(*def, instance.rotation);
        glm::vec2 pos(static_cast<float>(instance.tile.x) * map.tileSize,
                      static_cast<float>(instance.tile.y) * map.tileSize);
        glm::vec2 size(static_cast<float>(sizeTiles.x) * map.tileSize,
                       static_cast<float>(sizeTiles.y) * map.tileSize);
        drawFurnitureShape(*def, pos, size, 0.92f);
    }
    Draw2D::endFrame();
}

void renderPreview(const BuildingSystem& buildingSystem,
                   const TileMap& map,
                   const glm::mat4& viewProj,
                   const glm::vec2& mouseWorld) {
    if (!buildingSystem.isActive()) return;
    const FurnitureDef* def = buildingSystem.getPreviewDef();
    if (!def) return;

    glm::ivec2 tile = buildingSystem.getPreviewTile(map, mouseWorld);
    glm::ivec2 sizeTiles = rotatedSize(*def, buildingSystem.getPreviewRotation());
    glm::vec2 pos(static_cast<float>(tile.x) * map.tileSize,
                  static_cast<float>(tile.y) * map.tileSize);
    glm::vec2 size(static_cast<float>(sizeTiles.x) * map.tileSize,
                   static_cast<float>(sizeTiles.y) * map.tileSize);
    bool valid = buildingSystem.canPlacePreview(map, tile);

    Draw2D::beginFrame(viewProj);
    Draw2D::drawRectFilled(pos.x, pos.y, size.x, size.y,
                           valid ? glm::vec3(0.20f, 0.70f, 0.32f)
                                 : glm::vec3(0.90f, 0.24f, 0.18f),
                           0.25f);
    drawFurnitureShape(*def, pos, size, valid ? 0.55f : 0.38f);
    Draw2D::endFrame();
}

}  // namespace BuildingView
