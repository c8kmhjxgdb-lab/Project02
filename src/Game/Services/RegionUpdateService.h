#pragma once

#include <glm/vec2.hpp>

#include <string>

struct GameState;
class MapRegion;

namespace RegionUpdateService {

struct State {
    std::string lastRegionId;
};

void update(GameState& gs,
            float dt,
            const glm::vec2& playerPos,
            MapRegion* currentRegion,
            State& state);

}  // namespace RegionUpdateService
