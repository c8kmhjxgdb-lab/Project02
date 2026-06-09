#include "Game/Services/CombatCollisionService.h"

#include "Game/GameState.h"
#include "Utils/Math.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <algorithm>
#include <vector>

namespace {

float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

bool sameBody(b2BodyId a, b2BodyId b) {
    return a.index1 == b.index1 && a.world0 == b.world0 && a.generation == b.generation;
}

glm::vec2 closestPointOnSegment(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p) {
    glm::vec2 ab = b - a;
    float lenSq = ab.x * ab.x + ab.y * ab.y;
    if (lenSq <= 0.000001f) return b;

    glm::vec2 ap = p - a;
    float t = (ap.x * ab.x + ap.y * ab.y) / lenSq;
    t = clamp01(t);
    return a + ab * t;
}

bool blockPlayerDamageWithShield(GameState& gs,
                                 const glm::vec2& playerPos,
                                 const glm::vec2& sourcePos,
                                 float sourceRadius) {
    if (!gs.shield.isActive()) return false;

    float blockRange = gs.shield.getRadius() + std::max(0.35f, sourceRadius);
    if (glm::distance(playerPos, sourcePos) > blockRange) return false;

    gs.particleSystem.emitRing(playerPos, 18,
        glm::vec3(0.42f, 0.78f, 1.0f), gs.shield.getRadius(), 0.28f, 0.055f);
    gs.particleSystem.emitBurst(sourcePos, 10,
        glm::vec3(0.62f, 0.88f, 1.0f), 3.2f, 0.20f, 0.055f);
    return true;
}

}  // namespace

namespace CombatCollisionService {

void handleCollisions(GameState& gs) {
    struct HitInfo {
        ProjectileId projId;
        EnemyId enemyId;
        b2BodyId enemyBody;
        glm::vec2 hitPos;
        float damage;
        ProjectileType type;
    };
    std::vector<HitInfo> hits;

    const auto& projectiles = gs.projectileManager.getActive();
    auto aliveEnemies = gs.enemyManager.getAlive();

    for (const auto& proj : projectiles) {
        if (!proj.active) continue;
        if (!b2Body_IsValid(proj.bodyId)) continue;

        b2Vec2 projPos = b2Body_GetPosition(proj.bodyId);
        glm::vec2 currentPos(projPos.x, projPos.y);
        glm::vec2 previousPos = proj.previousPosition;

        for (const Enemy* enemy : aliveEnemies) {
            if (!enemy->active) continue;
            if (enemy->state == Enemy::State::Dead) continue;
            if (!b2Body_IsValid(enemy->bodyId)) continue;
            if (sameBody(proj.ownerBodyId, enemy->bodyId)) continue;

            b2Vec2 enemyPos = b2Body_GetPosition(enemy->bodyId);
            glm::vec2 ePos(enemyPos.x, enemyPos.y);
            glm::vec2 hitPos = closestPointOnSegment(previousPos, currentPos, ePos);

            float dist = glm::distance(hitPos, ePos);
            if (dist <= (proj.radius + enemy->radius + 0.05f)) {
                hits.push_back({proj.id, enemy->id, enemy->bodyId, hitPos, proj.damage, proj.type});
                break;
            }
        }
    }

    for (const auto& hit : hits) {
        gs.projectileManager.onHit(hit.projId, hit.enemyBody);
        gs.enemyManager.damage(hit.enemyId, hit.damage);

        const Enemy* enemy = gs.enemyManager.find(hit.enemyId);
        if (enemy) {
            if (hit.type == ProjectileType::IceSpike) {
                const_cast<Enemy*>(enemy)->applySlow(3.0f, 0.4f);
            }
            gs.particleSystem.emitBurst(hit.hitPos, 8,
                hit.type == ProjectileType::IceSpike ? glm::vec3(0.5f, 0.8f, 1.0f) :
                hit.type == ProjectileType::Thunder ? glm::vec3(0.8f, 0.9f, 1.0f) :
                glm::vec3(1.0f, 0.5f, 0.0f), 3.0f, 0.3f, 0.1f);
        }
    }

    b2Vec2 playerPos = b2Body_GetPosition(gs.playerBodyId);
    glm::vec2 pPos2(playerPos.x, playerPos.y);

    if (!gs.playerHealth.isInvincible() && !gs.isDead && !gs.isFlying) {
        for (const Enemy* enemy : aliveEnemies) {
            if (!enemy->active) continue;
            if (enemy->state == Enemy::State::Dead) continue;
            if (!b2Body_IsValid(enemy->bodyId)) continue;

            b2Vec2 enemyPos = b2Body_GetPosition(enemy->bodyId);
            glm::vec2 ePos2(enemyPos.x, enemyPos.y);

            float dist = glm::distance(pPos2, ePos2);
            if (dist < (0.3f + enemy->radius)) {
                if (blockPlayerDamageWithShield(gs, pPos2, ePos2, enemy->radius)) {
                    gs.shield.checkAndRepelEnemy(enemy->bodyId, pPos2, 20.0f);
                    break;
                }

                DamageInfo info;
                info.amount = enemy->damage;
                info.sourcePosition = ePos2;
                info.sourceBody = enemy->bodyId;
                info.victimBody = gs.playerBodyId;
                info.type = DamageType::Normal;

                gs.playerHealth.takeDamage(info);
                gs.playerHealth.setInvincible(0.5f);

                glm::vec2 kbDir = Math::normalize(pPos2 - ePos2);
                b2Vec2 kbForce = { kbDir.x * 10.0f, kbDir.y * 10.0f };
                b2Body_ApplyLinearImpulseToCenter(gs.playerBodyId, kbForce, true);

                gs.particleSystem.emitBurst(pPos2, 5, glm::vec3(1, 0, 0), 2.0f, 0.2f, 0.08f);

                if (!gs.playerHealth.isAlive()) {
                    gs.isDead = true;
                    gs.deathTimer = 3.0f;
                }
                break;
            }
        }
    }

    const auto& allEnemies = gs.enemyManager.getActive();
    for (const Enemy& enemy : allEnemies) {
        if (!enemy.active || enemy.type != EnemyType::Exploder) continue;
        if (enemy.state != Enemy::State::Dead) continue;
        if (!b2Body_IsValid(enemy.bodyId)) continue;
        if (enemy.explosionTriggered) continue;

        if (enemy.deathTimer > 0.0f) {
            Enemy* mutableEnemy = gs.enemyManager.find(enemy.id);
            if (!mutableEnemy) continue;
            mutableEnemy->explosionTriggered = true;

            b2Vec2 enemyPos = b2Body_GetPosition(enemy.bodyId);
            glm::vec2 ePos3(enemyPos.x, enemyPos.y);

            gs.particleSystem.emitBurst(ePos3, 30, glm::vec3(1, 0.5f, 0.1f), 5.0f, 0.5f, 0.2f);
            gs.particleSystem.emitRing(ePos3, 20, glm::vec3(1, 0.3f, 0.0),
                enemy.explosionRange, 0.3f, 0.15f);

            float distToPlayer = glm::distance(pPos2, ePos3);
            if (distToPlayer < enemy.explosionRange && !gs.playerHealth.isInvincible() && !gs.isDead && !gs.isFlying) {
                if (blockPlayerDamageWithShield(gs, pPos2, ePos3, enemy.explosionRange)) {
                    continue;
                }

                DamageInfo info;
                info.amount = enemy.explosionDamage;
                info.sourcePosition = ePos3;
                info.sourceBody = enemy.bodyId;
                info.victimBody = gs.playerBodyId;
                info.type = DamageType::Explosion;

                gs.playerHealth.takeDamage(info);
                gs.playerHealth.setInvincible(0.5f);

                glm::vec2 kbDir = Math::normalize(pPos2 - ePos3);
                b2Vec2 kbForce = { kbDir.x * 15.0f, kbDir.y * 15.0f };
                b2Body_ApplyLinearImpulseToCenter(gs.playerBodyId, kbForce, true);

                if (!gs.playerHealth.isAlive()) {
                    gs.isDead = true;
                    gs.deathTimer = 3.0f;
                }
            }
        }
    }
}

}  // namespace CombatCollisionService
