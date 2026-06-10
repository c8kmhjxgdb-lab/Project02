#include "Game/Services/ProgressionUpdateService.h"

#include "Game/Drop.h"
#include "Game/GameState.h"
#include "Game/Progress/StoryProgress.h"
#include "Game/Inventory/Inventory.h"
#include "Game/Quest/QuestSystem.h"
#include "Game/Services/NoticeService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/Toy/ToySystem.h"
#include "Game/World/MapRegion.h"
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

bool hasActiveDropWithCollectionFlag(const DropManager& dropManager,
                                     const std::string& collectionFlag) {
    if (collectionFlag.empty()) return false;
    for (const Drop& drop : dropManager.getActive()) {
        if (drop.active && drop.collectionFlag == collectionFlag) {
            return true;
        }
    }
    return false;
}

void spawnPickupIfNeeded(Context& context,
                         const MapRegion* region,
                         const std::string& poiId,
                         const glm::vec2& offset,
                         const std::string& itemId,
                         int count,
                         const std::string& collectionFlag) {
    if (context.storyProgress.getFlag(collectionFlag)) return;
    if (hasActiveDropWithCollectionFlag(context.dropManager, collectionFlag)) return;

    glm::vec2 position(0.0f, 0.0f);
    if (!WorldQuery::tryGetPOIWorldPosition(region, poiId, position)) return;
    context.dropManager.spawnItem(
        context.worldId,
        position + offset,
        itemId,
        count,
        collectionFlag);
}

void updateChapterPickups(Context& context) {
    const MapRegion* region = context.regionManager.getCurrentRegion();
    if (!region || region->getId() != "popup_arcade") {
        return;
    }

    spawnPickupIfNeeded(context, region, "trial_token_1", glm::vec2(0.0f, 0.0f),
        "trial_token", 1, "collected_trial_token_1");
    spawnPickupIfNeeded(context, region, "trial_token_2", glm::vec2(0.0f, 0.0f),
        "trial_token", 1, "collected_trial_token_2");
    spawnPickupIfNeeded(context, region, "trial_token_3", glm::vec2(0.0f, 0.0f),
        "trial_token", 1, "collected_trial_token_3");

    spawnPickupIfNeeded(context, region, "trapped_player_shadow", glm::vec2(-2.0f, 0.0f),
        "recovery_candy", 1, "collected_recovery_candy_1");
    spawnPickupIfNeeded(context, region, "popup_vendor", glm::vec2(1.5f, 0.0f),
        "recovery_candy", 1, "collected_recovery_candy_2");
    spawnPickupIfNeeded(context, region, "arcade_boss_door", glm::vec2(-2.0f, 1.0f),
        "recovery_candy", 1, "collected_recovery_candy_3");
    spawnPickupIfNeeded(context, region, "base_return_gate", glm::vec2(2.0f, -1.0f),
        "recovery_candy", 1, "collected_recovery_candy_4");

    spawnPickupIfNeeded(context, region, "tieyi_cage", glm::vec2(-1.0f, 1.5f),
        "color_battery", 1, "collected_color_battery_1");
    spawnPickupIfNeeded(context, region, "gray_bureau_notice", glm::vec2(1.0f, 1.0f),
        "color_battery", 1, "collected_color_battery_2");

    if (context.emotionSystem.canSeeHiddenPickups()) {
        spawnPickupIfNeeded(context, region, "popup_crown_arena", glm::vec2(-3.0f, 0.0f),
            "half_melody_arcade", 1, "collected_half_melody_arcade");
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
    const MapRegion* currentRegion = context.regionManager.getCurrentRegion();
    if (currentRegion) {
        questSnapshot.currentRegionId = currentRegion->getId();
        questSnapshot.facts.push_back({"enter_region", questSnapshot.currentRegionId, 1});
    }
    for (const ItemStack& stack : context.inventory.getItemStacks()) {
        questSnapshot.facts.push_back({"collect", stack.itemId, stack.count});
    }
    if (context.storyProgress.isPartnerUnlocked("tieyi")) {
        questSnapshot.facts.push_back({"interact", "tieyi_cage", 1});
    }
    if (context.storyProgress.getFlag("popup_crown_defeated")) {
        questSnapshot.facts.push_back({"clear_boss", "popup_crown", 1});
    }
    return questSnapshot;
}

void applyQuestReward(Context& context, const QuestReward& reward) {
    context.inventory.addCoins(reward.coins);
    for (const QuestItemReward& itemReward : reward.itemRewards) {
        context.inventory.addItem(itemReward.itemId, itemReward.count);
    }
    if (!reward.unlockFurniture.empty()) {
        context.inventory.unlockFurniture(reward.unlockFurniture);
        context.inventory.addFurniture(reward.unlockFurniture, 1);
    }
    if (reward.childlikeHeart > 0.0f) {
        context.emotionSystem.addChildlikeHeart(reward.childlikeHeart);
    }
    if (!reward.storyFlag.empty()) {
        context.storyProgress.setFlag(reward.storyFlag, true);
    }
    if (reward.maxChildlikeHeart > 0.0f) {
        context.storyProgress.setFlag("max_childlike_bonus_" + reward.questId, true);
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
        gs.storyProgress,
        gs.dropManager,
        gs.worldId,
        gs.princess.get(),
        gs.weatherSystem,
        gs.questSystem,
        gs.ui.talkedWithPrincessAtBaseThisFrame,
        [&gs](const std::string& notice) {
            NoticeService::Context noticeContext = NoticeService::makeContext(gs);
            NoticeService::showNotice(noticeContext, notice);
        }
    };
}

void update(Context& context, float dt, const glm::vec2& playerPos, State& state) {
    updateEmotionProgress(context, dt, playerPos, state);
    updateToyProgress(context, dt);
    updateChapterPickups(context);
    updateQuestProgress(context);
    context.talkedWithPrincessAtBaseThisFrame = false;
}

}  // namespace ProgressionUpdateService
