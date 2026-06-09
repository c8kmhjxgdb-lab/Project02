#pragma once

#include <glm/vec2.hpp>

class MapRegion;
struct GameState;

namespace PlayerMotionService {

struct Result {
    glm::vec2 playerPos;
    MapRegion* currentRegion = nullptr;
};

Result update(GameState& gs, float dt);

}  // namespace PlayerMotionService
