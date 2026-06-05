#include "Health.h"
#include <algorithm>

HealthComponent::HealthComponent(float maxHp)
    : currentHealth(maxHp)
    , maxHealth(maxHp)
    , invincibleTimer(0.0f)
    , burnTimer(0.0f)
    , burnDamageTimer(0.0f)
    , slowTimer(0.0f)
    , speedMultiplier(1.0f)
{}

void HealthComponent::takeDamage(const DamageInfo& info) {
    if (!isAlive() || isInvincible()) return;

    currentHealth = std::max(0.0f, currentHealth - info.amount);

    // 根据伤害类型应用持续效果
    switch (info.type) {
    case DamageType::Fire:
        burnTimer = 3.0f;       // 燃烧3秒
        burnDamageTimer = 0.0f;
        break;
    case DamageType::Ice:
        slowTimer = 2.0f;       // 减速2秒
        speedMultiplier = 0.5f;
        break;
    default:
        break;
    }

    // 击退效果（爆炸类型更强）
    if (b2Body_IsValid(info.sourceBody)) {
        b2Vec2 sourcePos = { info.sourcePosition.x, info.sourcePosition.y };
        // 获取受伤物体的位置需要从外部传入，这里通过回调处理
    }

    // 触发受伤回调
    if (onHurt) {
        onHurt(info);
    }

    // 死亡检测
    if (!isAlive()) {
        if (onDeath) {
            onDeath();
        }
    }
}

void HealthComponent::heal(float amount) {
    if (!isAlive()) return;
    currentHealth = std::min(maxHealth, currentHealth + amount);
}

void HealthComponent::setInvincible(float duration) {
    invincibleTimer = duration;
}

void HealthComponent::updateInvincible(float dt) {
    if (invincibleTimer > 0.0f) {
        invincibleTimer = std::max(0.0f, invincibleTimer - dt);
    }
}

void HealthComponent::update(float dt) {
    // 更新无敌时间
    updateInvincible(dt);

    // 更新燃烧效果
    if (burnTimer > 0.0f && isAlive()) {
        burnTimer -= dt;
        burnDamageTimer += dt;

        // 每0.5秒造成一次伤害（总伤害的10%每次）
        if (burnDamageTimer >= 0.5f) {
            burnDamageTimer -= 0.5f;
            float burnDamage = maxHealth * 0.1f;
            currentHealth = std::max(0.0f, currentHealth - burnDamage);

            if (!isAlive() && onDeath) {
                onDeath();
            }
        }
    } else {
        // 已经死亡或燃烧结束，重置燃烧计时器
        burnTimer = 0.0f;
        burnDamageTimer = 0.0f;
    }

    // 更新减速效果
    if (slowTimer > 0.0f) {
        slowTimer -= dt;
        if (slowTimer <= 0.0f) {
            speedMultiplier = 1.0f;
        }
    }
}

void HealthComponent::kill() {
    if (!isAlive()) return;
    currentHealth = 0.0f;
    if (onDeath) {
        onDeath();
    }
}

void HealthComponent::respawn(float spawnHealthPercent) {
    currentHealth = maxHealth * spawnHealthPercent;
    invincibleTimer = 2.0f;  // 复活后2秒无敌
    burnTimer = 0.0f;
    slowTimer = 0.0f;
    speedMultiplier = 1.0f;
}
