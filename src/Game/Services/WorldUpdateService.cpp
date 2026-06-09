#include "Game/Services/WorldUpdateService.h"

#include "Game/GameState.h"
#include "Game/Services/AbilityUpdateService.h"
#include "Game/Services/EnemySpawnService.h"
#include "Game/Services/PlayerLifecycleService.h"
#include "Game/Services/PlayerMotionService.h"
#include "Game/Services/ProgressionUpdateService.h"
#include "Game/Services/WorldCombatUpdateService.h"
#include "Game/Services/WorldNpcUiUpdateService.h"

namespace WorldUpdateService {

void update(GameState& gs, float dt, State& state) {
    gs.totalPlayTimeSeconds += dt;

    // Skip update if dead (but still render)
    if (!gs.isDead) {
        PlayerLifecycleService::updateAlive(gs, dt);

        AbilityUpdateService::update(gs, dt);

        PlayerMotionService::Result motion = PlayerMotionService::update(gs, dt);

        WorldCombatUpdateService::updateAlive(gs, dt, motion.playerPos);

        RegionUpdateService::update(gs, dt, motion.playerPos, motion.currentRegion, state.regionState);

        ProgressionUpdateService::update(gs, dt, motion.playerPos, state.progressionState);

        WorldNpcUiUpdateService::update(gs, dt);

        EnemySpawnService::update(gs, dt);
    } else {
        PlayerLifecycleService::updateDeathRespawn(gs, dt);
    }

    // Systems that should keep running while the player is dead so the
    // world doesn't freeze (in-flight particles, vent/dialogue animations,
    // minimap camera follow). The !isDead branch above already updated
    // these for the alive case, so we run them only when dead to avoid
    // double-ticking.
    if (gs.isDead) {
        PlayerLifecycleService::updateWhileDead(gs, dt);
        WorldCombatUpdateService::updateWhileDead(gs, dt);
        WorldNpcUiUpdateService::update(gs, dt);
    }
}

}  // namespace WorldUpdateService
