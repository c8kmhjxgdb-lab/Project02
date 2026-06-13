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
        return {"弹窗游乐厅", RegionType::Dungeon, 24, 20};
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

    // arcade_gate 传送门(东墙,x=map.width-1=23)
    if (map.isInBounds(doorX + 5, doorY - 1)) map.setTile(doorX + 5, doorY - 1, TileType::Portal);
    if (map.isInBounds(doorX + 5, doorY - 2)) map.setTile(doorX + 5, doorY - 2, TileType::Path);

    addPoi(region, PointOfInterest::Type::Home,
        "base_exit", "返回小卖部门口 / Back to Street", {doorX, doorY - 1});
    addPoi(region, PointOfInterest::Type::Waypoint,
        "base_map_table", "基地地图桌 / Map Table", {7, 11});
    addPoi(region, PointOfInterest::Type::Home,
        "save_bed", "存档床 / Save Bed", {4, 4});
    addPoi(region, PointOfInterest::Type::Quest,
        "pixel_controller_spot", "像素手柄摆放处 / Controller Spot", {12, 8});
    // arcade_gate POI 放在传送门瓦片上,和 Connection sourceTile 一致
    addPoi(region, PointOfInterest::Type::Teleport,
        "arcade_gate", "弹窗游乐厅入口 / Popup Arcade Gate", {doorX + 5, doorY - 1});

    // popup_arcade: 入口传送门在 popup_arcade 顶部中央 {12,1},玩家落在 {12,2}(传送门下方1格)
    addConnection(region, MapConnection::Direction::East,
        "popup_arcade", {doorX + 5, doorY - 1}, {12, 2});
}

void configurePopupArcade(MapRegion& region) {
    TileMap& map = region.getTileMap();

    // 内部使用 Dirt 作为可行走地板(不创建物理刚体),整体尺寸 24×20。
    // 地图布局(从上到下):
    //   y=0    : 顶部墙壁(北边界)
    //   y=1    : 入口平台,中央放入口传送门(来自 home_base 的 arcade_gate)
    //   y=2-6  : 上部探索区,含试玩币/公告
    //   y=7-13 : 中部开阔区,含商贩/影子/Boss 门
    //   y=14-17: 下部区,含返回传送门
    //   y=18   : 底部墙壁(南边界)
    //   y=19   : 底部墙壁

    // 填满 Dirt(可行走,无物理刚体)
    for (int y = 1; y < map.height - 1; ++y) {
        for (int x = 1; x < map.width - 1; ++x) {
            map.setTile(x, y, TileType::Dirt);
        }
    }

    // 顶部入口平台 Path(y=1, x=10-14)
    for (int x = 10; x <= 14; ++x) {
        if (map.isInBounds(x, 1)) map.setTile(x, 1, TileType::Path);
    }
    // 入口传送门(中央 x=12, y=1),来自 home_base 的 arcade_gate {20,9}
    if (map.isInBounds(12, 1)) map.setTile(12, 1, TileType::Portal);

    // 中部主路径(y=9, x=5-19)
    for (int x = 5; x <= 19; ++x) {
        if (map.isInBounds(x, 9)) map.setTile(x, 9, TileType::Path);
    }
    // 底部返回平台 Path(y=17, x=10-14)
    for (int x = 10; x <= 14; ++x) {
        if (map.isInBounds(x, 17)) map.setTile(x, 17, TileType::Path);
    }
    // 返回传送门(中央 x=12, y=17),通往 home_base
    if (map.isInBounds(12, 17)) map.setTile(12, 17, TileType::Portal);

    // Boss 门路径(y=7, x=10-14)
    for (int x = 10; x <= 14; ++x) {
        if (map.isInBounds(x, 7)) map.setTile(x, 7, TileType::Path);
    }

    // 少量边界石柱作为掩体(不放在传送门和路径上)
    auto tryStone = [&map](int x, int y) {
        if (map.isInBounds(x, y) &&
            map.getTile(x, y) == TileType::Dirt) {
            map.setTile(x, y, TileType::Stone);
        }
    };
    // 左边界石柱列(x=3)
    for (int y = 3; y <= 15; ++y) { tryStone(3, y); }
    // 右边界石柱列(x=20)
    for (int y = 3; y <= 15; ++y) { tryStone(20, y); }
    // 少量散点石柱
    tryStone(6, 4); tryStone(18, 4);
    tryStone(6, 12); tryStone(18, 12);

    // POI:严格放在 Path/Dirt 格子上,远离传送门
    addPoi(region, PointOfInterest::Type::NPC_Spawn,
        "trapped_player_shadow", "被困玩家影子 / Trapped Shadow", {8, 11});
    addPoi(region, PointOfInterest::Type::Shop,
        "popup_vendor", "弹窗商贩 / Popup Vendor", {12, 10});
    addPoi(region, PointOfInterest::Type::Treasure,
        "trial_token_1", "试玩币 1 / Trial Token 1", {7, 5});
    addPoi(region, PointOfInterest::Type::Treasure,
        "trial_token_2", "试玩币 2 / Trial Token 2", {17, 5});
    addPoi(region, PointOfInterest::Type::Treasure,
        "trial_token_3", "试玩币 3 / Trial Token 3", {12, 13});
    addPoi(region, PointOfInterest::Type::Quest,
        "tieyi_cage", "铁翼牢笼 / Tieyi Cage", {17, 11});
    addPoi(region, PointOfInterest::Type::Waypoint,
        "gray_bureau_notice", "灰色管理局公告 / Gray Bureau Notice", {5, 9});
    addPoi(region, PointOfInterest::Type::Dungeon,
        "arcade_boss_door", "六元冠冕门 / Crown Door", {12, 7});
    addPoi(region, PointOfInterest::Type::Dungeon,
        "popup_crown_arena", "六元冠冕竞技场 / Popup Crown Arena", {12, 5});
    addPoi(region, PointOfInterest::Type::Teleport,
        "base_return_gate", "返回秘密基地 / Return to Base", {12, 16});

    // 从 home_base {20,9} 进入,落在 popup_arcade 入口传送门 {12,1}
    // Direction::North 表示入口在 popup_arcade 的北端
    addConnection(region, MapConnection::Direction::North,
        "home_base", {12, 1}, {12, 2});
    // 返回:popup_arcade {12,17} → home_base {20,9}
    // Direction::South 表示返回口在 popup_arcade 的南端
    addConnection(region, MapConnection::Direction::South,
        "home_base", {12, 17}, {20, 9});
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

void configureRegion(MapRegion& region) {
    ::configureRegionSpecials(region);
}

std::unique_ptr<MapRegion> createRegion(const std::string& regionId) {
    RegionSpec spec = getRegionSpec(regionId);
    auto region = std::make_unique<MapRegion>();
    region->generate(regionId, spec.name, spec.type, getRegionSeed(regionId), spec.width, spec.height);
    configureRegionSpecials(*region);
    return region;
}

}  // namespace RegionFactory
