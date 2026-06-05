#include "PhysicsWorld.h"
#include <algorithm>
#include <cmath>

PhysicsWorld* PhysicsWorld::sInstance = nullptr;

PhysicsWorld::PhysicsWorld() : worldId(b2_nullWorldId) {}

PhysicsWorld::~PhysicsWorld() {
    destroy();
}

bool PhysicsWorld::create() {
    if (b2World_IsValid(worldId)) return false;

    b2WorldDef worldDef = b2DefaultWorldDef();
    b2Vec2 grav = { 0.0f, 0.0f };  // 俯视视角，无重力
    worldDef.gravity = grav;

    worldId = b2CreateWorld(&worldDef);

    sInstance = this;
    return true;
}

void PhysicsWorld::destroy() {
    if (b2World_IsValid(worldId)) {
        b2DestroyWorld(worldId);
        worldId = b2_nullWorldId;
    }
    sInstance = nullptr;
}

void PhysicsWorld::step(float dt, int /*velocityIterations*/, int subStepCount) {
    if (!isValid()) return;
    b2World_Step(worldId, dt, subStepCount);

    // Process contact events (Box2D v3 uses event polling, not callbacks)
    if (collisionCallback) {
        b2ContactEvents events = b2World_GetContactEvents(worldId);
        for (int i = 0; i < events.beginCount; ++i) {
            const b2ContactBeginTouchEvent& evt = events.beginEvents[i];
            CollisionInfo info;
            info.bodyA = b2Shape_GetBody(evt.shapeIdA);
            info.bodyB = b2Shape_GetBody(evt.shapeIdB);
            if (evt.manifold.pointCount > 0) {
                info.contactPoint = glm::vec2(evt.manifold.points[0].point.x, evt.manifold.points[0].point.y);
            }
            info.impulse = 0;
            collisionCallback(info);
        }
        for (int i = 0; i < events.hitCount; ++i) {
            const b2ContactHitEvent& evt = events.hitEvents[i];
            CollisionInfo info;
            info.bodyA = b2Shape_GetBody(evt.shapeIdA);
            info.bodyB = b2Shape_GetBody(evt.shapeIdB);
            info.contactPoint = glm::vec2(evt.point.x, evt.point.y);
            info.impulse = evt.approachSpeed;
            collisionCallback(info);
        }
    }
}

void PhysicsWorld::clampVelocity(b2BodyId bodyId, float maxSpeed) {
    if (!b2Body_IsValid(bodyId)) return;

    b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    if (speed > maxSpeed) {
        float scale = maxSpeed / speed;
        vel.x *= scale;
        vel.y *= scale;
        b2Body_SetLinearVelocity(bodyId, vel);
    }
}

// Free function callbacks for Box2D (C-style function pointers).
// In Box2D v3 the cast callback returns:
//   - 0     : stop the cast (treat as "this is the hit, don't go further")
//   - frac  : clip the cast to this fraction (continue but only up to here)
//   - 1     : ignore this shape, continue the cast
// Returning `fraction` was equivalent to "cast to end of ray", which made the
// "closest hit" code below non-deterministic. Returning 0 stops immediately
// after recording the hit, which is what the caller (rayCast) actually wants.
static float rayCastCallbackFn(b2ShapeId shapeId, b2Vec2, b2Vec2, float fraction, void* userdata) {
    PhysicsWorld::RayCastResult* result = static_cast<PhysicsWorld::RayCastResult*>(userdata);
    b2BodyId bodyId = b2Shape_GetBody(shapeId);

    result->bodyId = bodyId;
    result->fraction = fraction;
    result->hit = true;

    return 0.0f;  // stop the cast — we got our first/closest hit
}

PhysicsWorld::RayCastResult PhysicsWorld::rayCast(const glm::vec2& start, const glm::vec2& end) {
    RayCastResult result;
    if (!isValid()) return result;

    b2Vec2 b2start = { start.x, start.y };
    b2Vec2 translation = { end.x - start.x, end.y - start.y };

    b2QueryFilter filter;
    filter.categoryBits = B2_DEFAULT_CATEGORY_BITS;
    filter.maskBits = B2_DEFAULT_MASK_BITS;

    b2World_CastRay(worldId, b2start, translation, filter, rayCastCallbackFn, &result);

    return result;
}

static bool queryAABBCallbackFn(b2ShapeId shapeId, void* userdata) {
    std::vector<b2BodyId>* bodies = static_cast<std::vector<b2BodyId>*>(userdata);
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    if (b2Body_IsValid(bodyId)) {
        // Manual dedup check since b2BodyId has no operator==
        bool found = false;
        for (b2BodyId existing : *bodies) {
            if (existing.index1 == bodyId.index1 &&
                existing.world0 == bodyId.world0 &&
                existing.generation == bodyId.generation) {
                found = true;
                break;
            }
        }
        if (!found) {
            bodies->push_back(bodyId);
        }
    }
    return true;
}

std::vector<b2BodyId> PhysicsWorld::queryAABB(const glm::vec2& center, float radius) {
    std::vector<b2BodyId> bodies;
    if (!isValid()) return bodies;

    b2AABB aabb;
    aabb.lowerBound = { center.x - radius, center.y - radius };
    aabb.upperBound = { center.x + radius, center.y + radius };

    b2QueryFilter filter;
    filter.categoryBits = B2_DEFAULT_CATEGORY_BITS;
    filter.maskBits = B2_DEFAULT_MASK_BITS;

    b2World_OverlapAABB(worldId, aabb, filter, queryAABBCallbackFn, &bodies);

    return bodies;
}
