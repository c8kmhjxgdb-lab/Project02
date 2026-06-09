#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

struct Camera2D;
class HealthComponent;
struct GameState;

namespace PlayerLifecycleService {

struct Context {
    float& flightCooldown;
    float& shieldCooldown;
    float& stage7NoticeTimer;
    float& playerMana;
    float& playerMaxMana;
    float& manaRegen;
    float& deathTimer;
    bool& isDead;
    b2BodyId playerBodyId;
    glm::vec2& spawnPoint;
    Camera2D& camera;
    HealthComponent& playerHealth;
};

Context makeContext(GameState& gs);

void updateAlive(Context& context, float dt);
void updateDeathRespawn(Context& context, float dt);
void updateWhileDead(Context& context, float dt);

}  // namespace PlayerLifecycleService
