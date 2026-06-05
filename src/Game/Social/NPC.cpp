#include "NPC.h"
#include "Engine/Physics/PhysicsWorld.h"
#include "Engine/Renderer/Draw2D.h"
#include <cmath>

NPC::NPC(const std::string& name)
    : name(name)
    , bodyId(b2_nullBodyId)
    , bodyColor(0.7f, 0.7f, 0.7f)
    , currentScheduleIndex(0)
    , state(State::Idle)
    , targetPosition(0.0f, 0.0f)
{}

NPC::~NPC() = default;

void NPC::createBody(b2WorldId world, const glm::vec2& pos) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2Vec2 b2pos;
    b2pos.x = pos.x;
    b2pos.y = pos.y;
    bodyDef.position = b2pos;
    bodyDef.type = b2_dynamicBody;
    bodyId = b2CreateBody(world, &bodyDef);

    b2Polygon shape = b2MakeBox(0.3f, 0.3f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.filter.categoryBits = 0x0004;
    shapeDef.filter.maskBits = 0x0001;
    b2CreatePolygonShape(bodyId, &shapeDef, &shape);

    b2Body_SetLinearDamping(bodyId, 3.0f);
}

void NPC::update(float dt, float gameTime) {
    if (currentSchedule.empty()) return;
    if (!hasBody()) return;

    int targetIndex = findScheduleForTime(gameTime);
    if (targetIndex != currentScheduleIndex) {
        currentScheduleIndex = targetIndex;
        const auto& entry = currentSchedule[currentScheduleIndex];
        targetPosition = entry.position;

        if (entry.action == "idle") {
            state = State::Idle;
        } else if (entry.action == "walk") {
            state = State::Walking;
        }
    }

    if (state == State::Walking) {
        b2Vec2 pos = b2Body_GetPosition(bodyId);
        glm::vec2 currentPos(pos.x, pos.y);
        glm::vec2 dir = targetPosition - currentPos;
        float dist = glm::length(dir);

        if (dist > 0.1f) {
            dir = glm::normalize(dir);
            b2Vec2 force = { dir.x * 2.0f, dir.y * 2.0f };
            b2Body_ApplyForceToCenter(bodyId, force, true);
        } else {
            state = State::Idle;
        }
    }

    // Clamp velocity
    PhysicsWorld::clampVelocity(bodyId, 3.0f);
}

void NPC::render(const glm::mat4& viewProj) {
    if (!hasBody()) return;

    b2Vec2 pos = b2Body_GetPosition(bodyId);

    Draw2D::beginFrame(viewProj);

    float s = 0.3f;
    float sh = 0.6f;
    Draw2D::drawRectFilled(pos.x - s, pos.y - sh, s * 2.0f, sh * 2.0f, bodyColor);
    Draw2D::drawRect(pos.x - s, pos.y - sh, s * 2.0f, sh * 2.0f, bodyColor * 0.6f, 0.02f);

    Draw2D::endFrame();
}

bool NPC::canInteract(const glm::vec2& playerPos, float range) const {
    if (!hasBody()) return false;
    b2Vec2 pos = b2Body_GetPosition(bodyId);
    glm::vec2 npcPos(pos.x, pos.y);
    return glm::distance(playerPos, npcPos) <= range;
}

void NPC::onInteract() {
}

void NPC::setSchedule(const std::vector<ScheduleEntry>& schedule) {
    currentSchedule = schedule;
    currentScheduleIndex = 0;
}

void NPC::setCurrentSchedule(const std::string&) {
}

glm::vec2 NPC::getPosition() const {
    if (!hasBody()) return glm::vec2(0.0f, 0.0f);
    b2Vec2 pos = b2Body_GetPosition(bodyId);
    return glm::vec2(pos.x, pos.y);
}

int NPC::findScheduleForTime(float gameTime) const {
    while (gameTime < 0.0f) gameTime += 24.0f;
    while (gameTime >= 24.0f) gameTime -= 24.0f;

    for (int i = 0; i < static_cast<int>(currentSchedule.size()); ++i) {
        const auto& entry = currentSchedule[i];
        float start = entry.startTime;
        float end = entry.endTime;

        if (start < end) {
            if (gameTime >= start && gameTime < end) return i;
        } else if (start > end) {
            if (gameTime >= start || gameTime < end) return i;
        }
    }
    return 0;
}
