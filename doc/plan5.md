# 阶段5：能力深化与地图系统升级

## Context

阶段4已完成情感与公主系统：Lua脚本、对话树、公主小夏（好感度/日程）、委屈值系统、后处理暗角、蒙头哭动画。阶段5目标是**能力深化**与**地图系统升级**——从固定测试地图迈向真正的游戏世界。

**阶段5具体目标：**
1. 地图系统核心升级（通行性、物理同步、噪声地形、装饰物、渲染排序）
2. 新技能/能力扩展
3. 小地图系统
4. 代码清理（移除测试代码、重构调试键）

**留给阶段6的内容：** 多区域系统、A*寻路、存档/读档、日夜循环、天气系统、室内地图。

---

## 1. 地图系统核心升级（P0优先级）

### 1.1 通行性系统重构

**问题**：当前 `TileMap::isSolid()` 只返回 bool，无法表达多级通行属性（可通行/阻挡/水域/熔岩/门/传送门）。

**解决方案**：引入 `Passability` 枚举，每个瓦片类型关联完整的通行属性。

```cpp
// TileMap.h

/// 通行属性（独立于瓦片类型）
enum class Passability : uint8_t {
    Walkable = 0,    // 可自由通行（Grass, Dirt, Path...）
    Blocked = 1,     // 完全阻挡（Wall, Stone...）
    Water = 2,       // 可通行但有效果（减速）
    Lava = 3,        // 可通行但持续伤害
    Door = 4,        // 条件通行（需要钥匙/开关）
    Portal = 5,      // 触发区域切换
};

/// 瓦片类型扩展
enum class TileType : uint8_t {
    // 地面（Walkable）
    Grass = 0, Dirt = 1, Stone = 2, Path = 5,
    Sand = 6, Snow = 7,
    WoodFloor = 20, StoneFloor = 21, Carpet = 22,
    // 液体（Water/Lava）
    Water = 3, DeepWater = 9, Lava = 8,
    // 阻挡（Blocked）
    Wall = 4,
    // 特殊
    Bridge = 10, Door = 11, Portal = 12,
    Count
};

/// 瓦片元数据：每个瓦片类型的静态属性
struct TileDef {
    TileType type;
    Passability passability;
    const char* name;
    glm::vec3 color;
    float movementCost;    // 移动消耗倍率（1.0=正常，2.0=减速一半）
    float damagePerSecond; // 持续伤害（Lava=10, Water=0）
    bool createsPhysicsBody; // 是否创建Box2D静态刚体
};

// 全局瓦片定义表（编译期常量）
constexpr TileDef TILE_DEFS[] = {
    {TileType::Grass,   Passability::Walkable, "草地", {0.3f, 0.7f, 0.25f}, 1.0f, 0.0f, false},
    {TileType::Dirt,    Passability::Walkable, "泥土", {0.6f, 0.42f, 0.25f}, 1.0f, 0.0f, false},
    {TileType::Stone,   Passability::Blocked,  "石头", {0.6f, 0.6f, 0.55f}, 0.0f, 0.0f, true},
    {TileType::Water,   Passability::Water,    "水",   {0.15f, 0.5f, 0.8f},  1.5f, 0.0f, false},
    {TileType::Wall,    Passability::Blocked,  "墙",   {0.45f, 0.42f, 0.45f}, 0.0f, 0.0f, true},
    {TileType::Path,    Passability::Walkable, "路径", {0.7f, 0.58f, 0.4f},  0.8f, 0.0f, false},
    {TileType::Sand,    Passability::Walkable, "沙地", {0.76f, 0.7f, 0.5f},  1.2f, 0.0f, false},
    {TileType::Lava,    Passability::Lava,     "熔岩", {0.8f, 0.2f, 0.0f},   2.0f, 10.0f, false},
    {TileType::DeepWater, Passability::Water,  "深水", {0.1f, 0.3f, 0.6f},   2.5f, 0.0f, false},
    {TileType::Door,    Passability::Door,     "门",   {0.5f, 0.35f, 0.2f},  0.0f, 0.0f, true},
    {TileType::Portal,  Passability::Portal,   "传送门",{0.6f, 0.2f, 0.9f},   0.0f, 0.0f, false},
};

inline const TileDef& getTileDef(TileType type) {
    return TILE_DEFS[static_cast<size_t>(type)];
}

inline bool isWalkable(TileType type) {
    return getTileDef(type).passability != Passability::Blocked;
}
```

**与物理系统的对应**：
- `createsPhysicsBody == true` → 创建 Box2D 静态刚体
- `Water/Lava` → 不创建刚体，通过每帧检测玩家所在瓦片施加效果
- `Door` → 初始创建刚体，触发开关后销毁刚体

**迁移步骤**：
1. 在 `TileMap.h` 中添加 `Passability` 枚举和 `TileDef` 结构
2. 更新 `TileMap::isSolid()` 使用 `getTileDef().createsPhysicsBody`
3. 添加 `TileMap::getPassability(int x, int y)` 方法
4. 在玩家移动代码中，根据 `movementCost` 调整速度
5. 在每帧更新中，检测玩家所在瓦片的 `damagePerSecond` 并扣血

### 1.2 MapTileManager — 统一瓦片管理与物理同步

**问题**：当前 `TileMap::setTile()` 只修改数据，不会同步更新 Box2D 刚体。破坏墙体/填水时会出现"空气墙"。

**解决方案**：引入 `MapTileManager` 作为 TileMap 与 Box2D 之间的协调层。

```cpp
// MapTileManager.h
#pragma once

#include "TileMap.h"
#include <box2d/box2d.h>
#include <unordered_map>
#include <glm/ivec2.hpp>

/**
 * 瓦片修改记录（用于存档）
 */
struct TileModification {
    int x, y;
    TileType oldType, newType;
    double timestamp;
};

/**
 * MapTileManager — 统一瓦片管理与物理同步
 *
 * 所有瓦片修改必须通过此类进行，以确保：
 * 1. Box2D 物理刚体自动同步
 * 2. 修改记录可用于存档
 * 3. 批量操作的事务性
 */
class MapTileManager {
public:
    // 初始化：从 TileMap 创建所有必要的物理刚体
    void init(TileMap& map, b2WorldId world);

    // 安全修改瓦片类型（自动同步物理）
    bool setTile(int x, int y, TileType newType);

    // 批量修改（事务性）
    bool setTiles(const std::vector<glm::ivec2>& positions, TileType newType);

    // 查询：获取某位置的 Box2D 刚体
    b2BodyId getBodyAt(int x, int y) const;

    // 清理：释放所有物理资源
    void shutdown();

    // 获取修改记录（用于存档）
    const std::vector<TileModification>& getModifications() const { return modifications; }
    void clearModifications() { modifications.clear(); }

    // 加载时临时禁用修改记录
    void setReplayingMode(bool replaying) { isReplaying = replaying; }

private:
    TileMap* tileMap = nullptr;
    b2WorldId worldId;

    // 瓦片坐标 → Box2D 刚体ID 映射
    std::unordered_map<uint64_t, b2BodyId> tileBodies;

    std::vector<TileModification> modifications;
    bool isReplaying = false;

    // 编码瓦片坐标为单一 uint64 键
    static uint64_t packCoord(int x, int y) {
        return (static_cast<uint64_t>(x) << 32) | static_cast<uint32_t>(y);
    }

    // 为单个瓦片创建/移除物理刚体
    void createBodyFor(int x, int y);
    void destroyBodyFor(int x, int y);

    // 判断瓦片类型是否需要物理刚体
    static bool needsBody(TileType type) {
        return getTileDef(type).createsPhysicsBody;
    }
};
```

**setTile 实现流程**：

```cpp
// MapTileManager.cpp
bool MapTileManager::setTile(int x, int y, TileType newType) {
    if (!tileMap->isInBounds(x, y)) return false;

    TileType oldType = tileMap->getTile(x, y);
    if (oldType == newType) return true;

    bool oldNeedsBody = needsBody(oldType);
    bool newNeedsBody = needsBody(newType);

    // 更新瓦片数据
    tileMap->setTile(x, y, newType);

    // 物理同步
    if (oldNeedsBody && !newNeedsBody) {
        destroyBodyFor(x, y);
    } else if (!oldNeedsBody && newNeedsBody) {
        createBodyFor(x, y);
    }

    // 记录修改（用于存档）
    if (!isReplaying) {
        modifications.push_back({x, y, oldType, newType, getTime()});
    }

    return true;
}

void MapTileManager::createBodyFor(int x, int y) {
    glm::vec2 worldPos = tileMap->tileToWorld(x, y);
    b2BodyDef bd = b2BodyDef{.position = {worldPos.x, worldPos.y}};
    b2BodyId bodyId = b2CreateBody(worldId, &bd);

    // 1x1 瓦片的 AABB 形状
    b2ShapeDef sd = b2ShapeDef{.density = 0.0f, .friction = 0.3f};
    b2BoxShape box = b2MakeBox(0.5f, 0.5f);
    b2CreateBoxShape(bodyId, &sd, &box);

    tileBodies[packCoord(x, y)] = bodyId;
}

void MapTileManager::destroyBodyFor(int x, int y) {
    auto it = tileBodies.find(packCoord(x, y));
    if (it != tileBodies.end()) {
        b2DestroyBody(it->second);
        tileBodies.erase(it);
    }
}
```

**使用方式**：

```cpp
// 旧方式（有问题）：
gs.tileMap.setTile(5, 3, TileType::Grass);  // 物理不同步！

// 新方式：
gs.tileManager.setTile(5, 3, TileType::Grass);  // 自动移除墙体刚体
```

### 1.3 噪声地形生成

**问题**：当前 `TileMap::generate()` 每瓦片独立随机，地形呈噪点状，缺乏连贯性。

**解决方案**：使用 Perlin 噪声 + fBm（分形布朗运动）生成连贯地形。

```cpp
// TerrainGenerator.h
#pragma once

#include "TileMap.h"

class TerrainGenerator {
public:
    // 使用 Perlin 噪声生成连贯地形
    static void generateCoherent(
        TileMap& map,
        int seed,
        float scale = 0.1f,        // 噪声缩放
        float persistence = 0.5f,  // 持久度
        int octaves = 4,           // 八度数
        float waterLevel = 0.35f   // 水平面高度
    );

    // 生成路径网络（连接关键区域）
    static void generatePaths(TileMap& map, int numPaths = 3, unsigned int seed = 0);

private:
    // 2D Perlin 噪声（基于种子）
    static float perlinNoise(float x, float y, unsigned int seed);

    // 分形布朗运动
    static float fbm(float x, float y, int octaves, float persistence, unsigned int seed);

    // 高度 → 瓦片类型映射
    static TileType heightToTile(float height, float waterLevel);
};
```

**生成流程**：

```
1. 初始化噪声发生器（种子）
2. 为每个瓦片计算噪声值（-1.0 ~ 1.0）
3. 根据噪声值映射地形类型：
   - < -0.3: DeepWater
   - -0.3 ~ 0.0: Water
   - 0.0 ~ 0.2: Sand
   - 0.2 ~ 0.5: Grass
   - 0.5 ~ 0.7: Dirt
   - 0.7 ~ 0.85: Stone
   - > 0.85: Wall（山峰）
4. 后处理：
   - 平滑边界
   - 生成路径连接关键区域
   - 确保可通行区域连通
```

**Perlin 噪声实现**（简化版，可替换为库）：

```cpp
// 简化的 Perlin 噪声实现
float TerrainGenerator::perlinNoise(float x, float y, unsigned int seed) {
    // 使用哈希函数生成伪随机梯度
    auto hash = [seed](int ix, int iy) -> float {
        unsigned int n = static_cast<unsigned int>(ix * 374761393 + iy * 668265263 + seed);
        n = (n ^ (n >> 13)) * 1274126177;
        return static_cast<float>(n & 0x7fffffff) / 2147483647.0f * 2.0f - 1.0f;
    };

    // 双线性插值
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - ix;
    float fy = y - iy;

    float g00 = hash(ix, iy);
    float g10 = hash(ix + 1, iy);
    float g01 = hash(ix, iy + 1);
    float g11 = hash(ix + 1, iy + 1);

    float u = fx * fx * (3.0f - 2.0f * fx);
    float v = fy * fy * (3.0f - 2.0f * fy);

    return glm::mix(
        glm::mix(g00, g10, u),
        glm::mix(g01, g11, u),
        v
    );
}

float TerrainGenerator::fbm(float x, float y, int octaves, float persistence, unsigned int seed) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += perlinNoise(x * frequency, y * frequency, seed + i * 1000) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue;
}

TileType TerrainGenerator::heightToTile(float height, float waterLevel) {
    if (height < waterLevel - 0.3f) return TileType::DeepWater;
    if (height < waterLevel) return TileType::Water;
    if (height < waterLevel + 0.2f) return TileType::Sand;
    if (height < waterLevel + 0.5f) return TileType::Grass;
    if (height < waterLevel + 0.7f) return TileType::Dirt;
    if (height < waterLevel + 0.85f) return TileType::Stone;
    return TileType::Wall;
}
```

### 1.4 装饰物系统基础

**目标**：添加树木、灌木、花草等装饰物，使用独立 SDF 渲染器（不通过 Draw2D）。

```cpp
// Decoration.h
#pragma once

#include <glm/vec2.hpp>
#include <cstdint>

enum class DecorType : uint8_t {
    None = 0,
    Tree = 1,
    Bush = 2,
    Flower = 3,
    TallGrass = 4,
    Rock = 5,
    Stump = 6,
    Count
};

struct Decoration {
    DecorType type;
    uint8_t variant;      // 变种（同类型不同外观）
    uint8_t rotation;     // 旋转角度（0-3，每90度）
    uint8_t scale;        // 缩放（0=小, 1=中, 2=大）
    int16_t tileX;
    int16_t tileY;

    float getScaleFactor() const {
        return scale == 0 ? 0.7f : (scale == 1 ? 1.0f : 1.3f);
    }
};
```

**DecorRenderer — 装饰物批处理渲染器**：

```cpp
// DecorRenderer.h
#pragma once

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include "Decoration.h"

class DecorRenderer {
public:
    void init();
    void shutdown();

    void beginFrame(const glm::mat4& viewProj);
    void addDecor(const glm::vec2& pos, DecorType type, int variant,
                  float rotation = 0.0f, float scale = 1.0f);
    void endFrame();

private:
    GLuint shader = 0;
    GLuint vao = 0, vbo = 0, instanceVBO = 0;

    struct DecorInstance {
        glm::vec2 position;
        float rotation;
        float scale;
        int type;
        int variant;
        float time;
    };
    std::vector<DecorInstance> instances;

    float currentTime = 0.0f;
};
```

**着色器 `assets/shaders/decor.vert`**：

```glsl
#version 330 core
layout(location = 0) in vec2 aPos;        // 单位四边形顶点（-0.5~0.5）
layout(location = 1) in vec2 iPosition;   // 实例：世界位置
layout(location = 2) in float iRotation;  // 实例：旋转
layout(location = 3) in float iScale;     // 实例：缩放
layout(location = 4) in int iType;        // 实例：类型
layout(location = 5) in int iVariant;     // 实例：变种

uniform mat4 uViewProj;
uniform float uTime;

out vec2 vWorldPos;
out float vTime;
out int vType;
out int vVariant;

void main() {
    vec2 pos = aPos * iScale;

    float c = cos(iRotation);
    float s = sin(iRotation);
    pos = vec2(c * pos.x - s * pos.y, s * pos.x + c * pos.y);

    vec2 worldPos = iPosition + pos;
    vWorldPos = worldPos;
    vTime = uTime;
    vType = iType;
    vVariant = iVariant;

    gl_Position = uViewProj * vec4(worldPos, 0.0, 1.0);
}
```

**着色器 `assets/shaders/decor.frag`**：

```glsl
#version 330 core
in vec2 vWorldPos;
in float vTime;
in int vType;
in int vVariant;
out vec4 FragColor;

float sdCircle(vec2 p, float r) { return length(p) - r; }

float sdCapsule(vec2 p, float len, float rad) {
    p.y -= clamp(p.y, 0.0, len);
    return length(p) - rad;
}

void renderTree(vec2 uv, int variant, float time) {
    // 树干
    float trunk = sdCapsule(uv - vec2(0.0, -0.2), 0.3, 0.08);
    vec3 trunkColor = vec3(0.4, 0.25, 0.1);

    // 树冠（根据 variant 变化）
    float crown1 = sdCircle(uv - vec2(0.0, 0.3), 0.35);
    float crown2 = sdCircle(uv - vec2(-0.15, 0.15), 0.25);
    float crown3 = sdCircle(uv - vec2(0.15, 0.15), 0.25);
    float crown = min(crown1, min(crown2, crown3));
    vec3 leafColor = vec3(0.2, 0.6, 0.15);

    // 风吹效果
    leafColor += sin(time * 2.0 + vVariant) * 0.03;

    float tree = min(trunk, crown);
    float alpha = 1.0 - smoothstep(0.0, 0.02, tree);
    vec3 color = trunk < crown ? trunkColor : leafColor;
    FragColor = vec4(color, alpha);
}

void renderBush(vec2 uv, int variant) {
    float b1 = sdCircle(uv - vec2(-0.1, 0.0), 0.25);
    float b2 = sdCircle(uv - vec2(0.1, 0.05), 0.2);
    float bush = min(b1, b2);
    float alpha = 1.0 - smoothstep(0.0, 0.02, bush);
    vec3 color = vec3(0.15, 0.5, 0.1);
    color += variant * 0.05; // 颜色变化
    FragColor = vec4(color, alpha);
}

void renderFlower(vec2 uv) {
    float flower = sdCircle(uv, 0.1);
    float alpha = 1.0 - smoothstep(0.0, 0.02, flower);
    FragColor = vec4(0.9, 0.3, 0.5, alpha);
}

void renderRock(vec2 uv) {
    // 简单岩石形状
    float rock = sdCircle(uv, 0.2);
    rock = max(rock, -sdCircle(uv + vec2(0.1, 0.05), 0.15));
    float alpha = 1.0 - smoothstep(0.0, 0.02, rock);
    FragColor = vec4(0.5, 0.48, 0.45, alpha);
}

void main() {
    if (vType == 1) {
        renderTree(vWorldPos, vVariant, vTime);
    } else if (vType == 2) {
        renderBush(vWorldPos, vVariant);
    } else if (vType == 3) {
        renderFlower(vWorldPos);
    } else if (vType == 5) {
        renderRock(vWorldPos);
    } else {
        discard;
    }
}
```

**装饰物生成**：

```cpp
// 在 TileMap 或 TerrainGenerator 中添加
void generateDecorations(TileMap& map, std::vector<Decoration>& decors,
                         int seed, float density = 0.15f) {
    std::mt19937 rng(static_cast<unsigned>(seed));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            if (dist(rng) > density) continue;

            TileType tile = map.getTile(x, y);
            if (tile != TileType::Grass && tile != TileType::Dirt && tile != TileType::Sand)
                continue;

            Decoration decor;
            decor.tileX = x;
            decor.tileY = y;

            // 根据地面类型选择装饰物
            float r = dist(rng);
            if (tile == TileType::Grass) {
                if (r < 0.4f) decor.type = DecorType::Tree;
                else if (r < 0.7f) decor.type = DecorType::Bush;
                else if (r < 0.9f) decor.type = DecorType::Flower;
                else decor.type = DecorType::TallGrass;
            } else if (tile == TileType::Sand) {
                decor.type = DecorType::Rock;
            } else {
                decor.type = DecorType::Rock;
            }

            decor.variant = static_cast<uint8_t>(dist(rng) * 4);
            decor.rotation = static_cast<uint8_t>(dist(rng) * 4);
            decor.scale = static_cast<uint8_t>(dist(rng) * 3);

            decors.push_back(decor);
        }
    }
}
```

### 1.5 渲染图层排序（Y轴深度排序）

**问题**：当地图引入装饰物（树木）后，需要处理角色与装饰物的前后遮挡关系。

**解决方案**：所有需要深度排序的对象收集到渲染队列，按图层 + y 坐标排序后绘制。

```cpp
// RenderLayer.h
#pragma once

/// 渲染图层（按绘制顺序）
enum class RenderLayer : int {
    Ground = 0,      // 地面瓦片层
    Water = 1,       // 水面层（半透明）
    DecorLow = 2,    // 低矮装饰（花草）
    Objects = 3,     // 可交互对象 + 角色（按 y 排序）
    DecorHigh = 4,   // 高大装饰（树木）
    Effects = 5,     // 粒子特效
    UI = 6,          // UI层
    Count
};

struct Renderable {
    float y;              // 世界 y 坐标（排序键）
    RenderLayer layer;

    bool operator<(const Renderable& other) const {
        if (layer != other.layer)
            return static_cast<int>(layer) < static_cast<int>(other.layer);
        return y < other.y;
    }
};
```

**渲染流程**：

```cpp
void renderScene(GameState& gs, const glm::mat4& viewProj) {
    // 1. 地面层（固定顺序）
    renderGroundLayer(gs.tileMap, viewProj);

    // 2. 水面层
    renderWaterLayer(gs.tileMap, viewProj);

    // 3. 低矮装饰层
    renderDecorLow(gs.decorations, viewProj);

    // 4. 收集对象层所有可渲染实体
    std::vector<Renderable> renderQueue;

    // 添加角色
    renderQueue.push_back({player.position.y, RenderLayer::Objects});

    // 添加掉落物
    for (auto& drop : gs.drops) {
        renderQueue.push_back({drop.position.y, RenderLayer::Objects});
    }

    // 添加高大装饰物（树木）
    for (auto& decor : gs.decorations) {
        if (isTallDecor(decor.type)) {
            glm::vec2 worldPos = gs.tileMap.tileToWorld(decor.tileX, decor.tileY);
            renderQueue.push_back({worldPos.y, RenderLayer::DecorHigh});
        }
    }

    // 排序
    std::sort(renderQueue.begin(), renderQueue.end());

    // 按排序顺序绘制
    for (const auto& r : renderQueue) {
        if (r.layer == RenderLayer::Objects) {
            // 绘制角色/掉落物
        } else if (r.layer == RenderLayer::DecorHigh) {
            // 绘制高大装饰物
        }
    }

    // 5-6. 特效、UI
    renderEffects(gs.particles, viewProj);
    renderUI(gs);
}
```

---

## 2. 新技能/能力扩展

### 2.1 技能配置 Lua 化

将 `assets/scripts/abilities.lua` 扩展为完整的技能数据库：

```lua
-- abilities.lua
abilities = {
    fireball = {
        name = "火球术",
        manaCost = 15,
        cooldown = 0.3,
        projectileSpeed = 400,
        damage = 25,
        lifetime = 2.0,
        particleColor = {1.0, 0.4, 0.0},
        description = "发射一枚火球"
    },

    ice_spike = {
        name = "冰锥术",
        manaCost = 20,
        cooldown = 0.5,
        projectileSpeed = 500,
        damage = 35,
        lifetime = 1.5,
        particleColor = {0.5, 0.8, 1.0},
        effect = "slow",  -- 减速效果
        effectDuration = 2.0,
        description = "发射冰锥，减速敌人"
    },

    dash = {
        name = "冲刺",
        manaCost = 10,
        cooldown = 1.0,
        distance = 5.0,
        duration = 0.2,
        invincible = true,  -- 冲刺期间无敌
        description = "快速向前冲刺"
    }
}
```

### 2.2 冰锥术（减速投射物）

```cpp
// 在 Projectile.h 中添加
struct ProjectileDef {
    float speed;
    float damage;
    float lifetime;
    glm::vec3 particleColor;
    // 新增
    std::string effectType;       // "slow", "burn", etc.
    float effectDuration;
};

// 在 Enemy.h 中添加减速状态
struct Enemy {
    float slowTimer = 0.0f;
    float slowMultiplier = 1.0f;

    void applySlow(float duration, float multiplier) {
        slowTimer = duration;
        slowMultiplier = multiplier;
    }

    void update(float dt) {
        if (slowTimer > 0) {
            slowTimer -= dt;
            if (slowTimer <= 0) {
                slowMultiplier = 1.0f;
            }
        }
    }
};
```

### 2.3 冲刺能力

```cpp
// 在 GameState 或 Player 中添加
struct DashState {
    bool active = false;
    float timer = 0.0f;
    float duration = 0.2f;
    glm::vec2 direction;
    float distance = 5.0f;
    bool invincible = false;
};

// 在输入处理中
if (scancode == SDL_SCANCODE_L && !isDashing) {
    isDashing = true;
    dashTimer = 0.0f;
    dashDirection = glm::normalize(playerVelocity);
    player.invincible = true;
}

// 在更新中
if (isDashing) {
    dashTimer += dt;
    b2Vec2 dashForce = {dashDirection.x * 20.0f, dashDirection.y * 20.0f};
    b2Body_ApplyForceToCenter(playerBodyId, dashForce, true);

    if (dashTimer >= dashDuration) {
        isDashing = false;
        player.invincible = false;
    }
}
```

---

## 3. 小地图系统

```cpp
// MiniMap.h
#pragma once

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include "TileMap.h"

class MiniMap {
public:
    void init(int size = 150);
    void shutdown();

    void requestUpdate();
    void update(float deltaTime, const TileMap& map, const glm::vec2& playerPos);
    void render(const glm::mat4& orthoProj, int screenWidth, int screenHeight);
    void forceUpdate(const TileMap& map, const glm::vec2& playerPos);

    void setVisible(bool visible) { isVisible = visible; }
    bool isVisible() const { return isVisible; }

private:
    GLuint texture = 0;
    GLuint vao = 0, vbo = 0;
    int mapSize = 150;
    std::vector<uint8_t> minimapData;

    float updateTimer = 0.0f;
    float updateInterval = 1.0f;
    bool isDirty = false;
    bool isVisible = true;

    void updateTexture(const TileMap& map, const glm::vec2& playerPos);
    glm::ivec2 worldToMinimap(const glm::vec2& worldPos, const TileMap& map);
};
```

**关键优化**：小地图纹理不每帧更新，而是每秒更新一次（或事件触发）。

---

## 4. 代码清理

### 4.1 删除测试代码

| 代码 | 位置 | 操作 |
|------|------|------|
| `showTestShapes` 覆盖层 | main.cpp | 删除 |
| T 键切换测试覆盖层 | main.cpp | 删除 |
| `basic.vert/.frag` | assets/shaders/ | 删除 |

### 4.2 调试键包装

将 H 键（回血）和 E 键（调试交互）包装在 `#ifdef DEBUG` 中：

```cpp
#ifdef DEBUG
if (scancode == SDL_SCANCODE_H) {
    playerHealth.heal(30);
}
#endif
```

### 4.3 重构 TileMap::generate()

替换为 `TerrainGenerator::generateCoherent()`。

---

## 5. 实现顺序

1. **通行性系统重构**（TileMap.h 扩展）— 基础架构，影响后续所有模块
2. **MapTileManager** — 物理同步，解决核心 bug
3. **噪声地形生成** — 替换现有 generate()
4. **装饰物系统** — DecorRenderer + 着色器 + 生成逻辑
5. **渲染图层排序** — 整合角色与装饰物
6. **小地图** — 独立模块
7. **新技能** — 冰锥、冲刺
8. **代码清理** — 穿插进行

---

## 6. 验收标准

- [ ] 通行性系统支持 6 种 Passability，与 Box2D 正确对应
- [ ] MapTileManager 正确同步物理（破坏墙体后无空气墙）
- [ ] 地形生成具有连贯性（Perlin 噪声，非纯随机）
- [ ] 装饰物正确渲染（树木、灌木、花草、岩石）
- [ ] 角色与高大装饰物正确遮挡（y 轴排序）
- [ ] 小地图正确显示地形和玩家位置（每秒更新）
- [ ] 新技能可用（冰锥减速、冲刺无敌）
- [ ] 测试代码已清理
- [ ] 帧率稳定 60FPS（100×100 地图 + 装饰物）
- [ ] 无内存泄漏

---

## 7. 留给阶段6的内容

- 多区域系统（MapRegion / MapConnection / RegionManager）
- A* 寻路系统
- 存档/读档（种子 + 差异策略）
- 日夜循环对地图的影响
- 天气系统（雨、雾）
- 室内地图

详见 `doc/map-spec.md` 第5、6、8节。
