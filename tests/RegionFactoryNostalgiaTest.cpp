#include "Game/World/RegionFactory.h"
#include "TestSupport.h"

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

#include <string>

namespace {

bool hasPoi(const MapRegion& region, const std::string& id) {
    for (const PointOfInterest& poi : region.getPOIs()) {
        if (poi.id == id) return true;
    }
    return false;
}

bool hasConnectionTo(const MapRegion& region, const std::string& targetId) {
    for (const MapConnection& connection : region.getConnections()) {
        if (connection.targetRegionId == targetId) return true;
    }
    return false;
}

const MapConnection* findConnectionTo(const MapRegion& region, const std::string& targetId) {
    for (const MapConnection& connection : region.getConnections()) {
        if (connection.targetRegionId == targetId) return &connection;
    }
    return nullptr;
}

const PointOfInterest* findPoi(const MapRegion& region, const std::string& id) {
    for (const PointOfInterest& poi : region.getPOIs()) {
        if (poi.id == id) return &poi;
    }
    return nullptr;
}

bool targetTileOutsidePoiRange(const MapConnection& connection,
                               const PointOfInterest& poi,
                               float range) {
    glm::vec2 target(static_cast<float>(connection.targetTile.x),
                     static_cast<float>(connection.targetTile.y));
    glm::vec2 poiTile(static_cast<float>(poi.tilePos.x),
                      static_cast<float>(poi.tilePos.y));
    return glm::distance(target, poiTile) > range;
}

}  // namespace

int main() {
    auto prologue = RegionFactory::createRegion("real_street_prologue");
    TestSupport::require(prologue->getName() == "小卖部门口", "prologue region name");
    TestSupport::require(hasPoi(*prologue, "star_candy"), "prologue has star candy POI");
    TestSupport::require(hasConnectionTo(*prologue, "home_base"), "prologue connects to home base");

    auto base = RegionFactory::createRegion("home_base");
    TestSupport::require(hasPoi(*base, "base_map_table"), "home base has map table");
    TestSupport::require(hasPoi(*base, "save_bed"), "home base has save bed");
    TestSupport::require(hasConnectionTo(*base, "popup_arcade"), "home base connects to popup arcade");

    auto arcade = RegionFactory::createRegion("popup_arcade");
    TestSupport::require(arcade->getName() == "弹窗游乐厅", "arcade region name");
    TestSupport::require(hasPoi(*arcade, "arcade_boss_door"), "arcade has boss door");
    TestSupport::require(hasPoi(*arcade, "tieyi_cage"), "arcade has Tieyi cage");
    const MapConnection* arcadeEntry = findConnectionTo(*base, "popup_arcade");
    const PointOfInterest* returnGate = findPoi(*arcade, "base_return_gate");
    TestSupport::require(arcadeEntry != nullptr, "home base connects to popup arcade");
    TestSupport::require(returnGate != nullptr, "popup arcade has base return gate");
    TestSupport::require(
        targetTileOutsidePoiRange(*arcadeEntry, *returnGate, 1.8f),
        "arcade entry connection target avoids the manual return gate range");
    const MapConnection* arcadeReturn = findConnectionTo(*arcade, "home_base");
    const PointOfInterest* arcadeGate = findPoi(*base, "arcade_gate");
    TestSupport::require(arcadeReturn != nullptr, "arcade connects back to home base");
    TestSupport::require(arcadeGate != nullptr, "home base has arcade gate");
    TestSupport::require(
        targetTileOutsidePoiRange(*arcadeReturn, *arcadeGate, 1.6f),
        "arcade return connection target avoids the manual arcade gate range");
    TestSupport::require(
        getTileDef(TileType::Portal).type == TileType::Portal,
        "portal tile definition is indexed by its explicit enum value");
    TestSupport::require(
        getTileDef(TileType::Portal).passability == Passability::Portal,
        "portal tile keeps portal passability");
    TestSupport::require(
        getTileDef(TileType::Door).type == TileType::Door,
        "door tile definition is indexed by its explicit enum value");
    TestSupport::require(
        getTileDef(TileType::Lava).damagePerSecond > 0.0f,
        "lava tile keeps damage definition");
    TestSupport::require(
        getTileDef(TileType::Snow).type == TileType::Snow,
        "snow tile definition is indexed by its explicit enum value");
    return 0;
}
