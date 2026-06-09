#include "Game/Presentation/MiniMapPresenter.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

#include <vector>

namespace MiniMapPresenter {

void update(GameState& gs, float dt) {
    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
    glm::vec2 playerPos(pPos.x, pPos.y);

    std::vector<MiniMap::EntityMarker> markers;
    markers.push_back({playerPos, MiniMap::EntityMarker::Type::Player});

    if (gs.princess && gs.princess->hasBody()) {
        b2Vec2 princessPos = b2Body_GetPosition(gs.princess->getBodyId());
        markers.push_back({
            glm::vec2(princessPos.x, princessPos.y),
            MiniMap::EntityMarker::Type::Princess
        });
    }

    auto aliveEnemies = gs.enemyManager.getAlive();
    for (const Enemy* enemy : aliveEnemies) {
        if (enemy && b2Body_IsValid(enemy->bodyId)) {
            b2Vec2 ePos = b2Body_GetPosition(enemy->bodyId);
            glm::vec2 enemyPos(ePos.x, ePos.y);
            if (glm::distance(playerPos, enemyPos) < 50.0f) {
                markers.push_back({enemyPos, MiniMap::EntityMarker::Type::Enemy});
            }
        }
    }

    gs.miniMap.setEntities(markers);
    gs.miniMap.update(dt, playerPos);
}

}  // namespace MiniMapPresenter
