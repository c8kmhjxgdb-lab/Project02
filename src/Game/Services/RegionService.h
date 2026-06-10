#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <string>

class BondTechniqueSystem;
class BuildingSystem;
class DropManager;
class EnemyManager;
class Lightning;
class ParticleSystem;
class ProjectileManager;
class RegionManager;
class Shield;
class WeatherSystem;
struct GameState;
namespace Engine { namespace Audio { class AudioSystem; } }

namespace RegionService {

struct GameplayContext {
    RegionManager& regionManager;
    WeatherSystem& weatherSystem;
    BuildingSystem& buildingSystem;
    ProjectileManager& projectileManager;
    EnemyManager& enemyManager;
    DropManager& dropManager;
    ParticleSystem& particleSystem;
    Shield& shield;
    Lightning& lightning;
    BondTechniqueSystem& bondTechnique;
    Engine::Audio::AudioSystem* audioSystem = nullptr;
};

struct DoorContext {
    GameplayContext gameplay;
    b2WorldId worldId;
};

GameplayContext makeGameplayContext(GameState& gs);
DoorContext makeDoorContext(GameState& gs);

void refreshWeatherContext(RegionManager& regionManager, WeatherSystem& weatherSystem);
void clearTransientCombat(GameplayContext& context);
void refreshGameplayContext(GameplayContext& context);

bool tryUseHomeBaseDoor(DoorContext& context, const glm::vec2& playerPos);

}  // namespace RegionService
