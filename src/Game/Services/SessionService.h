#pragma once

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

namespace SessionService {

struct RegionGameplayContext {
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
};

RegionGameplayContext makeRegionGameplayContext(GameState& gs);

void showNotice(GameState& gs, const std::string& notice);

void refreshWeatherRegionContext(RegionManager& regionManager,
                                 WeatherSystem& weatherSystem);
void refreshWeatherRegionContext(GameState& gs);

void clearTransientCombat(GameState& gs);

void refreshRegionGameplayContext(RegionGameplayContext& context);
void refreshRegionGameplayContext(GameState& gs);

bool tryUseHomeBaseDoor(GameState& gs, const glm::vec2& playerPos);

}  // namespace SessionService
