#pragma once

#include "Game/Building/Furniture.h"
#include "Game/World/TileMap.h"

#include <glm/vec2.hpp>
#include <vector>

namespace BuildingPlacementRules {

glm::ivec2 rotatedSize(const FurnitureDef& def, uint8_t rotation);

bool containsTile(const FurnitureInstance& instance,
                  const FurnitureDef& def,
                  const glm::ivec2& tile);

bool canPlace(const FurnitureDef& def,
              const TileMap& map,
              const glm::ivec2& tile,
              uint8_t rotation,
              const std::vector<FurnitureInstance>& instances,
              const std::vector<FurnitureDef>& definitions,
              int ignoreInstanceId = 0);

}  // namespace BuildingPlacementRules
