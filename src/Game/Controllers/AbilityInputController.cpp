#include "Game/Controllers/AbilityInputController.h"

#include "Game/GameState.h"
#include "Game/Services/PlayerInputQuery.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cmath>

namespace {

bool isGameplayActionAllowed(const AbilityInputController::Context& context) {
    return !context.isDead &&
           !context.gameMenuOpen &&
           !context.buildingSystem.isActive() &&
           !context.toySystem.isMiniCarActive();
}

glm::vec2 getPlayerPosition(const AbilityInputController::Context& context) {
    b2Vec2 pos = b2Body_GetPosition(context.playerBodyId);
    return glm::vec2(pos.x, pos.y);
}

glm::vec2 getAimDirection(const AbilityInputController::Context& context,
                          const AbilityInputController::Callbacks& callbacks,
                          const glm::vec2& origin) {
    glm::vec2 target = callbacks.getMouseWorldPoint
        ? callbacks.getMouseWorldPoint(context)
        : origin + context.facingDir;
    glm::vec2 dir = target - origin;
    float lenSq = glm::dot(dir, dir);
    if (lenSq <= 0.0001f) {
        dir = context.facingDir;
        lenSq = glm::dot(dir, dir);
    }
    if (lenSq <= 0.0001f) {
        return glm::vec2(1.0f, 0.0f);
    }
    return dir / std::sqrt(lenSq);
}

}  // namespace

namespace AbilityInputController {

Context makeContext(GameState& gs) {
    return {
        gs.isDead,
        gs.buildingSystem,
        gs.toySystem,
        gs.superStrength,
        gs.shield,
        gs.bondTechnique,
        gs.particleSystem,
        gs.princess,
        gs.worldId,
        gs.playerBodyId,
        gs.facingDir,
        gs.playerMana,
        gs.isFlying,
        gs.flightHeight,
        gs.flightHeightTarget,
        gs.flightMaxHeight,
        gs.flightCooldown,
        gs.shieldCooldown,
        gs.shieldCooldownMax,
        gs.ui.gameMenuOpen,
        CombatService::makeCastContext(gs)
    };
}

Callbacks makeCallbacks(GameState& gs) {
    return {
        [&gs](const Context&) {
            return PlayerInputQuery::getMouseWorldPoint(gs);
        }
    };
}

void handleKeyDown(Context& context, SDL_Scancode scancode, const Callbacks& callbacks) {
    if (scancode == SDL_SCANCODE_J && isGameplayActionAllowed(context)) {
        glm::vec2 playerPos = getPlayerPosition(context);
        glm::vec2 aimDir = getAimDirection(context, callbacks, playerPos);
        CombatService::tryCastProjectile(
            context.castContext,
            ProjectileType::Fireball,
            playerPos,
            aimDir);
    }

    if (scancode == SDL_SCANCODE_L && isGameplayActionAllowed(context)) {
        glm::vec2 playerPos = getPlayerPosition(context);
        glm::vec2 aimDir = getAimDirection(context, callbacks, playerPos);
        CombatService::tryCastProjectile(
            context.castContext,
            ProjectileType::IceSpike,
            playerPos,
            aimDir);
    }

    if (scancode == SDL_SCANCODE_K && isGameplayActionAllowed(context)) {
        glm::vec2 playerPos = getPlayerPosition(context);
        glm::vec2 aimDir = getAimDirection(context, callbacks, playerPos);
        context.facingDir = aimDir;
        if (context.superStrength.isGrabbing()) {
            context.superStrength.throwObject(aimDir, 20.0f);
        } else {
            context.superStrength.tryGrab(context.worldId, context.playerBodyId, aimDir);
        }
    }

    if (scancode == SDL_SCANCODE_SPACE && isGameplayActionAllowed(context)
        && context.flightCooldown <= 0.0f
        && context.playerMana >= 5.0f
        && context.flightHeight <= 0.1f) {
        if (!context.isFlying) {
            context.isFlying = true;
            context.flightHeightTarget = context.flightMaxHeight;
        }
    }

    if (scancode == SDL_SCANCODE_F && isGameplayActionAllowed(context)
        && !context.shield.isActive()
        && context.shieldCooldown <= 0.0f
        && context.playerMana >= 15.0f) {
        context.shield.activate(15.0f);
        context.playerMana -= 15.0f;
        context.shieldCooldown = context.shieldCooldownMax;

        b2Vec2 pPos = b2Body_GetPosition(context.playerBodyId);
        context.particleSystem.emitRing(glm::vec2(pPos.x, pPos.y),
            16, glm::vec3(0.3f, 0.7f, 1.0f),
            context.shield.getRadius(), 0.5f, 0.1f);
    }

    if (scancode == SDL_SCANCODE_Q && isGameplayActionAllowed(context)) {
        glm::vec2 playerPos = getPlayerPosition(context);
        glm::vec2 aimDir = getAimDirection(context, callbacks, playerPos);
        CombatService::tryCastLightning(context.castContext, playerPos, aimDir);
    }

    if (scancode == SDL_SCANCODE_G && isGameplayActionAllowed(context)
        && context.princess && context.princess->isFollowing()
        && context.princess->isUltimateReady()
        && context.bondTechnique.canActivate()) {
        b2Vec2 pPos = b2Body_GetPosition(context.playerBodyId);
        glm::vec2 centerPos(pPos.x, pPos.y);

        context.bondTechnique.activate(centerPos);
        context.princess->ultimateCharge = 0.0f;
        context.bondTechnique.setCooldown(30.0f);

        context.particleSystem.emitBurst(centerPos, 40,
            glm::vec3(1.0f, 0.9f, 0.5f), 8.0f, 0.8f, 0.15f);
        context.particleSystem.emitRing(centerPos, 24,
            glm::vec3(1.0f, 0.7f, 0.3f),
            context.bondTechnique.getMaxRadius(), 1.0f, 0.2f);
    }
}

void handleMouseButtonDown(Context& context, Uint8 button, const Callbacks& callbacks) {
    if (!isGameplayActionAllowed(context)) {
        return;
    }

    if (button == SDL_BUTTON_LEFT) {
        glm::vec2 playerPos = getPlayerPosition(context);
        glm::vec2 aimDir = getAimDirection(context, callbacks, playerPos);
        CombatService::tryCastProjectile(
            context.castContext,
            ProjectileType::Fireball,
            playerPos,
            aimDir);
    } else if (button == SDL_BUTTON_RIGHT) {
        glm::vec2 playerPos = getPlayerPosition(context);
        glm::vec2 aimDir = getAimDirection(context, callbacks, playerPos);
        CombatService::tryCastProjectile(
            context.castContext,
            ProjectileType::IceSpike,
            playerPos,
            aimDir);
    } else if (button == SDL_BUTTON_MIDDLE) {
        glm::vec2 playerPos = getPlayerPosition(context);
        glm::vec2 aimDir = getAimDirection(context, callbacks, playerPos);
        CombatService::tryCastLightning(context.castContext, playerPos, aimDir);
    }
}

}  // namespace AbilityInputController
