#include "SuperStrength.h"
#include <glm/gtc/constants.hpp>
#include <cmath>

SuperStrength::SuperStrength()
    : currentWorld(b2_nullWorldId)
    , playerBodyId(b2_nullBodyId)
    , grabbedBodyId(b2_nullBodyId)
    , grabJointId(b2_nullJointId)
    , grabDistance(1.5f)
    , maxGrabMass(10.0f)
{}

// Helper: do two b2BodyId values refer to the same body?
// Box2D v3 b2BodyId is a POD {index1, world0, generation} with no operator==.
static inline bool bodyIdEquals(b2BodyId a, b2BodyId b) {
    return a.index1 == b.index1
        && a.world0 == b.world0
        && a.generation == b.generation;
}

bool SuperStrength::tryGrab(b2WorldId world, b2BodyId playerBody,
                            const glm::vec2& grabDirection, float grabRange) {
    // If already grabbing, release first so we don't leak the previous joint.
    if (isGrabbing()) {
        release();
    }

    if (!b2Body_IsValid(playerBody) || !b2World_IsValid(world)) {
        return false;
    }

    currentWorld = world;
    playerBodyId = playerBody;

    // Normalize direction (safe against zero vector).
    glm::vec2 dir = grabDirection;
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f) {
        dir = glm::vec2(1, 0);
    } else {
        dir /= len;
    }

    // Cast a ray from the player in the normalized direction, up to grabRange.
    // Previously this discarded grabRange and used grabDistance (1.5f) instead,
    // so the player could never grab anything beyond ~1.5 units away.
    b2Vec2 playerPos = b2Body_GetPosition(playerBody);
    b2Vec2 start = { playerPos.x, playerPos.y };
    b2Vec2 translation = { dir.x * grabRange, dir.y * grabRange };

    b2QueryFilter filter;
    filter.categoryBits = grabCategoryBits;
    filter.maskBits = grabMaskBits;

    b2RayResult result = b2World_CastRayClosest(world, start, translation, filter);
    if (!result.hit || !b2Shape_IsValid(result.shapeId)) {
        return false;
    }

    b2BodyId hitBody = b2Shape_GetBody(result.shapeId);
    if (!b2Body_IsValid(hitBody)) return false;

    // Skip the player itself.
    if (bodyIdEquals(hitBody, playerBody)) return false;

    // Only grab dynamic bodies (static walls shouldn't be liftable).
    if (b2Body_GetType(hitBody) != b2_dynamicBody) return false;

    // Mass check.
    float mass = b2Body_GetMass(hitBody);
    if (mass > maxGrabMass) return false;

    // Build a distance joint pinning the grabbed body in front of the player.
    grabbedBodyId = hitBody;

    b2Vec2 anchorWorld = { playerPos.x + dir.x * grabDistance,
                           playerPos.y + dir.y * grabDistance };
    b2Vec2 grabbedPos = b2Body_GetPosition(hitBody);

    b2DistanceJointDef jd = b2DefaultDistanceJointDef();
    jd.bodyIdA = playerBodyId;
    jd.bodyIdB = grabbedBodyId;
    jd.localAnchorA = b2Body_GetLocalPoint(playerBodyId, anchorWorld);
    jd.localAnchorB = b2Body_GetLocalPoint(grabbedBodyId, grabbedPos);
    jd.maxLength = grabDistance;
    jd.minLength = grabDistance * 0.5f;
    jd.enableSpring = true;
    jd.hertz = 200.0f;
    jd.dampingRatio = 10.0f;
    jd.collideConnected = false;

    grabJointId = b2CreateDistanceJoint(currentWorld, &jd);

    return true;
}

void SuperStrength::throwObject(const glm::vec2& throwDirection, float throwForce) {
    if (!isGrabbing()) return;

    // Normalize direction.
    glm::vec2 dir = throwDirection;
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f) {
        dir = glm::vec2(1, 0);
    } else {
        dir /= len;
    }

    // Apply impulse and tear down the joint.
    b2Vec2 impulse = { dir.x * throwForce, dir.y * throwForce };
    b2Body_ApplyLinearImpulseToCenter(grabbedBodyId, impulse, true);

    release();
}

void SuperStrength::release() {
    if (b2Joint_IsValid(grabJointId)) {
        b2DestroyJoint(grabJointId);
        grabJointId = b2_nullJointId;
    }
    grabbedBodyId = b2_nullBodyId;
    playerBodyId = b2_nullBodyId;
}

glm::vec2 SuperStrength::getGrabbedPosition() const {
    if (!isGrabbing()) return glm::vec2(0);

    b2Vec2 pos = b2Body_GetPosition(grabbedBodyId);
    return glm::vec2(pos.x, pos.y);
}
