#include "Game/Services/PlayerInputQuery.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>

#include <algorithm>
#include <cmath>

namespace PlayerInputQuery {

glm::vec2 getPlayerPosition(const GameState& gs) {
    b2Vec2 pos = b2Body_GetPosition(gs.playerBodyId);
    return glm::vec2(pos.x, pos.y);
}

glm::vec2 getMouseWorldPoint(const GameState& gs) {
    float screenW = static_cast<float>(std::max(gs.screenWidth, 1));
    float screenH = static_cast<float>(std::max(gs.screenHeight, 1));
    return gs.camera.screenToWorld(gs.input.mousePos.x, gs.input.mousePos.y, screenW, screenH);
}

glm::vec2 getAimDirection(const GameState& gs, const glm::vec2& origin) {
    glm::vec2 target = getMouseWorldPoint(gs);
    glm::vec2 dir = target - origin;
    float lenSq = dir.x * dir.x + dir.y * dir.y;
    if (lenSq <= 0.0001f) {
        dir = gs.facingDir;
        lenSq = dir.x * dir.x + dir.y * dir.y;
    }
    if (lenSq <= 0.0001f) {
        return glm::vec2(1.0f, 0.0f);
    }
    return dir / std::sqrt(lenSq);
}

}  // namespace PlayerInputQuery
