#include "Game/World/RegionFactory.h"
#include "TestSupport.h"

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
    return 0;
}
