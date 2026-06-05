#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <chrono>

/**
 * Shield - 护盾技能系统
 *
 * 在玩家周围生成一个旋转的虚线光环，持续时间内：
 * - 阻挡敌人近战攻击（对接触的敌人施加击退力）
 * - 消耗魔力值维持
 * - 受到攻击时产生粒子效果
 */
class Shield {
public:
    Shield();

    // 激活护盾
    // 返回是否成功激活
    bool activate(float manaCost = 15.0f);

    // 更新护盾状态
    void update(float dt);

    // 是否正在激活
    bool isActive() const { return active; }

    // 获取护盾剩余时间
    float getRemainingTime() const { return remainingTime; }

    // 获取护盾旋转角度（用于渲染）
    float getRotationAngle() const { return rotationAngle; }

    // 护盾半径
    float getRadius() const { return shieldRadius; }

    // 设置护盾持续时间
    void setDuration(float dur) { maxDuration = dur; }
    float getDuration() const { return maxDuration; }

    // 设置护盾半径
    void setRadius(float r) { shieldRadius = r; }

    // 设置旋转速度
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    float getRotationSpeed() const { return rotationSpeed; }

    // 获取护盾强度（0-1，用于视觉效果）
    float getIntensity() const;

    // 护盾被击中时的回调
    using HitCallback = std::function<void(const glm::vec2& hitPos)>;
    void setOnHit(HitCallback cb) { onHit = std::move(cb); }

    // 检查敌人是否撞到护盾，如果是则击退
    // 返回是否发生碰撞
    bool checkAndRepelEnemy(b2WorldId world, b2BodyId enemyBody,
                           const glm::vec2& playerPos, float knockbackForce = 15.0f);

private:
    bool active = false;
    float remainingTime = 0.0f;
    float maxDuration = 3.0f;       // 护盾持续3秒
    float rotationAngle = 0.0f;
    float rotationSpeed = 3.0f;     // 弧度/秒
    float shieldRadius = 1.2f;      // 护盾半径

    float hitCooldown = 0.0f;       // 击中冷却，避免每帧触发
    const float HIT_COOLDOWN_MIN = 0.1f;

    HitCallback onHit;

    // 击退敌人
    void repelEnemy(b2WorldId world, b2BodyId enemyBody,
                   const glm::vec2& playerPos, const glm::vec2& enemyPos,
                   float knockbackForce);
};
