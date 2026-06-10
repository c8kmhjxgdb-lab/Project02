#include "Game/Services/WorldQuery.h"

#include <glm/geometric.hpp>

namespace WorldQuery {

const PointOfInterest* findPOI(const MapRegion* region, const std::string& poiId) {
    if (!region) return nullptr;
    for (const auto& poi : region->getPOIs()) {
        if (poi.id == poiId) return &poi;
    }
    return nullptr;
}

bool tryGetPOIWorldPosition(const MapRegion* region,
                            const std::string& poiId,
                            glm::vec2& outPosition) {
    const PointOfInterest* poi = findPOI(region, poiId);
    if (!region || !poi) return false;
    outPosition = region->getTileMap().tileToWorld(poi->tilePos.x, poi->tilePos.y);
    return true;
}

bool isNearPOI(const MapRegion* region,
               const std::string& poiId,
               const glm::vec2& worldPos,
               float range) {
    const PointOfInterest* poi = findPOI(region, poiId);
    if (!poi) return false;
    glm::vec2 poiWorld = region->getTileMap().tileToWorld(poi->tilePos.x, poi->tilePos.y);
    return glm::distance(worldPos, poiWorld) <= range;
}

bool isCurrentRegion(const RegionManager& regionManager, const std::string& regionId) {
    const MapRegion* region = regionManager.getCurrentRegion();
    return region && region->getId() == regionId;
}

bool isPlayerAtHome(const RegionManager& regionManager,
                    const glm::vec2& fallbackHomePosition,
                    float homeRadius,
                    const glm::vec2& playerPos) {
    const MapRegion* region = regionManager.getCurrentRegion();
    if (!region) {
        return fallbackHomePosition != glm::vec2(0, 0) &&
               glm::distance(playerPos, fallbackHomePosition) <= homeRadius;
    }

    if (region->getId() == "home_base") return true;

    if (region->getId() == "starter_village") {
        return isNearPOI(region, "player_home", playerPos, homeRadius);
    }

    return false;
}

bool hasPlacedFurniture(const BuildingSystem& buildingSystem, const std::string& defId) {
    for (const FurnitureInstance& instance : buildingSystem.getInstances()) {
        if (instance.defId == defId) return true;
    }
    return false;
}

int countPlacedFurniture(const BuildingSystem& buildingSystem, const std::string& defId) {
    int count = 0;
    for (const FurnitureInstance& instance : buildingSystem.getInstances()) {
        if (instance.defId == defId) {
            ++count;
        }
    }
    return count;
}

bool isNearFurniture(const BuildingSystem& buildingSystem,
                     const TileMap& map,
                     const std::string& defId,
                     const glm::vec2& worldPos,
                     float range) {
    for (const FurnitureInstance& instance : buildingSystem.getInstances()) {
        if (instance.defId != defId) continue;
        glm::vec2 center = map.tileToWorld(instance.tile.x, instance.tile.y);
        center.x += map.tileSize * 0.5f;
        center.y += map.tileSize * 0.5f;
        if (glm::distance(center, worldPos) <= range) {
            return true;
        }
    }
    return false;
}

}  // namespace WorldQuery
