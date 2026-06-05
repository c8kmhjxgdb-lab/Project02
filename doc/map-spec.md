# 地图系统详细设计规范

## 1. 现状分析

### 1.1 已有功能
- **基础瓦片系统**：6种瓦片类型（Grass、Dirt、Stone、Water、Wall、Path）
- **程序化生成**：基于种子的确定性伪随机生成
- **物理集成**：固体瓦片（Wall/Water）自动创建Box2D静态刚体
- **视锥剔除**：只渲染摄像机可见范围内的瓦片
- **坐标转换**：世界坐标↔瓦片坐标双向转换

### 1.2 当前局限
- 地图尺寸固定（40×30），仅用于测试，无多区域支持
- 生成算法简单（纯随机，无地形连贯性）
- 无装饰物系统（树木、花草、岩石等）
- 无动态元素（水流动画、天气效果）
- 无室内/室外区分
- 无兴趣点（POI）标记
- 无路径引导系统
- 瓦片颜色固定，无季节/时间变化

## 2. 地图系统架构设计

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                     MapManager（地图管理器）                   │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ Region      │  │ Region      │  │ Region              │  │
│  │ (室外区域)   │  │ (室内区域)   │  │ (地下城区域)         │  │
│  ├─────────────┤  ├─────────────┤  ├─────────────────────┤  │
│  │ TileLayer   │  │ TileLayer   │  │ TileLayer           │  │
│  │ DecorLayer  │  │ ObjectLayer │  │ TrapLayer           │  │
│  │ ObjectLayer │  │ LightLayer  │  │ ObjectLayer         │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ TerrainGen  │  │ DecorationGen│ │ PathfindingSystem   │  │
│  │ (地形生成器) │  │ (装饰生成器)  │ │ (寻路系统)           │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 核心数据结构

```cpp
// 通行属性（独立于瓦片类型）
enum class Passability : uint8_t {
    Walkable = 0,    // 可自由通行（Grass, Dirt, Path, WoodFloor...）
    Blocked = 1,     // 完全阻挡（Wall, Stone, Boulder...）
    Water = 2,       // 可通行但有效果（减速/游泳/受伤）
    Lava = 3,        // 可通行但持续伤害
    Door = 4,        // 条件通行（需要钥匙/开关）
    Portal = 5,      // 触发区域切换
};

// 瓦片类型扩展
enum class TileType : uint8_t {
    // 地面（Walkable）
    Grass = 0, Dirt = 1, Stone = 2, Sand = 6, Snow = 7,
    Path = 5, WoodFloor = 20, StoneFloor = 21, Carpet = 22,
    // 液体（Water/Lava）
    Water = 3, DeepWater = 9, Lava = 8,
    // 阻挡（Blocked）
    Wall = 4,
    // 特殊
    Bridge = 10, Door = 11, Portal = 12,
    Count
};

// 瓦片元数据：每个瓦片类型的静态属性
struct TileDef {
    TileType type;
    Passability passability;
    const char* name;
    glm::vec3 color;
    float movementCost;    // 移动消耗倍率（1.0=正常，2.0=减速一半）
    float damagePerSecond; // 持续伤害（Lava=10, Water=0）
    bool isTransparent;    // 是否透明（用于渲染层次）
};

// 全局瓦片定义表（编译期常量）
constexpr TileDef TILE_DEFS[] = {
    {TileType::Grass,   Passability::Walkable, "草地",  {0.3f, 0.7f, 0.25f}, 1.0f, 0.0f, false},
    {TileType::Dirt,    Passability::Walkable, "泥土",  {0.6f, 0.42f, 0.25f}, 1.0f, 0.0f, false},
    {TileType::Stone,   Passability::Blocked,  "石头",  {0.6f, 0.6f, 0.55f}, 0.0f, 0.0f, false},
    {TileType::Water,   Passability::Water,    "水",    {0.15f, 0.5f, 0.8f},  1.5f, 0.0f, true},
    {TileType::Wall,    Passability::Blocked,  "墙",    {0.45f, 0.42f, 0.45f}, 0.0f, 0.0f, false},
    {TileType::Path,    Passability::Walkable, "路径",  {0.7f, 0.58f, 0.4f},  0.8f, 0.0f, false},
    {TileType::Sand,    Passability::Walkable, "沙地",  {0.76f, 0.7f, 0.5f},  1.2f, 0.0f, false},
    {TileType::Lava,    Passability::Lava,     "熔岩",  {0.8f, 0.2f, 0.0f},   2.0f, 10.0f, true},
    {TileType::DeepWater, Passability::Water,  "深水",  {0.1f, 0.3f, 0.6f},   2.5f, 0.0f, true},
    {TileType::Door,    Passability::Door,     "门",    {0.5f, 0.35f, 0.2f},  0.0f, 0.0f, false},
    {TileType::Portal,  Passability::Portal,   "传送门",{0.6f, 0.2f, 0.9f},   0.0f, 0.0f, true},
    // ... 更多瓦片
};

// 快速查找函数
inline const TileDef& getTileDef(TileType type) {
    return TILE_DEFS[static_cast<size_t>(type)];
}

inline bool isWalkable(TileType type) {
    return getTileDef(type).passability != Passability::Blocked;
}
```

**通行性设计原则：**
- `Blocked`：Box2D 创建静态刚体，完全阻挡移动和视线
- `Water`：不创建静态刚体（可游泳通过），但施加减速效果和/或持续伤害
- `Lava`：不阻挡移动，但造成高额持续伤害
- `Door`：初始为 Blocked，触发开关后变为 Walkable
- `Portal`：不阻挡移动，进入时触发区域切换

**与物理系统的对应：**
- `Blocked` 瓦片 → 创建 Box2D 静态刚体
- `Water/Lava` 瓦片 → 创建 Box2D 传感器（sensor），通过接触回调施加效果
- `Walkable/Door/Portal` 瓦片 → 不创建物理刚体

// 装饰物数据
struct Decoration {
    DecorType type;
    uint8_t variant;      // 变种（同类型不同外观）
    uint8_t rotation;     // 旋转角度（0-3）
    uint8_t scale;        // 缩放（0=小, 1=中, 2=大）
    uint8_t seasonMask;   // 季节可见性位掩码
    int16_t tileX;        // 瓦片坐标 X
    int16_t tileY;        // 瓦片坐标 Y
};

// 兴趣点（POI）
struct PointOfInterest {
    enum class Type {
        NPC_Spawn,    // NPC生成点
        Treasure,     // 宝藏
        Quest,        // 任务点
        Teleport,     // 传送点
        Home,         // 玩家之家
        Shop,         // 商店
        Dungeon,      // 地下城入口
    };
    
    Type type;
    glm::ivec2 tilePos;
    std::string id;           // 唯一标识
    std::string displayName;  // 显示名称
    int metadata;             // 附加数据（如NPC ID、任务ID）
};

// 区域定义
struct MapRegion {
    std::string name;
    std::string description;
    glm::ivec2 regionSize;    // 区域尺寸（瓦片数，如 40×30）
    float tileSize = 1.0f;    // 每个瓦片的世界单位大小
    int regionId;
    
    // 多图层
    std::vector<TileType> groundLayer;     // 地面层
    std::vector<Decoration> decorLayer;    // 装饰层
    std::vector<PointOfInterest> pois;     // 兴趣点
    
    // 物理缓存
    std::vector<b2BodyId> physicsBodies;
    
    // 边界连接（用于区域切换）
    std::vector<MapConnection> connections;
};

// 区域连接
struct MapConnection {
    enum class Direction { North, South, East, West };
    
    Direction direction;
    std::string targetRegionId;
    glm::ivec2 sourceTile;      // 本区域出口位置
    glm::ivec2 targetTile;      // 目标区域入口位置
};
```

## 3. 地形生成算法升级

### 3.1 基于噪声的连贯地形生成

```cpp
class TerrainGenerator {
public:
    // 使用Perlin/Simplex噪声生成连贯地形
    static void generateCoherent(
        TileMap& map,
        int seed,
        float scale = 0.1f,        // 噪声缩放
        float persistence = 0.5f,  // 持久度（细节层次）
        int octaves = 4            // 八度数（叠加层数）
    );
    
    // 生成水域（基于高度图）
    static void generateWaterBodies(TileMap& map, float waterLevel = 0.35f);
    
    // 生成路径网络
    static void generatePaths(TileMap& map, int numPaths = 3);
    
    // 生成建筑群
    static void generateBuildings(TileMap& map, int numBuildings = 2);
    
private:
    // 2D Perlin噪声
    static float perlinNoise(float x, float y, unsigned int seed);
    
    // 分形布朗运动（多八度噪声叠加）
    static float fbm(float x, float y, int octaves, float persistence, unsigned int seed);
    
    // 地形类型映射（高度→瓦片类型）
    static TileType heightToTile(float height, float waterLevel);
};
```

### 3.2 生成流程

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
   - > 0.85: Wall (山峰)
4. 后处理：
   - 平滑边界
   - 确保可通行区域连通
   - 生成路径连接关键区域
   - 放置建筑和兴趣点
```

### 3.3 装饰物生成

```cpp
class DecorationGenerator {
public:
    static void generateDecorations(
        TileMap& map,
        std::vector<Decoration>& decors,
        int seed,
        float density = 0.15f  // 装饰密度
    );
    
private:
    // 根据地面类型选择合适的装饰物
    static std::vector<DecorType> getApplicableDecors(TileType ground);
    
    // 放置规则检查（不与建筑/路径重叠）
    static bool canPlaceDecor(const TileMap& map, int x, int y, DecorType type);
};
```

## 4. 渲染系统升级

### 4.1 分层渲染

```cpp
// 渲染图层枚举（按绘制顺序）
enum class RenderLayer : int {
    Ground = 0,      // 地面瓦片层（不透明）
    Water = 1,       // 水面层（半透明，动态效果）
    DecorLow = 2,    // 低矮装饰（花草，不遮挡角色）
    Objects = 3,     // 可交互对象 + 角色（按 y 坐标排序）
    DecorHigh = 4,   // 高大装饰（树木，可能遮挡角色）
    Effects = 5,     // 粒子特效层
    UI = 6,          // UI层（POI标记、小地图）
    Count
};

class MapRenderer {
public:
    void render(const MapRegion& region, const glm::mat4& viewProj, 
                const Camera2D& camera, int screenWidth, int screenHeight);
    
private:
    // 1. 地面层（不透明）
    void renderGroundLayer(const MapRegion& region, const glm::mat4& viewProj);
    
    // 2. 水面层（半透明，动态效果）
    void renderWaterLayer(const MapRegion& region, const glm::mat4& viewProj);
    
    // 3. 低矮装饰层（花草，不遮挡）
    void renderDecorLowLayer(const MapRegion& region, const glm::mat4& viewProj);
    
    // 4. 对象层（角色、掉落物、可交互对象 — y轴排序）
    void renderObjectLayer(const MapRegion& region, const glm::mat4& viewProj);
    
    // 5. 高大装饰层（树木，可能遮挡角色）
    void renderDecorHighLayer(const MapRegion& region, const glm::mat4& viewProj);
    
    // 6. 特效层（粒子、动画）
    void renderEffectLayer(const MapRegion& region, const glm::mat4& viewProj);
    
    // 7. UI层（POI标记、小地图）
    void renderUILayer(const MapRegion& region, const glm::mat4& viewProj);
};
```

### 4.2 角色渲染与遮挡关系

**问题**：当前角色使用独立 SDF 着色器绘制，不属于 Draw2D 批处理管线。当地图引入装饰物（树木）后，需要处理角色与装饰物的前后遮挡关系。

**解决方案：y 轴深度排序**

```cpp
// 所有需要 y 轴排序的可渲染对象
struct Renderable {
    float y;              // 世界 y 坐标（排序键）
    RenderLayer layer;    // 图层
    int sortKey;          // 同图层内的次级排序
    
    // 排序比较：先按图层，再按 y 坐标
    bool operator<(const Renderable& other) const {
        if (layer != other.layer) return static_cast<int>(layer) < static_cast<int>(other.layer);
        return y < other.y;
    }
};

// 角色（SDF 渲染）
struct CharacterRenderable : Renderable {
    glm::vec2 position;
    int expression;
    float time;
    
    CharacterRenderable(const glm::vec2& pos) 
        : Renderable{pos.y, RenderLayer::Objects, 0}, position(pos) {}
};

// 高大装饰物（SDF 或精灵渲染）
struct DecorRenderable : Renderable {
    glm::vec2 position;
    DecorType type;
    int variant;
    
    DecorRenderable(const glm::vec2& pos, DecorType t, int v)
        : Renderable{pos.y, RenderLayer::DecorHigh, 0}, position(pos), type(t), variant(v) {}
};
```

**渲染流程**：

```cpp
void MapRenderer::render(const MapRegion& region, ...) {
    // 1-3. 地面、水面、低矮装饰（固定顺序，无需排序）
    renderGroundLayer(region, viewProj);
    renderWaterLayer(region, viewProj);
    renderDecorLowLayer(region, viewProj);
    
    // 4. 收集对象层所有可渲染实体
    std::vector<Renderable*> renderQueue;
    
    // 添加角色
    CharacterRenderable charRender(player.position);
    renderQueue.push_back(&charRender);
    
    // 添加掉落物
    for (auto& drop : drops) {
        renderQueue.push_back(&drop.renderable);
    }
    
    // 添加高大装饰物（树木等）
    for (auto& decor : region.decorLayer) {
        if (isTallDecor(decor.type)) {
            renderQueue.push_back(&decor.renderable);
        }
    }
    
    // 按 y 坐标排序
    std::sort(renderQueue.begin(), renderQueue.end(), 
              [](const Renderable* a, const Renderable* b) { return *a < *b; });
    
    // 按排序顺序绘制
    for (auto* r : renderQueue) {
        if (r->layer == RenderLayer::Objects) {
            // 绘制角色/掉落物（SDF 着色器）
            renderCharacter(...);
        } else if (r->layer == RenderLayer::DecorHigh) {
            // 绘制高大装饰物（可能遮挡角色）
            renderTallDecor(...);
        }
    }
    
    // 5-7. 特效、UI（固定顺序）
    renderEffectLayer(region, viewProj);
    renderUILayer(region, viewProj);
}
```

**视觉效果**：
- 角色 y 坐标 < 树木 y 坐标 → 角色先绘制，树木遮挡角色（正确）
- 角色 y 坐标 > 树木 y 坐标 → 树木先绘制，角色遮挡树木（正确）

### 4.3 装饰物渲染架构

**关键架构决策**：装饰物渲染不通过 Draw2D 管线，而是使用独立的 SDF 着色器，复用角色渲染的批处理思路。

**原因**：
- Draw2D 设计为简单颜色填充（顶点颜色插值），不支持复杂形状
- 装饰物需要 per-object uniform（type、variant、rotation、time）
- SDF 可以实现更丰富的视觉效果（树叶摇动、季节变化）

**实现方案：装饰物批处理渲染器**

```cpp
// 装饰物渲染器（类似角色 SDF 渲染，但批量提交）
class DecorRenderer {
public:
    void init();
    void shutdown();
    
    // 开始一帧的装饰物收集
    void beginFrame(const glm::mat4& viewProj);
    
    // 添加一个装饰物到渲染队列（不立即绘制）
    void addDecor(const glm::vec2& pos, DecorType type, int variant, 
                  float rotation = 0.0f, float scale = 1.0f);
    
    // 提交所有装饰物绘制（一次性 glDrawArraysInstanced）
    void endFrame();

private:
    GLuint shader;
    GLuint vao, vbo, instanceVBO;
    
    // 实例数据缓冲区
    struct DecorInstance {
        glm::vec2 position;
        float rotation;
        float scale;
        int type;
        int variant;
        float time;
    };
    std::vector<DecorInstance> instances;
    
    // 装饰物包围盒尺寸表（每种类型的宽×高）
    static constexpr glm::vec2 DECOR_SIZES[] = {
        {},                         // None
        {0.6f, 1.2f},              // Tree
        {0.4f, 0.4f},              // Bush
        {0.15f, 0.2f},             // Flower
        {0.3f, 0.5f},              // TallGrass
        // ...
    };
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
    // 应用缩放
    vec2 pos = aPos * iScale;
    
    // 应用旋转
    float c = cos(iRotation);
    float s = sin(iRotation);
    pos = vec2(c * pos.x - s * pos.y, s * pos.x + c * pos.y);
    
    // 世界坐标
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

// SDF 基础函数
float sdCircle(vec2 p, float r) { return length(p) - r; }
float sdCapsule(vec2 p, float len, float rad) { /* ... */ }

void renderTree(vec2 uv, int variant, float time) {
    // 树干（棕色胶囊）
    float trunk = sdCapsule(uv - vec2(0.0, -0.2), 0.3, 0.08);
    vec3 trunkColor = vec3(0.4, 0.25, 0.1);
    
    // 树冠（三个圆叠加，根据 variant 变化）
    float crown1 = sdCircle(uv - vec2(0.0, 0.3), 0.35);
    float crown2 = sdCircle(uv - vec2(-0.15, 0.15), 0.25);
    float crown3 = sdCircle(uv - vec2(0.15, 0.15), 0.25);
    float crown = min(crown1, min(crown2, crown3));
    vec3 leafColor = vec3(0.2, 0.6, 0.15);
    
    // 组合
    float tree = min(trunk, crown);
    float alpha = 1.0 - smoothstep(0.0, 0.02, tree);
    
    vec3 color = trunk < crown ? trunkColor : leafColor;
    FragColor = vec4(color, alpha);
}

void renderBush(vec2 uv, int variant) {
    // 灌木（两个圆叠加）
    float b1 = sdCircle(uv - vec2(-0.1, 0.0), 0.25);
    float b2 = sdCircle(uv - vec2(0.1, 0.05), 0.2);
    float bush = min(b1, b2);
    float alpha = 1.0 - smoothstep(0.0, 0.02, bush);
    FragColor = vec4(0.15, 0.5, 0.1, alpha);
}

void main() {
    if (vType == 1) {
        renderTree(vWorldPos, vVariant, vTime);
    } else if (vType == 2) {
        renderBush(vWorldPos, vVariant);
    } else if (vType == 3) {
        // Flower: 简单圆
        float flower = sdCircle(vWorldPos, 0.1);
        float alpha = 1.0 - smoothstep(0.0, 0.02, flower);
        FragColor = vec4(0.9, 0.3, 0.5, alpha);
    } else {
        discard;
    }
}
```

**与 Draw2D 的对比**：

| 特性 | Draw2D | DecorRenderer |
|------|--------|---------------|
| 着色器 | 简单颜色插值 | SDF 复杂形状 |
| 实例 uniform | 不支持（全局 uniform） | 支持（实例化属性） |
| 批处理方式 | 顶点缓存 → 一次性提交 | 实例数据 → `glDrawArraysInstanced` |
| 适用场景 | 地面瓦片、UI 矩形 | 装饰物、角色、投射物 |

**使用示例**：

```cpp
// 在 MapRenderer::renderDecorLayer 中
void MapRenderer::renderDecorLayer(const MapRegion& region, const glm::mat4& viewProj) {
    decorRenderer.beginFrame(viewProj);
    
    for (const auto& decor : region.decorLayer) {
        glm::vec2 worldPos = tileMap.tileToWorld(decor.tileX, decor.tileY);
        decorRenderer.addDecor(worldPos, decor.type, decor.variant, 
                               decor.rotation * (3.14159f / 2.0f), 
                               decor.scale == 0 ? 0.7f : (decor.scale == 1 ? 1.0f : 1.3f));
    }
    
    decorRenderer.endFrame();
}
```

**资源管理**：
- `DecorRenderer` 在 `init()` 中创建 VAO/VBO/着色器
- 在 `shutdown()` 中释放所有 GL 资源
- 每帧 `beginFrame()` 清空实例数据，`endFrame()` 提交绘制
- 与 Draw2D 完全独立，不共享资源

### 4.4 动态效果

**重要说明**：本节中的动态效果（水面波动、装饰物摇动等）均为**纯视觉效果**，不影响 Box2D 物理世界。物理刚体位置保持静态，碰撞检测使用原始未偏移的位置。

```cpp
// 水面波动（顶点着色器偏移）
// 注意：这是纯视觉效果，不影响物理碰撞
// 物理刚体位置保持在 uPosition，不会随顶点偏移
// 偏移幅度 0.02 单位，相对于 1.0 的瓦片尺寸可忽略
#version 330 core
in vec2 aPos;
uniform mat4 uViewProj;
uniform vec2 uPosition;
uniform float uTime;
out vec2 vWorldPos;

void main() {
    vec2 offset = vec2(
        sin(uTime * 2.0 + aPos.x * 3.14) * 0.02,
        cos(uTime * 1.5 + aPos.y * 3.14) * 0.02
    );
    vec2 worldPos = uPosition + aPos + offset;
    gl_Position = uViewProj * vec4(worldPos, 0.0, 1.0);
    vWorldPos = worldPos;
}
```

**物理同步策略**：
- Water/Lava 瓦片的 Box2D 刚体使用**传感器（sensor）**，不阻挡移动
- 碰撞检测通过接触回调实现（减速、伤害），与视觉偏移无关
- 装饰物的风吹动画同理，不影响其碰撞体积（如有）

## 5. 区域与场景切换

### 5.1 区域管理器

```cpp
class RegionManager {
public:
    // 加载区域
    bool loadRegion(const std::string& regionId);
    
    // 卸载当前区域
    void unloadCurrentRegion();
    
    // 切换到相邻区域
    bool transitionTo(const MapConnection& connection);
    
    // 获取当前区域
    const MapRegion* getCurrentRegion() const { return currentRegion; }
    
    // 查找最近的POI
    PointOfInterest* findNearestPOI(const glm::vec2& worldPos, float range);
    
private:
    MapRegion* currentRegion;
    std::unordered_map<std::string, MapRegion> loadedRegions;
    
    // 区域过渡效果
    void playTransitionEffect();
};
```

### 5.2 区域过渡效果

```cpp
// 过渡动画
void RegionManager::playTransitionEffect() {
    // 1. 屏幕淡出
    fadeToBlack(0.5f);
    
    // 2. 卸载旧区域，加载新区域
    unloadCurrentRegion();
    loadRegion(targetRegionId);
    
    // 3. 设置玩家位置
    player.setPosition(targetTileWorldPos);
    camera.setPosition(targetTileWorldPos);
    
    // 4. 屏幕淡入
    fadeFromBlack(0.5f);
}
```

## 6. 路径finding与导航

### 6.1 A* 寻路系统

```cpp
class PathfindingSystem {
public:
    // 寻找路径
    std::vector<glm::ivec2> findPath(
        const TileMap& map,
        const glm::ivec2& start,
        const glm::ivec2& end,
        bool avoidWater = true
    );
    
    // 获取路径上的下一个移动方向
    glm::vec2 getNextDirection(
        const std::vector<glm::ivec2>& path,
        const glm::ivec2& currentPos
    );
    
    // 简化路径（移除冗余点）
    std::vector<glm::ivec2> simplifyPath(const std::vector<glm::ivec2>& path);
    
    // 射线投射（视线检查）
    bool hasLineOfSight(const TileMap& map, 
                       const glm::ivec2& a, const glm::ivec2& b);
    
private:
    // A* 启发式函数（曼哈顿距离）
    float heuristic(glm::ivec2 a, glm::ivec2 b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }
    
    // 获取相邻可通行瓦片
    std::vector<glm::ivec2> getNeighbors(const TileMap& map, const glm::ivec2& pos);
};
```

### 6.2 导航网格（可选优化）

对于大地图，可以使用导航网格替代瓦片寻路：

```cpp
struct NavMesh {
    std::vector<glm::vec2> vertices;      // 导航网格顶点
    std::vector<glm::ivec3> triangles;    // 三角形索引
    
    // 构建导航网格（从瓦片地图）
    void buildFromTileMap(const TileMap& map);
    
    // 在导航网格上寻路
    std::vector<glm::vec2> findPath(const glm::vec2& start, const glm::vec2& end);
};
```

## 7. 小地图系统

### 7.1 小地图渲染

**更新策略（关键性能优化）**：

小地图纹理**不每帧更新**，而是采用以下策略：
- **定期更新**：每 1 秒更新一次（约 60 帧）
- **事件触发更新**：当地图切换、玩家修改瓦片、发现新区域时立即更新
- **增量更新**：只更新玩家周围新探索的区域，已探索区域保持不变

```cpp
class MiniMap {
public:
    void init(int size = 150);  // 小地图尺寸（像素）
    
    // 请求更新（不立即执行，标记为脏）
    void requestUpdate();
    
    // 每帧调用：检查是否需要实际更新
    void update(float deltaTime, const TileMap& map, const glm::vec2& playerPos);
    
    // 渲染小地图（每帧调用，但纹理可能不更新）
    void render();
    
    // 强制立即更新（用于地图切换等关键事件）
    void forceUpdate(const TileMap& map, const glm::vec2& playerPos);
    
private:
    GLuint texture;
    int mapSize;
    std::vector<uint8_t> minimapData;  // RGBA像素数据
    
    // 更新控制
    float updateTimer = 0.0f;
    float updateInterval = 1.0f;       // 1秒更新一次
    bool isDirty = false;              // 是否需要更新
    bool isVisible = false;            // 小地图是否显示（可被UI隐藏）
    
    // 已探索区域记录（用于战争迷雾效果）
    std::vector<bool> exploredTiles;
    
    // 将世界坐标映射到小地图坐标
    glm::ivec2 worldToMinimap(const glm::vec2& worldPos);
    
    // 更新小地图纹理（CPU端填充像素）
    void updateTexture(const TileMap& map, const glm::vec2& playerPos);
    
    // 填充单个瓦片到小地图
    void fillTileOnMinimap(int tx, int ty, const TileMap& map);
};
```

**更新流程**：

```cpp
void MiniMap::update(float deltaTime, const TileMap& map, const glm::vec2& playerPos) {
    if (!isVisible || (!isDirty && updateTimer < updateInterval)) {
        updateTimer += deltaTime;
        return;
    }
    
    updateTimer = 0.0f;
    isDirty = false;
    updateTexture(map, playerPos);
}

void MiniMap::requestUpdate() {
    isDirty = true;
    updateTimer = updateInterval;  // 立即满足更新时间条件
}

void MiniMap::updateTexture(const TileMap& map, const glm::vec2& playerPos) {
    // 1. 清空小地图数据（或只清空未探索区域）
    // 2. 遍历所有瓦片，计算在小地图上的位置
    // 3. 根据瓦片类型填充对应颜色
    // 4. 绘制玩家位置标记
    // 5. 上传到 GPU 纹理
    
    // 性能优化：只更新视锥附近的瓦片
    // 使用 Bresenham 线算法填充从玩家位置到边界的射线
    // 这样未探索区域保持黑色（战争迷雾）
    
    glTextureSubImage2D(texture, 0, 0, 0, mapSize, mapSize, 
                        GL_RGBA, GL_UNSIGNED_BYTE, minimapData.data());
}
```

**性能分析**：
- 200×200 地图 = 40,000 瓦片
- 每帧遍历 40,000 瓦片填充像素 ≈ 0.5ms（可接受但不必要）
- 每秒更新一次 ≈ 0.008ms/帧平均开销（优秀）
- 增量更新（只更新新探索区域）≈ 0.001ms/帧（最佳）

### 7.2 小地图UI

```
┌─────────────────────────────────────┐
│  [小地图]         金币: 1234        │
│  ┌─────────────┐   时间: 14:30     │
│  │  ▓▓░░▓▓    │   区域: 新手村     │
│  │  ░▓▓▓░▓    │                   │
│  │  ▓▓●▓▓░    │   N               │
│  │  ░░▓▓▓▓    │                   │
│  └─────────────┘                   │
└─────────────────────────────────────┘
```

**UI 交互**：
- 点击小地图可展开为大地图（显示更多细节）
- 按 M 键切换小地图显示/隐藏
- 小地图边框颜色表示当前区域危险程度（绿=安全，黄=警告，红=危险）

## 8. 存档与加载

### 8.1 存档策略：种子 + 差异修改

**核心原则**：不存储完整的瓦片数据，只存储生成种子和玩家的修改记录。

**理由**：
- 程序生成的地图可以通过种子完全重建
- 玩家的修改通常是少量的（破坏墙体、放置家具等）
- 存储差异数据比全量数据小 1-2 个数量级
- 避免全量数据与种子重建数据不一致的问题

**例外**：当玩家修改瓦片数量超过阈值（如 > 10%）时，可以考虑切换到全量存储，但这种情况在正常游戏中极少发生。

### 8.2 存档格式

```json
{
  "version": 2,
  "regions": [
    {
      "id": "village_001",
      "name": "新手村",
      "seed": 42,
      "size": [40, 30],
      "tileSize": 1.0,
      
      "modifications": [
        {"x": 5, "y": 3, "oldType": 4, "newType": 0, "timestamp": 1234567890},
        {"x": 10, "y": 10, "oldType": 0, "newType": 20, "timestamp": 1234567891}
      ],
      
      "decorModifications": [
        {"x": 7, "y": 5, "type": 1, "variant": 0, "rotation": 0, "scale": 1}
      ],
      
      "pois": [
        {"type": "home", "x": 5, "y": 3, "id": "player_home"},
        {"type": "shop", "x": 12, "y": 8, "id": "general_store"}
      ]
    }
  ],
  "playerProgress": {
    "discoveredRegions": ["village_001"],
    "collectedItems": [...],
    "completedQuests": [...]
  }
}
```

**字段说明**：
- `seed`：地图生成种子，用于重建原始地形
- `modifications`：瓦片修改记录（`oldType` 用于验证，防止重复应用）
- `decorModifications`：装饰物修改（添加/移除）
- `pois`：玩家放置的兴趣点（如家园标记）

### 8.3 加载流程

```cpp
bool MapSaveSystem::loadRegion(const std::string& savePath, MapRegion& region) {
    // 1. 读取 JSON 存档数据
    SaveData data = readJson(savePath);
    
    // 2. 使用种子重建原始地图
    region.tileMap = TileMap(data.size.x, data.size.y);
    TerrainGenerator::generateCoherent(region.tileMap, data.seed);
    
    // 3. 应用玩家的瓦片修改
    for (const auto& mod : data.modifications) {
        TileType currentType = region.tileMap.getTile(mod.x, mod.y);
        // 验证 oldType 防止重复应用（可选）
        if (currentType == mod.oldType || mod.oldType == 255) {
            region.tileMap.setTile(mod.x, mod.y, mod.newType);
        }
    }
    
    // 4. 应用装饰物修改
    for (const auto& decorMod : data.decorModifications) {
        region.decorLayer.push_back(Decoration{
            decorMod.type, decorMod.variant,
            decorMod.rotation, decorMod.scale,
            decorMod.x, decorMod.y
        });
    }
    
    // 5. 恢复兴趣点
    region.pois = data.pois;
    
    return true;
}
```

### 8.4 保存流程

```cpp
void MapSaveSystem::saveRegion(const MapRegion& region, const std::string& savePath) {
    SaveData data;
    data.seed = region.seed;
    data.size = {region.tileMap.width, region.tileMap.height};
    
    // 收集所有修改（对比原始生成结果）
    // 注意：需要在 MapTileManager 中记录每次修改
    data.modifications = region.tileManager.getModificationsSinceLastSave();
    data.decorModifications = collectDecorModifications(region);
    data.pois = region.pois;
    
    writeJson(savePath, data);
}
```

### 8.5 修改记录管理

```cpp
class MapTileManager {
public:
    // 安全修改瓦片类型（自动同步物理 + 记录修改）
    bool setTile(int x, int y, TileType newType) {
        TileType oldType = tileMap->getTile(x, y);
        if (oldType == newType) return false;
        
        // 记录修改（用于存档）
        if (!isReplayingModifications) {  // 加载时不记录
            modifications.push_back({x, y, oldType, newType, getTime()});
        }
        
        // ... 执行实际的瓦片和物理更新
    }
    
    // 获取自上次保存以来的修改
    std::vector<TileModification> getModificationsSinceLastSave() {
        return std::move(modifications);  // 移动语义，清空已保存的修改
    }
    
    // 加载时临时禁用修改记录
    void setReplayingMode(bool replaying) {
        isReplayingModifications = replaying;
    }

private:
    std::vector<TileModification> modifications;
    bool isReplayingModifications = false;
};
```

### 8.6 存档压缩优化

对于大量修改的情况，可以使用以下优化：

```cpp
// 压缩连续的同类型修改
struct CompressedModification {
    int startX, startY;
    int count;           // 连续修改数量
    TileType newType;
    std::vector<int8_t> offsets;  // 相对偏移（dx, dy 交替存储）
};

// 或者使用游程编码（RLE）压缩修改列表
```

**压缩效果**：
- 正常游戏：每次保存约 10-100 条修改记录，JSON 大小 < 5KB
- 建造模式：可能有数千条修改，使用 RLE 压缩后可减少 80-90%

### 8.7 与旧版本的兼容性

如果存档格式从「全量 groundLayer」升级到「种子 + 差异」，加载逻辑应为：

```cpp
if (data contains "groundLayer") {
    // 旧版本：直接使用全量数据
    region.tileMap.tiles = decodeBase64(data.groundLayer);
} else if (data contains "seed") {
    // 新版本：种子 + 差异重建
    TerrainGenerator::generateCoherent(region.tileMap, data.seed);
    applyModifications(region, data.modifications);
}
```

## 9. 实现计划

### 9.1 阶段5（能力深化）- 地图部分
- [ ] 升级TileMap支持多区域
- [ ] 实现基于噪声的地形生成
- [ ] 添加装饰物系统
- [ ] 实现小地图

### 9.2 阶段6（世界完整化）- 地图部分
- [ ] 实现区域切换系统
- [ ] 添加日夜循环对地图的影响
- [ ] 实现天气系统（雨、雾对地图视觉效果的影响）
- [ ] 添加A*寻路系统
- [ ] 实现室内地图

### 9.3 阶段7（秘密基地）- 地图部分
- [ ] 实现建造模式（网格对齐放置）
- [ ] 家具系统（Box2D静态刚体 + 渲染）
- [ ] 存档/读档系统

## 10. 验收标准

- [ ] 地图尺寸可扩展至 200×200 以上
- [ ] 地形生成具有连贯性（非纯随机）
- [ ] 装饰物密度可调，不与建筑重叠
- [ ] 视锥剔除正确，帧率稳定 60FPS
- [ ] 区域切换流畅，过渡效果自然
- [ ] 小地图正确显示地形和玩家位置
- [ ] 寻路系统能避开障碍物
- [ ] 存档/读档后地图状态正确恢复
- [ ] 无内存泄漏，区域切换时正确释放资源

## 11. 当前实现状态（2024年更新）

### 11.1 已完成 ✅

- **基础瓦片系统**：6种瓦片类型（Grass、Dirt、Stone、Water、Wall、Path），定义在 `src/Game/World/TileMap.h`
- **程序化生成**：`TileMap::generate(seed)` 使用 `std::mt19937` 确定性伪随机
- **物理集成**：`isSolid()` 判断 Wall/Water，`main.cpp` 中 `initTileMap()` 为固体瓦片创建 Box2D 静态刚体
- **视锥剔除**：`renderTileMap()` 使用 `camera.getViewportBounds()` 计算可见瓦片范围
- **坐标转换**：`worldToTile()` / `tileToWorld()` 双向转换已实现
- **瓦片颜色**：`TileColors` 结构体管理各类型颜色

### 11.2 待实现 ❌

按优先级排序：

**P0 - 核心体验（阶段5）**
- [ ] **通行性系统重构**：将 `isSolid()` 升级为基于 `Passability` 枚举的多级通行判定（Walkable/Blocked/Water/Lava/Door/Portal），与 Box2D 物理正确对应
- [ ] **MapTileManager 物理同步**：实现规范第12节的统一瓦片管理接口，解决 `setTile()` 与 Box2D 不同步问题
- [ ] **渲染图层排序**：实现规范第4.1-4.2节的 RenderLayer 分层 + y 轴深度排序，解决角色与装饰物的遮挡关系
- [ ] **噪声地形生成**：当前 `generate()` 每瓦片独立随机，地形呈噪点状。需升级为 Perlin 噪声 + fBm 高度图映射
- [ ] **扩展瓦片类型**：至少添加 Sand、WoodFloor、StoneFloor（规范第2.2节）
- [ ] **装饰物系统基础**：`Decoration` 结构体 + `DecorRenderer` 独立 SDF 渲染器（不通过 Draw2D），实现树木、岩石等基础装饰

**P1 - 世界完整（阶段6）**
- [ ] **多区域系统**：`MapRegion` / `MapConnection` / `RegionManager`
- [ ] **A* 寻路**：`PathfindingSystem` 供敌人AI使用
- [ ] **小地图**：`MiniMap` 系统
- [ ] **存档/加载**：地图状态持久化

**P2 - 体验优化（阶段7）**
- [ ] **动态效果**：水波动画、天气粒子
- [ ] **区域过渡**：淡入淡出效果
- [ ] **室内地图**：独立区域类型

### 11.3 已知技术问题

1. **地图尺寸固定**：当前硬编码 `TileMap(40, 30, 1.0f)`，目标 200×200+
2. **渲染效率**：每帧对可见区域逐瓦片调用 `Draw2D::drawRectFilled()`，大地图需预烘焙静态 VBO 或实例化绘制
3. **物理不同步**：`TileMap::setTile()` 只修改数据，不会同步更新 Box2D 刚体（破坏墙体/填水时需手动管理）
4. **文件加载未使用**：`TileMap::load()` 已实现但 `main.cpp` 中未调用，始终使用程序生成

## 13. 代码清理建议

### 13.1 应删除的测试代码

| 代码 | 位置 | 说明 |
|------|------|------|
| **showTestShapes 覆盖层** | main.cpp:136, 1058-1063 | 渲染红色矩形和蓝色圆，纯测试 Draw2D 功能 |
| **T 键切换** | main.cpp:873-874 | 控制测试覆盖层的开关 |
| **basic.vert/frag** | assets/shaders/ | Stage 1 遗留着色器，已无引用 |

**删除时机**：可在任意时间删除，不影响任何游戏功能。

### 13.2 应重构的代码

| 代码 | 位置 | 问题 | 解决方案 |
|------|------|------|---------|
| **TileMap::generate()** | TileMap.cpp:55-86 | 纯随机噪声，无地形连贯性 | 替换为 Perlin 噪声 + fBm 算法（见第3节） |
| **TileMap::load()** | TileMap.cpp:24-53 | 从未被调用 | 若无需手动画则删除，否则添加使用示例 |
| **E/H 调试键** | main.cpp:909-918 | 发布版本不应包含 | 包裹在 `#ifdef DEBUG` 预处理器中 |

**重构优先级**：TileMap::generate() 为 P0，其余为 P2。

### 13.3 可保留的生产代码

| 代码 | 评估 |
|------|------|
| **renderTileMap()** | ✅ 视锥剔除正确，Draw2D 批处理有效，对 ≤100×100 地图足够 |
| **Draw2D 系统** | ✅ 完整的即时模式 2D API，批处理策略正确 |
| **Camera2D** | ✅ 平滑跟随、缩放、坐标转换功能完善 |
| **角色/投射物/敌人 SDF 着色器** | ✅ 生产级实现，效果良好 |
| **ParticleSystem** | ✅ 粒子发射、更新、渲染完整 |
| **ProjectileManager** | ✅ 投射物管理、碰撞检测完整 |

### 13.4 代码质量评分

| 系统 | 评分 | 说明 |
|------|------|------|
| 物理系统 (Box2D) | 8/10 | 零重力设置正确，阻尼合理，但物理同步需改进 |
| 渲染系统 (SDF) | 9/10 | 角色、投射物、敌人着色器质量优秀 |
| 游戏系统 (战斗) | 7/10 | 功能完整，但调试键需清理 |
| 世界生成 (TileMap) | 3/10 | 纯随机算法，需替换为连贯地形生成 |
| 代码整洁度 | 6/10 | 测试代码与生产代码混合，需分离 |

## 12. 物理同步设计（关键架构决策）

### 12.1 问题描述

当前 `TileMap` 只存储瓦片数据，Box2D 刚体在 `initTileMap()` 中一次性创建。当运行时瓦片类型变化时（如超人投掷砸墙、技能破坏地形、建造模式放置家具），物理世界不会自动更新，导致：
- 视觉上墙体已消失，但玩家仍被空气墙阻挡
- 视觉上已填水，但物理上仍可通行

### 12.2 解决方案：MapTileManager 统一接口

不推荐"逻辑碰撞"方案（用 A* 寻路代替物理），因为 Box2D 同时负责：
- 玩家与墙壁的碰撞响应（滑动、阻挡）
- 投射物反弹
- 敌人 AI 的物理推动
- 掉落物的物理交互

**正确方案**：引入 `MapTileManager` 作为 TileMap 与 Box2D 之间的协调层，所有瓦片修改必须通过它进行。

```cpp
class MapTileManager {
public:
    // 初始化：从 TileMap 创建所有必要的物理刚体
    void init(TileMap& map, b2WorldId world);
    
    // 安全修改瓦片类型（自动同步物理）
    bool setTile(int x, int y, TileType newType);
    
    // 批量修改（事务性，只重建受影响区域的物理）
    bool setTiles(const std::vector<glm::ivec2>& positions, TileType newType);
    
    // 查询：获取某位置的 Box2D 刚体（用于破坏/交互）
    b2BodyId getBodyAt(int x, int y) const;
    
    // 清理：释放所有物理资源
    void shutdown();

private:
    TileMap* tileMap;
    b2WorldId worldId;
    
    // 瓦片坐标 → Box2D 刚体ID 映射
    // 只对 Blocked 类型的瓦片存储刚体
    std::unordered_map<uint64_t, b2BodyId> tileBodies;
    
    // 编码瓦片坐标为单一 uint64 键
    static uint64_t packCoord(int x, int y) {
        return (static_cast<uint64_t>(x) << 32) | static_cast<uint32_t>(y);
    }
    
    // 为单个瓦片创建/移除物理刚体
    void createBodyFor(int x, int y);
    void destroyBodyFor(int x, int y);
    
    // 判断瓦片类型是否需要物理刚体
    static bool needsBody(TileType type) {
        return getTileDef(type).passability == Passability::Blocked;
    }
};
```

### 12.3 setTile 实现流程

```cpp
bool MapTileManager::setTile(int x, int y, TileType newType) {
    if (!tileMap->isInBounds(x, y)) return false;
    
    TileType oldType = tileMap->getTile(x, y);
    if (oldType == newType) return true; // 无变化
    
    bool oldNeedsBody = needsBody(oldType);
    bool newNeedsBody = needsBody(newType);
    
    // 更新瓦片数据
    tileMap->setTile(x, y, newType);
    
    // 物理同步
    if (oldNeedsBody && !newNeedsBody) {
        // 从阻挡变为可通行：移除刚体
        destroyBodyFor(x, y);
    } else if (!oldNeedsBody && newNeedsBody) {
        // 从可通行变为阻挡：创建刚体
        createBodyFor(x, y);
    }
    // 如果都是阻挡或都是可通行，物理状态不变
    
    return true;
}
```

### 12.4 与现有代码的集成

**替换 `main.cpp` 中的直接调用：**

```cpp
// 旧方式（有问题）：
gs.tileMap.setTile(5, 3, TileType::Grass);  // 物理不同步！

// 新方式：
gs.tileManager.setTile(5, 3, TileType::Grass);  // 自动移除墙体刚体
```

**超人投掷砸墙示例：**

```cpp
// 当投掷物击中墙体瓦片时
void onProjectileHitWall(int tileX, int tileY) {
    // 墙体变为碎石（可通行）
    tileManager.setTile(tileX, tileY, TileType::Dirt);
    
    // 产生碎石粒子效果
    particleSystem.emitBurst(tileMap.tileToWorld(tileX, tileY), 8, 
                             glm::vec3(0.6f, 0.6f, 0.55f), 1.0f, 0.1f);
}
```

### 12.5 性能考虑

- **局部更新**：每次 `setTile` 只影响 1 个 Box2D 刚体，不会重建整个物理世界
- **批量操作**：`setTiles()` 可以一次性处理多个瓦片，减少 Box2D 的唤醒次数
- **延迟同步**：大量瓦片变化时，可以标记"脏区域"，在帧末统一更新物理

### 12.6 替代方案对比

| 方案 | 优点 | 缺点 |
|------|------|------|
| **MapTileManager（推荐）** | 物理响应准确，与 Box2D 生态兼容 | 需要管理刚体生命周期 |
| 纯逻辑碰撞 | 无需物理同步 | 失去滑动、反弹等物理效果 |
| 每帧全量重建 | 实现最简单 | 性能极差，不可接受 |

**结论**：采用 `MapTileManager` 方案，在阶段 5 实现。
