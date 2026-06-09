#pragma once

#include <glm/vec2.hpp>

struct GameState;

namespace PlayerInputQuery {

glm::vec2 getPlayerPosition(const GameState& gs);

glm::vec2 getMouseWorldPoint(const GameState& gs);

glm::vec2 getAimDirection(const GameState& gs, const glm::vec2& origin);

}  // namespace PlayerInputQuery
