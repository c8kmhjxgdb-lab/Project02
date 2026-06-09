#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <box2d/box2d.h>
#include <functional>
#include <cstdint>

/**
 * 伤害类型枚举
 */
enum class DamageType : uint8_t {
    Normal,     // 普通伤害
    Fire,       // 火焰伤害（持续燃烧）
    Ice,        // 冰冻伤害（减速）
    Thunder,    // 雷电伤害（连锁）
    Explosion   // 爆炸伤害（范围+击退）
};

/**
 * 伤害信息结构
 */
struct DamageInfo {
    float amount;
    glm::vec2 sourcePosition;   // 伤害来源位置（用于击退）
    b2BodyId sourceBody;        // 来源刚体（可为null）
    b2BodyId victimBody;        // 受伤者刚体（用于击退方向）
    DamageType type;

    DamageInfo() : amount(0), sourcePosition(0, 0),
                   sourceBody(b2_nullBodyId), victimBody(b2_nullBodyId),
                   type(DamageType::Normal) {}
};

/**
 * HealthComponent - 生命值组件
 *
 * 管理实体的生命值、无敌时间、伤害和治疗的回调。
 */
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
    void setDeathCallback(DeathCallback cb) { onDeath = std::move(cb); }

    // 受伤回调
    using HurtCallback = std::function<void(const DamageInfo&)>;
    void setHurtCallback(HurtCallback cb) { onHurt = std::move(cb); }

    float getCurrentHealth() const { return currentHealth; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const {
        return maxHealth > 0 ? currentHealth / maxHealth : 0.0f;
    }

    // Restore persisted values without triggering damage/heal side effects.
    void restore(float health, float newMaxHealth);

    // 无敌时间管理
    void setInvincible(float duration);
    void updateInvincible(float dt);
    bool isInvincible() const { return invincibleTimer > 0.0f; }

    // 持续效果更新（燃烧、减速等）
    void update(float dt);

    // 获取当前移动速度修正（减速效果）
    float getSpeedMultiplier() const { return speedMultiplier; }

    // 设置死亡状态（外部触发）
    void kill();

    // 复活
    void respawn(float spawnHealthPercent = 0.5f);

private:
    float currentHealth;
    float maxHealth;
    float invincibleTimer;

    // 持续效果
    float burnTimer;        // 燃烧剩余时间
    float burnDamageTimer;  // 燃烧伤害计时器
    float slowTimer;        // 减速剩余时间
    float speedMultiplier;  // 当前速度倍率

    DeathCallback onDeath;
    HurtCallback onHurt;
};
