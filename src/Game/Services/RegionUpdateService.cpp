#include "Game/Services/RegionUpdateService.h"

#include "Game/GameState.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <string>

namespace RegionUpdateService {

Context makeContext(GameState& gs) {
    return {
        gs.timeSystem,
        gs.weatherSystem,
        gs.camera,
        gs.regionManager,
        gs.miniMap,
        gs.gameTime,
        gs.worldId,
        gs.playerBodyId,
        RegionService::makeGameplayContext(gs)
    };
}

void update(Context& context,
            float dt,
            const glm::vec2& playerPos,
            MapRegion* currentRegion,
            State& state) {
    // Stage 6: Update time system (replaces manual gameTime update)
    context.timeSystem.update(dt);
    // Keep gameTime in sync for backward compatibility with Stage 4 code
    context.gameTime = context.timeSystem.getHour();

    // Stage 6: Update weather system
    context.weatherSystem.update(dt, context.camera);

    // Stage 6: Check for region transitions
    if (!context.regionManager.isTransitioning() && currentRegion) {
        MapConnection* conn = currentRegion->getConnectionAt(
            glm::vec2(playerPos.x, playerPos.y), 1.5f);
        if (conn) {
            // Ask the manager to transition. It returns:
            //   true  -> swap happened immediately (cached or no-fade path).
            //           RegionManager has already teleported the player.
            //   false -> fade-deferred path; the manager will teleport the
            //           player when completeTransition() runs.
            bool done = context.regionManager.transitionTo(*conn, context.worldId);

            if (done) {
                // Update minimap for the new region (only when swap is
                // actually complete; fade path does this later).
                MapRegion* newRegion = context.regionManager.getCurrentRegion();
                if (newRegion) {
                    glm::ivec2 targetTile = conn->targetTile;
                    glm::vec2 targetWorld = newRegion->getTileMap().tileToWorld(
                        targetTile.x, targetTile.y);
                    context.miniMap.setMapDimensions(newRegion->getWidth(),
                                                     newRegion->getHeight(),
                                                     newRegion->getTileMap().tileSize);
                    context.miniMap.forceUpdate(targetWorld);
                }
            }
        }
    }

    // Stage 6: Update region manager (transition animations)
    context.regionManager.update(dt);

    // Detect region change (including the fade-deferred completion) and refresh
    // the minimap for the new region.
    MapRegion* cur = context.regionManager.getCurrentRegion();
    std::string curId = cur ? cur->getId() : std::string{};
    if (cur && curId != state.lastRegionId) {
        b2Vec2 pp = b2Body_GetPosition(context.playerBodyId);
        glm::vec2 playerPosNow(pp.x, pp.y);
        context.miniMap.setMapDimensions(cur->getWidth(), cur->getHeight(),
                                         cur->getTileMap().tileSize);
        context.miniMap.forceUpdate(playerPosNow);
        state.lastRegionId = curId;
        RegionService::refreshGameplayContext(context.regionGameplay);
    }
}

}  // namespace RegionUpdateService
