#include "MapRegion.h"
#include "TerrainGenerator.h"
#include "DecorationGen.h"
#include <random>
#include <algorithm>

MapRegion::MapRegion() = default;
MapRegion::~MapRegion() {
    clearPhysics();
}

void MapRegion::generate(const std::string& newId, const std::string& newName,
                         RegionType regionType, int newSeed,
                         int w, int h, float ts) {
    id = newId;
    name = newName;
    type = regionType;
    seed = newSeed;

    tileMap = TileMap(w, h, ts);
    decorations.clear();
    connections.clear();
    pois.clear();

    // 根据区域类型使用不同的生成策略
    switch (regionType) {
        case RegionType::Overworld:
            TerrainGenerator::generateCoherent(tileMap, seed, 0.08f, 0.5f, 4, 0.35f);
            TerrainGenerator::generatePaths(tileMap, 4, seed + 1);
            break;

        case RegionType::Dungeon:
            // 阶段7实现：使用房间-走廊算法生成地下城布局
            // 暂时使用简化版：纯墙壁区域中间开辟通道
            generateSimpleDungeon(tileMap, seed);
            break;

        case RegionType::Indoor:
            // 阶段7实现：使用预定义房间模板
            // 暂时使用简化版：四周墙壁，中间空地
            generateSimpleIndoor(tileMap, seed);
            break;

        case RegionType::Arena:
            // 阶段7实现：使用对称竞技场布局
            // 暂时使用简化版：开放区域带少量掩体
            generateSimpleArena(tileMap, seed);
            break;
    }

    // 生成装饰物（复用阶段5的 DecorationGen）
    generateDecorations(tileMap, decorations, seed, 0.12f);
}

// 简化版地下城生成（阶段7升级为完整实现）
void MapRegion::generateSimpleDungeon(TileMap& map, int seed) {
    std::mt19937 rng(static_cast<unsigned>(seed));
    // 四周墙壁，内部随机开辟通道
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            if (x == 0 || y == 0 || x == map.width - 1 || y == map.height - 1) {
                map.setTile(x, y, TileType::Wall);
            } else if (x == map.width / 2 || y == map.height / 2) {
                map.setTile(x, y, TileType::Path);  // 十字通道
            } else {
                map.setTile(x, y, TileType::Stone);
            }
        }
    }
}

// 简化版室内生成
void MapRegion::generateSimpleIndoor(TileMap& map, int) {
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            if (x == 0 || y == 0 || x == map.width - 1 || y == map.height - 1) {
                map.setTile(x, y, TileType::Wall);
            } else {
                map.setTile(x, y, TileType::Path);  // 使用 Path 替代不存在的 WoodFloor
            }
        }
    }
}

// 简化版竞技场生成
void MapRegion::generateSimpleArena(TileMap& map, int seed) {
    std::mt19937 rng(static_cast<unsigned>(seed));
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            map.setTile(x, y, TileType::Stone);
            // 随机放置一些掩体
            if (rng() % 20 == 0 && x > 2 && y > 2 && x < map.width - 3 && y < map.height - 3) {
                map.setTile(x, y, TileType::Wall);
            }
        }
    }
}

void MapRegion::buildPhysics(b2WorldId world) {
    tileManager.init(tileMap, world);
}

void MapRegion::clearPhysics() {
    tileManager.shutdown();
}

void MapRegion::addConnection(const MapConnection& conn) {
    connections.push_back(conn);

    // 在连接点放置传送门瓦片
    glm::ivec2 boundaryTile = conn.getBoundaryTile(getSize());
    if (tileMap.isInBounds(boundaryTile.x, boundaryTile.y)) {
        tileMap.setTile(boundaryTile.x, boundaryTile.y, TileType::Portal);
    }
}

void MapRegion::addPOI(const PointOfInterest& poi) {
    pois.push_back(poi);
}

PointOfInterest* MapRegion::findPOI(const std::string& poiId) {
    auto it = std::find_if(pois.begin(), pois.end(),
        [&poiId](const PointOfInterest& poi) { return poi.id == poiId; });
    return it != pois.end() ? &*it : nullptr;
}

bool MapRegion::isNearBoundary(const glm::vec2& worldPos, float threshold) const {
    glm::ivec2 tile = tileMap.worldToTile(worldPos.x, worldPos.y);
    return tile.x < static_cast<int>(threshold) ||
           tile.x >= tileMap.width - static_cast<int>(threshold) ||
           tile.y < static_cast<int>(threshold) ||
           tile.y >= tileMap.height - static_cast<int>(threshold);
}

MapConnection* MapRegion::getConnectionAt(const glm::vec2& worldPos, float range) {
    glm::ivec2 tile = tileMap.worldToTile(worldPos.x, worldPos.y);

    for (auto& conn : connections) {
        glm::ivec2 boundaryTile = conn.getBoundaryTile(getSize());
        if (boundaryTile.x == tile.x && boundaryTile.y == tile.y) {
            return &conn;
        }
        // 检查范围内
        int dx = std::abs(boundaryTile.x - tile.x);
        int dy = std::abs(boundaryTile.y - tile.y);
        if (dx <= static_cast<int>(range) && dy <= static_cast<int>(range)) {
            return &conn;
        }
    }
    return nullptr;
}

bool MapRegion::loadFromSave(const std::string& /*saveData*/) {
    // 阶段6后续实现：从JSON存档数据加载
    return false;
}

// MapConnection 实现
glm::ivec2 MapConnection::getBoundaryTile(const glm::ivec2& regionSize) const {
    switch (direction) {
        case Direction::North:
            return glm::ivec2(regionSize.x / 2, 0);
        case Direction::South:
            return glm::ivec2(regionSize.x / 2, regionSize.y - 1);
        case Direction::East:
            return glm::ivec2(regionSize.x - 1, regionSize.y / 2);
        case Direction::West:
            return glm::ivec2(0, regionSize.y / 2);
        default:
            return glm::ivec2(0, 0);
    }
}
