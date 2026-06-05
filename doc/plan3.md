# 阶段3：物理与基础战斗系统实现计划

## Context

阶段2已完成渲染与绘制系统：Draw2D批处理绘图、Camera2D摄像机跟随、SDF角色渲染、瓦片地图。阶段3目标是建立完整的战斗系统基础，包括投射物、敌人AI、伤害系统和物理交互。

**阶段3具体目标（来自技术栈.md）：**
1. 火球技能：刚体投射物 + 粒子尾迹 + 碰撞伤害
2. 超人抓举：距离关节投掷
3. 简单怨念兽AI（追击、碰撞）
4. 伤害与生命值系统，死亡掉落

## 实现方案

### 1. 投射物系统（火球）

在 `src/Game/Ability/` 下创建 `Projectile.h/.cpp`：

```cpp
// Projectile.h
#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

enum class ProjectileType {
    Fireball,   // 火球：圆形，火焰粒子
    IceSpike,   // 冰锥：三角形，冰冻效果
    Thunder     // 雷电：链式，自动寻敌
};

struct Projectile {
    b2BodyId bodyId;
    ProjectileType type;
    glm::vec2 velocity;
    float damage;
    float lifetime;      // 剩余生存时间
    float maxLifetime;
    b2BodyId ownerBodyId; // 防止击中自己
    bool active;

    // 火球特定参数
    glm::vec3 particleColor;
    float particleEmitRate;
    float particleTimer;
};

// 投射物管理器
class ProjectileManager {
public:
    void init();
    void shutdown();

    // 创建投射物
    ProjectileId fire(b2WorldId world, const glm::vec2& pos, 
                      const glm::vec2& dir, ProjectileType type, 
                      float damage, float speed, b2BodyId owner);

    // 更新所有投射物
    void update(float dt, b2WorldId world);

    // 获取活跃投射物列表（用于渲染）
    const std::vector<Projectile>& getActive() const { return activeProjectiles; }

    // 碰撞回调（由物理系统调用）
    void onCollision(ProjectileId id, b2BodyId otherBody);

private:
    std::vector<Projectile> activeProjectiles;
    // 对象池复用（可选优化）
};
```

**火球物理配置：**
- 形状：圆形，半径 0.15
- 密度：0.1（轻质量，不易被碰撞偏转）
- 重力：0（俯视视角）
- 碰撞过滤：不与玩家碰撞（通过 `ownerBodyId` 过滤）
- 碰撞回调：命中敌人时触发伤害，自身销毁

**渲染方案：**
- 火球：GLSL径向渐变圆，外部叠加噪声火焰
- 尾迹：粒子系统，每帧在火球位置发射小粒子

### 2. 超人抓举系统

在 `src/Game/Ability/` 下创建 `SuperStrength.h/.cpp`：

```cpp
// SuperStrength.h
#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

class SuperStrength {
public:
    // 尝试抓取最近的物体
    bool tryGrab(b2WorldId world, b2BodyId playerBody, 
                 const glm::vec2& grabDirection, float grabRange = 2.0f);

    // 投掷抓取的物体
    void throwObject(const glm::vec2& throwDirection, float throwForce);

    // 释放物体（不投掷）
    void release();

    // 是否正在抓取
    bool isGrabbing() const { return grabbedBodyId.id != b2_nullBodyId; }

    // 获取被抓物体的位置（用于渲染抓取指示器）
    glm::vec2 getGrabbedPosition() const;

private:
    b2WorldId currentWorld = b2_nullWorldId;
    b2BodyId playerBodyId = b2_nullBodyId;
    b2BodyId grabbedBodyId = b2_nullBodyId;
    b2JointId grabJointId = b2_nullJointId;

    // 抓取时的距离关节参数
    float grabDistance = 1.5f;  // 物体保持在玩家前方1.5单位
};
```

**实现要点：**
1. **抓取检测**：从玩家位置沿面向方向发射射线（`b2World_RayCast`），找到第一个动态刚体
2. **距离关节**：使用 `b2DistanceJoint` 将物体吸附在玩家前方固定位置
   - 关节锚点：玩家位置 + 面向方向 × 1.5
   - 物体锚点：物体质心
   - 刚度：高（硬连接感）
3. **投掷**：计算投掷方向的速度增量，施加冲量，然后销毁关节
4. **限制**：只能抓取质量小于某个阈值的物体（防止举起建筑）

**视觉反馈：**
- 抓取时：物体周围显示旋转的虚线光环（GLSL动态虚线圆）
- 投掷时：物体拖尾显示力方向箭头

### 3. 怨念兽AI系统

在 `src/Game/AI/` 下创建 `Enemy.h/.cpp`：

```cpp
// Enemy.h
#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <vector>

enum class EnemyType {
    Chaser,    // 追击型：直线追踪玩家
    Shooter,   // 射击型：保持距离，发射弹幕
    Exploder   // 爆炸型：接近后自爆
};

struct Enemy {
    b2BodyId bodyId;
    EnemyType type;
    
    // 状态
    float health;
    float maxHealth;
    float damage;
    float speed;
    
    // AI参数
    float detectionRange;    // 发现玩家的范围
    float attackRange;       // 攻击范围
    float attackCooldown;    // 攻击间隔
    float attackTimer;
    
    // 状态机
    enum class State {
        Idle,      // 待机：随机移动
        Chase,     // 追击：追踪玩家
        Attack,    // 攻击：在范围内攻击
        Dead       // 死亡：播放死亡动画
    } state;
    
    float stateTimer;
    
    // 掉落
    int coinDrop;      // 掉落星币数量
    bool dropsItem;    // 是否掉落物品
};

class EnemyManager {
public:
    void init();
    void shutdown();
    
    // 生成敌人（在指定位置）
    EnemyId spawn(b2WorldId world, const glm::vec2& pos, EnemyType type);
    
    // 更新所有敌人
    void update(float dt, b2WorldId world, const glm::vec2& playerPos);
    
    // 获取活跃敌人列表
    const std::vector<Enemy>& getActive() const { return activeEnemies; }
    
    // 伤害处理
    void damage(EnemyId id, float amount);
    
    // 死亡回调（触发掉落、任务更新等）
    void onDeath(EnemyId id);
    
private:
    std::vector<Enemy> activeEnemies;
    
    // AI行为函数
    void updateChaser(Enemy& enemy, float dt, const glm::vec2& playerPos);
    void updateShooter(Enemy& enemy, float dt, const glm::vec2& playerPos);
    void updateExploder(Enemy& enemy, float dt, const glm::vec2& playerPos);
};
```

**AI状态机：**

```
Idle --(玩家进入检测范围)--> Chase
Chase --(进入攻击范围)--> Attack
Chase --(玩家离开检测范围)--> Idle
Attack --(攻击冷却完成)--> 回到Chase或Idle
任何状态 --(生命≤0)--> Dead
```

**追击型（Chaser）：**
- 直接朝玩家位置移动
- 速度恒定，无碰撞规避
- 碰撞玩家时造成伤害

**射击型（Shooter）：**
- 保持与玩家的距离（1.5~3.0单位）
- 距离太近时后退，太远时前进
- 攻击时发射小型投射物（类似火球但速度较慢）

**爆炸型（Exploder）：**
- 追击玩家
- 进入爆炸范围（0.8单位）后触发爆炸
- 爆炸对范围内所有目标（包括玩家）造成伤害
- 爆炸后死亡

**渲染方案：**
- 敌人用不同颜色的SDF几何体表示
  - Chaser：红色三角形
  - Shooter：紫色菱形
  - Exploder：橙色圆形，脉冲发光
- 生命条：敌人上方显示小矩形生命条

### 4. 伤害与生命系统

在 `src/Game/` 下创建 `Health.h/.cpp`：

```cpp
// Health.h
#pragma once

#include <glm/vec2.hpp>
#include <functional>

struct DamageInfo {
    float amount;
    glm::vec2 sourcePosition;  // 伤害来源位置（用于击退）
    b2BodyId sourceBody;       // 可选：来源刚体
    enum class Type {
        Normal,    // 普通伤害
        Fire,      // 火焰伤害
        Ice,       // 冰冻伤害
        Thunder,   // 雷电伤害
        Explosion  // 爆炸伤害
    } type;
};

class HealthComponent {
public:
    HealthComponent(float maxHp = 100.0f);
    
    // 受到伤害
    void takeDamage(const DamageInfo& info);
    
    // 治疗
    void heal(float amount);
    
    // 是否存活
    bool isAlive() const { return currentHealth > 0.0f; }
    
    // 死亡回调
    using DeathCallback = std::function<void()>;
    void setDeathCallback(DeathCallback cb) { onDeath = cb; }
    
    // 受伤回调
    using HurtCallback = std::function<void(const DamageInfo&)>;
    void setHurtCallback(HurtCallback cb) { onHurt = cb; }
    
    float getCurrentHealth() const { return currentHealth; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return currentHealth / maxHealth; }
    
    // 无敌时间（受伤后短暂无敌）
    void setInvincible(float duration);
    bool isInvincible() const;
    
private:
    float currentHealth;
    float maxHealth;
    float invincibleTimer;
    DeathCallback onDeath;
    HurtCallback onHurt;
};
```

**玩家生命系统：**
- 最大生命值：100
- 受伤后 0.5秒无敌时间（闪烁）
- 生命值≤0 时触发死亡，3秒后在出生点复活，生命值恢复50%

**敌人生命系统：**
- 不同类型敌人不同生命值
- 死亡时触发掉落和粒子效果

**伤害类型效果：**
- `Normal`：基础伤害
- `Fire`：附加持续燃烧（3秒内每秒10%伤害）
- `Ice`：附加减速（移动速度×0.5，持续2秒）
- `Thunder`：附加连锁（跳跃到附近另一个敌人）
- `Explosion`：范围伤害，附加击退

### 5. 掉落与拾取系统

在 `src/Game/` 下创建 `Drop.h/.cpp`：

```cpp
// Drop.h
#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

enum class DropType {
    Coin,      // 星币
    Health,    // 生命药水
    Mana,      // 魔力药水
    Item       // 随机物品
};

struct Drop {
    b2BodyId bodyId;
    DropType type;
    int value;           // 数量/值
    glm::vec3 color;
    float lifetime;      // 存在时间（自动消失）
    bool collected;
};

class DropManager {
public:
    void init();
    void shutdown();
    
    // 生成掉落物
    void spawn(b2WorldId world, const glm::vec2& pos, 
               DropType type, int value);
    
    // 更新（自动拾取检测、生命周期）
    void update(float dt, const glm::vec2& playerPos);
    
    // 获取活跃掉落物列表（用于渲染）
    const std::vector<Drop>& getActive() const { return activeDrops; }
    
    // 玩家拾取
    void collect(DropId id);
    
private:
    std::vector<Drop> activeDrops;
    
    // 自动拾取范围
    float collectRange = 1.0f;
};
```

**掉落规则：**
- Chaser敌人：1-3星币
- Shooter敌人：2-5星币 + 小概率生命药水
- Exploder敌人：3-6星币 + 小概率魔力药水

**视觉表现：**
- 星币：金色旋转小圆片
- 药水：对应颜色（红/蓝）的瓶子形状
- 拾取时：向上飘出数字（+10）然后淡出

### 6. 文件结构更新

```
src/
├── main.cpp                      # 更新：集成战斗系统
├── Engine/
│   ├── Renderer/
│   │   ├── Draw2D.h/.cpp
│   │   └── ParticleSystem.h/.cpp # 新增：粒子系统
│   ├── Camera/
│   │   └── Camera2D.h/.cpp
│   └── Physics/
│       └── PhysicsWorld.h/.cpp   # 新增：物理世界封装（碰撞回调）
├── Game/
│   ├── Ability/
│   │   ├── Projectile.h/.cpp     # 投射物系统
│   │   └── SuperStrength.h/.cpp  # 超人抓举
│   ├── AI/
│   │   └── Enemy.h/.cpp          # 敌人AI
│   ├── World/
│   │   └── TileMap.h/.cpp
│   ├── Health.h/.cpp             # 生命系统
│   ├── Drop.h/.cpp               # 掉落系统
│   └── Player.h/.cpp             # 新增：玩家组件封装
└── Utils/
    └── Math.h/.cpp               # 新增：数学工具（方向计算等）
```

### 7. 着色器更新

在 `assets/shaders/` 下新增：

```
assets/shaders/
├── character.vert/.frag          # 角色SDF（已有）
├── draw2d.vert/.frag             # 2D绘图（已有）
├── projectile.frag               # 投射物特效
│   └── 火球：径向渐变+噪声火焰
│   └── 冰锥：三角形渐变+冰晶粒子
│   └── 雷电：链式折线+闪光
├── enemy.frag                    # 敌人SDF渲染
│   └── Chaser：红色三角形
│   └── Shooter：紫色菱形
│   └── Exploder：橙色脉冲圆
└── particle.frag                 # 粒子通用着色器
    └── 支持点精灵、软粒子、颜色混合
```

**火球着色器示例：**
```glsl
// projectile.frag - 火球
#version 330 core
in vec2 vWorldPos;
uniform vec2 uPosition;
uniform float uTime;
uniform vec3 uColor;
out vec4 FragColor;

// 简单的伪噪声函数
float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = vWorldPos - uPosition;
    float d = length(uv);
    
    // 基础圆形
    float radius = 0.15;
    float circle = 1.0 - smoothstep(radius * 0.8, radius, d);
    
    // 火焰噪声（随时间变化）
    float flame = noise(uv * 20.0 + uTime * 5.0);
    float fireIntensity = smoothstep(0.3, 0.7, flame) * circle;
    
    // 颜色：核心白色 -> 橙色 -> 外部红色
    vec3 coreColor = vec3(1.0, 1.0, 0.8);
    vec3 midColor = vec3(1.0, 0.5, 0.0);
    vec3 outerColor = vec3(1.0, 0.2, 0.0);
    
    float t = d / radius;
    vec3 color = mix(coreColor, midColor, t);
    color = mix(color, outerColor, t * t);
    color += vec3(fireIntensity * 0.3); // 火焰高光
    
    float alpha = circle * (1.0 - t * 0.5);
    FragColor = vec4(color, alpha);
}
```

### 8. 主循环集成

在 `main.cpp` 中更新游戏循环：

```cpp
// 新增成员
ProjectileManager projectileManager;
EnemyManager enemyManager;
DropManager dropManager;
HealthComponent playerHealth(100.0f);
SuperStrength superStrength;

// 输入处理新增
if (scancode == SDL_SCANCODE_J) {
    // 火球攻击
    glm::vec2 dir = getAimDirection(); // 朝向鼠标或最后移动方向
    projectileManager.fire(gs.worldId, playerPos, dir, 
                          ProjectileType::Fireball, 25.0f, 400.0f, 
                          gs.playerBodyId);
}

if (scancode == SDL_SCANCODE_K) {
    // 超人抓举
    if (!superStrength.isGrabbing()) {
        glm::vec2 dir = getAimDirection();
        superStrength.tryGrab(gs.worldId, gs.playerBodyId, dir);
    } else {
        glm::vec2 dir = getAimDirection();
        superStrength.throwObject(dir, 20.0f);
    }
}

// 更新阶段
projectileManager.update(dt, gs.worldId);
enemyManager.update(dt, gs.worldId, playerPos);
dropManager.update(dt, playerPos);

// 碰撞检测（Box2D回调中触发）
// - 投射物 vs 敌人 → 伤害敌人，销毁投射物
// - 敌人 vs 玩家 → 伤害玩家（若玩家非无敌）
// - 掉落物 vs 玩家 → 自动拾取
```

### 9. 验证步骤

1. **投射物验证**：
   - 按J键发射火球，火球沿正确方向飞行
   - 火球有粒子尾迹
   - 火球命中敌人时敌人扣血，火球消失
   - 火球飞行2秒后自动消失

2. **抓举验证**：
   - 按K键抓取前方物体（如箱子）
   - 被抓物体跟随玩家移动
   - 再次按K键投掷，物体沿方向飞出
   - 投掷力度正确，物体飞行轨迹符合物理

3. **敌人AI验证**：
   - Chaser敌人发现玩家后直线追击
   - Shooter敌人保持距离并射击
   - Exploder敌人接近后爆炸
   - 敌人死亡时播放粒子效果并掉落物品

4. **生命系统验证**：
   - 玩家受伤时闪烁并后退
   - 受伤后0.5秒内不再受伤
   - 生命值为0时死亡，3秒后复活
   - 敌人生命条正确显示

5. **集成验证**：
   - 完整的战斗循环：发现敌人→攻击→躲避→拾取掉落
   - 帧率稳定60FPS（同屏10+敌人，50+投射物）
   - 无内存泄漏，无崩溃

### 10. 验收标准

- [ ] 火球技能可发射，有视觉特效和碰撞伤害
- [ ] 超人抓举可抓取并投掷物体
- [ ] 三种敌人AI行为正确（追击、射击、爆炸）
- [ ] 伤害系统完整（受伤、无敌、死亡、复活）
- [ ] 掉落物自动生成并可拾取
- [ ] 粒子系统支持火球尾迹、爆炸效果
- [ ] 帧率稳定60FPS（1080p窗口，同屏10敌人）
- [ ] 无内存泄漏，RAII管理所有资源
- [ ] 代码编译无警告（`/W4`）

### 11. 扩展预留

本阶段设计预留了以下扩展点：

1. **新投射物类型**：`ProjectileType` 枚举添加 `IceSpike`、`Thunder`，在 `ProjectileManager::fire()` 中根据类型创建不同形状和效果
2. **新敌人类型**：`EnemyType` 枚举添加新类型，在 `EnemyManager` 中添加对应的 `updateXXX()` 函数
3. **伤害类型效果**：`DamageInfo::Type` 已定义火焰、冰冻、雷电，在 `HealthComponent::takeDamage()` 中实现持续效果
4. **技能冷却系统**：在 `main.cpp` 中添加 `skillCooldown` 计时器，防止技能连发

这些扩展将在阶段5（能力深化）中实现。
