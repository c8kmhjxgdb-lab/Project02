#include "Game/Services/WorldNpcUiUpdateService.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

namespace WorldNpcUiUpdateService {

void update(GameState& gs, float dt) {
    if (gs.isVenting) {
        gs.ventAnimation.update(dt);
        if (!gs.ventAnimation.isActive()) {
            gs.isVenting = false;
        }
    }

    if (gs.dialogueUI.isVisible()) {
        gs.dialogueUI.update(dt);
    }

    if (gs.princess) {
        b2Vec2 playerPos = b2Body_GetPosition(gs.playerBodyId);
        gs.princess->setLastPlayerPos(glm::vec2(playerPos.x, playerPos.y));
        gs.princess->update(dt, gs.gameTime);
    }
}

}  // namespace WorldNpcUiUpdateService
