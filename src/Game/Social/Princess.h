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

    void triggerSpecialEvent(const std::string&) {}

    void render(const glm::mat4& viewProj) override {
        if (!hasBody()) return;

        b2Vec2 pos = b2Body_GetPosition(bodyId);

        Draw2D::beginFrame(viewProj);

        float s = 0.35f;
        float sh = 0.7f;

        Draw2D::drawRectFilled(pos.x - s, pos.y - sh, s * 2.0f, sh * 2.0f, bodyColor);
        Draw2D::drawRect(pos.x - s, pos.y - sh, s * 2.0f, sh * 2.0f, bodyColor * 0.5f, 0.03f);

        float haloIntensity = affection / 1000.0f * 0.3f;
        if (haloIntensity > 0.01f) {
            float haloR = s * 1.5f;
            Draw2D::drawCircle(pos.x, pos.y, haloR, bodyColor * haloIntensity, 0.02f);
        }

        if (following) {
            float indicatorY = pos.y + sh + 0.2f;
            Draw2D::drawCircleFilled(pos.x, indicatorY, 0.05f, glm::vec3(1.0f, 0.8f, 0.2f));
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

    void updateBehavior(float dt, const glm::vec2& playerPos) {
        if (!hasBody()) return;

        b2Vec2 npcPos = b2Body_GetPosition(bodyId);
        glm::vec2 currentPos(npcPos.x, npcPos.y);

        if (following) {
            glm::vec2 dir = playerPos - currentPos;
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

            // Clamp velocity
            PhysicsWorld::clampVelocity(bodyId, 4.0f);
        }

        (void)dt;
    }

    // Override NPC update to also follow the player when `following` is enabled.
    // Previously the follow behavior lived in updateBehavior() but it was never
    // called from main, so the princess always walked on her schedule.
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
