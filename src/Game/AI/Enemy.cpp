#include "Enemy.h"
#include "Engine/Physics/PhysicsWorld.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

EnemyManager::EnemyManager() : nextId(0) {
    enemies.reserve(64);
}

EnemyManager::~EnemyManager() {
    clear();
}

void EnemyManager::init() {
    enemies.reserve(64);
}

b2BodyId EnemyManager::createBody(b2WorldId world, const glm::vec2& pos, EnemyType type) {
    b2BodyDef bd = b2DefaultBodyDef();
    b2Vec2 b2pos = { pos.x, pos.y };
    bd.position = b2pos;
    bd.type = b2_dynamicBody;

    b2BodyId bodyId = b2CreateBody(world, &bd);

    b2Circle circle;
    circle.center = { 0, 0 };
    circle.radius = 0.3f;

    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density = 1.0f;
    // friction is 0.3f by default via b2DefaultShapeDef material, or set via b2Shape_SetFriction after creation

    // 敌人碰撞类别
    sd.filter.categoryBits = 0x0004;  // 敌人类别
    sd.filter.maskBits = 0x0001 | 0x0002;  // 与玩家(0x0001)和投射物(0x0002)碰撞

    b2CreateCircleShape(bodyId, &sd, &circle);

    return bodyId;
}

EnemyId EnemyManager::spawn(b2WorldId world, const glm::vec2& pos, EnemyType type) {
    b2BodyId bodyId = createBody(world, pos, type);

    Enemy enemy;
    enemy.bodyId = bodyId;
    enemy.id = makeEnemyId(nextId++);
    enemy.type = type;

    // 根据类型设置参数
    switch (type) {
    case EnemyType::Chaser:
        enemy.health = 30.0f;
        enemy.maxHealth = 30.0f;
        enemy.damage = 10.0f;
        enemy.speed = 3.0f;
        enemy.detectionRange = 10.0f;
        enemy.attackRange = 0.8f;
        enemy.attackCooldown = 1.0f;
        enemy.color = glm::vec3(1.0f, 0.2f, 0.2f);  // 红色
        enemy.radius = 0.3f;
        enemy.coinDropMin = 1;
        enemy.coinDropMax = 3;
        break;

    case EnemyType::Shooter:
        enemy.health = 20.0f;
        enemy.maxHealth = 20.0f;
        enemy.damage = 8.0f;
        enemy.speed = 2.0f;
        enemy.detectionRange = 12.0f;
        enemy.attackRange = 5.0f;
        enemy.attackCooldown = 1.5f;
        enemy.color = glm::vec3(0.6f, 0.2f, 0.8f);  // 紫色
        enemy.radius = 0.25f;
        enemy.coinDropMin = 2;
        enemy.coinDropMax = 5;
        break;

    case EnemyType::Exploder:
        enemy.health = 15.0f;
        enemy.maxHealth = 15.0f;
        enemy.damage = 25.0f;
        enemy.speed = 4.0f;
        enemy.detectionRange = 8.0f;
        enemy.attackRange = 0.8f;
        enemy.attackCooldown = 0.5f;
        enemy.color = glm::vec3(1.0f, 0.5f, 0.1f);  // 橙色
        enemy.radius = 0.35f;
        enemy.coinDropMin = 3;
        enemy.coinDropMax = 6;
        enemy.explosionRange = 1.5f;
        enemy.explosionDamage = 30.0f;
        break;
    }

    enemy.state = Enemy::State::Idle;
    enemy.stateTimer = 0;
    enemy.attackTimer = 0;
    enemy.deathTimer = 0;

    enemies.push_back(enemy);
    return enemy.id;
}

Enemy* EnemyManager::find(EnemyId id) {
    for (auto& e : enemies) {
        if (e.active && e.id == id) return &e;
    }
    return nullptr;
}

const Enemy* EnemyManager::find(EnemyId id) const {
    for (const auto& e : enemies) {
        if (e.active && e.id == id) return &e;
    }
    return nullptr;
}

void EnemyManager::update(float dt, b2WorldId world, const glm::vec2& playerPos) {
    for (auto& enemy : enemies) {
        if (!enemy.active) continue;

        // Update slow effect
        enemy.updateSlow(dt);

        // 计算与玩家的距离
        b2Vec2 epos = b2Body_GetPosition(enemy.bodyId);
        glm::vec2 enemyPos(epos.x, epos.y);
        float dx = enemyPos.x - playerPos.x;
        float dy = enemyPos.y - playerPos.y;
        float distToPlayer = sqrtf(dx * dx + dy * dy);

        // 状态机更新
        switch (enemy.state) {
        case Enemy::State::Idle:
            // 检测玩家
            if (distToPlayer < enemy.detectionRange) {
                enemy.state = Enemy::State::Chase;
            } else {
                //  idle状态下轻微随机移动
                enemy.stateTimer += dt;
                if (enemy.stateTimer > 2.0f) {
                    // 随机方向移动一小段
                    float angle = std::atan2(
                        std::sin(enemy.stateTimer * 1.3f),
                        std::cos(enemy.stateTimer * 0.7f)
                    );
                    float spd = enemy.speed * enemy.getSpeedMultiplier() * 0.3f;
                    b2Vec2 force = { std::cos(angle) * spd,
                                     std::sin(angle) * spd };
                    b2Body_ApplyForceToCenter(enemy.bodyId, force, true);
                }
            }
            break;

        case Enemy::State::Chase:
            if (distToPlayer > enemy.detectionRange * 1.5f) {
                // 玩家跑太远，返回idle
                enemy.state = Enemy::State::Idle;
                enemy.stateTimer = 0;
            } else if (distToPlayer <= enemy.attackRange) {
                // 进入攻击范围
                enemy.state = Enemy::State::Attack;
                enemy.attackTimer = 0;
            } else {
                // 追击玩家
                float spd = enemy.speed * enemy.getSpeedMultiplier();
                if (enemy.type == EnemyType::Shooter && distToPlayer < enemy.attackRange * 0.5f) {
                    // 射击型：保持距离，太近时后退
                    glm::vec2 dir = enemyPos - playerPos;
                    float dlen = sqrtf(dir.x * dir.x + dir.y * dir.y);
                    if (dlen > 0.001f) { dir.x /= dlen; dir.y /= dlen; }
                    b2Vec2 force = { dir.x * spd * 2.0f, dir.y * spd * 2.0f };
                    b2Body_ApplyForceToCenter(enemy.bodyId, force, true);
                } else {
                    // 其他类型：直接追击
                    glm::vec2 dir = playerPos - enemyPos;
                    float dlen = sqrtf(dir.x * dir.x + dir.y * dir.y);
                    if (dlen > 0.001f) { dir.x /= dlen; dir.y /= dlen; }
                    b2Vec2 force = { dir.x * spd * 3.0f, dir.y * spd * 3.0f };
                    b2Body_ApplyForceToCenter(enemy.bodyId, force, true);
                }
            }
            break;

        case Enemy::State::Attack:
            enemy.attackTimer += dt;

            if (distToPlayer > enemy.attackRange * 1.5f) {
                // 玩家逃出攻击范围，回到追击
                enemy.state = Enemy::State::Chase;
            } else if (enemy.attackTimer >= enemy.attackCooldown) {
                enemy.attackTimer = 0;

                // 执行攻击
                switch (enemy.type) {
                case EnemyType::Chaser:
                    // 追击型：接触伤害，无需额外处理（碰撞回调处理）
                    break;

                case EnemyType::Shooter:
                    // Shooter fires a projectile toward the player's current position.
                    // Implemented via the onShoot callback so EnemyManager stays
                    // decoupled from ProjectileManager.
                    if (onShoot) {
                        glm::vec2 shootDir = playerPos - enemyPos;
                        float dlen = sqrtf(shootDir.x * shootDir.x + shootDir.y * shootDir.y);
                        if (dlen > 0.001f) {
                            shootDir.x /= dlen;
                            shootDir.y /= dlen;
                        } else {
                            shootDir = glm::vec2(1, 0);
                        }
                        onShoot(enemyPos, shootDir, enemy.bodyId);
                    }
                    break;

                case EnemyType::Exploder:
                    // 爆炸型：自爆
                    // 触发爆炸效果（在main中处理）
                    if (onEnemyDeath) {
                        onEnemyDeath(enemy.id, enemyPos, enemy.coinDropMin, enemy.coinDropMax);
                    }
                    enemy.state = Enemy::State::Dead;
                    enemy.deathTimer = 0.5f;  // 爆炸动画时间
                    break;

                default:
                    break;
                }
            }
            break;

        case Enemy::State::Dead:
            enemy.deathTimer -= dt;
            if (enemy.deathTimer <= 0.0f) {
                // 完全死亡，移除
                if (b2Body_IsValid(enemy.bodyId)) {
                    b2DestroyBody(enemy.bodyId);
                    enemy.bodyId = b2_nullBodyId;
                }
                enemy.active = false;
            }
            break;
        }

        // 限制速度
        PhysicsWorld::clampVelocity(enemy.bodyId, enemy.speed * enemy.getSpeedMultiplier() * 2.0f);
    }
}

void EnemyManager::cleanup() {
    // 清理死亡敌人（单独调用，避免update中erase导致指针失效）
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e) { return !e.active; }),
        enemies.end()
    );
}

std::vector<const Enemy*> EnemyManager::getAlive() const {
    std::vector<const Enemy*> alive;
    for (const auto& e : enemies) {
        if (e.active && e.state != Enemy::State::Dead) {
            alive.push_back(&e);
        }
    }
    return alive;
}

void EnemyManager::damage(EnemyId id, float amount) {
    Enemy* enemy = find(id);
    if (!enemy || enemy->state == Enemy::State::Dead) return;

    enemy->health -= amount;

    if (enemy->health <= 0.0f) {
        // 死亡
        b2Vec2 pos = b2Body_GetPosition(enemy->bodyId);
        if (onEnemyDeath) {
            onEnemyDeath(id, glm::vec2(pos.x, pos.y), enemy->coinDropMin, enemy->coinDropMax);
        }
        enemy->state = Enemy::State::Dead;
        enemy->deathTimer = 0.5f;
    }
}

void EnemyManager::destroy(EnemyId id) {
    Enemy* enemy = find(id);
    if (!enemy) return;

    enemy->active = false;
    if (b2Body_IsValid(enemy->bodyId)) {
        b2DestroyBody(enemy->bodyId);
        enemy->bodyId = b2_nullBodyId;
    }
}

void EnemyManager::clear() {
    for (auto& e : enemies) {
        if (b2Body_IsValid(e.bodyId)) {
            b2DestroyBody(e.bodyId);
        }
    }
    enemies.clear();
}
