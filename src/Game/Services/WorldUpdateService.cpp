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
        PlayerLifecycleService::Context playerLifecycleContext = PlayerLifecycleService::makeContext(gs);
        PlayerLifecycleService::updateAlive(playerLifecycleContext, dt);

        AbilityUpdateService::Context abilityContext = AbilityUpdateService::makeContext(gs);
        AbilityUpdateService::update(abilityContext, dt);

        PlayerMotionService::Context motionContext = PlayerMotionService::makeContext(gs);
        PlayerMotionService::Result motion = PlayerMotionService::update(motionContext, dt);

        WorldCombatUpdateService::Context combatContext = WorldCombatUpdateService::makeContext(gs);
        WorldCombatUpdateService::updateAlive(combatContext, dt, motion.playerPos);

        RegionUpdateService::Context regionContext = RegionUpdateService::makeContext(gs);
        RegionUpdateService::update(regionContext,
                                    dt,
                                    motion.playerPos,
                                    motion.currentRegion,
                                    state.regionState);

        ProgressionUpdateService::Context progressionContext = ProgressionUpdateService::makeContext(gs);
        ProgressionUpdateService::update(progressionContext, dt, motion.playerPos, state.progressionState);

        WorldNpcUiUpdateService::Context npcUiContext = WorldNpcUiUpdateService::makeContext(gs);
        WorldNpcUiUpdateService::update(npcUiContext, dt);

        EnemySpawnService::Context enemySpawnContext = EnemySpawnService::makeContext(gs);
        EnemySpawnService::update(enemySpawnContext, dt);
    } else {
        PlayerLifecycleService::Context playerLifecycleContext = PlayerLifecycleService::makeContext(gs);
        PlayerLifecycleService::updateDeathRespawn(playerLifecycleContext, dt);
    }

    // Systems that should keep running while the player is dead so the
    // world doesn't freeze (in-flight particles, vent/dialogue animations,
    // minimap camera follow). The !isDead branch above already updated
    // these for the alive case, so we run them only when dead to avoid
    // double-ticking.
    if (gs.isDead) {
        PlayerLifecycleService::Context playerLifecycleContext = PlayerLifecycleService::makeContext(gs);
        PlayerLifecycleService::updateWhileDead(playerLifecycleContext, dt);
        WorldCombatUpdateService::Context combatContext = WorldCombatUpdateService::makeContext(gs);
        WorldCombatUpdateService::updateWhileDead(combatContext, dt);
        WorldNpcUiUpdateService::Context npcUiContext = WorldNpcUiUpdateService::makeContext(gs);
        WorldNpcUiUpdateService::update(npcUiContext, dt);
    }
}

}  // namespace WorldUpdateService
