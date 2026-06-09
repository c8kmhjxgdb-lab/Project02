#include "Game/Services/CombatService.h"

#include "Game/GameState.h"
#include "Game/Services/CombatCollisionService.h"
#include "Game/Services/PlayerInputQuery.h"
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

bool tryCastProjectile(GameState& gs, ProjectileType type) {
    if (gs.isDead || gs.fireballCooldown > 0.0f) return false;

    glm::vec2 playerPos = PlayerInputQuery::getPlayerPosition(gs);
    glm::vec2 aimDir = PlayerInputQuery::getAimDirection(gs, playerPos);
    gs.facingDir = aimDir;

    float damage = 25.0f;
    float speed = 18.0f;
    glm::vec3 particleColor(1.0f, 0.5f, 0.0f);

    if (type == ProjectileType::IceSpike) {
        damage = 20.0f;
        speed = 14.0f;
        particleColor = glm::vec3(0.4f, 0.7f, 1.0f);
    }

    glm::vec2 muzzlePos = playerPos + aimDir * 0.48f;
    gs.projectileManager.fire(gs.worldId, muzzlePos, aimDir, type, damage, speed, gs.playerBodyId);
    gs.fireballCooldown = gs.fireballCooldownMax;
    if (type == ProjectileType::Fireball) {
        gs.particleSystem.emitBurst(muzzlePos, 8, particleColor, 2.5f, 0.18f, 0.075f);
        gs.particleSystem.emitRing(muzzlePos, 8, glm::vec3(1.0f, 0.25f, 0.05f), 0.18f, 0.16f, 0.030f);
    } else {
        gs.particleSystem.emitBurst(muzzlePos, 7, particleColor, 1.8f, 0.22f, 0.060f);
        gs.particleSystem.emitRing(muzzlePos, 10, glm::vec3(0.72f, 0.92f, 1.0f), 0.22f, 0.20f, 0.026f);
    }
    return true;
}

bool tryCastLightning(GameState& gs) {
    if (gs.isDead || !gs.lightning.canCast() || gs.playerMana < gs.lightning.getManaCost()) {
        return false;
    }

    glm::vec2 playerPos = PlayerInputQuery::getPlayerPosition(gs);
    glm::vec2 aimDir = PlayerInputQuery::getAimDirection(gs, playerPos);
    gs.facingDir = aimDir;

    gs.lightning.begin(playerPos);
    gs.playerMana -= gs.lightning.getManaCost();
    gs.lightning.setCooldown(3.0f);

    float currentDamage = gs.lightning.getBaseDamage();
    glm::vec2 currentPos = playerPos;
    float range = gs.lightning.getChainRange();
    auto aliveEnemies = gs.enemyManager.getAlive();

    for (int chainIndex = 0; chainIndex < gs.lightning.getMaxChains(); ++chainIndex) {
        const Enemy* target = findLightningTarget(
            aliveEnemies,
            gs.lightning.getCurrentChain(),
            currentPos,
            aimDir,
            range,
            chainIndex == 0);

        // If the player aims into empty space, fall back to the nearest target
        // so the skill still works as an area chain once cast.
        if (!target && chainIndex == 0) {
            target = findLightningTarget(
                aliveEnemies,
                gs.lightning.getCurrentChain(),
                currentPos,
                aimDir,
                range,
                false);
        }
        if (!target) break;

        b2Vec2 ePos = b2Body_GetPosition(target->bodyId);
        glm::vec2 enemyPos(ePos.x, ePos.y);
        gs.lightning.addHit(enemyPos, currentDamage, target->bodyId);
        gs.enemyManager.damage(target->id, currentDamage);
        gs.particleSystem.emitBurst(enemyPos, 6,
            glm::vec3(0.5f, 0.8f, 1.0f), 4.0f, 0.3f, 0.08f);

        currentDamage *= gs.lightning.getCurrentChain().damageDecay;
        currentPos = enemyPos;
    }

    return true;
}

void spawnEnemy(GameState& gs, const glm::vec2& pos) {
    // Pick random type based on distance from spawn
    b2Vec2 playerPos = b2Body_GetPosition(gs.playerBodyId);
    float dist = glm::distance(glm::vec2(playerPos.x, playerPos.y), pos);

    EnemyType type;
    float r = Math::hashRandom(static_cast<unsigned int>(gs.charTime * 1000.0f + gs.enemiesKilled));
    if (dist > 15.0f && r < 0.3f) {
        type = EnemyType::Shooter;
    } else if (r < 0.6f) {
        type = EnemyType::Exploder;
    } else {
        type = EnemyType::Chaser;
    }

    gs.enemyManager.spawn(gs.worldId, pos, type);
}

void handleCollisions(GameState& gs) {
    CombatCollisionService::handleCollisions(gs);
}

}  // namespace CombatService
