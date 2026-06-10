#include "Engine/Renderer/ParticleSystem.h"
#include "Game/AI/Enemy.h"
#include "Game/Ability/Projectile.h"
#include "Game/Ability/Shield.h"
#include "Game/Drop.h"
#include "Game/Emotion/EmotionSystem.h"
#include "Game/Health.h"
#include "Game/Services/CombatCollisionService.h"
#include "TestSupport.h"

#include <box2d/box2d.h>

namespace {

struct Fixture {
    b2WorldId worldId = b2_nullWorldId;
    b2BodyId playerBodyId = b2_nullBodyId;
    ProjectileManager projectileManager;
    EnemyManager enemyManager;
    DropManager dropManager;
    ParticleSystem particleSystem{128};
    Shield shield;
    HealthComponent playerHealth;
    bool isDead = false;
    bool isFlying = false;
    float deathTimer = 0.0f;
    int score = 0;
    int enemiesKilled = 0;
    ChildlikeHeartTier tier = ChildlikeHeartTier::Normal;

    Fixture() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, 0.0f};
        worldId = b2CreateWorld(&worldDef);

        b2BodyDef playerDef = b2DefaultBodyDef();
        playerDef.type = b2_dynamicBody;
        playerDef.position = b2Vec2{0.0f, 0.0f};
        playerBodyId = b2CreateBody(worldId, &playerDef);

        b2Polygon playerShape = b2MakeBox(0.3f, 0.3f);
        b2ShapeDef playerShapeDef = b2DefaultShapeDef();
        playerShapeDef.density = 1.0f;
        playerShapeDef.filter.categoryBits = 0x0001;
        playerShapeDef.filter.maskBits = 0x0002 | 0x0004 | 0x0008;
        b2CreatePolygonShape(playerBodyId, &playerShapeDef, &playerShape);

        projectileManager.init();
        enemyManager.init();
        dropManager.init();
        particleSystem.init();

        enemyManager.setDeathCallback([this](EnemyId, const glm::vec2& pos, int coinMin, int) {
            ++enemiesKilled;
            score += 50;
            dropManager.spawn(worldId, pos, DropType::Coin, coinMin);
        });
    }

    ~Fixture() {
        projectileManager.clear();
        enemyManager.clear();
        dropManager.clear();
        if (b2World_IsValid(worldId)) {
            b2DestroyWorld(worldId);
        }
    }

    CombatCollisionService::Context makeContext() {
        return {
            projectileManager,
            enemyManager,
            particleSystem,
            shield,
            playerHealth,
            playerBodyId,
            isDead,
            isFlying,
            deathTimer,
            tier
        };
    }

    void advanceProjectileIntoEnemy() {
        b2World_Step(worldId, 0.10f, 4);
    }
};

void playerProjectileKillsEnemyAndEmitsDeathSideEffects() {
    Fixture fixture;
    EnemyId enemyId = fixture.enemyManager.spawn(
        fixture.worldId,
        glm::vec2(2.0f, 0.0f),
        EnemyType::Chaser);
    fixture.projectileManager.fire(
        fixture.worldId,
        glm::vec2(0.5f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        ProjectileType::Fireball,
        40.0f,
        18.0f,
        fixture.playerBodyId);
    fixture.advanceProjectileIntoEnemy();

    CombatCollisionService::Context context = fixture.makeContext();
    CombatCollisionService::handleCollisions(context);

    const auto& projectiles = fixture.projectileManager.getActive();
    TestSupport::require(projectiles.size() == 1, "hit projectile record remains until projectile update cleanup");
    TestSupport::require(!projectiles.front().active, "hit projectile is deactivated");
    TestSupport::require(!b2Body_IsValid(projectiles.front().bodyId), "hit projectile body is destroyed");

    const Enemy* enemy = fixture.enemyManager.find(enemyId);
    TestSupport::require(enemy != nullptr, "dead enemy remains inspectable before enemy cleanup");
    TestSupport::require(enemy->state == Enemy::State::Dead, "lethal projectile marks enemy dead");
    TestSupport::require(enemy->health == 0.0f, "lethal projectile reduces enemy health to zero");
    TestSupport::require(fixture.enemiesKilled == 1, "enemy death callback records one kill");
    TestSupport::require(fixture.score == 50, "enemy death callback awards score");
    TestSupport::require(fixture.dropManager.getActive().size() == 1, "enemy death callback spawns a coin drop");
    TestSupport::require(fixture.particleSystem.getActiveCount() > 0, "projectile hit emits particles");
    TestSupport::require(fixture.playerHealth.getCurrentHealth() == fixture.playerHealth.getMaxHealth(),
                         "distant projectile hit does not damage player");
}

void projectileOwnerBodyCannotHitSameEnemy() {
    Fixture fixture;
    EnemyId enemyId = fixture.enemyManager.spawn(
        fixture.worldId,
        glm::vec2(2.0f, 0.0f),
        EnemyType::Chaser);
    const Enemy* enemyBefore = fixture.enemyManager.find(enemyId);
    TestSupport::require(enemyBefore != nullptr, "enemy exists before owner-body collision test");

    fixture.projectileManager.fire(
        fixture.worldId,
        glm::vec2(0.5f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        ProjectileType::Fireball,
        40.0f,
        18.0f,
        enemyBefore->bodyId);
    fixture.advanceProjectileIntoEnemy();

    CombatCollisionService::Context context = fixture.makeContext();
    CombatCollisionService::handleCollisions(context);

    const auto& projectiles = fixture.projectileManager.getActive();
    TestSupport::require(projectiles.size() == 1, "owner-body skipped collision keeps projectile record");
    TestSupport::require(projectiles.front().active, "owner-body skipped collision keeps projectile active");
    TestSupport::require(b2Body_IsValid(projectiles.front().bodyId), "owner-body skipped collision keeps projectile body");

    const Enemy* enemyAfter = fixture.enemyManager.find(enemyId);
    TestSupport::require(enemyAfter != nullptr, "enemy remains after skipped owner-body collision");
    TestSupport::require(enemyAfter->state != Enemy::State::Dead, "owner-body skipped collision does not kill enemy");
    TestSupport::require(enemyAfter->health == enemyAfter->maxHealth, "owner-body skipped collision does not damage enemy");
    TestSupport::require(fixture.enemiesKilled == 0, "owner-body skipped collision does not record kills");
    TestSupport::require(fixture.dropManager.getActive().empty(), "owner-body skipped collision does not spawn drops");
    TestSupport::require(fixture.particleSystem.getActiveCount() == 0, "owner-body skipped collision emits no particles");
}

void deadPlayerIgnoresEnemyContactDamage() {
    Fixture fixture;
    fixture.isDead = true;
    fixture.enemyManager.spawn(
        fixture.worldId,
        glm::vec2(0.4f, 0.0f),
        EnemyType::Chaser);

    CombatCollisionService::Context context = fixture.makeContext();
    CombatCollisionService::handleCollisions(context);

    TestSupport::require(fixture.playerHealth.getCurrentHealth() == fixture.playerHealth.getMaxHealth(),
                         "dead player ignores enemy contact damage");
    TestSupport::require(!fixture.playerHealth.isInvincible(), "dead player contact does not start invincibility");
    TestSupport::require(fixture.deathTimer == 0.0f, "dead player contact does not restart death timer");
    TestSupport::require(fixture.particleSystem.getActiveCount() == 0,
                         "dead player ignored contact emits no particles");
}

void fadedTierUsesWeakerIceSlow() {
    Fixture fixture;
    fixture.tier = ChildlikeHeartTier::Faded;
    EnemyId enemyId = fixture.enemyManager.spawn(
        fixture.worldId,
        glm::vec2(2.0f, 0.0f),
        EnemyType::Chaser);
    fixture.projectileManager.fire(
        fixture.worldId,
        glm::vec2(0.5f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        ProjectileType::IceSpike,
        1.0f,
        18.0f,
        fixture.playerBodyId);
    fixture.advanceProjectileIntoEnemy();

    CombatCollisionService::Context context = fixture.makeContext();
    CombatCollisionService::handleCollisions(context);

    const Enemy* enemy = fixture.enemyManager.find(enemyId);
    TestSupport::require(enemy != nullptr, "ice target remains alive");
    TestSupport::require(enemy->slowTimer > 0.0f, "ice hit applies slow");
    TestSupport::require(enemy->slowMultiplier == 0.75f, "faded tier uses weaker ice slow");
}

}  // namespace

int main() {
    playerProjectileKillsEnemyAndEmitsDeathSideEffects();
    projectileOwnerBodyCannotHitSameEnemy();
    deadPlayerIgnoresEnemyContactDamage();
    fadedTierUsesWeakerIceSlow();
    return 0;
}
