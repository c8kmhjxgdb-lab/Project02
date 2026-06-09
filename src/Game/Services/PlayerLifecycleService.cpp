#include "Game/Services/PlayerLifecycleService.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>

#include <algorithm>

namespace PlayerLifecycleService {

namespace {

void updateNoticeTimer(GameState& gs, float dt) {
    if (gs.stage7NoticeTimer > 0.0f) {
        gs.stage7NoticeTimer = std::max(0.0f, gs.stage7NoticeTimer - dt);
    }
}

}  // namespace

void updateAlive(GameState& gs, float dt) {
    if (gs.flightCooldown > 0.0f) {
        gs.flightCooldown -= dt;
    }
    if (gs.shieldCooldown > 0.0f) {
        gs.shieldCooldown -= dt;
    }

    updateNoticeTimer(gs, dt);

    if (gs.playerMana < gs.playerMaxMana) {
        gs.playerMana = std::min(gs.playerMaxMana, gs.playerMana + gs.manaRegen * dt);
    }
}

void updateDeathRespawn(GameState& gs, float dt) {
    gs.deathTimer -= dt;
    if (gs.deathTimer > 0.0f) {
        return;
    }

    gs.isDead = false;
    b2Body_SetTransform(gs.playerBodyId, { gs.spawnPoint.x, gs.spawnPoint.y }, b2Rot_identity);
    b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);
    gs.playerHealth.respawn(0.5f);
    gs.camera.position = gs.spawnPoint;
}

void updateWhileDead(GameState& gs, float dt) {
    updateNoticeTimer(gs, dt);
}

}  // namespace PlayerLifecycleService
