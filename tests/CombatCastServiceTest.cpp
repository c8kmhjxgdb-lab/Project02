#include "Engine/Renderer/ParticleSystem.h"
#include "Game/AI/Enemy.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/Projectile.h"
#include "Game/Services/CombatService.h"
#include "TestSupport.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>

namespace {

struct Fixture {
    b2WorldId worldId = b2_nullWorldId;
    b2BodyId playerBodyId = b2_nullBodyId;
    bool isDead = false;
    glm::vec2 facingDir = glm::vec2(1.0f, 0.0f);
    ProjectileManager projectileManager;
    ParticleSystem particleSystem{128};
    float fireballCooldown = 0.0f;
    float fireballCooldownMax = 0.3f;
    Lightning lightning;
    float playerMana = 100.0f;
    EnemyManager enemyManager;

    Fixture() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, 0.0f};
        worldId = b2CreateWorld(&worldDef);

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = b2Vec2{0.0f, 0.0f};
        playerBodyId = b2CreateBody(worldId, &bodyDef);

        projectileManager.init();
        particleSystem.init();
        enemyManager.init();
    }

    ~Fixture() {
        projectileManager.clear();
        enemyManager.clear();
        if (b2World_IsValid(worldId)) {
            b2DestroyWorld(worldId);
        }
    }

    CombatService::CastContext makeCastContext() {
        return {
            isDead,
            worldId,
            playerBodyId,
            facingDir,
            projectileManager,
            particleSystem,
            fireballCooldown,
            fireballCooldownMax,
            lightning,
            playerMana,
            enemyManager,
            nullptr
        };
    }
};

void castsProjectileAndStartsCooldown() {
    Fixture fixture;
    CombatService::CastContext context = fixture.makeCastContext();

    bool cast = CombatService::tryCastProjectile(
        context,
        ProjectileType::Fireball,
        glm::vec2(2.0f, 3.0f),
        glm::vec2(0.0f, 1.0f));

    TestSupport::require(cast, "fireball cast succeeds");
    TestSupport::require(fixture.projectileManager.getActive().size() == 1, "fireball creates one projectile");
    TestSupport::require(fixture.fireballCooldown == fixture.fireballCooldownMax, "fireball starts cooldown");
    TestSupport::require(fixture.facingDir == glm::vec2(0.0f, 1.0f), "fireball updates facing direction");
    TestSupport::require(fixture.particleSystem.getActiveCount() > 0, "fireball emits particles");
}

void cooldownBlocksProjectileCast() {
    Fixture fixture;
    fixture.fireballCooldown = 0.1f;
    CombatService::CastContext context = fixture.makeCastContext();

    bool cast = CombatService::tryCastProjectile(
        context,
        ProjectileType::Fireball,
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f));

    TestSupport::require(!cast, "cooldown blocks fireball cast");
    TestSupport::require(fixture.projectileManager.getActive().empty(), "cooldown creates no projectile");
    TestSupport::require(fixture.particleSystem.getActiveCount() == 0, "cooldown emits no particles");
}

void deathBlocksProjectileCast() {
    Fixture fixture;
    fixture.isDead = true;
    CombatService::CastContext context = fixture.makeCastContext();

    bool cast = CombatService::tryCastProjectile(
        context,
        ProjectileType::IceSpike,
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f));

    TestSupport::require(!cast, "death blocks projectile cast");
    TestSupport::require(fixture.projectileManager.getActive().empty(), "death creates no projectile");
}

void zeroAimFallsBackToRightDirection() {
    Fixture fixture;
    CombatService::CastContext context = fixture.makeCastContext();

    bool cast = CombatService::tryCastProjectile(
        context,
        ProjectileType::IceSpike,
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f));

    TestSupport::require(cast, "zero aim cast succeeds");
    const auto& projectiles = fixture.projectileManager.getActive();
    TestSupport::require(projectiles.size() == 1, "zero aim creates one projectile");
    TestSupport::require(
        glm::length(projectiles.front().velocity - glm::vec2(14.0f, 0.0f)) < 0.001f,
        "zero aim falls back to right direction");
}

}  // namespace

int main() {
    castsProjectileAndStartsCooldown();
    cooldownBlocksProjectileCast();
    deathBlocksProjectileCast();
    zeroAimFallsBackToRightDirection();
    return 0;
}
