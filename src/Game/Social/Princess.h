#pragma once

#include "NPC.h"
#include "Engine/Physics/PhysicsWorld.h"
#include "Engine/Renderer/Draw2D.h"
#include <glm/vec3.hpp>
#include <glm/ext/scalar_common.hpp>
#include <string>
#include <algorithm>

enum class AffectionLevel : int {
    Stranger,
    Acquaintance,
    Friend,
    CloseFriend,
    Beloved
};

inline const char* affectionLevelName(AffectionLevel level) {
    switch (level) {
        case AffectionLevel::Stranger:     return "Stranger";
        case AffectionLevel::Acquaintance: return "Acquaintance";
        case AffectionLevel::Friend:       return "Friend";
        case AffectionLevel::CloseFriend:  return "CloseFriend";
        case AffectionLevel::Beloved:      return "Beloved";
        default: return "?";
    }
}

class Princess : public NPC {
public:
    Princess(const std::string& name)
        : NPC(name)
        , affection(0.0f)
        , following(false)
        , ultimateCharge(0.0f)
    {
        bodyColor = glm::vec3(0.9f, 0.6f, 0.8f);
    }

    float affection;
    float getAffection() const { return affection; }
    void addAffection(float amount) {
        affection = glm::clamp(affection + amount, 0.0f, 1000.0f);
    }
    AffectionLevel getAffectionLevel() const {
        if (affection < 100.0f) return AffectionLevel::Stranger;
        if (affection < 300.0f) return AffectionLevel::Acquaintance;
        if (affection < 500.0f) return AffectionLevel::Friend;
        if (affection < 700.0f) return AffectionLevel::CloseFriend;
        return AffectionLevel::Beloved;
    }
    const char* getAffectionLevelName() const {
        return affectionLevelName(getAffectionLevel());
    }

    bool following;
    void setFollowing(bool follow) { following = follow; }
    bool isFollowing() const { return following; }

    float getCombatBonus() const {
        return affection / 1000.0f * 0.2f;
    }

    void render(const glm::mat4& viewProj) override {
        if (!hasBody()) return;

        b2Vec2 pos = b2Body_GetPosition(bodyId);

        Draw2D::beginFrame(viewProj);

        glm::vec3 skin(0.96f, 0.80f, 0.68f);
        glm::vec3 hair(0.42f, 0.22f, 0.16f);
        glm::vec3 outline(0.10f, 0.07f, 0.08f);
        glm::vec3 dress = bodyColor;
        glm::vec3 shoe(0.35f, 0.18f, 0.24f);
        glm::vec3 gold(1.0f, 0.82f, 0.24f);
        glm::vec3 eye(0.05f, 0.05f, 0.07f);

        float x = pos.x;
        float y = pos.y;

        Draw2D::drawCircleFilled(x, y - 0.58f, 0.42f, glm::vec3(0.0f), 0.18f);

        Draw2D::drawRectFilled(x - 0.22f, y - 0.74f, 0.18f, 0.10f, shoe);
        Draw2D::drawRectFilled(x + 0.04f, y - 0.74f, 0.18f, 0.10f, shoe);
        Draw2D::drawRectFilled(x - 0.38f, y - 0.36f, 0.76f, 0.68f, dress);
        Draw2D::drawRect(x - 0.38f, y - 0.36f, 0.76f, 0.68f, outline, 0.025f, 0.7f);
        Draw2D::drawRectFilled(x - 0.24f, y + 0.08f, 0.48f, 0.16f, dress * 1.12f);

        Draw2D::drawLine(x - 0.38f, y + 0.08f, x - 0.55f, y - 0.24f, skin, 0.08f);
        Draw2D::drawLine(x + 0.38f, y + 0.08f, x + 0.55f, y - 0.24f, skin, 0.08f);

        Draw2D::drawCircleFilled(x, y + 0.50f, 0.35f, skin);
        Draw2D::drawCircle(x, y + 0.50f, 0.35f, outline, 0.025f, 24, 0.7f);
        Draw2D::drawCircleFilled(x - 0.24f, y + 0.56f, 0.18f, hair);
        Draw2D::drawCircleFilled(x + 0.24f, y + 0.56f, 0.18f, hair);
        Draw2D::drawRectFilled(x - 0.32f, y + 0.64f, 0.64f, 0.13f, hair);
        Draw2D::drawRectFilled(x - 0.20f, y + 0.82f, 0.12f, 0.12f, gold);
        Draw2D::drawRectFilled(x - 0.04f, y + 0.86f, 0.12f, 0.16f, gold);
        Draw2D::drawRectFilled(x + 0.12f, y + 0.82f, 0.12f, 0.12f, gold);
        Draw2D::drawCircleFilled(x - 0.11f, y + 0.50f, 0.035f, eye);
        Draw2D::drawCircleFilled(x + 0.11f, y + 0.50f, 0.035f, eye);
        Draw2D::drawRectFilled(x - 0.08f, y + 0.35f, 0.16f, 0.025f, glm::vec3(0.58f, 0.16f, 0.22f));

        float haloIntensity = affection / 1000.0f * 0.3f;
        if (haloIntensity > 0.01f) {
            float haloR = 0.78f;
            Draw2D::drawCircle(x, y + 0.18f, haloR, bodyColor, 0.03f, 32, 0.25f + haloIntensity);
        }

        if (following) {
            float indicatorY = y + 1.08f;
            Draw2D::drawCircleFilled(x, indicatorY, 0.07f, gold);
            Draw2D::drawCircle(x, indicatorY, 0.12f, gold, 0.02f, 24, 0.7f);
        }

        Draw2D::endFrame();
    }

    float ultimateCharge;
    bool isUltimateReady() const { return ultimateCharge >= 100.0f; }
    void addUltimateCharge(float amount) {
        ultimateCharge = glm::min(100.0f, ultimateCharge + amount);
    }

    void onInteract() override {
        addAffection(2.0f);
    }

    // Override NPC update to also follow the player when `following` is enabled.
    // (An earlier draft of the princess also had updateBehavior(); that whole
    // block was dead code — never called from main — and has been removed.)
    void update(float dt, float gameTime) override {
        // Skip schedule/walk behavior when actively following the player.
        if (!following) {
            NPC::update(dt, gameTime);
            return;
        }

        // Follow mode: chase the player body, no schedule.
        if (!hasBody()) return;
        b2Vec2 npcPos = b2Body_GetPosition(bodyId);
        glm::vec2 currentPos(npcPos.x, npcPos.y);

        // Player position is read from outside — but update() doesn't get it.
        // To keep the API unchanged, store the last known player position via
        // a small helper. Default to current position if never set.
        glm::vec2 target = (lastPlayerPos.x == 0.0f && lastPlayerPos.y == 0.0f)
            ? currentPos : lastPlayerPos;
        glm::vec2 dir = target - currentPos;
        float dist = glm::length(dir);

        if (dist > 1.5f) {
            dir = glm::normalize(dir);
            b2Vec2 force = { dir.x * 3.0f, dir.y * 3.0f };
            b2Body_ApplyForceToCenter(bodyId, force, true);
        } else if (dist < 1.0f) {
            dir = -glm::normalize(dir);
            b2Vec2 force = { dir.x * 2.0f, dir.y * 2.0f };
            b2Body_ApplyForceToCenter(bodyId, force, true);
        }

        PhysicsWorld::clampVelocity(bodyId, 4.0f);
    }

    // Called by main with the player's actual position so follow mode works.
    void setLastPlayerPos(const glm::vec2& pos) { lastPlayerPos = pos; }

private:
    glm::vec2 lastPlayerPos{0.0f, 0.0f};
};
