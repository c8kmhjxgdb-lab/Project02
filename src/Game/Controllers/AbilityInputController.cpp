#include "Game/Controllers/AbilityInputController.h"

#include "Game/GameState.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/PlayerInputQuery.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace {

bool isGameplayActionAllowed(const GameState& gs) {
    return !gs.isDead && !gs.buildingSystem.isActive() && !gs.toySystem.isMiniCarActive();
}

}  // namespace

namespace AbilityInputController {

void handleKeyDown(GameState& gs, SDL_Scancode scancode) {
    if (scancode == SDL_SCANCODE_J && isGameplayActionAllowed(gs)) {
        CombatService::tryCastProjectile(gs, ProjectileType::Fireball);
    }

    if (scancode == SDL_SCANCODE_L && isGameplayActionAllowed(gs)) {
        CombatService::tryCastProjectile(gs, ProjectileType::IceSpike);
    }

    if (scancode == SDL_SCANCODE_K && isGameplayActionAllowed(gs)) {
        glm::vec2 playerPos = PlayerInputQuery::getPlayerPosition(gs);
        glm::vec2 aimDir = PlayerInputQuery::getAimDirection(gs, playerPos);
        gs.facingDir = aimDir;
        if (gs.superStrength.isGrabbing()) {
            gs.superStrength.throwObject(aimDir, 20.0f);
        } else {
            gs.superStrength.tryGrab(gs.worldId, gs.playerBodyId, aimDir);
        }
    }

    if (scancode == SDL_SCANCODE_SPACE && !gs.isDead
        && !gs.buildingSystem.isActive()
        && !gs.toySystem.isMiniCarActive()
        && gs.flightCooldown <= 0.0f
        && gs.playerMana >= 5.0f
        && gs.flightHeight <= 0.1f) {
        if (!gs.isFlying) {
            gs.isFlying = true;
            gs.flightHeightTarget = gs.flightMaxHeight;
        }
    }

    if (scancode == SDL_SCANCODE_F && !gs.isDead && !gs.shield.isActive()
        && !gs.buildingSystem.isActive()
        && !gs.toySystem.isMiniCarActive()
        && gs.shieldCooldown <= 0.0f
        && gs.playerMana >= 15.0f) {
        gs.shield.activate(15.0f);
        gs.playerMana -= 15.0f;
        gs.shieldCooldown = gs.shieldCooldownMax;

        b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
        gs.particleSystem.emitRing(glm::vec2(pPos.x, pPos.y),
            16, glm::vec3(0.3f, 0.7f, 1.0f),
            gs.shield.getRadius(), 0.5f, 0.1f);
    }

    if (scancode == SDL_SCANCODE_Q && isGameplayActionAllowed(gs)) {
        CombatService::tryCastLightning(gs);
    }

    if (scancode == SDL_SCANCODE_G && !gs.isDead
        && !gs.buildingSystem.isActive()
        && !gs.toySystem.isMiniCarActive()
        && gs.princess && gs.princess->isFollowing()
        && gs.princess->isUltimateReady()
        && gs.bondTechnique.canActivate()) {
        b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
        glm::vec2 centerPos(pPos.x, pPos.y);

        gs.bondTechnique.activate(centerPos);
        gs.princess->ultimateCharge = 0.0f;
        gs.bondTechnique.setCooldown(30.0f);

        gs.particleSystem.emitBurst(centerPos, 40,
            glm::vec3(1.0f, 0.9f, 0.5f), 8.0f, 0.8f, 0.15f);
        gs.particleSystem.emitRing(centerPos, 24,
            glm::vec3(1.0f, 0.7f, 0.3f),
            gs.bondTechnique.getMaxRadius(), 1.0f, 0.2f);
    }
}

void handleMouseButtonDown(GameState& gs, Uint8 button) {
    if (button == SDL_BUTTON_LEFT) {
        CombatService::tryCastProjectile(gs, ProjectileType::Fireball);
    } else if (button == SDL_BUTTON_RIGHT) {
        CombatService::tryCastProjectile(gs, ProjectileType::IceSpike);
    } else if (button == SDL_BUTTON_MIDDLE) {
        CombatService::tryCastLightning(gs);
    }
}

}  // namespace AbilityInputController
