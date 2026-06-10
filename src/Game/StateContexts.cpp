#include "Game/StateContexts.h"

#include "Game/GameState.h"

WorldState makeWorldState(GameState& gs) {
    return {
        gs.regionManager,
        gs.tileMap,
        gs.tileManager,
        gs.timeSystem,
        gs.weatherSystem,
        gs.gameTime
    };
}

PlayerState makePlayerState(GameState& gs) {
    return {
        gs.input,
        gs.camera,
        gs.spawnPoint,
        gs.facingDir,
        gs.playerMana,
        gs.playerMaxMana,
        gs.isDead,
        gs.isFlying
    };
}

CombatState makeCombatState(GameState& gs) {
    return {
        gs.enemyManager,
        gs.projectileManager,
        gs.dropManager,
        gs.particleSystem,
        gs.score,
        gs.enemiesKilled
    };
}

UIState makeUIState(GameState& gs) {
    return {
        gs.ui.menuSelection,
        gs.dialogueTree,
        gs.dialogueUI
    };
}

RuntimeServices makeRuntimeServices(GameState& gs) {
    return {
        gs.physicsWorld,
        gs.luaVM
    };
}
