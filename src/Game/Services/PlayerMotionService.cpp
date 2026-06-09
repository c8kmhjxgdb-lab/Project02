#include "Game/Services/PlayerMotionService.h"

#include "Game/GameState.h"

#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <algorithm>
#include <cmath>

namespace PlayerMotionService {

namespace {

glm::vec2 getMouseWorldPoint(const Context& context) {
    float screenW = static_cast<float>(std::max(context.screenWidth, 1));
    float screenH = static_cast<float>(std::max(context.screenHeight, 1));
    return context.camera.screenToWorld(context.input.mousePos.x,
                                        context.input.mousePos.y,
                                        screenW,
                                        screenH);
}

glm::vec2 getAimDirection(const Context& context, const glm::vec2& origin) {
    glm::vec2 target = getMouseWorldPoint(context);
    glm::vec2 dir = target - origin;
    float lenSq = dir.x * dir.x + dir.y * dir.y;
    if (lenSq <= 0.0001f) {
        dir = context.facingDir;
        lenSq = dir.x * dir.x + dir.y * dir.y;
    }
    if (lenSq <= 0.0001f) {
        return glm::vec2(1.0f, 0.0f);
    }
    return dir / std::sqrt(lenSq);
}

void updateFlight(Context& context, float dt) {
    if (context.isFlying) {
        if (context.flightHeight < context.flightHeightTarget) {
            context.flightHeight += context.flightSpeed * dt;
            if (context.flightHeight >= context.flightHeightTarget) {
                context.flightHeight = context.flightHeightTarget;
            }
        }

        context.playerMana -= context.flightManaDrain * dt;
        if (context.playerMana <= 0) {
            context.playerMana = 0;
            context.isFlying = false;
            context.flightHeightTarget = 0.0f;
            context.flightCooldown = context.flightCooldownMax;
        }

        if (context.flightHeight > 1.0f) {
            b2Vec2 pPos = b2Body_GetPosition(context.playerBodyId);
            context.particleSystem.emit(
                glm::vec2(pPos.x, pPos.y - context.flightHeight * 0.3f),
                glm::vec2(0.0f, 0.5f),
                glm::vec3(0.8f, 0.8f, 0.7f),
                0.5f + context.flightHeight * 0.1f,
                0.05f + context.flightHeight * 0.02f,
                ParticleType::Circle
            );
        }
    } else if (context.flightHeight > 0.0f) {
        context.flightHeight -= context.flightSpeed * context.flightDescentSpeedMult * dt;
        if (context.flightHeight <= 0.0f) {
            context.flightHeight = 0.0f;
        }
    }

    context.shadowScale = 1.0f - (context.flightHeight / context.flightMaxHeight) * 0.6f;
    if (context.shadowScale < 0.4f) {
        context.shadowScale = 0.4f;
    }

    if (!context.input.isDown(SDL_SCANCODE_SPACE) && context.isFlying) {
        context.isFlying = false;
        context.flightHeightTarget = 0.0f;
        context.flightCooldown = context.flightCooldownMax;
    }
}

b2Vec2 buildMovementForce(const Context& context) {
    b2Vec2 force;
    force.x = 0.0f;
    force.y = 0.0f;
    if (context.input.isDown(SDL_SCANCODE_W) || context.input.isDown(SDL_SCANCODE_UP)) {
        force.y += context.playerForce;
    }
    if (context.input.isDown(SDL_SCANCODE_S) || context.input.isDown(SDL_SCANCODE_DOWN)) {
        force.y -= context.playerForce;
    }
    if (context.input.isDown(SDL_SCANCODE_A) || context.input.isDown(SDL_SCANCODE_LEFT)) {
        force.x -= context.playerForce;
    }
    if (context.input.isDown(SDL_SCANCODE_D) || context.input.isDown(SDL_SCANCODE_RIGHT)) {
        force.x += context.playerForce;
    }

    float speedMult = context.emotionSystem.getSpeedMultiplier();
    force.x *= speedMult;
    force.y *= speedMult;
    return force;
}

glm::ivec2 getPlayerTile(const Context& context,
                         MapRegion* currentRegion,
                         const glm::vec2& playerPos) {
    if (currentRegion) {
        return currentRegion->getTileMap().worldToTile(playerPos.x, playerPos.y);
    }
    return context.tileMap.worldToTile(playerPos.x, playerPos.y);
}

void applyTerrainAndWeatherMovement(const Context& context,
                                    MapRegion* currentRegion,
                                    const glm::ivec2& playerTile,
                                    b2Vec2& force) {
    if (!context.isFlying) {
        float terrainCost = currentRegion ?
            currentRegion->getTileMap().getMovementCost(playerTile.x, playerTile.y) :
            context.tileMap.getMovementCost(playerTile.x, playerTile.y);

        if (terrainCost > 0.0f) {
            force.x /= terrainCost;
            force.y /= terrainCost;
        }
    } else {
        force.x *= 1.5f;
        force.y *= 1.5f;
    }

    float weatherMult = context.weatherSystem.getMovementMultiplier();
    force.x *= weatherMult;
    force.y *= weatherMult;
}

void applyTerrainDamage(Context& context,
                        float dt,
                        MapRegion* currentRegion,
                        const glm::ivec2& playerTile) {
    float dps = currentRegion ?
        currentRegion->getTileMap().getDamagePerSecond(playerTile.x, playerTile.y) :
        context.tileMap.getDamagePerSecond(playerTile.x, playerTile.y);
    if (dps > 0.0f && !context.playerHealth.isInvincible() && !context.isDead) {
        DamageInfo info{};
        info.amount = dps * dt;
        info.victimBody = context.playerBodyId;
        info.type = DamageType::Normal;
        context.playerHealth.takeDamage(info);
    }
}

void updateCharacterMotion(Context& context, float dt, const b2Vec2& force) {
    context.charTime += dt;

    if (force.x != 0.0f || force.y != 0.0f) {
        context.armAngle = static_cast<float>(std::sin(context.charTime * 8.0f)) * 0.3f;
    } else {
        context.armAngle = 0.0f;
    }
}

}  // namespace

Context makeContext(GameState& gs) {
    return {
        gs.input,
        gs.camera,
        gs.regionManager,
        gs.tileMap,
        gs.weatherSystem,
        gs.emotionSystem,
        gs.projectileManager,
        gs.particleSystem,
        gs.physicsWorld,
        gs.playerHealth,
        gs.playerBodyId,
        gs.playerForce,
        gs.facingDir,
        gs.playerMana,
        gs.isDead,
        gs.isFlying,
        gs.flightHeight,
        gs.flightHeightTarget,
        gs.flightMaxHeight,
        gs.flightSpeed,
        gs.flightDescentSpeedMult,
        gs.flightManaDrain,
        gs.flightCooldown,
        gs.flightCooldownMax,
        gs.shadowScale,
        gs.charTime,
        gs.armAngle,
        gs.screenWidth,
        gs.screenHeight
    };
}

Result update(Context& context, float dt) {
    updateFlight(context, dt);

    b2Vec2 force = buildMovementForce(context);

    b2Vec2 playerPosVec = b2Body_GetPosition(context.playerBodyId);
    glm::vec2 playerPos(playerPosVec.x, playerPosVec.y);
    context.facingDir = getAimDirection(context, playerPos);

    MapRegion* currentRegion = context.regionManager.getCurrentRegion();
    glm::ivec2 playerTile = getPlayerTile(context, currentRegion, playerPos);

    applyTerrainAndWeatherMovement(context, currentRegion, playerTile, force);

    context.projectileManager.capturePreviousPositions();

    b2Body_ApplyForceToCenter(context.playerBodyId, force, true);
    context.physicsWorld.step(dt, 8, 3);

    applyTerrainDamage(context, dt, currentRegion, playerTile);
    context.physicsWorld.clampVelocity(context.playerBodyId, 8.0f);

    updateCharacterMotion(context, dt, force);

    playerPosVec = b2Body_GetPosition(context.playerBodyId);
    context.camera.smoothFollow(glm::vec2(playerPosVec.x, playerPosVec.y), dt, 5.0f);

    return {playerPos, currentRegion};
}

}  // namespace PlayerMotionService
