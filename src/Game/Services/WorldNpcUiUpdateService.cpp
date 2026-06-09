#include "Game/Services/WorldNpcUiUpdateService.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

namespace WorldNpcUiUpdateService {

Context makeContext(GameState& gs) {
    return {
        gs.isVenting,
        gs.ventAnimation,
        gs.dialogueUI,
        gs.princess.get(),
        gs.playerBodyId,
        gs.gameTime
    };
}

void update(Context& context, float dt) {
    if (context.isVenting) {
        context.ventAnimation.update(dt);
        if (!context.ventAnimation.isActive()) {
            context.isVenting = false;
        }
    }

    if (context.dialogueUI.isVisible()) {
        context.dialogueUI.update(dt);
    }

    if (context.princess) {
        b2Vec2 playerPos = b2Body_GetPosition(context.playerBodyId);
        context.princess->setLastPlayerPos(glm::vec2(playerPos.x, playerPos.y));
        context.princess->update(dt, context.gameTime);
    }
}

}  // namespace WorldNpcUiUpdateService
