#include "Game/Services/RegionUpdateService.h"

#include "Game/GameState.h"
#include "Game/Services/SessionService.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <string>

namespace RegionUpdateService {

void update(GameState& gs,
            float dt,
            const glm::vec2& playerPos,
            MapRegion* currentRegion,
            State& state) {
    // Stage 6: Update time system (replaces manual gameTime update)
    gs.timeSystem.update(dt);
    // Keep gameTime in sync for backward compatibility with Stage 4 code
    gs.gameTime = gs.timeSystem.getHour();

    // Stage 6: Update weather system
    gs.weatherSystem.update(dt, gs.camera);

    // Stage 6: Check for region transitions
    if (!gs.regionManager.isTransitioning() && currentRegion) {
        MapConnection* conn = currentRegion->getConnectionAt(
            glm::vec2(playerPos.x, playerPos.y), 1.5f);
        if (conn) {
            // Ask the manager to transition. It returns:
            //   true  -> swap happened immediately (cached or no-fade path).
            //           RegionManager has already teleported the player.
            //   false -> fade-deferred path; the manager will teleport the
            //           player when completeTransition() runs.
            bool done = gs.regionManager.transitionTo(*conn, gs.worldId);

            if (done) {
                // Update minimap for the new region (only when swap is
                // actually complete; fade path does this later).
                MapRegion* newRegion = gs.regionManager.getCurrentRegion();
                if (newRegion) {
                    glm::ivec2 targetTile = conn->targetTile;
                    glm::vec2 targetWorld = newRegion->getTileMap().tileToWorld(
                        targetTile.x, targetTile.y);
                    gs.miniMap.setMapDimensions(newRegion->getWidth(), newRegion->getHeight(),
                                               newRegion->getTileMap().tileSize);
                    gs.miniMap.forceUpdate(targetWorld);
                }
            }
        }
    }

    // Stage 6: Update region manager (transition animations)
    gs.regionManager.update(dt);

    // Detect region change (including the fade-deferred completion) and refresh
    // the minimap for the new region.
    MapRegion* cur = gs.regionManager.getCurrentRegion();
    std::string curId = cur ? cur->getId() : std::string{};
    if (cur && curId != state.lastRegionId) {
        b2Vec2 pp = b2Body_GetPosition(gs.playerBodyId);
        glm::vec2 playerPosNow(pp.x, pp.y);
        gs.miniMap.setMapDimensions(cur->getWidth(), cur->getHeight(),
                                   cur->getTileMap().tileSize);
        gs.miniMap.forceUpdate(playerPosNow);
        state.lastRegionId = curId;
        SessionService::refreshRegionGameplayContext(gs);
    }
}

}  // namespace RegionUpdateService
