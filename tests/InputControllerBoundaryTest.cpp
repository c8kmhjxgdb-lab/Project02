#include "Engine/Renderer/ParticleSystem.h"
#include "Game/AI/Enemy.h"
#include "Game/Ability/BondTechnique.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/Projectile.h"
#include "Game/Ability/Shield.h"
#include "Game/Ability/SuperStrength.h"
#include "Game/Building/BuildingSystem.h"
#include "Game/Controllers/AbilityInputController.h"
#include "Game/Controllers/BuildingInputController.h"
#include "Game/Inventory/Inventory.h"
#include "Game/Social/Princess.h"
#include "Game/Toy/ToySystem.h"
#include "Game/World/RegionManager.h"
#include "Game/World/TimeSystem.h"
#include "TestSupport.h"

#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

#include <memory>
#include <string>

namespace {

struct Fixture {
    b2WorldId worldId = b2_nullWorldId;
    b2BodyId playerBodyId = b2_nullBodyId;
    bool isDead = false;
    glm::vec2 facingDir = glm::vec2(1.0f, 0.0f);
    float playerMana = 100.0f;
    bool isFlying = false;
    float flightHeight = 0.0f;
    float flightHeightTarget = 0.0f;
    float flightMaxHeight = 5.0f;
    float flightCooldown = 0.0f;
    float shieldCooldown = 0.0f;
    float shieldCooldownMax = 5.0f;
    float fireballCooldown = 0.0f;
    float fireballCooldownMax = 0.3f;

    BuildingSystem buildingSystem;
    ToySystem toySystem;
    SuperStrength superStrength;
    Shield shield;
    BondTechniqueSystem bondTechnique;
    ParticleSystem particleSystem{128};
    std::unique_ptr<Princess> princess;
    ProjectileManager projectileManager;
    Lightning lightning;
    EnemyManager enemyManager;
    RegionManager regionManager;
    TimeSystem timeSystem;
    Inventory inventory;
    bool gameMenuOpen = false;

    Fixture() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, 0.0f};
        worldId = b2CreateWorld(&worldDef);

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = b2Vec2{0.0f, 0.0f};
        playerBodyId = b2CreateBody(worldId, &bodyDef);

        buildingSystem.init(worldId);
        toySystem.init();
        particleSystem.init();
        projectileManager.init();
        enemyManager.init();
        timeSystem.init(9.0f);

        regionManager.setWorldId(worldId);
        regionManager.setPlayerBody(playerBodyId);
        regionManager.setTransitionEffectEnabled(false);
        regionManager.init();
        if (MapRegion* region = regionManager.getCurrentRegion()) {
            region->buildPhysics(worldId);
        }
    }

    ~Fixture() {
        princess.reset();
        regionManager.shutdown();
        buildingSystem.shutdown();
        projectileManager.clear();
        enemyManager.clear();
        particleSystem.clear();
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

    AbilityInputController::Context makeAbilityContext() {
        return {
            isDead,
            buildingSystem,
            toySystem,
            superStrength,
            shield,
            bondTechnique,
            particleSystem,
            princess,
            worldId,
            playerBodyId,
            facingDir,
            playerMana,
            isFlying,
            flightHeight,
            flightHeightTarget,
            flightMaxHeight,
            flightCooldown,
            shieldCooldown,
            shieldCooldownMax,
            gameMenuOpen,
            makeCastContext()
        };
    }

    AbilityInputController::Callbacks makeAbilityCallbacks() {
        return {
            [](const AbilityInputController::Context& context) {
                b2Vec2 playerPos = b2Body_GetPosition(context.playerBodyId);
                return glm::vec2(playerPos.x, playerPos.y) + glm::vec2(0.0f, 1.0f);
            }
        };
    }

    BuildingInputController::Context makeBuildingContext() {
        return {
            isDead,
            buildingSystem,
            regionManager,
            toySystem,
            timeSystem,
            inventory
        };
    }

    void resetAbilityState() {
        isDead = false;
        buildingSystem.setBuildMode(false);
        toySystem.stopMiniCar();
        projectileManager.clear();
        particleSystem.clear();
        fireballCooldown = 0.0f;
        facingDir = glm::vec2(1.0f, 0.0f);
    }

    void resetFlightState() {
        resetAbilityState();
        playerMana = 100.0f;
        isFlying = false;
        flightHeight = 0.0f;
        flightHeightTarget = 0.0f;
        flightCooldown = 0.0f;
    }

    void enterHomeBase() {
        regionManager.transitionTo("home_base", glm::ivec2(9, 11), worldId);
    }
};

void handleAbilityKey(Fixture& fixture,
                      SDL_Scancode scancode,
                      const AbilityInputController::Callbacks& callbacks) {
    AbilityInputController::Context context = fixture.makeAbilityContext();
    AbilityInputController::handleKeyDown(context, scancode, callbacks);
}

void handleAbilityMouse(Fixture& fixture,
                        Uint8 button,
                        const AbilityInputController::Callbacks& callbacks) {
    AbilityInputController::Context context = fixture.makeAbilityContext();
    AbilityInputController::handleMouseButtonDown(context, button, callbacks);
}

void handleBuildingToggle(Fixture& fixture, SDL_Scancode scancode) {
    BuildingInputController::Context context = fixture.makeBuildingContext();
    BuildingInputController::handleToggleKey(context, scancode);
}

bool handleBuildingWheel(Fixture& fixture, int wheelY) {
    BuildingInputController::Context context = fixture.makeBuildingContext();
    return BuildingInputController::handleMouseWheel(context, wheelY);
}

void abilityKeyAndMouseBlockProjectileAtActionBoundaries() {
    Fixture fixture;
    AbilityInputController::Callbacks callbacks = fixture.makeAbilityCallbacks();

    fixture.isDead = true;
    handleAbilityKey(fixture, SDL_SCANCODE_J, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().empty(),
        "J does not cast while dead");

    fixture.resetAbilityState();
    fixture.buildingSystem.setBuildMode(true);
    handleAbilityKey(fixture, SDL_SCANCODE_J, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().empty(),
        "J does not cast while build mode is active");

    fixture.resetAbilityState();
    fixture.toySystem.startMiniCar(fixture.timeSystem.getDay());
    handleAbilityKey(fixture, SDL_SCANCODE_J, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().empty(),
        "J does not cast while mini car is active");

    fixture.resetAbilityState();
    fixture.isDead = true;
    handleAbilityMouse(fixture, SDL_BUTTON_LEFT, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().empty(),
        "left mouse does not cast while dead");

    fixture.resetAbilityState();
    fixture.buildingSystem.setBuildMode(true);
    handleAbilityMouse(fixture, SDL_BUTTON_LEFT, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().empty(),
        "left mouse does not cast while build mode is active");

    fixture.resetAbilityState();
    fixture.toySystem.startMiniCar(fixture.timeSystem.getDay());
    handleAbilityMouse(fixture, SDL_BUTTON_LEFT, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().empty(),
        "left mouse does not cast while mini car is active");
}

void abilityKeyAndMouseBlockProjectileWhileGameMenuOpen() {
    Fixture fixture;
    AbilityInputController::Callbacks callbacks = fixture.makeAbilityCallbacks();
    fixture.gameMenuOpen = true;

    handleAbilityKey(fixture, SDL_SCANCODE_J, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().empty(),
        "J does not cast while game menu is open");

    handleAbilityMouse(fixture, SDL_BUTTON_LEFT, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().empty(),
        "left mouse does not cast while game menu is open");
}

void abilityKeyAndMouseCastProjectileDuringGameplay() {
    Fixture fixture;
    AbilityInputController::Callbacks callbacks = fixture.makeAbilityCallbacks();

    handleAbilityKey(fixture, SDL_SCANCODE_J, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().size() == 1,
        "J creates one projectile during gameplay");

    fixture.resetAbilityState();
    handleAbilityMouse(fixture, SDL_BUTTON_LEFT, callbacks);
    TestSupport::require(
        fixture.projectileManager.getActive().size() == 1,
        "left mouse creates one projectile during gameplay");
}

void abilitySpaceStartsFlightOnlyWhenAllowed() {
    Fixture fixture;
    AbilityInputController::Callbacks callbacks = fixture.makeAbilityCallbacks();

    handleAbilityKey(fixture, SDL_SCANCODE_SPACE, callbacks);
    TestSupport::require(fixture.isFlying, "SPACE starts flight when allowed");
    TestSupport::require(
        fixture.flightHeightTarget == fixture.flightMaxHeight,
        "SPACE targets max flight height when allowed");

    fixture.resetFlightState();
    fixture.isDead = true;
    handleAbilityKey(fixture, SDL_SCANCODE_SPACE, callbacks);
    TestSupport::require(!fixture.isFlying, "SPACE does not start flight while dead");
    TestSupport::require(
        fixture.flightHeightTarget == 0.0f,
        "SPACE leaves flight target unchanged while dead");

    fixture.resetFlightState();
    fixture.playerMana = 4.0f;
    handleAbilityKey(fixture, SDL_SCANCODE_SPACE, callbacks);
    TestSupport::require(!fixture.isFlying, "SPACE does not start flight without mana");
    TestSupport::require(
        fixture.flightHeightTarget == 0.0f,
        "SPACE leaves flight target unchanged without mana");
}

void buildingToggleRequiresHomeBaseAndNoMiniCar() {
    Fixture fixture;

    handleBuildingToggle(fixture, SDL_SCANCODE_TAB);
    TestSupport::require(
        !fixture.buildingSystem.isActive(),
        "TAB does not enable build mode outside home base");

    handleBuildingToggle(fixture, SDL_SCANCODE_B);
    TestSupport::require(
        !fixture.buildingSystem.isActive(),
        "B does not enable build mode outside home base");

    fixture.enterHomeBase();
    handleBuildingToggle(fixture, SDL_SCANCODE_TAB);
    TestSupport::require(
        fixture.buildingSystem.isActive(),
        "TAB enables build mode in home base");

    handleBuildingToggle(fixture, SDL_SCANCODE_TAB);
    TestSupport::require(
        !fixture.buildingSystem.isActive(),
        "TAB disables build mode in home base");

    handleBuildingToggle(fixture, SDL_SCANCODE_B);
    TestSupport::require(
        fixture.buildingSystem.isActive(),
        "B enables build mode in home base");

    handleBuildingToggle(fixture, SDL_SCANCODE_B);
    TestSupport::require(
        !fixture.buildingSystem.isActive(),
        "B disables build mode in home base");

    fixture.toySystem.startMiniCar(fixture.timeSystem.getDay());
    handleBuildingToggle(fixture, SDL_SCANCODE_TAB);
    TestSupport::require(
        !fixture.buildingSystem.isActive(),
        "TAB does not enable build mode while mini car is active");

    handleBuildingToggle(fixture, SDL_SCANCODE_B);
    TestSupport::require(
        !fixture.buildingSystem.isActive(),
        "B does not enable build mode while mini car is active");
}

void buildingMouseWheelConsumesOnlyInBuildMode() {
    Fixture fixture;

    const FurnitureDef* selected = fixture.buildingSystem.getSelectedDef();
    TestSupport::require(selected != nullptr, "building fixture has a selected furniture def");
    std::string initialId = selected->id;

    bool consumed = handleBuildingWheel(fixture, 1);
    TestSupport::require(!consumed, "mouse wheel is not consumed outside build mode");
    selected = fixture.buildingSystem.getSelectedDef();
    TestSupport::require(
        selected != nullptr && selected->id == initialId,
        "mouse wheel does not change selection outside build mode");

    fixture.buildingSystem.setBuildMode(true);
    consumed = handleBuildingWheel(fixture, 1);
    TestSupport::require(consumed, "mouse wheel is consumed in build mode");
    selected = fixture.buildingSystem.getSelectedDef();
    TestSupport::require(
        selected != nullptr && selected->id != initialId,
        "mouse wheel changes selection in build mode");
}

}  // namespace

int main() {
    abilityKeyAndMouseBlockProjectileAtActionBoundaries();
    abilityKeyAndMouseBlockProjectileWhileGameMenuOpen();
    abilityKeyAndMouseCastProjectileDuringGameplay();
    abilitySpaceStartsFlightOnlyWhenAllowed();
    buildingToggleRequiresHomeBaseAndNoMiniCar();
    buildingMouseWheelConsumesOnlyInBuildMode();
    return 0;
}
