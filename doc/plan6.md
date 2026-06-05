# 阶段6：世界完整化实现计划

## Context

阶段5已完成能力深化与地图系统升级：通行性系统、MapTileManager物理同步、噪声地形生成、装饰物系统、Y轴深度排序、小地图。阶段6目标是**世界完整化**——从单一测试地图迈向真正的多区域游戏世界。

**阶段6具体目标：**
1. 多区域系统（区域定义、连接、切换）
2. A*寻路系统（敌人AI路径规划）
3. 存档/读档系统（种子+差异策略）
4. 日夜循环系统
5. 天气系统基础（雨、雾效果）

**留给阶段7-8的内容：** 室内地图细节、建造模式、家具系统、音频系统、UI polish、主线剧情。

---

## 1. 多区域系统（P0优先级）

### 1.1 区域数据结构

在 `src/Game/World/` 下创建 `MapRegion.h/.cpp`：

```cpp
// MapRegion.h
#pragma once

#include "TileMap.h"
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/ivec2.hpp>

/// 区域类型
enum class RegionType : uint8_t {
    Overworld,   // 室外大地图
    Dungeon,     // 地下城/洞穴
    Indoor,      // 室内（房屋、商店）
    Arena        // 竞技场/战斗场地
};

/// 区域连接（用于区域切换）
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

/// 兴趣点（POI）
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

/// 地图区域
class MapRegion {
public:
    MapRegion();
    ~MapRegion();
    
    // 初始化（程序生成）
    void generate(const std::string& id, const std::string& name,
                  RegionType type, int seed, 
                  int width = 60, int height = 60,
                  float tileSize = 1.0f);
    
    // 从存档数据加载
    bool loadFromSave(const std::string& saveData);
    
    // 物理世界管理
    void buildPhysics(b2WorldId world);  // 为当前瓦片地图创建物理刚体
    void clearPhysics();                  // 销毁所有物理刚体
    
    // 区域属性
    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    RegionType getType() const { return type; }
    int getSeed() const { return seed; }
    
    // 瓦片地图
    TileMap& getTileMap() { return tileMap; }
    const TileMap& getTileMap() const { return tileMap; }
    
    // 装饰物
    std::vector<Decoration>& getDecorations() { return decorations; }
    const std::vector<Decoration>& getDecorations() const { return decorations; }
    
    // 连接
    void addConnection(const MapConnection& conn);
    const std::vector<MapConnection>& getConnections() const { return connections; }
    
    // POI
    void addPOI(const PointOfInterest& poi);
    const std::vector<PointOfInterest>& getPOIs() const { return pois; }
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
```

**MapRegion.cpp 关键实现：**

```cpp
void MapRegion::generate(const std::string& newId, const std::string& newName,
                         RegionType regionType, int newSeed,
                         int w, int h, float ts) {
    id = newId;
    name = newName;
    type = regionType;
    seed = newSeed;
    
    tileMap = TileMap(w, h, ts);
    
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
                map.setTile(x, y, TileType::StoneFloor);
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
                map.setTile(x, y, TileType::WoodFloor);
            }
        }
    }
}

// 简化版竞技场生成
void MapRegion::generateSimpleArena(TileMap& map, int seed) {
    std::mt19937 rng(static_cast<unsigned>(seed));
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            map.setTile(x, y, TileType::StoneFloor);
            // 随机放置一些掩体
            if (rng() % 20 == 0 && x > 2 && y > 2 && x < map.width - 3 && y < map.height - 3) {
                map.setTile(x, y, TileType::Stone);
            }
        }
    }
}

void MapRegion::addConnection(const MapConnection& conn) {
    connections.push_back(conn);
    
    // 在连接点放置传送门瓦片
    glm::ivec2 boundaryTile = conn.getBoundaryTile(getSize());
    if (tileMap.isInBounds(boundaryTile.x, boundaryTile.y)) {
        tileMap.setTile(boundaryTile.x, boundaryTile.y, TileType::Portal);
    }
}

bool MapRegion::isNearBoundary(const glm::vec2& worldPos, float threshold) const {
    glm::ivec2 tile = tileMap.worldToTile(worldPos.x, worldPos.y);
    return tile.x < threshold || tile.x >= tileMap.width - threshold ||
           tile.y < threshold || tile.y >= tileMap.height - threshold;
}
```

### 1.2 区域管理器

在 `src/Game/World/` 下创建 `RegionManager.h/.cpp`：

```cpp
// RegionManager.h
#pragma once

#include "MapRegion.h"
#include <unordered_map>
#include <memory>

class RegionManager {
public:
    // 初始化
    void init();
    void shutdown();
    
    // 区域加载/卸载
    bool loadRegion(const std::string& regionId);
    void unloadCurrentRegion();
    
    // 区域切换（所有权交换）
    bool transitionTo(const std::string& targetRegionId, const glm::ivec2& entryTile, b2WorldId world);
    bool transitionTo(const MapConnection& connection, b2WorldId world);
    
    // 设置 Box2D 世界引用（在初始化时调用）
    void setWorldId(b2WorldId world) { worldId = world; }
    
    // 获取当前区域
    MapRegion* getCurrentRegion();
    const MapRegion* getCurrentRegion() const;
    
    // 获取过渡透明度（0=完全可见，1=全黑）
    float getTransitionAlpha() const { return transitionAlpha; }
    
    // 区域查询（优先返回当前区域，再查加载列表）
    bool hasRegion(const std::string& regionId) const;
    MapRegion* getRegion(const std::string& regionId);
    
    // 获取已加载的区域列表
    const std::vector<std::string>& getDiscoveredRegions() const { return discoveredRegions; }
    
    // 区域过渡效果控制
    void setTransitionEffectEnabled(bool enabled) { useTransitionEffect = enabled; }
    void setTransitionDuration(float duration) { transitionDuration = duration; }
    
    // 更新（处理区域过渡动画）
    void update(float dt);
    bool isTransitioning() const { return isTransitioningFlag; }
    float getTransitionProgress() const { return transitionProgress; }
    
private:
    std::string currentRegionId;
    std::unique_ptr<MapRegion> currentRegion;
    std::unordered_map<std::string, std::unique_ptr<MapRegion>> loadedRegions;
    std::vector<std::string> discoveredRegions;
    b2WorldId worldId;  // Box2D 世界引用（用于物理同步）
    
    // 过渡效果
    bool useTransitionEffect = true;
    float transitionDuration = 0.5f;
    bool isTransitioningFlag = false;
    float transitionProgress = 0.0f;
    float transitionAlpha = 0.0f;  // 过渡透明度（0=完全可见，1=全黑）
    bool transitionFadeOut = true;
    
    // 过渡期间的临时数据
    std::string transitionTargetRegionId;
    glm::ivec2 transitionEntryTile;
    
    // 固定种子表（确保跨运行一致性）
    static int getRegionSeed(const std::string& regionId);
    
    // 过渡效果
    void beginTransition();
    void updateTransition(float dt);
    void completeTransition();
};
```

**RegionManager.cpp 关键实现：**

```cpp
void RegionManager::init() {
    // 创建初始区域（新手村）
    auto starterRegion = std::make_unique<MapRegion>();
    starterRegion->generate("starter_village", "新手村", 
                           RegionType::Overworld, 42, 60, 60);
    
    // 添加家的POI
    PointOfInterest home;
    home.type = PointOfInterest::Type::Home;
    home.id = "player_home";
    home.displayName = "玩家之家";
    home.tilePos = {5, 5};
    starterRegion->addPOI(home);
    
    currentRegionId = "starter_village";
    currentRegion = std::move(starterRegion);
    discoveredRegions.push_back("starter_village");
    // 注意：buildPhysics 在 main.cpp 中单独调用，因为此时 worldId 可能还未初始化
}

bool RegionManager::transitionTo(const std::string& targetRegionId, 
                                  const glm::ivec2& entryTile,
                                  b2WorldId world) {
    if (targetRegionId == currentRegionId) return false;
    
    worldId = world;  // 保存世界引用
    
    // 如果目标区域已在加载列表中，仅交换所有权（物理体已存在，无需重建）
    auto it = loadedRegions.find(targetRegionId);
    if (it != loadedRegions.end()) {
        // 将当前区域存入加载列表
        loadedRegions[currentRegionId] = std::move(currentRegion);
        // 从加载列表取出目标区域
        currentRegion = std::move(it->second);
        loadedRegions.erase(it);
        currentRegionId = targetRegionId;
        return true;
    }
    
    // 否则开始过渡流程（创建新区域）
    if (useTransitionEffect) {
        transitionTargetRegionId = targetRegionId;
        transitionEntryTile = entryTile;
        isTransitioningFlag = true;
        transitionProgress = 0.0f;
        transitionAlpha = 0.0f;
        transitionFadeOut = true;
        beginTransition();
        return true;
    } else {
        // 直接切换（无过渡效果）
        currentRegionId = targetRegionId;
        currentRegion = std::make_unique<MapRegion>();
        // 使用固定种子表确保跨运行一致性（见下文种子管理）
        int seed = getRegionSeed(targetRegionId);
        currentRegion->generate(targetRegionId, targetRegionId, 
                               RegionType::Overworld, seed, 60, 60);
        currentRegion->buildPhysics(worldId);
        discoveredRegions.push_back(targetRegionId);
        return true;
    }
}

void RegionManager::updateTransition(float dt) {
    transitionProgress += dt / transitionDuration;
    // 更新透明度（淡出：0→1，淡入：1→0）
    if (transitionFadeOut) {
        transitionAlpha = transitionProgress;
    } else {
        transitionAlpha = 1.0f - transitionProgress;
    }
    
    if (transitionFadeOut && transitionProgress >= 1.0f) {
        // 淡出完成，切换区域
        transitionProgress = 0.0f;
        transitionFadeOut = false;
        completeTransition();
    } else if (!transitionFadeOut && transitionProgress >= 1.0f) {
        // 淡入完成，过渡结束
        isTransitioningFlag = false;
        transitionProgress = 0.0f;
        transitionAlpha = 0.0f;
    }
}

void RegionManager::completeTransition() {
    // 将当前区域存入加载列表（所有权转移，不清除物理体）
    loadedRegions[currentRegionId] = std::move(currentRegion);
    
    // 创建新区域
    currentRegionId = transitionTargetRegionId;
    currentRegion = std::make_unique<MapRegion>();
    // 使用固定种子表确保跨运行一致性
    int seed = getRegionSeed(currentRegionId);
    currentRegion->generate(currentRegionId, currentRegionId,
                           RegionType::Overworld, seed, 60, 60);
    // 为新区域创建物理刚体
    currentRegion->buildPhysics(worldId);
    
    if (std::find(discoveredRegions.begin(), discoveredRegions.end(), 
                  currentRegionId) == discoveredRegions.end()) {
        discoveredRegions.push_back(currentRegionId);
    }
}

// 固定种子表：确保跨运行一致性
// 在实际项目中，可以从 Lua 配置文件读取
int RegionManager::getRegionSeed(const std::string& regionId) {
    static const std::unordered_map<std::string, int> REGION_SEEDS = {
        {"starter_village", 42},
        {"dark_forest", 12345},
        {"mountain_pass", 67890},
        {"coastal_town", 11111},
        // 默认种子：使用稳定的哈希函数
    };
    
    auto it = REGION_SEEDS.find(regionId);
    if (it != REGION_SEEDS.end()) {
        return it->second;
    }
    
    // 回退：使用稳定的哈希（FNV-1a 或简单的字符串哈希）
    unsigned int hash = 2166136261u;
    for (char c : regionId) {
        hash ^= static_cast<unsigned int>(c);
        hash *= 16777619u;
    }
    return static_cast<int>(hash);
}
```

### 1.3 主循环集成

在 `main.cpp` 中更新游戏循环以支持多区域：

```cpp
// 新增成员
RegionManager regionManager;

// 初始化
regionManager.init();

// 每帧检测区域切换
if (!regionManager.isTransitioning()) {
    MapConnection* conn = regionManager.getCurrentRegion()->
                          getConnectionAt(playerPos, 1.5f);
    if (conn) {
        // 执行区域切换（传递 worldId 用于物理同步）
        regionManager.transitionTo(*conn, worldId);

        // 计算目标世界坐标并设置玩家位置
        glm::ivec2 targetTile = conn->targetTile;
        glm::vec2 targetWorld = regionManager.getCurrentRegion()
                                ->getTileMap().tileToWorld(targetTile.x, targetTile.y);
        setPlayerPosition(targetWorld);

        // 重置玩家速度，避免惯性带入新区域
        b2Body_SetLinearVelocity(playerBody, b2Vec2_zero);
    }
}

// 更新过渡效果
regionManager.update(dt);

// 渲染时根据过渡进度调整
if (regionManager.isTransitioning()) {
    // 使用 transitionAlpha 作为透明度（淡出时 0→1，淡入时 1→0）
    float alpha = regionManager.getTransitionAlpha();
    
    // 渲染全屏半透明黑色矩形（需要 Draw2D 支持 alpha）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Draw2D::drawRectFilled(0, 0, screenWidth, screenHeight, 
                          glm::vec4(0.0f, 0.0f, 0.0f, alpha));
    glDisable(GL_BLEND);
}
```

---

## 2. A*寻路系统

### 2.1 寻路数据结构

在 `src/Game/AI/` 下创建 `PathfindingSystem.h/.cpp`：

```cpp
// PathfindingSystem.h
#pragma once

#include "TileMap.h"
#include <vector>
#include <glm/ivec2.hpp>
#include <queue>
#include <unordered_set>

/// A* 寻路节点
struct PathNode {
    glm::ivec2 position;
    float gCost;      // 从起点到当前节点的实际代价
    float hCost;      // 从当前节点到终点的启发式估计代价
    float fCost() const { return gCost + hCost; }
    glm::ivec2 parentPos;  // 父节点坐标（-1,-1 表示起点）
    
    bool operator>(const PathNode& other) const {
        return fCost() > other.fCost();
    }
};

/// 寻路结果
struct PathResult {
    std::vector<glm::ivec2> path;
    bool found;
    float totalCost;
    
    // 获取路径上的下一个移动方向
    glm::vec2 getNextDirection(const glm::ivec2& currentPos, float tileSize) const;
    
    // 简化路径（使用 Douglas-Peucker 算法移除共线点）
    PathResult simplify(float epsilon = 0.5f) const;
};

/// 寻路配置
struct PathfindingConfig {
    bool avoidWater = true;      // 避免水域
    bool avoidLava = true;       // 避免熔岩
    bool allowDiagonal = false;  // 允许对角移动
    float maxPathLength = 200.0f; // 最大路径长度
};

class PathfindingSystem {
public:
    // 寻找路径
    PathResult findPath(const TileMap& map,
                       const glm::ivec2& start,
                       const glm::ivec2& end,
                       const PathfindingConfig& config = {});
    
    // 批量寻路（多个目标，返回最近的可到达点）
    PathResult findPathToNearest(const TileMap& map,
                                const glm::ivec2& start,
                                const std::vector<glm::ivec2>& targets,
                                const PathfindingConfig& config = {});
    
    // 射线投射（视线检查）
    bool hasLineOfSight(const TileMap& map,
                       const glm::ivec2& a,
                       const glm::ivec2& b) const;
    
    // 获取可达的邻居瓦片
    std::vector<glm::ivec2> getWalkableNeighbors(const TileMap& map,
                                                 const glm::ivec2& pos,
                                                 bool diagonal = false) const;
    
    // 距离启发式（曼哈顿或欧几里得）
    float heuristic(const glm::ivec2& a, const glm::ivec2& b, 
                   bool diagonal = false) const;
    
private:
    // 开放列表（优先队列）
    std::priority_queue<PathNode, std::vector<PathNode>, 
                       std::greater<PathNode>> openList;
    
    // 关闭列表（已访问）
    std::unordered_set<uint64_t> closedSet;
    
    // 节点映射（坐标 → 节点数据）
    std::unordered_map<uint64_t, PathNode> nodeMap;
    
    // 编码坐标为单一键
    static uint64_t packCoord(const glm::ivec2& pos) {
        return (static_cast<uint64_t>(pos.x) << 32) | 
               static_cast<uint32_t>(pos.y);
    }
    
    // 判断瓦片是否可通行
    bool isWalkable(const TileMap& map, const glm::ivec2& pos,
                   const PathfindingConfig& config) const;
};
```

**PathfindingSystem.cpp 关键实现：**

```cpp
PathResult PathfindingSystem::findPath(const TileMap& map,
                                       const glm::ivec2& start,
                                       const glm::ivec2& end,
                                       const PathfindingConfig& config) {
    PathResult result;
    result.found = false;
    
    if (!map.isInBounds(start.x, start.y) || 
        !map.isInBounds(end.x, end.y)) {
        return result;
    }
    
    if (!isWalkable(map, end, config)) {
        // 目标不可达，尝试找最近的可到达点
        return findPathToNearest(map, start, {end}, config);
    }
    
    // 清空状态
    openList = {};
    closedSet.clear();
    nodeMap.clear();
    
    // 初始化起点
    PathNode startNode;
    startNode.position = start;
    startNode.gCost = 0.0f;
    startNode.hCost = heuristic(start, end, config.allowDiagonal);
    startNode.parentPos = glm::ivec2(-1, -1);  // 起点标记
    
    openList.push(startNode);
    nodeMap[packCoord(start)] = startNode;
    
    int iterations = 0;
    const int maxIterations = 50000;  // 对 60x60 地图足够（最多 3600 个节点）
    
    while (!openList.empty() && iterations < maxIterations) {
        ++iterations;
        
        // 取出fCost最小的节点
        PathNode current = openList.top();
        openList.pop();
        
        // 到达终点
        if (current.position == end) {
            // 回溯路径（通过坐标链）
            glm::ivec2 pos = end;
            while (pos.x != -1 && pos.y != -1) {
                result.path.insert(result.path.begin(), pos);
                auto it = nodeMap.find(packCoord(pos));
                if (it == nodeMap.end()) break;
                pos = it->second.parentPos;
            }
            result.found = true;
            return result;
        }
        
        closedSet.insert(packCoord(current.position));
        
        // 遍历邻居
        auto neighbors = getWalkableNeighbors(map, current.position, 
                                              config.allowDiagonal);
        for (const auto& neighborPos : neighbors) {
            uint64_t neighborKey = packCoord(neighborPos);
            
            if (closedSet.count(neighborKey)) continue;
            if (!isWalkable(map, neighborPos, config)) continue;
            
            float moveCost = 1.0f;
            if (config.allowDiagonal && 
                (neighborPos.x != current.position.x && 
                 neighborPos.y != current.position.y)) {
                moveCost = 1.414f; // sqrt(2)
            }
            
            // 地形代价
            TileType tile = map.getTile(neighborPos.x, neighborPos.y);
            const TileDef& def = getTileDef(tile);
            moveCost *= def.movementCost;
            
            float newGCost = current.gCost + moveCost;
            
            auto it = nodeMap.find(neighborKey);
            if (it == nodeMap.end() || newGCost < it->second.gCost) {
                // 新节点或更优路径
                PathNode newNode;
                newNode.position = neighborPos;
                newNode.gCost = newGCost;
                newNode.hCost = heuristic(neighborPos, end, config.allowDiagonal);
                newNode.parentPos = current.position;  // 存储父节点坐标，非指针
                
                nodeMap[neighborKey] = newNode;
                openList.push(newNode);
            }
        }
    }
    
    // 未找到路径
    return result;
}

float PathfindingSystem::heuristic(const glm::ivec2& a, const glm::ivec2& b,
                                   bool diagonal) const {
    if (diagonal) {
        // 对角距离（Chebyshev距离）
        float dx = std::abs(a.x - b.x);
        float dy = std::abs(a.y - b.y);
        return dx + dy + (std::sqrt(2.0f) - 2.0f) * std::min(dx, dy);
    }
    // 曼哈顿距离
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool PathfindingSystem::hasLineOfSight(const TileMap& map,
                                       const glm::ivec2& a,
                                       const glm::ivec2& b) const {
    // 使用Bresenham算法检查视线路径上的所有瓦片
    int dx = std::abs(b.x - a.x);
    int dy = std::abs(b.y - a.y);
    int sx = (a.x < b.x) ? 1 : -1;
    int sy = (a.y < b.y) ? 1 : -1;
    int err = dx - dy;
    
    int x = a.x, y = a.y;
    while (true) {
        if (!map.isInBounds(x, y)) return false;
        TileType tile = map.getTile(x, y);
        const TileDef& def = getTileDef(tile);
        if (def.passability == Passability::Blocked) return false;
        
        if (x == b.x && y == b.y) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
    
    return true;
}
```

### 2.2 敌人AI集成寻路

更新 `Enemy.cpp` 以使用寻路系统：

```cpp
// 在 Enemy 结构中添加
struct Enemy {
    // ... 现有字段 ...
    
    PathResult currentPath;
    int pathIndex = 0;
    float pathRecomputeTimer = 0.0f;
    const float PATH_RECOMPUTE_INTERVAL = 1.0f; // 每秒重新计算路径
};

void EnemyManager::updateChaser(Enemy& enemy, float dt, 
                                const glm::vec2& playerPos,
                                const TileMap& map,
                                PathfindingSystem& pathfinding) {
    glm::ivec2 enemyTile = map.worldToTile(getEnemyPosition(enemy));
    glm::ivec2 playerTile = map.worldToTile(playerPos);
    
    // 定期重新计算路径
    enemy.pathRecomputeTimer += dt;
    if (enemy.pathRecomputeTimer >= PATH_RECOMPUTE_INTERVAL || 
        enemy.currentPath.path.empty()) {
        enemy.pathRecomputeTimer = 0.0f;
        enemy.currentPath = pathfinding.findPath(map, enemyTile, playerTile);
        enemy.pathIndex = 0;
    }
    
    // 沿路径移动
    if (!enemy.currentPath.path.empty() && 
        enemy.pathIndex < enemy.currentPath.path.size()) {
        glm::ivec2 nextTile = enemy.currentPath.path[enemy.pathIndex];
        glm::vec2 nextWorldPos = map.tileToWorld(nextTile.x, nextTile.y);
        b2Vec2 pos = b2Body_GetPosition(enemy.bodyId);
        glm::vec2 currentPos(pos.x, pos.y);
        glm::vec2 dir = glm::normalize(nextWorldPos - currentPos);
        
        // 施加移动力
        b2Vec2 force = {dir.x * enemy.speed, dir.y * enemy.speed};
        b2Body_ApplyForceToCenter(enemy.bodyId, force, true);
        
        // 到达下一个路径点
        float distToNext = glm::distance(currentPos, nextWorldPos);
        if (distToNext < 0.3f) {
            enemy.pathIndex++;
        }
    } else {
        // 无路径时直接朝玩家移动
        b2Vec2 pos = b2Body_GetPosition(enemy.bodyId);
        glm::vec2 currentPos(pos.x, pos.y);
        glm::vec2 dir = glm::normalize(playerPos - currentPos);
        b2Vec2 force = {dir.x * enemy.speed, dir.y * enemy.speed};
        b2Body_ApplyForceToCenter(enemy.bodyId, force, true);
    }
}
```

---

## 3. 存档/读档系统

### 3.1 存档数据结构

在 `src/Game/` 下创建 `SaveSystem.h/.cpp`：

```cpp
// SaveSystem.h
#pragma once

#include "MapRegion.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/// 玩家进度数据
struct PlayerProgress {
    std::vector<std::string> discoveredRegions;
    std::vector<std::string> completedQuests;
    std::vector<std::string> collectedItems;
    float totalPlayTime;
    int maxHealth;
    int maxMana;
};

/// 存档数据
struct SaveData {
    int version = 2;
    std::string timestamp;
    
    // 玩家数据
    struct PlayerData {
        glm::vec2 position;
        float health;
        float maxHealth;
        float mana;
        float maxMana;
        int coins;
        PlayerProgress progress;
    } player;
    
    // 区域数据（种子+差异）
    struct RegionData {
        std::string id;
        std::string name;
        int seed;
        glm::ivec2 size;
        float tileSize;
        std::vector<TileModification> modifications;
        std::vector<Decoration> decorModifications;
        std::vector<PointOfInterest> pois;
    };
    
    std::vector<RegionData> regions;
    
    // 情感状态
    float grievance;
    float joy;
};

// MapRegion 中需要暴露修改记录接口（与阶段5的 MapTileManager 联动）
// 在 MapRegion.h 中添加：
class MapRegion {
public:
    // ... 现有接口 ...
    
    // 获取瓦片修改记录（委托给 MapTileManager）
    std::vector<TileModification> getModifications() const {
        return tileManager.getModifications();
    }
    
private:
    // 阶段5中已实现的 MapTileManager
    MapTileManager tileManager;
};

class SaveSystem {
public:
    // 保存游戏
    bool saveGame(const std::string& slot,
                 const glm::vec2& playerPos,
                 float playerHealth,
                 float playerMaxHealth,
                 int playerCoins,
                 float playerMaxMana,
                 const PlayerProgress& progress,
                 const RegionManager& regionManager,
                 float grievance);
    
    // 加载游戏
    bool loadGame(const std::string& slot,
                 SaveData& outData);
    
    // 删除存档
    bool deleteSave(const std::string& slot);
    
    // 检查存档是否存在
    bool hasSave(const std::string& slot) const;
    
    // 获取存档列表
    std::vector<std::string> getSaveSlots() const;
    
    // 获取存档元数据（不加载完整数据）
    struct SaveMeta {
        std::string slot;
        std::string timestamp;
        std::string regionName;
        float playTime;
    };
    SaveMeta getSaveMeta(const std::string& slot) const;
    
private:
    std::string getSavePath(const std::string& slot) const;
    
    // JSON 序列化/反序列化
    json saveDataToJson(const SaveData& data) const;
    SaveData jsonToSaveData(const json& data) const;
    
    // 压缩修改记录（游程编码）
    std::vector<TileModification> compressModifications(
        const std::vector<TileModification>& modifications) const;
    std::vector<TileModification> decompressModifications(
        const std::vector<TileModification>& compressed) const;
};
```

**SaveSystem.cpp 关键实现：**

```cpp
bool SaveSystem::saveGame(const std::string& slot,
                         const glm::vec2& playerPos,
                         float playerHealth,
                         float playerMaxHealth,
                         int playerCoins,
                         float playerMaxMana,
                         const PlayerProgress& progress,
                         const RegionManager& regionManager,
                         float grievance) {
    SaveData data;
    data.version = 2;
    data.timestamp = getCurrentTimestamp();
    
    // 玩家数据
    data.player.position = playerPos;
    data.player.health = playerHealth;
    data.player.maxHealth = playerMaxHealth;
    data.player.coins = playerCoins;
    data.player.maxMana = playerMaxMana;
    data.player.progress = progress;
    
    // 情感状态
    data.grievance = grievance;
    
    // 区域数据
    for (const auto& regionId : regionManager.getDiscoveredRegions()) {
        MapRegion* region = regionManager.getRegion(regionId);
        if (!region) continue;
        
        SaveData::RegionData regionData;
        regionData.id = region->getId();
        regionData.name = region->getName();
        regionData.seed = region->getSeed();
        regionData.size = {region->getWidth(), region->getHeight()};
        regionData.tileSize = region->getTileMap().tileSize;
        
        // 获取修改记录（从 MapTileManager）
        // 注意：需要 MapTileManager 提供 getModifications() 接口
        regionData.modifications = region->getTileMap().getModifications();
        regionData.decorModifications = region->getDecorations();
        regionData.pois = region->getPOIs();
        
        data.regions.push_back(regionData);
    }
    
    // 序列化并保存
    json jsonData = saveDataToJson(data);
    
    std::string path = getSavePath(slot);
    std::ofstream file(path);
    if (!file.is_open()) return false;
    
    file << jsonData.dump(2); // 格式化输出
    return true;
}

bool SaveSystem::loadGame(const std::string& slot, SaveData& outData) {
    std::string path = getSavePath(slot);
    std::ifstream file(path);
    if (!file.is_open()) return false;
    
    json jsonData;
    file >> jsonData;
    
    outData = jsonToSaveData(jsonData);
    return true;
}

json SaveSystem::saveDataToJson(const SaveData& data) const {
    json j;
    j["version"] = data.version;
    j["timestamp"] = data.timestamp;
    
    // 玩家数据
    j["player"]["position"] = {data.player.position.x, data.player.position.y};
    j["player"]["health"] = data.player.health;
    j["player"]["maxHealth"] = data.player.maxHealth;
    j["player"]["coins"] = data.player.coins;
    j["player"]["discoveredRegions"] = data.player.progress.discoveredRegions;
    j["player"]["playTime"] = data.player.progress.totalPlayTime;
    
    // 情感状态
    j["emotion"]["grievance"] = data.grievance;
    
    // 区域数据
    j["regions"] = json::array();
    for (const auto& region : data.regions) {
        json rj;
        rj["id"] = region.id;
        rj["name"] = region.name;
        rj["seed"] = region.seed;
        rj["size"] = {region.size.x, region.size.y};
        rj["tileSize"] = region.tileSize;
        
        // 修改记录
        rj["modifications"] = json::array();
        for (const auto& mod : region.modifications) {
            rj["modifications"].push_back({
                {"x", mod.x},
                {"y", mod.y},
                {"oldType", static_cast<int>(mod.oldType)},
                {"newType", static_cast<int>(mod.newType)},
                {"timestamp", mod.timestamp}
            });
        }
        
        j["regions"].push_back(rj);
    }
    
    return j;
}
```

### 3.2 存档格式

```json
{
  "version": 2,
  "timestamp": "2024-01-15T14:30:00Z",
  "player": {
    "position": [12.5, 8.3],
    "health": 85.0,
    "maxHealth": 100.0,
    "coins": 250,
    "discoveredRegions": ["starter_village", "dark_forest"],
    "playTime": 3600.0
  },
  "emotion": {
    "grievance": 25.0
  },
  "regions": [
    {
      "id": "starter_village",
      "name": "新手村",
      "seed": 42,
      "size": [60, 60],
      "tileSize": 1.0,
      "modifications": [
        {"x": 10, "y": 5, "oldType": 4, "newType": 0, "timestamp": 1234567890}
      ]
    }
  ]
}
```

---

## 4. 日夜循环系统

### 4.1 时间系统

在 `src/Game/World/` 下创建 `TimeSystem.h/.cpp`：

```cpp
// TimeSystem.h
#pragma once

#include <glm/vec3.hpp>

class TimeSystem {
public:
    // 初始化
    void init(float startHour = 6.0f);  // 默认从早上6点开始
    
    // 更新
    void update(float dt);
    
    // 时间控制
    void setHour(float hour);
    void setDaySpeed(float multiplier);  // 1.0 = 真实时间速度
    void pauseDayNightCycle(bool paused);
    
    // 获取当前时间
    float getHour() const { return currentHour; }
    int getHourInt() const { return static_cast<int>(currentHour); }
    int getMinuteInt() const { 
        return static_cast<int>((currentHour - getHourInt()) * 60); 
    }
    int getDay() const { return currentDay; }
    
    // 获取光照颜色（基于时间）
    glm::vec3 getAmbientLight() const;
    glm::vec3 getSkyColor() const;
    
    // 判断时间段
    bool isDaytime() const { return currentHour >= 6.0f && currentHour < 18.0f; }
    bool isNighttime() const { return !isDaytime(); }
    bool isDawn() const { return currentHour >= 5.0f && currentHour < 7.0f; }
    bool isDusk() const { return currentHour >= 17.0f && currentHour < 19.0f; }
    
    // 回调
    using TimeCallback = std::function<void(float hour)>;
    void setOnHourChange(TimeCallback cb) { onHourChange = cb; }
    
private:
    float currentHour;
    int currentDay;
    float daySpeed;  // 每小时真实秒数（默认 1.0 = 1真实秒 = 1游戏分钟，即1游戏小时=60真实秒）
    bool paused;
    
    float lastHour;
    TimeCallback onHourChange;
    
    // 根据时间计算光照
    glm::vec3 computeAmbientLight() const;
};
```

**TimeSystem.cpp 关键实现：**

```cpp
void TimeSystem::update(float dt) {
    if (paused) return;
    
    // 更新小时（daySpeed = 1.0 意味着 60 真实秒 = 1 游戏小时）
    currentHour += dt / (60.0f / daySpeed);
    
    // 日期循环
    if (currentHour >= 24.0f) {
        currentHour -= 24.0f;
        currentDay++;
    }
    
    // 整点回调
    int currentHourInt = getHourInt();
    if (currentHourInt != lastHour) {
        lastHour = currentHourInt;
        if (onHourChange) onHourChange(currentHour);
    }
}

glm::vec3 TimeSystem::computeAmbientLight() const {
    // 根据时间插值光照颜色
    float h = currentHour;
    
    if (h >= 6.0f && h < 12.0f) {
        // 早晨：暗黄 -> 亮白
        float t = (h - 6.0f) / 6.0f;
        return glm::mix(glm::vec3(0.8f, 0.6f, 0.4f), 
                       glm::vec3(1.0f, 0.98f, 0.9f), t);
    } else if (h >= 12.0f && h < 17.0f) {
        // 白天：亮白 -> 金黄
        float t = (h - 12.0f) / 5.0f;
        return glm::mix(glm::vec3(1.0f, 0.98f, 0.9f),
                       glm::vec3(1.0f, 0.9f, 0.6f), t);
    } else if (h >= 17.0f && h < 20.0f) {
        // 黄昏：金黄 -> 暗红
        float t = (h - 17.0f) / 3.0f;
        return glm::mix(glm::vec3(1.0f, 0.9f, 0.6f),
                       glm::vec3(0.6f, 0.3f, 0.2f), t);
    } else if (h >= 20.0f && h < 23.0f) {
        // 夜晚过渡：暗红 -> 深蓝
        float t = (h - 20.0f) / 3.0f;
        return glm::mix(glm::vec3(0.6f, 0.3f, 0.2f),
                       glm::vec3(0.1f, 0.1f, 0.3f), t);
    } else {
        // 深夜：深蓝
        return glm::vec3(0.1f, 0.1f, 0.3f);
    }
}
```

### 4.2 主循环集成

```cpp
// 新增成员
TimeSystem timeSystem;
WeatherSystem weatherSystem;

// 初始化
timeSystem.init(8.0f);  // 从早上8点开始
timeSystem.setDaySpeed(2.0f);  // 2倍速度（30真实秒=1游戏小时）
weatherSystem.init(&particleSystem, &camera);
weatherSystem.setRandomWeather(300.0f);  // 每5分钟随机变化

// 更新
timeSystem.update(dt);
weatherSystem.update(dt, camera);

// 渲染时应用光照（通过全局 uniform 传递给所有着色器）
// 最终环境光 = 时间系统基础光照 × 天气修正系数
glm::vec3 ambient = timeSystem.getAmbientLight() * weatherSystem.getLightModifier();

// 传递给 Draw2D（需在 Draw2D 中增加 setAmbientLight 方法）
Draw2D::setAmbientLight(ambient);

// 传递给角色/装饰渲染器
characterRenderer.setAmbientLight(ambient);
decorRenderer.setAmbientLight(ambient);
```

**着色器修改示例：**

```glsl
// 在 draw2d.frag 中添加
uniform vec3 uAmbientLight;

void main() {
    vec3 baseColor = texture(uTexture, vTexCoord).rgb;
    // 地面、装饰物等受环境光影响，UI 层应跳过乘法（通过 renderLayer 判断）
    vec3 finalColor = (renderLayer < LAYER_UI) ? baseColor * uAmbientLight : baseColor;
    FragColor = vec4(finalColor, texture(uTexture, vTexCoord).a);
}
```

```glsl
// 在 decor.frag 中添加
uniform vec3 uAmbientLight;

void main() {
    // ... 原有 SDF 计算 ...
    vec3 finalColor = sdfColor * uAmbientLight;
    FragColor = vec4(finalColor, alpha);
}
```

**注意**：后处理着色器不再承担环境光职责，避免影响 UI 渲染。UI 元素（血条、对话框等）应保持不受环境光影响。

---

## 5. 天气系统基础

### 5.1 天气管理器

在 `src/Game/World/` 下创建 `WeatherSystem.h/.cpp`：

```cpp
// WeatherSystem.h
#pragma once

#include <glm/vec3.hpp>
#include <random>

enum class WeatherType : uint8_t {
    Clear,      // 晴天
    Cloudy,     // 多云
    Rain,       // 雨天
    HeavyRain,  // 大雨
    Fog,        // 雾天
    Snow        // 雪天
};

class WeatherSystem {
public:
    // 初始化（注入依赖）
    void init(ParticleSystem* particleSys, Camera2D* camera);
    
    void update(float dt, const Camera2D& camera);
    void render();
    
    // 天气控制
    void setWeather(WeatherType type);
    void setRandomWeather(float changeInterval = 300.0f);  // 每5分钟随机变化
    void clearWeather();
    
    // 获取当前天气
    WeatherType getCurrentWeather() const { return currentWeather; }
    
    // 天气效果强度（0-1）
    float getIntensity() const { return intensity; }
    
    // 获取天气对移动的影响
    float getMovementMultiplier() const;
    
    // 获取天气对视线的影响
    float getVisibility() const;
    
    // 获取天气对光照的影响（返回 RGB 系数）
    glm::vec3 getLightModifier() const;
    
private:
    WeatherType currentWeather;
    WeatherType targetWeather;
    float intensity;           // 当前强度 0-1
    float targetIntensity;
    float transitionSpeed;     // 过渡速度
    
    float weatherTimer;
    float changeInterval;
    bool useRandomWeather;
    
    ParticleSystem* particleSystem;
    Camera2D* camera;  // 非拥有指针，仅用于获取视口信息
    
    // 粒子发射积分累积器
    float rainAccumulator = 0.0f;
    float snowAccumulator = 0.0f;
    
    // 天气过渡
    void updateTransition(float dt);
    
    // 天气效果
    void updateRainEffect(float dt, const Camera2D& cam);
    void updateFogEffect(float dt);
    void updateSnowEffect(float dt, const Camera2D& cam);
    
    // 随机天气选择
    WeatherType chooseRandomWeather() const;
};
```

**WeatherSystem.cpp 关键实现：**

```cpp
void WeatherSystem::update(float dt, const Camera2D& cam) {
    // 天气过渡
    updateTransition(dt);
    
    // 随机天气变化
    if (useRandomWeather) {
        weatherTimer += dt;
        if (weatherTimer >= changeInterval) {
            weatherTimer = 0.0f;
            targetWeather = chooseRandomWeather();
            targetIntensity = 1.0f;
        }
    }
    
    // 天气效果
    switch (currentWeather) {
        case WeatherType::Rain:
        case WeatherType::HeavyRain:
            updateRainEffect(dt, cam);
            break;
        case WeatherType::Fog:
            updateFogEffect(dt);
            break;
        case WeatherType::Snow:
            updateSnowEffect(dt, cam);
            break;
        default:
            break;
    }
}

void WeatherSystem::updateRainEffect(float dt, const Camera2D& cam) {
    if (!particleSystem) return;
    
    // 雨天粒子
    float emitRate = (currentWeather == WeatherType::HeavyRain) ? 200.0f : 80.0f;
    emitRate *= intensity;
    
    // 计算视口范围（通过 screenToWorld 转换屏幕角点）
    int screenWidth = 800, screenHeight = 600;  // 从主循环传入或使用全局配置
    glm::vec2 topLeft = cam.screenToWorld(0, screenHeight);
    glm::vec2 bottomRight = cam.screenToWorld(screenWidth, 0);
    float viewportWidth = bottomRight.x - topLeft.x;
    float viewportHeight = bottomRight.y - topLeft.y;
    
    // 积分发射：限制每帧最大发射数，避免 dt 过大时粒子爆炸
    float maxParticlesPerFrame = 50.0f;
    float particlesToEmit = std::min(emitRate * dt, maxParticlesPerFrame);
    
    for (int i = 0; i < static_cast<int>(particlesToEmit); ++i) {
        float x = cam.position.x - viewportWidth / 2 + 
                  (rand() % 1000) / 1000.0f * viewportWidth;
        float y = cam.position.y + viewportHeight / 2 + 1.0f;
        
        Particle p;
        p.position = {x, y};
        p.velocity = {0.0f, -8.0f};  // 向下
        p.lifetime = 2.0f;
        p.maxLifetime = 2.0f;
        p.color = glm::vec3(0.6f, 0.7f, 0.9f);
        p.size = 0.05f;
        p.active = true;
        
        particleSystem->emit(p);
    }
    
    // 处理小数部分：累积到下一帧
    rainAccumulator += emitRate * dt - particlesToEmit;
}

float WeatherSystem::getMovementMultiplier() const {
    switch (currentWeather) {
        case WeatherType::Rain:
            return 1.0f - 0.1f * intensity;  // 减速10%
        case WeatherType::HeavyRain:
            return 1.0f - 0.2f * intensity;  // 减速20%
        case WeatherType::Snow:
            return 1.0f - 0.15f * intensity; // 减速15%
        default:
            return 1.0f;
    }
}

float WeatherSystem::getVisibility() const {
    switch (currentWeather) {
        case WeatherType::Fog:
            return 1.0f - 0.5f * intensity;  // 能见度50%
        case WeatherType::HeavyRain:
            return 1.0f - 0.3f * intensity;  // 能见度70%
        default:
            return 1.0f;
    }
}

glm::vec3 WeatherSystem::getLightModifier() const {
    switch (currentWeather) {
        case WeatherType::Clear:
            return glm::vec3(1.0f, 1.0f, 1.0f);
        case WeatherType::Cloudy:
            return glm::vec3(0.8f, 0.8f, 0.8f);
        case WeatherType::Rain:
            return glm::vec3(0.6f, 0.7f, 0.9f);  // 偏冷色调
        case WeatherType::HeavyRain:
            return glm::vec3(0.5f, 0.6f, 0.8f);  // 更冷更暗
        case WeatherType::Fog:
            return glm::vec3(0.5f, 0.5f, 0.5f);  // 灰色
        case WeatherType::Snow:
            return glm::vec3(0.7f, 0.8f, 0.9f);  // 偏冷但明亮
        default:
            return glm::vec3(1.0f, 1.0f, 1.0f);
    }
}
```

---

## 6. 实现顺序

1. **多区域系统**（MapRegion + RegionManager）— 世界架构基础
2. **A*寻路系统** — 敌人AI升级
3. **存档/读档系统** — 数据持久化
4. **日夜循环系统** — 时间影响世界
5. **天气系统基础** — 雨、雾效果
6. **集成测试与优化**

---

## 7. 验收标准

- [ ] 多区域系统正常工作，区域切换流畅
- [ ] 区域过渡效果（淡入淡出）正确
- [ ] A*寻路系统能避开障碍物找到路径
- [ ] 敌人AI使用寻路系统追踪玩家
- [ ] 存档/读档系统正确保存和恢复游戏状态
- [ ] 存档格式为种子+差异策略，文件大小合理
- [ ] 日夜循环系统正确运行，光照颜色随时间变化
- [ ] 天气系统能切换不同天气效果
- [ ] 雨天粒子效果正确渲染
- [ ] 天气影响移动速度和能见度
- [ ] 帧率稳定60FPS（多区域+天气效果）
- [ ] 无内存泄漏
- [ ] 代码编译无警告（`/W4`）

---

## 8. 留给阶段7-8的内容

- 室内地图细节优化
- 建造模式（网格对齐放置）
- 家具系统
- 音频系统（BGM、音效）
- UI polish（主菜单、设置、成就）
- 主线剧情任务
- 更多敌人类型和Boss战
- 技能树系统
- 装备/道具系统

详见 `doc/技术栈.md` 第6-8阶段。

---

## 9. 已知问题与修正

### 9.1 A* 寻路节点指针失效（已修正）

**问题**：原设计使用 `PathNode* parent` 指针，当 `nodeMap` rehash 时指针悬挂。

**修正**：改用 `glm::ivec2 parentPos` 存储父节点坐标，回溯时通过坐标查找。

### 9.2 RegionManager 区域生命周期（已修正）

**问题**：原设计中 `currentRegion = it->second.get()` 只是原始指针，所有权混乱。

**修正**：统一使用 `std::move` 交换所有权，确保每个区域有且仅有一个 `unique_ptr` 持有。

### 9.3 天气系统依赖注入（已修正）

**问题**：`updateRainEffect` 中使用未定义的 `getCamera()`。

**修正**：通过构造函数注入 `Camera2D*`，并在 `update()` 中传入 `const Camera2D&`。

### 9.4 存档系统与 MapTileManager 联动（已补充）

**问题**：`SaveSystem` 需要获取修改记录，但 `MapRegion` 未暴露接口。

**修正**：在 `MapRegion` 中添加 `getModifications()` 方法，委托给内部的 `MapTileManager`。

### 9.5 区域种子确定性问题（已修正）

**问题**：`std::hash<std::string>` 不保证跨运行一致性。

**修正**：使用固定种子表 + FNV-1a 哈希回退，确保跨运行一致性。

### 9.6 日夜光照应用方式（已补充）

**问题**：`Draw2D` 无 `setGlobalColorMultiplier` 方法，且 `PostProcess` 无 `setUniform()` 方法，无法影响 SDF 着色器。

**修正**：直接通过 `glUseProgram` + `glUniform3f` 为后处理着色器和 SDF 着色器（角色、投射物、装饰物）分别设置 `uAmbientLight` uniform。

### 9.7 天气粒子发射优化（已补充）

**问题**：`emitRate * dt` 在 dt 较大时可能发射过多粒子。

**修正**：限制每帧最大发射数，使用积分累积处理小数部分。

### 9.8 区域查询接口补充（已补充）

**问题**：`getRegion()` 只遍历 `loadedRegions`，遗漏当前活动区域 `currentRegion`。

**修正**：在 `getRegion()` 中优先检查 `regionId == currentRegionId`，匹配则返回 `currentRegion.get()`。

### 9.9 过渡效果实现（已补充）

**问题**：`Draw2D::drawRectFilled` 不支持 alpha 参数，无法实现半透明淡入淡出。

**修正**：在 `Draw2D` 中新增 `drawRectFilledAlpha()` 方法，使用顶点颜色传递 alpha 值。过渡效果中调用此方法，alpha 由 `transitionAlpha` 驱动（淡出时 0→1，淡入时 1→0）。

### 9.10 天气对光照的影响（已补充）

**问题**：日夜循环计算的环境光未考虑天气因素（阴天、雾应降低亮度，雨天偏冷色调）。

**修正**：在 `WeatherSystem` 中添加 `getLightModifier()` 方法，返回天气对光照的影响系数和色调偏移：
- 晴天：系数 1.0，无偏移
- 多云：系数 0.8，无偏移
- 雨天：系数 0.6，偏蓝（冷色调）
- 雾天：系数 0.5，偏灰
最终环境光 = TimeSystem 基础光照 × WeatherSystem 修正系数。

### 9.11 Draw2D 透明矩形支持（已补充）

**问题**：`Draw2D::drawRectFilled` 不支持 alpha 参数，无法实现半透明过渡效果。

**修正**：扩展 `Draw2D::drawRectFilled` 接受 `glm::vec4` 颜色（含 alpha），并在着色器中启用混合。过渡效果中直接调用 `Draw2D::drawRectFilled(x, y, w, h, glm::vec4(0,0,0, transitionAlpha))`。

### 9.12 存档 coins 字段错误（已修正）

**问题**：`saveDataToJson` 中 `j["player"]["coins"] = data.player.progress.coins` 访问了不存在的 `PlayerProgress::coins`。

**修正**：改为 `j["player"]["coins"] = data.player.coins`，coins 字段在 `PlayerData` 结构体中。

### 9.13 物理世界初始化与玩家位置（已补充）

**问题**：`transitionTo` 调用未传递 `b2WorldId`，且切换后未设置玩家新位置。

**修正**：
1. 在 `RegionManager` 中添加 `setWorldId(b2WorldId)` 方法，在初始化后调用。
2. `transitionTo` 中获取连接的目标入口瓦片，切换后计算世界坐标并通知调用方设置玩家位置。
3. 初始区域在 `main.cpp` 中手动调用 `buildPhysics(worldId)`。

### 9.14 光照传递至所有着色器（已补充）

**问题**：`uAmbientLight` 只设置给后处理和角色着色器，但 Draw2D 绘制的地面、UI、粒子等也会受影响。

**修正**：
1. 为 `Draw2D` 着色器添加 `uniform vec3 uAmbientLight`，在 `beginFrame` 时传入。
2. 为 `DecorRenderer` 着色器添加相同 uniform。
3. 在渲染循环中统一计算：`ambient = timeSystem.getAmbientLight() * weatherSystem.getLightModifier()`。
4. 后处理着色器不再承担环境光职责，避免影响 UI。

### 9.15 区域物理体生命周期（已修正）

**问题**：`transitionTo` 中无论区域是否已加载都调用 `clearPhysics()`，导致切换回已访问区域时重复创建物理体。

**修正**：
1. `clearPhysics()` 只在区域被从 `loadedRegions` 中永久移除时调用（析构时）。
2. 对于已存在的区域，仅交换所有权，不重建物理体。
3. 新创建区域时才调用 `buildPhysics(worldId)`。

### 9.16 主循环区域切换缺少 worldId 和玩家位置设置（已修正）

**问题**：主循环中 `regionManager.transitionTo(*conn)` 调用缺少 `worldId` 参数，且切换后未将玩家传送到目标入口位置。

**修正**：
```cpp
if (conn) {
    regionManager.transitionTo(*conn, worldId);
    glm::ivec2 targetTile = conn->targetTile;
    glm::vec2 targetWorld = regionManager.getCurrentRegion()
                            ->getTileMap().tileToWorld(targetTile.x, targetTile.y);
    setPlayerPosition(targetWorld);
    b2Body_SetLinearVelocity(playerBody, b2Vec2_zero);  // 重置速度
}
```

### 9.17 光照传递方式与修正方案不一致（已修正）

**问题**：主循环代码中光照部分仍使用 `glUniform3f` 设置后处理着色器和角色着色器，而 9.14 修正已将方案改为：最终环境光 = timeSystem * weatherModifier，并传递给 Draw2D、DecorRenderer、角色 SDF 等所有着色器，后处理不再承担环境光。

**修正**：更新主循环代码为：
```cpp
glm::vec3 ambient = timeSystem.getAmbientLight() * weatherSystem.getLightModifier();
Draw2D::setAmbientLight(ambient);
characterRenderer.setAmbientLight(ambient);
decorRenderer.setAmbientLight(ambient);
```
并在 Draw2D 和 DecorRenderer 的着色器中添加 `uniform vec3 uAmbientLight`，在绘制时生效。后处理着色器不再需要 `uAmbientLight`。

### 9.18 RegionManager 头文件重复声明（已修正）

**问题**：`RegionManager.h` 中 `getTransitionAlpha()` 被声明了两次（第290行和第307行），会导致编译错误。

**修正**：删除重复声明，只保留一处：
```cpp
// 获取过渡透明度（0=完全可见，1=全黑）
float getTransitionAlpha() const { return transitionAlpha; }
```

### 9.19 着色器示例残留代码（已清理）

**问题**：4.2节末尾有一个孤立的 `void main()` 代码块，是旧版光照方式的残留，不属于任何着色器文件。

**修正**：删除该残留代码块，只保留 `draw2d.frag` 和 `decor.frag` 的修改示例。

### 9.20 存档系统缺少 coins 和 maxMana（已修正）

**问题**：`saveGame` 函数没有保存 `coins` 和 `maxMana`，导致加载时这些值未定义。

**修正**：在 `saveGame` 函数签名中增加 `int playerCoins, float playerMaxMana` 参数，并在实现中赋值：
```cpp
data.player.coins = playerCoins;
data.player.maxMana = playerMaxMana;
```

---

**修订记录**：
- 2024-XX-XX：初始版本
- 2024-XX-XX：修正 A* 寻路指针悬挂、RegionManager 生命周期、天气依赖注入等问题
- 2024-XX-XX：补充 MapTileManager 集成、物理世界切换、过渡效果实现等
- 2024-XX-XX：修正物理世界初始化、区域物理体生命周期、光照传递、粒子接口、存档字段等问题
- 2024-XX-XX：修正主循环区域切换缺少 worldId 和玩家位置设置、光照传递方式与修正方案不一致等问题
- 2024-XX-XX：修正 RegionManager 重复声明、着色器残留代码、存档系统缺少 coins/maxMana 等问题
