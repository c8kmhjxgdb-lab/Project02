#include "Game/Services/ProgressionUpdateService.h"

#include "Game/GameState.h"
#include "Game/Quest/QuestSystem.h"
#include "Game/Services/SessionService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/Toy/ToySystem.h"
#include "Game/World/WeatherSystem.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <string>

namespace ProgressionUpdateService {

void update(GameState& gs, float dt, const glm::vec2& playerPos, State& state) {
    bool atHome = WorldQuery::isPlayerAtHome(
        gs.regionManager,
        gs.homePosition,
        gs.homeRadius,
        playerPos);
    gs.emotionSystem.setAtHome(atHome);

    gs.emotionSystem.update(dt);
    if (atHome && gs.buildingSystem.getChildlikeRestoreBonus() > 0) {
        state.baseChildlikeBonusTimer += dt;
        if (state.baseChildlikeBonusTimer >= 60.0f) {
            state.baseChildlikeBonusTimer -= 60.0f;
            float bonus = std::min(6.0f,
                1.0f + static_cast<float>(gs.buildingSystem.getChildlikeRestoreBonus()) * 0.12f);
            gs.emotionSystem.addChildlikeHeart(bonus);
        }
    } else {
        state.baseChildlikeBonusTimer = 0.0f;
    }

    ToyReward toyReward = gs.toySystem.updateMiniCar(
        dt,
        gs.keys[SDL_SCANCODE_W] || gs.keys[SDL_SCANCODE_UP],
        gs.keys[SDL_SCANCODE_S] || gs.keys[SDL_SCANCODE_DOWN],
        gs.keys[SDL_SCANCODE_A] || gs.keys[SDL_SCANCODE_LEFT],
        gs.keys[SDL_SCANCODE_D] || gs.keys[SDL_SCANCODE_RIGHT],
        gs.timeSystem.getDay());
    if (toyReward.granted) {
        gs.inventory.addCoins(toyReward.coins);
        gs.emotionSystem.addChildlikeHeart(toyReward.childlikeHeart);
        if (gs.princess && toyReward.affection > 0.0f) {
            gs.princess->addAffection(toyReward.affection);
        }
        SessionService::showNotice(gs,
            "模型车完成 Mini-car: +" + std::to_string(toyReward.coins) +
            " coins, 童心 +" + std::to_string(static_cast<int>(toyReward.childlikeHeart)));
    }

    QuestSnapshot questSnapshot;
    questSnapshot.inHomeBase = WorldQuery::isCurrentRegion(gs.regionManager, "home_base");
    questSnapshot.placedBedCount = WorldQuery::countPlacedFurniture(gs.buildingSystem, "simple_bed");
    questSnapshot.placedDeskCount = WorldQuery::countPlacedFurniture(gs.buildingSystem, "writing_desk");
    questSnapshot.placedLampCount = WorldQuery::countPlacedFurniture(gs.buildingSystem, "star_lamp");
    questSnapshot.placedFlowerPotCount = WorldQuery::countPlacedFurniture(gs.buildingSystem, "flower_pot");
    questSnapshot.placedToyShelfCount = WorldQuery::countPlacedFurniture(gs.buildingSystem, "toy_shelf");
    questSnapshot.miniCarCollected = gs.toySystem.hasToy("mini_car");
    WeatherType currentWeather = gs.weatherSystem.getCurrentWeather();
    questSnapshot.isRainy = currentWeather == WeatherType::Rain ||
                            currentWeather == WeatherType::HeavyRain;
    questSnapshot.isNight = gs.timeSystem.isNighttime();
    questSnapshot.talkedWithPrincessAtBase = gs.talkedWithPrincessAtBaseThisFrame;
    questSnapshot.childlikeHeart = gs.emotionSystem.getState().childlikeHeart;
    questSnapshot.lowChildlikeHeartThreshold = gs.emotionSystem.getLowChildlikeHeartThreshold();

    for (const QuestReward& reward : gs.questSystem.update(questSnapshot)) {
        if (!reward.completed) continue;
        gs.inventory.addCoins(reward.coins);
        if (!reward.unlockFurniture.empty()) {
            gs.inventory.unlockFurniture(reward.unlockFurniture);
            gs.inventory.addFurniture(reward.unlockFurniture, 1);
        }
        if (reward.childlikeHeart > 0.0f) {
            gs.emotionSystem.addChildlikeHeart(reward.childlikeHeart);
        }
        if (reward.reduceGrievance > 0.0f) {
            gs.emotionSystem.reduceGrievance(reward.reduceGrievance);
        }
        if (gs.princess && reward.affection > 0.0f) {
            gs.princess->addAffection(reward.affection);
        }
        SessionService::showNotice(gs,
            "任务完成 Quest: " + reward.questId +
            " 童心 +" + std::to_string(static_cast<int>(reward.childlikeHeart)));
    }
    gs.talkedWithPrincessAtBaseThisFrame = false;
}

}  // namespace ProgressionUpdateService
