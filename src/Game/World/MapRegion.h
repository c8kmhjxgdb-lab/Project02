#pragma once

#include "TileMap.h"
#include "MapTileManager.h"
#include "Decoration.h"
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <box2d/box2d.h>

/**
 * 区域类型
 */
enum class RegionType : uint8_t {
    Overworld,   // 室外大地图
    Dungeon,     // 地下城/洞穴
    Indoor,      // 室内（房屋、商店）
    Arena        // 竞技场/战斗场地
};

/**
 * 区域连接（用于区域切换）
 */
struct MapConnection {
    enum class Direction : uint8_t {
        North, South, East, West
    };

    Direction direction;
    std::string targetRegionId;
    glm::ivec2 sourceTile;      // 本区域出口瓦片坐标
    glm::ivec2 targetTile;      // 目标区域入口瓦片坐标

    // 获取连接在地图边界上的瓦片坐标
    glm::ivec2 getBoundaryTile(const glm::ivec2& regionSize) const;
};

/**
 * 兴趣点（POI）
 */
struct PointOfInterest {
    enum class Type : uint8_t {
        NPC_Spawn,    // NPC生成点
        Treasure,     // 宝藏
        Quest,        // 任务点
        Teleport,     // 传送点
        Home,         // 玩家之家
        Shop,         // 商店
        Dungeon,      // 地下城入口
        Waypoint      // 路径点
    };

    Type type;
    glm::ivec2 tilePos;
    std::string id;           // 唯一标识
    std::string displayName;  // 显示名称
    int metadata;             // 附加数据（如NPC ID、任务ID）
};

/**
 * 地图区域
 *
 * 封装单个游戏区域的所有数据：瓦片地图、物理体、装饰物、连接点和POI。
 * 支持程序化生成和从存档加载。
 */
class MapRegion {
public:
    MapRegion();
    ~MapRegion();

    // 初始化（程序生成）— 仅生成地图数据，不创建物理
    void generate(const std::string& id, const std::string& name,
                  RegionType type, int seed,
                  int width = 60, int height = 60,
                  float tileSize = 1.0f);

    // 从存档数据加载
    bool loadFromSave(const std::string& saveData);

    // 物理世界管理
    void buildPhysics(b2WorldId world);  // 为当前瓦片地图创建物理刚体（必须在所有瓦片修改完成后调用）
    void clearPhysics();                  // 销毁所有物理刚体

    // 区域属性
    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    RegionType getType() const { return type; }
    int getSeed() const { return seed; }

    // 瓦片地图
    TileMap& getTileMap() { return tileMap; }
    const TileMap& getTileMap() const { return tileMap; }

    // 瓦片管理器（用于修改和物理同步）
    MapTileManager& getTileManager() { return tileManager; }
    const MapTileManager& getTileManager() const { return tileManager; }

    // 装饰物
    std::vector<Decoration>& getDecorations() { return decorations; }
    const std::vector<Decoration>& getDecorations() const { return decorations; }
    void setDecorations(const std::vector<Decoration>& savedDecorations) { decorations = savedDecorations; }

    // 连接
    void addConnection(const MapConnection& conn);
    const std::vector<MapConnection>& getConnections() const { return connections; }

    // POI
    void addPOI(const PointOfInterest& poi);
    const std::vector<PointOfInterest>& getPOIs() const { return pois; }
    void setPOIs(const std::vector<PointOfInterest>& savedPOIs) { pois = savedPOIs; }
    PointOfInterest* findPOI(const std::string& poiId);

    // 区域尺寸
    int getWidth() const { return tileMap.width; }
    int getHeight() const { return tileMap.height; }
    glm::ivec2 getSize() const { return {tileMap.width, tileMap.height}; }

    // 边界检查：判断世界坐标是否在区域边界附近（用于区域切换检测）
    bool isNearBoundary(const glm::vec2& worldPos, float threshold = 1.5f) const;
    MapConnection* getConnectionAt(const glm::vec2& worldPos, float range = 1.5f);

    // 修改记录（委托给 tileManager）
    const std::vector<TileModification>& getModifications() const {
        return tileManager.getModifications();
    }

private:
    std::string id;
    std::string name;
    RegionType type;
    int seed;

    TileMap tileMap;
    MapTileManager tileManager;  // 瓦片管理器（负责物理同步和修改记录）
    std::vector<Decoration> decorations;
    std::vector<MapConnection> connections;
    std::vector<PointOfInterest> pois;

    // 简化版区域生成（阶段7升级为完整实现）
    static void generateSimpleDungeon(TileMap& map, int seed);
    static void generateSimpleIndoor(TileMap& map, int seed);
    static void generateSimpleArena(TileMap& map, int seed);
};
