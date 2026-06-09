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
    (void)dt;

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

    glm::vec3 skin(0.95f, 0.78f, 0.65f);
    glm::vec3 hair(0.25f, 0.16f, 0.10f);
    glm::vec3 outline(0.08f, 0.07f, 0.06f);
    glm::vec3 trouser(0.22f, 0.34f, 0.58f);
    glm::vec3 shoe(0.18f, 0.13f, 0.09f);
    glm::vec3 eye(0.04f, 0.05f, 0.06f);

    float x = pos.x;
    float y = pos.y;
    float bob = std::sin(x * 1.7f + y * 0.9f) * 0.015f;

    Draw2D::drawCircleFilled(x, y - 0.58f, 0.38f, glm::vec3(0.0f), 0.18f);

    Draw2D::drawRectFilled(x - 0.22f, y - 0.58f + bob, 0.16f, 0.34f, trouser);
    Draw2D::drawRectFilled(x + 0.06f, y - 0.58f - bob, 0.16f, 0.34f, trouser);
    Draw2D::drawRectFilled(x - 0.25f, y - 0.75f + bob, 0.22f, 0.09f, shoe);
    Draw2D::drawRectFilled(x + 0.03f, y - 0.75f - bob, 0.22f, 0.09f, shoe);

    Draw2D::drawRectFilled(x - 0.32f, y - 0.26f, 0.64f, 0.52f, bodyColor);
    Draw2D::drawRect(x - 0.32f, y - 0.26f, 0.64f, 0.52f, outline, 0.025f, 0.7f);

    Draw2D::drawLine(x - 0.34f, y + 0.12f, x - 0.50f, y - 0.28f, skin, 0.08f);
    Draw2D::drawLine(x + 0.34f, y + 0.12f, x + 0.50f, y - 0.28f, skin, 0.08f);

    Draw2D::drawCircleFilled(x, y + 0.48f, 0.34f, skin);
    Draw2D::drawCircle(x, y + 0.48f, 0.34f, outline, 0.025f, 24, 0.7f);
    Draw2D::drawRectFilled(x - 0.30f, y + 0.62f, 0.60f, 0.16f, hair);
    Draw2D::drawCircleFilled(x - 0.18f, y + 0.66f, 0.15f, hair);
    Draw2D::drawCircleFilled(x + 0.12f, y + 0.66f, 0.13f, hair);
    Draw2D::drawCircleFilled(x - 0.11f, y + 0.48f, 0.035f, eye);
    Draw2D::drawCircleFilled(x + 0.11f, y + 0.48f, 0.035f, eye);
    Draw2D::drawRectFilled(x - 0.08f, y + 0.33f, 0.16f, 0.025f, glm::vec3(0.50f, 0.12f, 0.12f));

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
