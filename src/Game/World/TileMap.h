#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>
#include <cassert>

/**
 * 通行属性（独立于瓦片类型）
 */
enum class Passability : uint8_t {
    Walkable = 0,    // 可自由通行（Grass, Dirt, Path...）
    Blocked = 1,     // 完全阻挡（Wall, Stone...）
    Water = 2,       // 可通行但有效果（减速）
    Lava = 3,        // 可通行但持续伤害
    Door = 4,        // 条件通行（需要钥匙/开关）
    Portal = 5,      // 触发区域切换
};

/**
 * 瓦片类型枚举
 */
enum class TileType : uint8_t {
    // 地面（Walkable）
    Grass = 0,
    Dirt = 1,
    Stone = 2,
    Path = 5,
    Sand = 6,
    Snow = 7,
    // 液体（Water/Lava）
    Water = 3,
    DeepWater = 9,
    Lava = 8,
    // 阻挡（Blocked）
    Wall = 4,
    // 特殊
    Bridge = 10,
    Door = 11,
    Portal = 12,
    Count
};

/**
 * 瓦片元数据：每个瓦片类型的静态属性
 */
struct TileDef {
    TileType type;
    Passability passability;
    const char* name;
    glm::vec3 color;
    float movementCost;      // 移动消耗倍率（1.0=正常，2.0=减速一半）
    float damagePerSecond;   // 持续伤害（Lava=10, Water=0）
    bool createsPhysicsBody; // 是否创建Box2D静态刚体
};

// 全局瓦片定义表（编译期常量）
inline const TileDef& getTileDef(TileType type) {
    static constexpr TileDef TILE_DEFS[] = {
        {TileType::Grass,     Passability::Walkable, "草地",   {0.3f,  0.7f,  0.25f}, 1.0f, 0.0f, false},
        {TileType::Dirt,      Passability::Walkable, "泥土",   {0.6f,  0.42f, 0.25f}, 1.0f, 0.0f, false},
        {TileType::Stone,     Passability::Blocked,  "石头",   {0.6f,  0.6f,  0.55f}, 0.0f, 0.0f, true},
        {TileType::Water,     Passability::Water,    "水",     {0.15f, 0.5f,  0.8f},  1.5f, 0.0f, false},
        {TileType::Wall,      Passability::Blocked,  "墙",     {0.45f, 0.42f, 0.45f}, 0.0f, 0.0f, true},
        {TileType::Path,      Passability::Walkable, "路径",   {0.7f,  0.58f, 0.4f},  0.8f, 0.0f, false},
        {TileType::Sand,      Passability::Walkable, "沙地",   {0.76f, 0.7f,  0.5f},  1.2f, 0.0f, false},
        {TileType::Lava,      Passability::Lava,     "熔岩",   {0.8f,  0.2f,  0.0f},  2.0f, 10.0f, false},
        {TileType::DeepWater, Passability::Water,    "深水",   {0.1f,  0.3f,  0.6f},  2.5f, 0.0f, false},
        {TileType::Bridge,    Passability::Walkable, "桥",     {0.55f, 0.4f,  0.3f},  1.0f, 0.0f, false},
        {TileType::Door,      Passability::Door,     "门",     {0.5f,  0.35f, 0.2f},  0.0f, 0.0f, true},
        {TileType::Portal,    Passability::Portal,   "传送门", {0.6f,  0.2f,  0.9f},  0.0f, 0.0f, false},
        {TileType::Snow,      Passability::Walkable, "雪地",   {0.9f,  0.9f,  0.95f}, 1.1f, 0.0f, false},
    };
    // Bounds check — assert in debug, return Grass in release
    size_t idx = static_cast<size_t>(type);
    constexpr size_t count = sizeof(TILE_DEFS) / sizeof(TILE_DEFS[0]);
    assert(idx < count && "Invalid TileType in getTileDef");
    if (idx >= count) return TILE_DEFS[0];
    return TILE_DEFS[idx];
}

inline bool isWalkable(TileType type) {
    return getTileDef(type).passability != Passability::Blocked;
}

/**
 * 瓦片颜色配置（向后兼容的便捷接口）
 */
struct TileColors {
    glm::vec3 grass{0.3f, 0.7f, 0.25f};
    glm::vec3 dirt{0.6f, 0.42f, 0.25f};
    glm::vec3 stone{0.6f, 0.6f, 0.55f};
    glm::vec3 water{0.15f, 0.5f, 0.8f};
    glm::vec3 wall{0.45f, 0.42f, 0.45f};
    glm::vec3 path{0.7f, 0.58f, 0.4f};

    glm::vec3 get(TileType type) const { return getTileDef(type).color; }
};

/**
 * TileMap - 瓦片地图
 *
 * 存储2D瓦片网格，支持加载、程序化生成、坐标转换。
 * 瓦片数据按行优先存储：tiles[y * width + x]
 */
struct TileMap {
    int width = 0;
    int height = 0;
    float tileSize = 1.0f;  // 每个瓦片的世界单位大小
    std::vector<TileType> tiles;

    TileMap() = default;
    TileMap(int w, int h, float ts = 1.0f);

    // 从文本文件加载地图
    // 格式：第一行 "width height"，之后每行 width 个数字（瓦片类型）
    bool load(const char* filename);

    // 程序化生成地图（确定性伪随机）
    void generate(int seed);

    // 获取瓦片类型
    TileType getTile(int x, int y) const;

    // 设置瓦片类型
    void setTile(int x, int y, TileType type);

    // 是否不可通行（向后兼容：基于TileDef的createsPhysicsBody）
    bool isSolid(int x, int y) const;

    // 获取通行属性
    Passability getPassability(int x, int y) const;

    // 获取移动消耗倍率
    float getMovementCost(int x, int y) const;

    // 获取每秒伤害
    float getDamagePerSecond(int x, int y) const;

    // 世界坐标转瓦片坐标
    glm::ivec2 worldToTile(float wx, float wy) const;

    // 瓦片坐标转世界坐标（返回瓦片中心）
    glm::vec2 tileToWorld(int tx, int ty) const;

    // 检查坐标是否在地图范围内
    bool isInBounds(int x, int y) const;

    // 获取地图总瓦片数
    int getTileCount() const { return width * height; }

    // 清空地图
    void clear();
};
