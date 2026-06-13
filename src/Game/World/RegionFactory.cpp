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
    if (regionId == "real_street_prologue") {
        return {"小卖部门口", RegionType::Overworld, 36, 28};
    }
    if (regionId == "home_base") {
        return {"秘密基地", RegionType::Indoor, 24, 18};
    }
    if (regionId == "popup_arcade") {
        return {"弹窗游乐厅", RegionType::Dungeon, 60, 60};
    }
    return {regionId, RegionType::Overworld, 60, 60};
}

int getRegionSeed(const std::string& regionId) {
    static const std::unordered_map<std::string, int> regionSeeds = {
        {"starter_village", 42},
        {"real_street_prologue", 20260610},
        {"dark_forest", 12345},
        {"home_base", 20260607},
        {"popup_arcade", 20260611},
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

void addPoi(MapRegion& region,
            PointOfInterest::Type type,
            const std::string& id,
            const std::string& displayName,
            const glm::ivec2& tilePos) {
    PointOfInterest poi;
    poi.type = type;
    poi.id = id;
    poi.displayName = displayName;
    poi.tilePos = tilePos;
    poi.metadata = 0;
    region.addPOI(poi);
}

void addConnection(MapRegion& region,
                   MapConnection::Direction direction,
                   const std::string& targetRegionId,
                   const glm::ivec2& sourceTile,
                   const glm::ivec2& targetTile) {
    MapConnection connection;
    connection.direction = direction;
    connection.targetRegionId = targetRegionId;
    connection.sourceTile = sourceTile;
    connection.targetTile = targetTile;
    region.addConnection(connection);
}

void configureRealStreetPrologue(MapRegion& region) {
    TileMap& map = region.getTileMap();

    for (int y = 2; y < map.height - 2; ++y) {
        for (int x = 2; x < map.width - 2; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Grass);
        }
    }
    for (int x = 3; x <= 31; ++x) {
        if (map.isInBounds(x, 14)) map.setTile(x, 14, TileType::Path);
    }
    for (int y = 8; y <= 20; ++y) {
        if (map.isInBounds(18, y)) map.setTile(18, y, TileType::Path);
    }
    for (int y = 5; y <= 9; ++y) {
        for (int x = 5; x <= 12; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Stone);
        }
    }
    if (map.isInBounds(18, 26)) map.setTile(18, 26, TileType::Portal);
    if (map.isInBounds(18, 25)) map.setTile(18, 25, TileType::Path);
    if (map.isInBounds(18, 24)) map.setTile(18, 24, TileType::Path);
    if (map.isInBounds(18, 23)) map.setTile(18, 23, TileType::Path);

    addPoi(region, PointOfInterest::Type::Quest,
        "star_candy", "星星糖 / Star Candy", {10, 14});
    addPoi(region, PointOfInterest::Type::NPC_Spawn,
        "shopkeeper_shadow", "小卖部影子 / Shopkeeper Shadow", {8, 10});
    addPoi(region, PointOfInterest::Type::Teleport,
        "childhood_crack", "童年裂缝 / Childhood Crack", {18, 25});

    addConnection(region, MapConnection::Direction::South,
        "home_base", {18, 26}, {12, 14}); // targetTile 从 {12, 15} 改为 {12, 14}，避开 base_exit
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

    for (int y = 5; y <= 12; ++y) {
        for (int x = 5; x <= 18; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Dirt);
        }
    }
    for (int x = 7; x <= 16; ++x) {
        if (map.isInBounds(x, 2)) map.setTile(x, 2, TileType::Stone);
    }

    if (map.isInBounds(20, 9)) map.setTile(20, 9, TileType::Portal);
    if (map.isInBounds(19, 9)) map.setTile(19, 9, TileType::Path);

    addPoi(region, PointOfInterest::Type::Home,
        "base_exit", "返回小卖部门口 / Back to Street", {doorX, doorY - 1});
    addPoi(region, PointOfInterest::Type::Waypoint,
        "base_map_table", "基地地图桌 / Map Table", {7, 11});
    addPoi(region, PointOfInterest::Type::Home,
        "save_bed", "存档床 / Save Bed", {4, 4});
    addPoi(region, PointOfInterest::Type::Quest,
        "pixel_controller_spot", "像素手柄摆放处 / Controller Spot", {12, 8});
    addPoi(region, PointOfInterest::Type::Teleport,
        "arcade_gate", "弹窗游乐厅入口 / Popup Arcade Gate", {19, 9});

    addConnection(region, MapConnection::Direction::East,
        "popup_arcade", {20, 9}, {30, 54}); // targetTile 从 {30, 53} 改为 {30, 54} 远离 base_return_gate {30, 57}
}

void configurePopupArcade(MapRegion& region) {
    TileMap& map = region.getTileMap();

    // 内部使用 Dirt 作为可行走地板（不创建物理刚体），避免大量 Stone
    // 瓦片产生数千个 Box2D 静态刚体导致 FPS 骤降。
    for (int y = 1; y < map.height - 1; ++y) {
        for (int x = 1; x < map.width - 1; ++x) {
            map.setTile(x, y, TileType::Dirt);
        }
    }

    // 先设置主路径区域
    for (int y = 48; y <= 57; ++y) {
        for (int x = 24; x <= 36; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Path);
        }
    }
    for (int y = 26; y <= 47; ++y) {
        for (int x = 22; x <= 38; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Path);
        }
    }
    for (int y = 20; y <= 34; ++y) {
        for (int x = 40; x <= 55; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Dirt);
        }
    }
    for (int y = 4; y <= 18; ++y) {
        for (int x = 18; x <= 42; ++x) {
            if (map.isInBounds(x, y)) map.setTile(x, y, TileType::Path);
        }
    }

    // 在各区域的交界处放置少量 Stone 掩体（在路径区域覆盖之后放置）
    auto placeStone = [&map](int x, int y) {
        if (map.isInBounds(x, y) && map.getTile(x, y) != TileType::Wall) {
            map.setTile(x, y, TileType::Stone);
        }
    };
    for (int y = 4; y <= 18; ++y) {
        placeStone(18, y);   // 北区左边界石柱
        placeStone(43, y);   // 北区右边界石柱
    }
    for (int y = 20; y <= 34; ++y) {
        placeStone(39, y);   // 东区左边界石柱
        placeStone(56, y);   // 东区右边界石柱
    }
    for (int y = 26; y <= 47; ++y) {
        placeStone(21, y);   // 中区左边界石柱
        placeStone(39, y);   // 中区右边界石柱
    }
    for (int y = 48; y <= 57; ++y) {
        placeStone(23, y);   // 南区左边界石柱
        placeStone(37, y);   // 南区右边界石柱
    }

    if (map.isInBounds(30, 58)) map.setTile(30, 58, TileType::Portal);
    if (map.isInBounds(30, 4)) map.setTile(30, 4, TileType::Portal);

    addPoi(region, PointOfInterest::Type::NPC_Spawn,
        "trapped_player_shadow", "被困玩家影子 / Trapped Shadow", {28, 48});
    addPoi(region, PointOfInterest::Type::Shop,
        "popup_vendor", "弹窗商贩 / Popup Vendor", {34, 45});
    addPoi(region, PointOfInterest::Type::Treasure,
        "trial_token_1", "试玩币 1 / Trial Token 1", {24, 34});
    addPoi(region, PointOfInterest::Type::Treasure,
        "trial_token_2", "试玩币 2 / Trial Token 2", {36, 32});
    addPoi(region, PointOfInterest::Type::Treasure,
        "trial_token_3", "试玩币 3 / Trial Token 3", {49, 25});
    addPoi(region, PointOfInterest::Type::Quest,
        "tieyi_cage", "铁翼牢笼 / Tieyi Cage", {52, 30});
    addPoi(region, PointOfInterest::Type::Waypoint,
        "gray_bureau_notice", "灰色管理局公告 / Gray Bureau Notice", {43, 21});
    addPoi(region, PointOfInterest::Type::Dungeon,
        "arcade_boss_door", "六元冠冕门 / Crown Door", {30, 20});
    addPoi(region, PointOfInterest::Type::Dungeon,
        "popup_crown_arena", "六元冠冕竞技场 / Popup Crown Arena", {30, 9});
    addPoi(region, PointOfInterest::Type::Teleport,
        "base_return_gate", "返回秘密基地 / Return to Base", {30, 57});

    addConnection(region, MapConnection::Direction::South,
        "home_base", {30, 58}, {17, 9}); // 避开 arcade_gate {19, 9} 的手动交互范围和自动连接范围
}

void configureRegionSpecials(MapRegion& region) {
    if (region.getId() == "starter_village") {
        configureStarterVillage(region);
    } else if (region.getId() == "real_street_prologue") {
        configureRealStreetPrologue(region);
    } else if (region.getId() == "home_base") {
        configureHomeBase(region);
    } else if (region.getId() == "popup_arcade") {
        configurePopupArcade(region);
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
