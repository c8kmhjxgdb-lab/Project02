#pragma once

#include "Game/Building/BuildingSystem.h"
#include "Game/World/MapRegion.h"
#include "Game/World/RegionManager.h"

#include <glm/vec2.hpp>
#include <string>

namespace WorldQuery {

const PointOfInterest* findPOI(const MapRegion* region, const std::string& poiId);

bool isNearPOI(const MapRegion* region,
               const std::string& poiId,
               const glm::vec2& worldPos,
               float range);

bool isCurrentRegion(const RegionManager& regionManager, const std::string& regionId);

bool isPlayerAtHome(const RegionManager& regionManager,
                    const glm::vec2& fallbackHomePosition,
                    float homeRadius,
                    const glm::vec2& playerPos);

bool hasPlacedFurniture(const BuildingSystem& buildingSystem, const std::string& defId);

int countPlacedFurniture(const BuildingSystem& buildingSystem, const std::string& defId);

bool isNearFurniture(const BuildingSystem& buildingSystem,
                     const TileMap& map,
                     const std::string& defId,
                     const glm::vec2& worldPos,
                     float range);

}  // namespace WorldQuery
