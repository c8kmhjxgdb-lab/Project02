#include "Drop.h"
#include <algorithm>
#include <cstdio>

DropManager::DropManager() : nextId(0), collectRange(1.0f) {
    drops.reserve(128);
}

DropManager::~DropManager() {
    clear();
}

void DropManager::init() {
    drops.reserve(128);
}

b2BodyId DropManager::createBody(b2WorldId world, const glm::vec2& pos) {
    b2BodyDef bd = b2DefaultBodyDef();
    b2Vec2 b2pos = { pos.x, pos.y };
    bd.position = b2pos;
    bd.type = b2_dynamicBody;

    b2BodyId bodyId = b2CreateBody(world, &bd);

    b2Circle circle;
    circle.center = { 0, 0 };
    circle.radius = 0.15f;

    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density = 0.5f;
    sd.isSensor = true;  // 传感器，不产生物理碰撞

    b2CreateCircleShape(bodyId, &sd, &circle);

    return bodyId;
}

DropId DropManager::spawn(b2WorldId world, const glm::vec2& pos, DropType type, int value) {
    b2BodyId bodyId = createBody(world, pos);

    Drop drop;
    drop.bodyId = bodyId;
    drop.id = makeDropId(nextId++);
    drop.type = type;
    drop.value = value;
    drop.lifetime = 0;
    drop.maxLifetime = 15.0f;
    drop.collected = false;
    drop.bobTimer = static_cast<float>(nextId) * 0.5f;  // 错开浮动相位
    drop.collecting = false;
    drop.collectTimer = 0;

    // 根据类型设置颜色和值
    switch (type) {
    case DropType::Coin:
        drop.color = glm::vec3(1.0f, 0.85f, 0.2f);  // 金色
        break;
    case DropType::Health:
        drop.color = glm::vec3(1.0f, 0.3f, 0.3f);  // 红色
        break;
    case DropType::Mana:
        drop.color = glm::vec3(0.3f, 0.5f, 1.0f);  // 蓝色
        break;
    case DropType::Item:
        drop.color = glm::vec3(0.5f, 1.0f, 0.3f);  // 绿色
        break;
    }

    drops.push_back(drop);
    return drop.id;
}

Drop* DropManager::find(DropId id) {
    for (auto& d : drops) {
        if (d.active && d.id == id) return &d;
    }
    return nullptr;
}

const Drop* DropManager::find(DropId id) const {
    for (const auto& d : drops) {
        if (d.active && d.id == id) return &d;
    }
    return nullptr;
}

void DropManager::update(float dt, const glm::vec2& playerPos) {
    for (auto& drop : drops) {
        if (!drop.active) continue;

        // 更新生命周期
        drop.lifetime += dt;
        if (drop.lifetime >= drop.maxLifetime) {
            drop.active = false;
            if (b2Body_IsValid(drop.bodyId)) {
                b2DestroyBody(drop.bodyId);
                drop.bodyId = b2_nullBodyId;
            }
            continue;
        }

        // 拾取动画
        if (drop.collecting) {
            drop.collectTimer += dt * 3.0f;
            if (drop.collectTimer >= 1.0f) {
                drop.active = false;
                if (b2Body_IsValid(drop.bodyId)) {
                    b2DestroyBody(drop.bodyId);
                    drop.bodyId = b2_nullBodyId;
                }
            }
            continue;
        }

        // 浮动动画
        drop.bobTimer += dt * 3.0f;

        // 自动拾取检测
        b2Vec2 dpos = b2Body_GetPosition(drop.bodyId);
        glm::vec2 dropPos(dpos.x, dpos.y);
        float dx = dropPos.x - playerPos.x;
        float dy = dropPos.y - playerPos.y;
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist < collectRange) {
            drop.collecting = true;
            drop.collected = true;
            if (onCollect) {
                onCollect(drop);
            }
        }
    }

    // 清理非活跃掉落物
    int inactiveCount = 0;
    for (const auto& d : drops) {
        if (!d.active) inactiveCount++;
    }
    if (inactiveCount > static_cast<int>(drops.size()) / 2) {
        drops.erase(
            std::remove_if(drops.begin(), drops.end(),
                [](const Drop& d) { return !d.active; }),
            drops.end()
        );
    }
}

void DropManager::collect(DropId id) {
    Drop* drop = find(id);
    if (!drop) return;

    if (!drop->collecting) {
        drop->collecting = true;
        drop->collected = true;
        if (onCollect) {
            onCollect(*drop);
        }
    }
}

void DropManager::destroy(DropId id) {
    Drop* drop = find(id);
    if (!drop) return;

    drop->active = false;
    if (b2Body_IsValid(drop->bodyId)) {
        b2DestroyBody(drop->bodyId);
        drop->bodyId = b2_nullBodyId;
    }
}

void DropManager::clear() {
    for (auto& d : drops) {
        if (b2Body_IsValid(d.bodyId)) {
            b2DestroyBody(d.bodyId);
        }
    }
    drops.clear();
}
