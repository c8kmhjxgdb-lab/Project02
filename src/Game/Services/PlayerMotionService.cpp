#include "Game/Services/PlayerMotionService.h"

#include "Game/GameState.h"
#include "Game/Services/PlayerInputQuery.h"

#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <algorithm>
#include <cmath>

namespace PlayerMotionService {

namespace {

void updateFlight(GameState& gs, float dt) {
    if (gs.isFlying) {
        if (gs.flightHeight < gs.flightHeightTarget) {
            gs.flightHeight += gs.flightSpeed * dt;
            if (gs.flightHeight >= gs.flightHeightTarget) {
                gs.flightHeight = gs.flightHeightTarget;
            }
        }

        gs.playerMana -= gs.flightManaDrain * dt;
        if (gs.playerMana <= 0) {
            gs.playerMana = 0;
            gs.isFlying = false;
            gs.flightHeightTarget = 0.0f;
            gs.flightCooldown = gs.flightCooldownMax;
        }

        if (gs.flightHeight > 1.0f) {
            b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
            gs.particleSystem.emit(
                glm::vec2(pPos.x, pPos.y - gs.flightHeight * 0.3f),
                glm::vec2(0.0f, 0.5f),
                glm::vec3(0.8f, 0.8f, 0.7f),
                0.5f + gs.flightHeight * 0.1f,
                0.05f + gs.flightHeight * 0.02f,
                ParticleType::Circle
            );
        }
    } else if (gs.flightHeight > 0.0f) {
        gs.flightHeight -= gs.flightSpeed * gs.flightDescentSpeedMult * dt;
        if (gs.flightHeight <= 0.0f) {
            gs.flightHeight = 0.0f;
        }
    }

    gs.shadowScale = 1.0f - (gs.flightHeight / gs.flightMaxHeight) * 0.6f;
    if (gs.shadowScale < 0.4f) {
        gs.shadowScale = 0.4f;
    }

    if (!gs.keys[SDL_SCANCODE_SPACE] && gs.isFlying) {
        gs.isFlying = false;
        gs.flightHeightTarget = 0.0f;
        gs.flightCooldown = gs.flightCooldownMax;
    }
}

b2Vec2 buildMovementForce(GameState& gs) {
    b2Vec2 force;
    force.x = 0.0f;
    force.y = 0.0f;
    if (gs.keys[SDL_SCANCODE_W] || gs.keys[SDL_SCANCODE_UP])    force.y += gs.playerForce;
    if (gs.keys[SDL_SCANCODE_S] || gs.keys[SDL_SCANCODE_DOWN])  force.y -= gs.playerForce;
    if (gs.keys[SDL_SCANCODE_A] || gs.keys[SDL_SCANCODE_LEFT])  force.x -= gs.playerForce;
    if (gs.keys[SDL_SCANCODE_D] || gs.keys[SDL_SCANCODE_RIGHT]) force.x += gs.playerForce;

    float speedMult = gs.emotionSystem.getSpeedMultiplier();
    force.x *= speedMult;
    force.y *= speedMult;
    return force;
}

glm::ivec2 getPlayerTile(GameState& gs, MapRegion* currentRegion, const glm::vec2& playerPos) {
    if (currentRegion) {
        return currentRegion->getTileMap().worldToTile(playerPos.x, playerPos.y);
    }
    return gs.tileMap.worldToTile(playerPos.x, playerPos.y);
}

void applyTerrainAndWeatherMovement(GameState& gs,
                                    MapRegion* currentRegion,
                                    const glm::ivec2& playerTile,
                                    b2Vec2& force) {
    if (!gs.isFlying) {
        float terrainCost = currentRegion ?
            currentRegion->getTileMap().getMovementCost(playerTile.x, playerTile.y) :
            gs.tileMap.getMovementCost(playerTile.x, playerTile.y);

        if (terrainCost > 0.0f) {
            force.x /= terrainCost;
            force.y /= terrainCost;
        }
    } else {
        force.x *= 1.5f;
        force.y *= 1.5f;
    }

    float weatherMult = gs.weatherSystem.getMovementMultiplier();
    force.x *= weatherMult;
    force.y *= weatherMult;
}

void applyTerrainDamage(GameState& gs, float dt, MapRegion* currentRegion, const glm::ivec2& playerTile) {
    float dps = currentRegion ?
        currentRegion->getTileMap().getDamagePerSecond(playerTile.x, playerTile.y) :
        gs.tileMap.getDamagePerSecond(playerTile.x, playerTile.y);
    if (dps > 0.0f && !gs.playerHealth.isInvincible() && !gs.isDead) {
        DamageInfo info{};
        info.amount = dps * dt;
        info.victimBody = gs.playerBodyId;
        info.type = DamageType::Normal;
        gs.playerHealth.takeDamage(info);
    }
}

void updateCharacterMotion(GameState& gs, float dt, const b2Vec2& force) {
    gs.charTime += dt;

    if (force.x != 0.0f || force.y != 0.0f) {
        gs.armAngle = static_cast<float>(std::sin(gs.charTime * 8.0f)) * 0.3f;
    } else {
        gs.armAngle = 0.0f;
    }
}

}  // namespace

Result update(GameState& gs, float dt) {
    updateFlight(gs, dt);

    b2Vec2 force = buildMovementForce(gs);

    gs.facingDir = PlayerInputQuery::getAimDirection(
        gs,
        PlayerInputQuery::getPlayerPosition(gs));

    b2Vec2 playerPosVec = b2Body_GetPosition(gs.playerBodyId);
    glm::vec2 playerPos(playerPosVec.x, playerPosVec.y);
    MapRegion* currentRegion = gs.regionManager.getCurrentRegion();
    glm::ivec2 playerTile = getPlayerTile(gs, currentRegion, playerPos);

    applyTerrainAndWeatherMovement(gs, currentRegion, playerTile, force);

    gs.projectileManager.capturePreviousPositions();

    b2Body_ApplyForceToCenter(gs.playerBodyId, force, true);
    gs.physicsWorld.step(dt, 8, 3);

    applyTerrainDamage(gs, dt, currentRegion, playerTile);
    gs.physicsWorld.clampVelocity(gs.playerBodyId, 8.0f);

    updateCharacterMotion(gs, dt, force);

    playerPosVec = b2Body_GetPosition(gs.playerBodyId);
    gs.camera.smoothFollow(glm::vec2(playerPosVec.x, playerPosVec.y), dt, 5.0f);

    return {playerPos, currentRegion};
}

}  // namespace PlayerMotionService
