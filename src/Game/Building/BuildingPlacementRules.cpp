#include "Game/Building/BuildingPlacementRules.h"

#include <algorithm>

namespace {

const FurnitureDef* findDef(const std::vector<FurnitureDef>& definitions, const std::string& defId) {
    auto it = std::find_if(definitions.begin(), definitions.end(),
        [&defId](const FurnitureDef& def) { return def.id == defId; });
    return it == definitions.end() ? nullptr : &*it;
}

bool overlapsDoor(const glm::ivec2& tile, const glm::ivec2& size) {
    // Reserve the home base exit tile and one step inside it.
    constexpr int doorX = 9;
    constexpr int doorY0 = 11;
    constexpr int doorY1 = 12;
    int x0 = tile.x;
    int y0 = tile.y;
    int x1 = tile.x + size.x;
    int y1 = tile.y + size.y;

    return doorX >= x0 && doorX < x1 && doorY0 >= y0 && doorY0 < y1 ||
           doorX >= x0 && doorX < x1 && doorY1 >= y0 && doorY1 < y1;
}

bool overlapsExisting(const FurnitureDef& def,
                      const glm::ivec2& tile,
                      uint8_t rotation,
                      const std::vector<FurnitureInstance>& instances,
                      const std::vector<FurnitureDef>& definitions,
                      int ignoreInstanceId) {
    glm::ivec2 size = BuildingPlacementRules::rotatedSize(def, rotation);
    int ax0 = tile.x;
    int ay0 = tile.y;
    int ax1 = tile.x + size.x;
    int ay1 = tile.y + size.y;

    for (const FurnitureInstance& instance : instances) {
        if (instance.instanceId == ignoreInstanceId) continue;
        const FurnitureDef* otherDef = findDef(definitions, instance.defId);
        if (!otherDef) continue;

        glm::ivec2 otherSize = BuildingPlacementRules::rotatedSize(*otherDef, instance.rotation);
        int bx0 = instance.tile.x;
        int by0 = instance.tile.y;
        int bx1 = instance.tile.x + otherSize.x;
        int by1 = instance.tile.y + otherSize.y;

        if (ax0 < bx1 && ax1 > bx0 && ay0 < by1 && ay1 > by0) {
            return true;
        }
    }

    return false;
}

}  // namespace

namespace BuildingPlacementRules {

glm::ivec2 rotatedSize(const FurnitureDef& def, uint8_t rotation) {
    if ((rotation % 2) == 1) {
        return {def.size.y, def.size.x};
    }
    return def.size;
}

bool containsTile(const FurnitureInstance& instance,
                  const FurnitureDef& def,
                  const glm::ivec2& tile) {
    glm::ivec2 size = rotatedSize(def, instance.rotation);
    return tile.x >= instance.tile.x && tile.x < instance.tile.x + size.x &&
           tile.y >= instance.tile.y && tile.y < instance.tile.y + size.y;
}

bool canPlace(const FurnitureDef& def,
              const TileMap& map,
              const glm::ivec2& tile,
              uint8_t rotation,
              const std::vector<FurnitureInstance>& instances,
              const std::vector<FurnitureDef>& definitions,
              int ignoreInstanceId) {
    glm::ivec2 size = rotatedSize(def, rotation);
    if (overlapsDoor(tile, size)) return false;

    for (int y = tile.y; y < tile.y + size.y; ++y) {
        for (int x = tile.x; x < tile.x + size.x; ++x) {
            if (!map.isInBounds(x, y)) return false;
            TileType tileType = map.getTile(x, y);
            if (!isWalkable(tileType) || tileType == TileType::Portal || tileType == TileType::Door) {
                return false;
            }
        }
    }

    return !overlapsExisting(def, tile, rotation, instances, definitions, ignoreInstanceId);
}

}  // namespace BuildingPlacementRules
