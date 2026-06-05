#include "Projectile.h"
#include <algorithm>
#include <cstdio>

ProjectileManager::ProjectileManager() : nextId(0) {
    projectiles.reserve(128);
}

ProjectileManager::~ProjectileManager() {
    clear();
}

void ProjectileManager::init() {
    projectiles.reserve(128);
}

b2BodyId ProjectileManager::createBody(b2WorldId world, const glm::vec2& pos, ProjectileType type) {
    float radius = 0.15f;
    if (type == ProjectileType::IceSpike) radius = 0.12f;
    else if (type == ProjectileType::Thunder) radius = 0.08f;

    b2BodyDef bd = b2DefaultBodyDef();
    b2Vec2 b2pos = { pos.x, pos.y };
    bd.position = b2pos;
    bd.type = b2_dynamicBody;
    bd.isBullet = true;  // 启用连续碰撞检测，防止穿透

    b2BodyId bodyId = b2CreateBody(world, &bd);

    b2Circle circle;
    circle.center = { 0, 0 };
    circle.radius = radius;

    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density = 0.1f;      // 轻质量
    // friction and restitution default to 0 in b2DefaultShapeDef

    // 设置碰撞过滤：不与同类投射物碰撞
    sd.filter.categoryBits = 0x0002;  // 投射物类别
    sd.filter.maskBits = 0x0001 | 0x0004;  // 与玩家(0x0001)和敌人(0x0004)碰撞，不与其他投射物碰撞

    b2CreateCircleShape(bodyId, &sd, &circle);

    return bodyId;
}

ProjectileId ProjectileManager::fire(b2WorldId world, const glm::vec2& pos,
                                     const glm::vec2& dir, ProjectileType type,
                                     float damage, float speed, b2BodyId owner) {
    // 归一化方向
    glm::vec2 normDir = dir;
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f) {
        normDir = glm::vec2(1, 0);  // 默认向右
    } else {
        normDir /= len;
    }

    // 创建物理刚体
    b2BodyId bodyId = createBody(world, pos, type);

    // 设置速度
    b2Vec2 vel = { normDir.x * speed, normDir.y * speed };
    b2Body_SetLinearVelocity(bodyId, vel);

    // 创建投射物数据结构
    Projectile proj;
    proj.bodyId = bodyId;
    proj.id = makeProjectileId(nextId++);
    proj.type = type;
    proj.velocity = normDir * speed;
    proj.damage = damage;
    proj.maxLifetime = 2.0f;
    proj.lifetime = proj.maxLifetime;
    proj.ownerBodyId = owner;
    proj.active = true;

    // 根据类型设置参数
    switch (type) {
    case ProjectileType::Fireball:
        proj.color = glm::vec3(1.0f, 0.5f, 0.0f);
        proj.radius = 0.15f;
        proj.particleEmitRate = 0.02f;
        break;
    case ProjectileType::IceSpike:
        proj.color = glm::vec3(0.5f, 0.8f, 1.0f);
        proj.radius = 0.12f;
        proj.particleEmitRate = 0.05f;
        break;
    case ProjectileType::Thunder:
        proj.color = glm::vec3(0.8f, 0.9f, 1.0f);
        proj.radius = 0.08f;
        proj.particleEmitRate = 0.03f;
        break;
    }

    projectiles.push_back(proj);
    return proj.id;
}

Projectile* ProjectileManager::find(ProjectileId id) {
    for (auto& p : projectiles) {
        if (p.active && p.id == id) return &p;
    }
    return nullptr;
}

const Projectile* ProjectileManager::find(ProjectileId id) const {
    for (const auto& p : projectiles) {
        if (p.active && p.id == id) return &p;
    }
    return nullptr;
}

void ProjectileManager::update(float dt, b2WorldId /*world*/) {
    for (auto& proj : projectiles) {
        if (!proj.active) continue;

        // 更新生命周期
        proj.lifetime -= dt;
        if (proj.lifetime <= 0.0f) {
            proj.active = false;
            if (b2Body_IsValid(proj.bodyId)) {
                b2DestroyBody(proj.bodyId);
                proj.bodyId = b2_nullBodyId;
            }
            continue;
        }

        // 粒子发射计时
        proj.particleEmitTimer += dt;
    }

    // 清理非活跃投射物（每几帧做一次，避免频繁重分配）
    // 当非活跃达到或超过一半时清理（使用 >= 而非 >）
    int inactiveCount = 0;
    for (const auto& p : projectiles) {
        if (!p.active) inactiveCount++;
    }
    if (inactiveCount >= static_cast<int>(projectiles.size() + 1) / 2) {
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                [](const Projectile& p) { return !p.active; }),
            projectiles.end()
        );
    }
}

void ProjectileManager::onHit(ProjectileId id, b2BodyId /*otherBody*/) {
    Projectile* proj = find(id);
    if (!proj) return;

    proj->active = false;
    if (b2Body_IsValid(proj->bodyId)) {
        b2DestroyBody(proj->bodyId);
        proj->bodyId = b2_nullBodyId;
    }
}

void ProjectileManager::destroy(ProjectileId id) {
    Projectile* proj = find(id);
    if (!proj) return;

    proj->active = false;
    if (b2Body_IsValid(proj->bodyId)) {
        b2DestroyBody(proj->bodyId);
        proj->bodyId = b2_nullBodyId;
    }
}

void ProjectileManager::clear() {
    for (auto& proj : projectiles) {
        if (b2Body_IsValid(proj.bodyId)) {
            b2DestroyBody(proj.bodyId);
        }
    }
    projectiles.clear();
}
