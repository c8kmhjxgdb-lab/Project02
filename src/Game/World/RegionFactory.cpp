#include "Game/World/RegionFactory.h"

#include <unordered_map>

namespace {

struct RegionSpec {
    std::string name;
    RegionType type;
    int width;
    int height;
};

RegionSpec getRegionSpec(const std::string& regionId) {
    if (regionId == "home_base") {
        return {"秘密基地", RegionType::Indoor, 18, 14};
    }
    return {regionId, RegionType::Overworld, 60, 60};
}

int getRegionSeed(const std::string& regionId) {
    static const std::unordered_map<std::string, int> regionSeeds = {
        {"starter_village", 42},
        {"dark_forest", 12345},
        {"home_base", 20260607},
        {"mountain_pass", 67890},
        {"coastal_town", 11111},
    };

    auto it = regionSeeds.find(regionId);
    if (it != regionSeeds.end()) {
        return it->second;
    }

    unsigned int hash = 2166136261u;
    for (char c : regionId) {
        hash ^= static_cast<unsigned int>(c);
        hash *= 16777619u;
    }
    return static_cast<int>(hash);
}

void configureStarterVillage(MapRegion& region) {
    TileMap& map = region.getTileMap();

    for (int y = 2; y <= 10; ++y) {
        for (int x = 2; x <= 13; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Grass);
        }
    }
    for (int x = 2; x <= 18; ++x) {
        if (map.isInBounds(x, 8)) map.setTile(x, 8, TileType::Path);
    }
    for (int y = 5; y <= 10; ++y) {
        if (map.isInBounds(5, y)) map.setTile(5, y, TileType::Path);
    }
    for (int y = 3; y <= 6; ++y) {
        for (int x = 3; x <= 7; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Stone);
        }
    }
    if (map.isInBounds(5, 6)) map.setTile(5, 6, TileType::Door);
    if (map.isInBounds(5, 7)) map.setTile(5, 7, TileType::Path);
    if (map.isInBounds(8, 8)) map.setTile(8, 8, TileType::Path);

    PointOfInterest home;
    home.type = PointOfInterest::Type::Home;
    home.id = "player_home";
    home.displayName = "玩家之家 / Player Home";
    home.tilePos = {5, 7};
    home.metadata = 0;
    region.addPOI(home);

    PointOfInterest villageSign;
    villageSign.type = PointOfInterest::Type::Waypoint;
    villageSign.id = "starter_village_sign";
    villageSign.displayName = "新手村 / Starter Village";
    villageSign.tilePos = {8, 8};
    villageSign.metadata = 0;
    region.addPOI(villageSign);

    MapConnection forestConn;
    forestConn.direction = MapConnection::Direction::East;
    forestConn.targetRegionId = "dark_forest";
    forestConn.sourceTile = {59, 30};
    forestConn.targetTile = {0, 30};
    region.addConnection(forestConn);
}

void configureHomeBase(MapRegion& region) {
    TileMap& map = region.getTileMap();

    int doorX = map.width / 2;
    int doorY = map.height - 1;
    if (map.isInBounds(doorX, doorY)) {
        map.setTile(doorX, doorY, TileType::Portal);
    }
    if (map.isInBounds(doorX, doorY - 1)) {
        map.setTile(doorX, doorY - 1, TileType::Path);
    }

    for (int y = 5; y <= 8; ++y) {
        for (int x = 6; x <= 11; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Dirt);
        }
    }
    for (int x = 7; x <= 10; ++x) {
        if (map.isInBounds(x, 2)) map.setTile(x, 2, TileType::Stone);
    }

    PointOfInterest exit;
    exit.type = PointOfInterest::Type::Home;
    exit.id = "base_exit";
    exit.displayName = "返回新手村 / Back to Starter Village";
    exit.tilePos = {doorX, doorY - 1};
    exit.metadata = 0;
    region.addPOI(exit);
}

void configureRegionSpecials(MapRegion& region) {
    if (region.getId() == "starter_village") {
        configureStarterVillage(region);
    } else if (region.getId() == "home_base") {
        configureHomeBase(region);
    }
}

}  // namespace

namespace RegionFactory {

std::unique_ptr<MapRegion> createRegion(const std::string& regionId) {
    RegionSpec spec = getRegionSpec(regionId);
    auto region = std::make_unique<MapRegion>();
    region->generate(regionId, spec.name, spec.type, getRegionSeed(regionId), spec.width, spec.height);
    configureRegionSpecials(*region);
    return region;
}

}  // namespace RegionFactory
