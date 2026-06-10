#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>
#include <functional>
#include <string>

/**
 * 敌人类型枚举
 */
enum class EnemyType : uint8_t {
    Chaser,    // 追击型：直线追踪玩家
    Shooter,   // 射击型：保持距离，发射弹幕
    Exploder   // 爆炸型：接近后自爆
};

/**
 * 敌人唯一ID
 */
struct EnemyId {
    int id;
    bool operator==(const EnemyId& other) const { return id == other.id; }
    bool operator!=(const EnemyId& other) const { return id != other.id; }
};

inline EnemyId makeEnemyId(int i) { return EnemyId{ i }; }
inline const EnemyId ENEMY_NULL = EnemyId{ -1 };

/**
 * Enemy - 敌人数据结构
 */
struct Enemy {
    b2BodyId bodyId;
    EnemyId id;
    EnemyType type;
    std::string definitionId;
    uint8_t specialEffect;

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
    enum class State : uint8_t {
        Idle,      // 待机：随机移动
        Chase,     // 追击：追踪玩家
        Attack,    // 攻击：在范围内攻击
        Dead       // 死亡：播放死亡动画
    } state;

    float stateTimer;

    // 掉落
    int coinDropMin;
    int coinDropMax;

    // 视觉参数
    glm::vec3 color;
    float radius;
    float deathTimer;  // 死亡动画计时

    // 爆炸型专用
    float explosionRange;
    float explosionDamage;
    bool explosionTriggered;

    // 减速效果（冰锥）
    float slowTimer;
    float slowMultiplier;

    // 活跃状态
    bool active;

    Enemy()
        : bodyId(b2_nullBodyId), id(ENEMY_NULL), type(EnemyType::Chaser)
        , definitionId(), specialEffect(0)
        , health(0), maxHealth(0), damage(0), speed(0)
        , detectionRange(8.0f), attackRange(2.0f), attackCooldown(1.0f), attackTimer(0)
        , state(State::Idle), stateTimer(0)
        , coinDropMin(1), coinDropMax(3)
        , color(1, 0, 0), radius(0.3f), deathTimer(0)
        , explosionRange(1.5f), explosionDamage(20.0f), explosionTriggered(false)
        , slowTimer(0), slowMultiplier(1.0f)
        , active(true) {}

    void applySlow(float duration, float multiplier) {
        // Apply the strongest (lowest) multiplier seen this frame. If a new slow
        // arrives and is weaker than the current one, ignore it; if it's stronger,
        // take the new multiplier but keep the longer remaining time so a second
        // hit can extend an existing slow rather than truncating it.
        if (multiplier < slowMultiplier || slowTimer <= 0.0f) {
            slowMultiplier = multiplier;
        }
        if (duration > slowTimer) {
            slowTimer = duration;
        }
    }

    void updateSlow(float dt) {
        if (slowTimer > 0) {
            slowTimer -= dt;
            if (slowTimer <= 0) {
                slowMultiplier = 1.0f;
                slowTimer = 0;
            }
        }
    }

    float getSpeedMultiplier() const {
        return slowMultiplier;
    }
};

/**
 * 死亡回调类型
 */
using EnemyDeathCallback = std::function<void(EnemyId, const glm::vec2& pos, int coinMin, int coinMax)>;

// Callback for Shooter AI to fire a projectile. Enemy provides origin (its own
// position) and direction (toward the player). The owner is set to the enemy's
// body so the projectile doesn't immediately collide with the shooter.
using EnemyShootCallback = std::function<void(const glm::vec2& origin, const glm::vec2& direction, b2BodyId ownerBody)>;

/**
 * EnemyManager - 敌人管理器
 *
 * 管理所有敌人的生成、AI更新和死亡处理。
 */
class EnemyManager {
public:
    EnemyManager();
    ~EnemyManager();

    void init();

    // 生成敌人
    EnemyId spawn(b2WorldId world, const glm::vec2& pos, EnemyType type);
    EnemyId spawnByDefinition(b2WorldId world, const glm::vec2& pos, const std::string& enemyDefId);

    // 更新所有敌人
    void update(float dt, b2WorldId world, const glm::vec2& playerPos);

    // 清理非活跃敌人（应在handleCollisions等使用getAlive的代码之后调用）
    void cleanup();

    // 获取活跃敌人列表
    const std::vector<Enemy>& getActive() const { return enemies; }

    // 获取活跃敌人（仅存活的）
    std::vector<const Enemy*> getAlive() const;

    // 伤害处理
    void damage(EnemyId id, float amount);

    // 设置死亡回调
    void setDeathCallback(EnemyDeathCallback cb) { onEnemyDeath = std::move(cb); }

    // 设置 Shooter 发射回调
    void setShootCallback(EnemyShootCallback cb) { onShoot = std::move(cb); }

    // 强制销毁
    void destroy(EnemyId id);

    // 清空所有敌人
    void clear();

    // 获取下一个可用ID
    int getNextId() const { return nextId; }

    // 查找敌人（public for collision handling）
    Enemy* find(EnemyId id);
    const Enemy* find(EnemyId id) const;

private:
    std::vector<Enemy> enemies;
    int nextId;
    EnemyDeathCallback onEnemyDeath;
    EnemyShootCallback onShoot;

    // 创建Box2D刚体
    b2BodyId createBody(b2WorldId world, const glm::vec2& pos, EnemyType type);
};
