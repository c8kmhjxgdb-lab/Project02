#include "Game/Services/CombatService.h"

#include "Engine/Renderer/ParticleSystem.h"
#include "Game/Ability/Lightning.h"
#include "Game/GameState.h"
#include "Game/Services/AudioService.h"
#include "Game/Services/CombatCollisionService.h"
#include "Utils/Math.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace {

const Enemy* findLightningTarget(const std::vector<const Enemy*>& enemies,
                                 const LightningChain& chain,
                                 const glm::vec2& origin,
                                 const glm::vec2& aimDir,
                                 float range,
                                 bool aimFirstTarget) {
    const Enemy* best = nullptr;
    float bestScore = range + 100.0f;

    for (const Enemy* enemy : enemies) {
        if (!enemy || !b2Body_IsValid(enemy->bodyId)) continue;
        if (chain.hasHit(enemy->bodyId)) continue;

        b2Vec2 ePos = b2Body_GetPosition(enemy->bodyId);
        glm::vec2 enemyPos(ePos.x, ePos.y);
        glm::vec2 toEnemy = enemyPos - origin;
        float dist = glm::length(toEnemy);
        if (dist <= 0.001f || dist > range) continue;

        float score = dist;
        if (aimFirstTarget) {
            glm::vec2 dir = toEnemy / dist;
            float alignment = glm::dot(dir, aimDir);
            if (alignment < 0.28f) continue;
            score = dist * (1.35f - alignment);
        }

        if (score < bestScore) {
            bestScore = score;
            best = enemy;
        }
    }

    return best;
}

}  // namespace

namespace CombatService {

CastContext makeCastContext(GameState& gs) {
    return {
        gs.isDead,
        gs.worldId,
        gs.playerBodyId,
        gs.facingDir,
        gs.projectileManager,
        gs.particleSystem,
        gs.fireballCooldown,
        gs.fireballCooldownMax,
        gs.lightning,
        gs.playerMana,
        gs.enemyManager,
        &gs.audioSystem
    };
}

bool tryCastProjectile(CastContext& context,
                       ProjectileType type,
                       const glm::vec2& playerPos,
                       const glm::vec2& aimDir) {
    if (context.isDead || context.fireballCooldown > 0.0f) return false;

    context.facingDir = aimDir;

    float damage = 25.0f;
    float speed = 18.0f;
    glm::vec3 particleColor(1.0f, 0.5f, 0.0f);

    if (type == ProjectileType::IceSpike) {
        damage = 20.0f;
        speed = 14.0f;
        particleColor = glm::vec3(0.4f, 0.7f, 1.0f);
    }

    glm::vec2 muzzlePos = playerPos + aimDir * 0.48f;
    context.projectileManager.fire(
        context.worldId,
        muzzlePos,
        aimDir,
        type,
        damage,
        speed,
        context.playerBodyId);
    context.fireballCooldown = context.fireballCooldownMax;
    if (type == ProjectileType::Fireball) {
        context.particleSystem.emitBurst(muzzlePos, 8, particleColor, 2.5f, 0.18f, 0.075f);
        context.particleSystem.emitRing(
            muzzlePos,
            8,
            glm::vec3(1.0f, 0.25f, 0.05f),
            0.18f,
            0.16f,
            0.030f);
    } else {
        context.particleSystem.emitBurst(muzzlePos, 7, particleColor, 1.8f, 0.22f, 0.060f);
        context.particleSystem.emitRing(
            muzzlePos,
            10,
            glm::vec3(0.72f, 0.92f, 1.0f),
            0.22f,
            0.20f,
            0.026f);
    }
    if (context.audioSystem) {
        AudioService::playSkillSfx(
            *context.audioSystem,
            type == ProjectileType::IceSpike ? "ice_spike" : "fireball");
    }
    return true;
}

bool tryCastLightning(CastContext& context,
                      const glm::vec2& playerPos,
                      const glm::vec2& aimDir) {
    if (context.isDead ||
        !context.lightning.canCast() ||
        context.playerMana < context.lightning.getManaCost()) {
        return false;
    }

    context.facingDir = aimDir;

    context.lightning.begin(playerPos);
    context.playerMana -= context.lightning.getManaCost();
    context.lightning.setCooldown(3.0f);
    if (context.audioSystem) {
        AudioService::playSkillSfx(*context.audioSystem, "lightning");
    }

    float currentDamage = context.lightning.getBaseDamage();
    glm::vec2 currentPos = playerPos;
    float range = context.lightning.getChainRange();
    auto aliveEnemies = context.enemyManager.getAlive();

    for (int chainIndex = 0; chainIndex < context.lightning.getMaxChains(); ++chainIndex) {
        const Enemy* target = findLightningTarget(
            aliveEnemies,
            context.lightning.getCurrentChain(),
            currentPos,
            aimDir,
            range,
            chainIndex == 0);

        // If the player aims into empty space, fall back to the nearest target
        // so the skill still works as an area chain once cast.
        if (!target && chainIndex == 0) {
            target = findLightningTarget(
                aliveEnemies,
                context.lightning.getCurrentChain(),
                currentPos,
                aimDir,
                range,
                false);
        }
        if (!target) break;

        b2Vec2 ePos = b2Body_GetPosition(target->bodyId);
        glm::vec2 enemyPos(ePos.x, ePos.y);
        context.lightning.addHit(enemyPos, currentDamage, target->bodyId);
        context.enemyManager.damage(target->id, currentDamage);
        context.particleSystem.emitBurst(enemyPos, 6,
            glm::vec3(0.5f, 0.8f, 1.0f), 4.0f, 0.3f, 0.08f);

        currentDamage *= context.lightning.getCurrentChain().damageDecay;
        currentPos = enemyPos;
    }

    return true;
}

SpawnContext makeSpawnContext(GameState& gs) {
    return {
        gs.playerBodyId,
        gs.charTime,
        gs.enemiesKilled,
        gs.enemyManager,
        gs.worldId
    };
}

void spawnEnemy(SpawnContext& context, const glm::vec2& pos) {
    // Pick random type based on distance from spawn
    b2Vec2 playerPos = b2Body_GetPosition(context.playerBodyId);
    float dist = glm::distance(glm::vec2(playerPos.x, playerPos.y), pos);

    EnemyType type;
    float r = Math::hashRandom(static_cast<unsigned int>(context.charTime * 1000.0f +
                                                          context.enemiesKilled));
    if (dist > 15.0f && r < 0.3f) {
        type = EnemyType::Shooter;
    } else if (r < 0.6f) {
        type = EnemyType::Exploder;
    } else {
        type = EnemyType::Chaser;
    }

    context.enemyManager.spawn(context.worldId, pos, type);
}

void handleCollisions(CombatCollisionService::Context& context) {
    CombatCollisionService::handleCollisions(context);
}

}  // namespace CombatService
