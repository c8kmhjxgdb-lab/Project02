#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>

/**
 * 投射物类型枚举
 */
enum class ProjectileType : uint8_t {
    Fireball,   // 火球：圆形，火焰粒子
    IceSpike,   // 冰锥：三角形，冰冻效果（预留）
    Thunder     // 雷电：链式，自动寻敌（预留）
};

/**
 * 投射物唯一ID
 */
struct ProjectileId {
    int id;
    bool operator==(const ProjectileId& other) const { return id == other.id; }
    bool operator!=(const ProjectileId& other) const { return id != other.id; }
};

inline ProjectileId makeProjectileId(int i) { return ProjectileId{ i }; }
inline const ProjectileId PROJECTILE_NULL = ProjectileId{ -1 };

/**
 * Projectile - 投射物数据结构
 */
struct Projectile {
    b2BodyId bodyId;
    ProjectileId id;
    ProjectileType type;
    glm::vec2 previousPosition;
    glm::vec2 velocity;
    float damage;
    float lifetime;       // 剩余生存时间
    float maxLifetime;
    b2BodyId ownerBodyId; // 防止击中自己
    bool active;

    // 视觉效果参数
    glm::vec3 color;
    float radius;

    // 粒子发射
    float particleEmitTimer;
    float particleEmitRate;

    Projectile()
        : bodyId(b2_nullBodyId), id(PROJECTILE_NULL), type(ProjectileType::Fireball)
        , previousPosition(0, 0), velocity(0, 0), damage(0), lifetime(0), maxLifetime(0)
        , ownerBodyId(b2_nullBodyId), active(false)
        , color(1, 0.5, 0), radius(0.15f)
        , particleEmitTimer(0), particleEmitRate(0.02f) {}
};

/**
 * ProjectileManager - 投射物管理器
 *
 * 管理所有活跃投射物的创建、更新和销毁。
 */
class ProjectileManager {
public:
    ProjectileManager();
    ~ProjectileManager();

    // 初始化（预分配内存）
    void init();

    // 创建投射物
    ProjectileId fire(b2WorldId world, const glm::vec2& pos,
                      const glm::vec2& dir, ProjectileType type,
                      float damage, float speed, b2BodyId owner);

    // 更新所有投射物
    void update(float dt, b2WorldId world);

    // Capture positions immediately before stepping physics. Collision code
    // uses previousPosition -> current body position for swept hit tests.
    void capturePreviousPositions();

    // 获取活跃投射物列表（用于渲染）
    const std::vector<Projectile>& getActive() const { return projectiles; }

    // 碰撞回调（命中时调用）
    void onHit(ProjectileId id, b2BodyId otherBody);

    // 强制销毁投射物
    void destroy(ProjectileId id);

    // 销毁所有投射物
    void clear();

    // 获取下一个可用ID
    int getNextId() const { return nextId; }

    // 重置投射物的粒子发射计时器
    void resetParticleTimer(ProjectileId id);

private:
    std::vector<Projectile> projectiles;
    int nextId;

    // 查找投射物
    Projectile* find(ProjectileId id);
    const Projectile* find(ProjectileId id) const;

    // 创建Box2D刚体
    b2BodyId createBody(b2WorldId world, const glm::vec2& pos, ProjectileType type);
};
