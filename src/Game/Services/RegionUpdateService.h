#pragma once

#include "Game/Services/SessionService.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <string>

class MiniMap;
class RegionManager;
class TimeSystem;
class WeatherSystem;
struct Camera2D;
struct GameState;
class MapRegion;

namespace RegionUpdateService {

struct State {
    std::string lastRegionId;
};

struct Context {
    TimeSystem& timeSystem;
    WeatherSystem& weatherSystem;
    Camera2D& camera;
    RegionManager& regionManager;
    MiniMap& miniMap;
    float& gameTime;
    b2WorldId worldId;
    b2BodyId playerBodyId;
    SessionService::RegionGameplayContext regionGameplay;
};

Context makeContext(GameState& gs);

void update(Context& context,
            float dt,
            const glm::vec2& playerPos,
            MapRegion* currentRegion,
            State& state);

}  // namespace RegionUpdateService
