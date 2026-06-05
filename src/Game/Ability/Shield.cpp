#include "Shield.h"
#include "Utils/Math.h"
#include <box2d/box2d.h>
#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>
#include <glm/geometric.hpp>

Shield::Shield() {}

bool Shield::activate(float manaCost) {
    if (active) return false;

    active = true;
    remainingTime = maxDuration;
    rotationAngle = 0.0f;
    hitCooldown = 0.0f;

    // manaCost is handled by caller (GameState checks mana before calling)
    (void)manaCost;
    return true;
}

void Shield::update(float dt) {
    if (!active) return;

    // 更新旋转角度
    rotationAngle += rotationSpeed * dt;
    if (rotationAngle > glm::two_pi<float>()) {
        rotationAngle -= glm::two_pi<float>();
    }

    // 减少持续时间
    remainingTime -= dt;
    if (remainingTime <= 0.0f) {
        remainingTime = 0.0f;
        active = false;
    }

    // 更新击中冷却（clamp to avoid underflow)
    if (hitCooldown > 0.0f) {
        hitCooldown = std::max(0.0f, hitCooldown - dt);
    }
}

float Shield::getIntensity() const {
    if (!active) return 0.0f;
    // 持续时间最后25%时开始闪烁
    float ratio = remainingTime / maxDuration;
    if (ratio < 0.25f) {
        // 快速闪烁
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        return (ms % 200 < 100) ? ratio * 4.0f : 0.0f;
    }
    return 1.0f;
}

bool Shield::checkAndRepelEnemy(b2BodyId enemyBody,
                                const glm::vec2& playerPos, float knockbackForce) {
    if (!active || hitCooldown > 0.0f) return false;
    if (!b2Body_IsValid(enemyBody)) return false;

    b2Vec2 enemyVec = b2Body_GetPosition(enemyBody);
    glm::vec2 enemyPos(enemyVec.x, enemyVec.y);

    float dist = glm::distance(playerPos, enemyPos);

    // 护盾碰撞检测：敌人在护盾半径内
    if (dist < shieldRadius + 0.3f) {  // 0.3f 是敌人近似半径
        repelEnemy(enemyBody, playerPos, enemyPos, knockbackForce);

        hitCooldown = HIT_COOLDOWN_MIN;
        if (onHit) {
            onHit(enemyPos);
        }
        return true;
    }

    return false;
}

void Shield::repelEnemy(b2BodyId enemyBody,
                       const glm::vec2& playerPos, const glm::vec2& enemyPos,
                       float knockbackForce) {
    // 计算击退方向（从玩家指向敌人）
    glm::vec2 repelDir = glm::normalize(enemyPos - playerPos);

    // 施加击退力
    b2Vec2 repelForce = {repelDir.x * knockbackForce, repelDir.y * knockbackForce};
    b2Body_ApplyForceToCenter(enemyBody, repelForce, true);
}
