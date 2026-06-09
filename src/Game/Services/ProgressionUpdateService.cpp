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

namespace {

void updateEmotionProgress(Context& context,
                           float dt,
                           const glm::vec2& playerPos,
                           State& state) {
    bool atHome = WorldQuery::isPlayerAtHome(
        context.regionManager,
        context.homePosition,
        context.homeRadius,
        playerPos);
    context.emotionSystem.setAtHome(atHome);

    context.emotionSystem.update(dt);
    if (atHome && context.buildingSystem.getChildlikeRestoreBonus() > 0) {
        state.baseChildlikeBonusTimer += dt;
        if (state.baseChildlikeBonusTimer >= 60.0f) {
            state.baseChildlikeBonusTimer -= 60.0f;
            float bonus = std::min(6.0f,
                1.0f + static_cast<float>(context.buildingSystem.getChildlikeRestoreBonus()) * 0.12f);
            context.emotionSystem.addChildlikeHeart(bonus);
        }
    } else {
        state.baseChildlikeBonusTimer = 0.0f;
    }

}

void applyToyReward(Context& context, const ToyReward& toyReward) {
    context.inventory.addCoins(toyReward.coins);
    context.emotionSystem.addChildlikeHeart(toyReward.childlikeHeart);
    if (context.princess && toyReward.affection > 0.0f) {
        context.princess->addAffection(toyReward.affection);
    }
    context.showNotice(
        "模型车完成 Mini-car: +" + std::to_string(toyReward.coins) +
        " coins, 童心 +" + std::to_string(static_cast<int>(toyReward.childlikeHeart)));
}

void updateToyProgress(Context& context, float dt) {
    ToyReward toyReward = context.toySystem.updateMiniCar(
        dt,
        context.input.isDown(SDL_SCANCODE_W) || context.input.isDown(SDL_SCANCODE_UP),
        context.input.isDown(SDL_SCANCODE_S) || context.input.isDown(SDL_SCANCODE_DOWN),
        context.input.isDown(SDL_SCANCODE_A) || context.input.isDown(SDL_SCANCODE_LEFT),
        context.input.isDown(SDL_SCANCODE_D) || context.input.isDown(SDL_SCANCODE_RIGHT),
        context.timeSystem.getDay());
    if (toyReward.granted) {
        applyToyReward(context, toyReward);
    }
}

QuestSnapshot buildQuestSnapshot(const Context& context) {
    QuestSnapshot questSnapshot;
    questSnapshot.inHomeBase = WorldQuery::isCurrentRegion(context.regionManager, "home_base");
    questSnapshot.placedBedCount = WorldQuery::countPlacedFurniture(context.buildingSystem, "simple_bed");
    questSnapshot.placedDeskCount = WorldQuery::countPlacedFurniture(context.buildingSystem, "writing_desk");
    questSnapshot.placedLampCount = WorldQuery::countPlacedFurniture(context.buildingSystem, "star_lamp");
    questSnapshot.placedFlowerPotCount = WorldQuery::countPlacedFurniture(context.buildingSystem, "flower_pot");
    questSnapshot.placedToyShelfCount = WorldQuery::countPlacedFurniture(context.buildingSystem, "toy_shelf");
    questSnapshot.miniCarCollected = context.toySystem.hasToy("mini_car");
    WeatherType currentWeather = context.weatherSystem.getCurrentWeather();
    questSnapshot.isRainy = currentWeather == WeatherType::Rain ||
                            currentWeather == WeatherType::HeavyRain;
    questSnapshot.isNight = context.timeSystem.isNighttime();
    questSnapshot.talkedWithPrincessAtBase = context.talkedWithPrincessAtBaseThisFrame;
    questSnapshot.childlikeHeart = context.emotionSystem.getState().childlikeHeart;
    questSnapshot.lowChildlikeHeartThreshold = context.emotionSystem.getLowChildlikeHeartThreshold();
    return questSnapshot;
}

void applyQuestReward(Context& context, const QuestReward& reward) {
    context.inventory.addCoins(reward.coins);
    if (!reward.unlockFurniture.empty()) {
        context.inventory.unlockFurniture(reward.unlockFurniture);
        context.inventory.addFurniture(reward.unlockFurniture, 1);
    }
    if (reward.childlikeHeart > 0.0f) {
        context.emotionSystem.addChildlikeHeart(reward.childlikeHeart);
    }
    if (reward.reduceGrievance > 0.0f) {
        context.emotionSystem.reduceGrievance(reward.reduceGrievance);
    }
    if (context.princess && reward.affection > 0.0f) {
        context.princess->addAffection(reward.affection);
    }
    context.showNotice(
        "任务完成 Quest: " + reward.questId +
        " 童心 +" + std::to_string(static_cast<int>(reward.childlikeHeart)));
}

void updateQuestProgress(Context& context) {
    QuestSnapshot questSnapshot = buildQuestSnapshot(context);
    for (const QuestReward& reward : context.questSystem.update(questSnapshot)) {
        if (!reward.completed) continue;
        applyQuestReward(context, reward);
    }
}

}  // namespace

Context makeContext(GameState& gs) {
    return {
        gs.regionManager,
        gs.emotionSystem,
        gs.homePosition,
        gs.homeRadius,
        gs.buildingSystem,
        gs.toySystem,
        gs.input,
        gs.timeSystem,
        gs.inventory,
        gs.princess.get(),
        gs.weatherSystem,
        gs.questSystem,
        gs.talkedWithPrincessAtBaseThisFrame,
        [&gs](const std::string& notice) {
            SessionService::showNotice(gs, notice);
        }
    };
}

void update(Context& context, float dt, const glm::vec2& playerPos, State& state) {
    updateEmotionProgress(context, dt, playerPos, state);
    updateToyProgress(context, dt);
    updateQuestProgress(context);
    context.talkedWithPrincessAtBaseThisFrame = false;
}

}  // namespace ProgressionUpdateService
