#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <functional>
#include <vector>

/**
 * 碰撞信息
 */
struct CollisionInfo {
    b2BodyId bodyA;
    b2BodyId bodyB;
    glm::vec2 contactPoint;
    float impulse;

    CollisionInfo() : bodyA(b2_nullBodyId), bodyB(b2_nullBodyId),
                      contactPoint(0, 0), impulse(0) {}
};

/**
 * 碰撞回调类型
 */
using CollisionCallback = std::function<void(const CollisionInfo& info)>;

/**
 * PhysicsWorld - 物理世界封装
 *
 * 封装Box2D世界创建和碰撞事件处理。
 */
class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    // 创建世界（零重力，俯视视角）
    bool create();

    // 销毁世界
    void destroy();

    // 物理步进（并处理碰撞事件）
    void step(float dt, int velocityIterations = 8, int positionIterations = 3);

    // 获取世界ID
    b2WorldId getWorldId() const { return worldId; }

    // 是否有效
    bool isValid() const { return b2World_IsValid(worldId); }

    // 设置碰撞回调
    void setCollisionCallback(CollisionCallback cb) { collisionCallback = std::move(cb); }

    // 射线检测辅助函数
    struct RayCastResult {
        b2BodyId bodyId;
        glm::vec2 point;
        glm::vec2 normal;
        float fraction;
        bool hit;

        RayCastResult() : bodyId(b2_nullBodyId), point(0, 0), normal(0, 1),
                          fraction(0), hit(false) {}
    };

    RayCastResult rayCast(const glm::vec2& start, const glm::vec2& end);

    // 查询范围内所有刚体
    std::vector<b2BodyId> queryAABB(const glm::vec2& center, float radius);

    // 限制刚体速度（防止隧道效应）
    static void clampVelocity(b2BodyId bodyId, float maxSpeed);

private:
    b2WorldId worldId;
    CollisionCallback collisionCallback;

    // 用户数据指针（用于回调访问实例）
    static PhysicsWorld* sInstance;
};
