#pragma once

#include "Game/Ability/Projectile.h"
#include "Game/Services/CombatService.h"

#include <box2d/box2d.h>
#include <SDL2/SDL.h>
#include <glm/vec2.hpp>

#include <functional>
#include <memory>

class BondTechniqueSystem;
class BuildingSystem;
class ParticleSystem;
class Princess;
class Shield;
class SuperStrength;
struct GameState;
class ToySystem;

namespace AbilityInputController {

struct Context {
    bool& isDead;
    BuildingSystem& buildingSystem;
    ToySystem& toySystem;
    SuperStrength& superStrength;
    Shield& shield;
    BondTechniqueSystem& bondTechnique;
    ParticleSystem& particleSystem;
    std::unique_ptr<Princess>& princess;
    b2WorldId& worldId;
    b2BodyId& playerBodyId;
    glm::vec2& facingDir;
    float& playerMana;
    bool& isFlying;
    float& flightHeight;
    float& flightHeightTarget;
    float& flightMaxHeight;
    float& flightCooldown;
    float& shieldCooldown;
    float& shieldCooldownMax;
    CombatService::CastContext castContext;
};

struct Callbacks {
    std::function<glm::vec2(const Context&)> getMouseWorldPoint;
};

Context makeContext(GameState& gs);
Callbacks makeCallbacks(GameState& gs);

void handleKeyDown(Context& context, SDL_Scancode scancode, const Callbacks& callbacks);
void handleMouseButtonDown(Context& context, Uint8 button, const Callbacks& callbacks);

}  // namespace AbilityInputController
