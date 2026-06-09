#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

struct Camera2D;
class EmotionSystem;
class HealthComponent;
struct InputState;
class MapRegion;
class ParticleSystem;
class PhysicsWorld;
class ProjectileManager;
class RegionManager;
struct TileMap;
class WeatherSystem;
struct GameState;

namespace PlayerMotionService {

struct Context {
    InputState& input;
    Camera2D& camera;
    RegionManager& regionManager;
    TileMap& tileMap;
    WeatherSystem& weatherSystem;
    EmotionSystem& emotionSystem;
    ProjectileManager& projectileManager;
    ParticleSystem& particleSystem;
    PhysicsWorld& physicsWorld;
    HealthComponent& playerHealth;
    b2BodyId playerBodyId;
    float& playerForce;
    glm::vec2& facingDir;
    float& playerMana;
    bool& isDead;
    bool& isFlying;
    float& flightHeight;
    float& flightHeightTarget;
    float& flightMaxHeight;
    float& flightSpeed;
    float& flightDescentSpeedMult;
    float& flightManaDrain;
    float& flightCooldown;
    float& flightCooldownMax;
    float& shadowScale;
    float& charTime;
    float& armAngle;
    int screenWidth;
    int screenHeight;
};

struct Result {
    glm::vec2 playerPos;
    MapRegion* currentRegion = nullptr;
};

Context makeContext(GameState& gs);

Result update(Context& context, float dt);

}  // namespace PlayerMotionService
