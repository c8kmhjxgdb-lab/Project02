#include "Game/Services/PlayerLifecycleService.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>

#include <algorithm>

namespace PlayerLifecycleService {

namespace {

void updateNoticeTimer(Context& context, float dt) {
    if (context.stage7NoticeTimer > 0.0f) {
        context.stage7NoticeTimer = std::max(0.0f, context.stage7NoticeTimer - dt);
    }
}

}  // namespace

Context makeContext(GameState& gs) {
    return {
        gs.flightCooldown,
        gs.shieldCooldown,
        gs.stage7NoticeTimer,
        gs.playerMana,
        gs.playerMaxMana,
        gs.manaRegen,
        gs.deathTimer,
        gs.isDead,
        gs.playerBodyId,
        gs.spawnPoint,
        gs.camera,
        gs.playerHealth
    };
}

void updateAlive(Context& context, float dt) {
    if (context.flightCooldown > 0.0f) {
        context.flightCooldown -= dt;
    }
    if (context.shieldCooldown > 0.0f) {
        context.shieldCooldown -= dt;
    }

    updateNoticeTimer(context, dt);

    if (context.playerMana < context.playerMaxMana) {
        context.playerMana = std::min(context.playerMaxMana,
                                      context.playerMana + context.manaRegen * dt);
    }
}

void updateDeathRespawn(Context& context, float dt) {
    context.deathTimer -= dt;
    if (context.deathTimer > 0.0f) {
        return;
    }

    context.isDead = false;
    b2Body_SetTransform(context.playerBodyId,
                        { context.spawnPoint.x, context.spawnPoint.y },
                        b2Rot_identity);
    b2Body_SetLinearVelocity(context.playerBodyId, b2Vec2_zero);
    context.playerHealth.respawn(0.5f);
    context.camera.position = context.spawnPoint;
}

void updateWhileDead(Context& context, float dt) {
    updateNoticeTimer(context, dt);
}

}  // namespace PlayerLifecycleService
