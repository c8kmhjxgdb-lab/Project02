#include "Game/Services/GameSessionService.h"

#include "Game/GameState.h"
#include "Game/Scenes/SceneManager.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/SaveGameService.h"
#include "Game/Services/SessionService.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include <cmath>

namespace GameSessionService {

bool initializeWorld(GameState& gs) {
    // Stage 6: Use RegionManager for multi-region support.
    gs.regionManager.init();
    gs.regionManager.setWorldId(gs.worldId);
    // RegionManager owns the player-teleport step on region changes so it can
    // defer it correctly through the fade transition.
    gs.regionManager.setPlayerBody(gs.playerBodyId);

    MapRegion* currentRegion = gs.regionManager.getCurrentRegion();
    if (currentRegion) {
        currentRegion->buildPhysics(gs.worldId);

        glm::vec2 startWorld = currentRegion->getTileMap().tileToWorld(7, 8);
        b2Body_SetTransform(gs.playerBodyId, b2Vec2{startWorld.x, startWorld.y}, b2Rot{0});
        b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);

        if (!gs.miniMapInitialized) {
            gs.miniMap.init(150);
            gs.miniMapInitialized = true;
        }
        gs.miniMap.setMapDimensions(currentRegion->getWidth(), currentRegion->getHeight(),
                                   currentRegion->getTileMap().tileSize);
        gs.miniMap.setTileGetter([&gs](int x, int y) -> uint8_t {
            MapRegion* region = gs.regionManager.getCurrentRegion();
            if (!region) return 0;
            return static_cast<uint8_t>(region->getTileMap().getTile(x, y));
        });
        gs.miniMap.forceUpdate(startWorld);
    }

    if (!gs.decorRendererInitialized) {
        gs.decorRenderer.init();
        gs.decorRendererInitialized = true;
    }

    return true;
}

void startNewGame(GameState& gs) {
    SaveGameService::resetSessionState(gs);

    gs.regionManager.shutdown();
    initializeWorld(gs);
    SessionService::refreshRegionGameplayContext(gs);

    MapRegion* region = gs.regionManager.getCurrentRegion();
    glm::vec2 startWorld(7.0f, 8.0f);
    if (region) {
        startWorld = region->getTileMap().tileToWorld(7, 8);
    }
    b2Body_SetTransform(gs.playerBodyId, b2Vec2{startWorld.x, startWorld.y}, b2Rot{0});
    b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);
    gs.camera.position = startWorld;
    gs.spawnPoint = startWorld;

    gs.playerHealth.restore(100.0f, 100.0f);
    gs.playerMana = 100.0f;
    gs.playerMaxMana = 100.0f;
    gs.inventory.setCoins(60);
    gs.inventory.loadFurnitureStock({});
    gs.inventory.loadUnlockedFurniture({});
    gs.inventory.addFurniture("simple_bed", 1);
    gs.inventory.addFurniture("writing_desk", 1);
    gs.inventory.addFurniture("star_lamp", 1);
    gs.inventory.addFurniture("soft_rug", 1);

    gs.emotionSystem.setChildlikeHeart(950.0f);
    gs.emotionSystem.setGrievance(0.0f);
    gs.emotionSystem.setJoy(50.0f);
    gs.emotionSystem.setStress(0.0f);
    gs.emotionSystem.setMood(CharacterMood::Calm);

    gs.timeSystem.setDay(1);
    gs.timeSystem.setHour(10.0f);
    gs.gameTime = 10.0f;
    gs.weatherSystem.setWeatherImmediate(WeatherType::Clear, 0.0f);
    gs.weatherSystem.setRandomWeather(gs.weatherSystem.getDefaultChangeInterval());
    SessionService::refreshWeatherRegionContext(gs);

    gs.buildingSystem.clearInstances();
    gs.toySystem.init();
    gs.toySystem.loadDefinitions(gs.luaVM, "assets/scripts/toys.lua");
    gs.questSystem.init();
    gs.questSystem.loadDefinitions(gs.luaVM, "assets/scripts/quests.lua");

    if (gs.princess) {
        gs.princess->affection = 0.0f;
        gs.princess->setFollowing(false);
        gs.princess->ultimateCharge = 0.0f;
        if (gs.princess->hasBody()) {
            b2Body_SetTransform(gs.princess->getBodyId(), b2Vec2{5.0f, 3.0f}, b2Rot{0});
            b2Body_SetLinearVelocity(gs.princess->getBodyId(), b2Vec2_zero);
        }
    }

    gs.totalPlayTimeSeconds = 0.0f;
    for (int i = 0; i < 5; ++i) {
        float angle = (static_cast<float>(i) / 5.0f) * 6.28318f;
        glm::vec2 spawnPos = gs.spawnPoint + glm::vec2(std::cos(angle), std::sin(angle)) * 10.0f;
        CombatService::spawnEnemy(gs, spawnPos);
    }
    SceneManager::requestMode(gs, AppMode::Playing);
    SessionService::showNotice(gs, "新游戏开始 New Game");
}

bool activateMainMenuSelection(GameState& gs) {
    if (gs.menuSelection == 0) {
        startNewGame(gs);
    } else if (gs.menuSelection == 1) {
        if (!gs.saveSystem.hasSave("autosave")) {
            gs.menuMessage = "没有可读取的存档 / No save found";
            gs.menuMessageTimer = 3.0f;
        } else {
            SaveGameService::loadGameSlot(gs, "autosave");
        }
    } else if (gs.menuSelection == 2) {
        return true;
    }

    return false;
}

}  // namespace GameSessionService
