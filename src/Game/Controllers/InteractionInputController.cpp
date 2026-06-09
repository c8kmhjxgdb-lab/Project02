#include "Game/Controllers/InteractionInputController.h"

#include "Game/GameState.h"
#include "Game/Services/SessionService.h"
#include "Game/Services/WorldQuery.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

namespace {

bool isGameplayActionAllowed(const GameState& gs) {
    return !gs.isDead && !gs.buildingSystem.isActive() && !gs.toySystem.isMiniCarActive();
}

}  // namespace

namespace InteractionInputController {

void handleDialogueNavigation(GameState& gs, SDL_Scancode scancode) {
    if (!gs.dialogueUI.isVisible()) return;

    if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP) {
        gs.dialogueUI.navigateUp();
    } else if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN) {
        gs.dialogueUI.navigateDown();
    } else if (scancode == SDL_SCANCODE_J || scancode == SDL_SCANCODE_SPACE) {
        gs.dialogueUI.confirm();
        if (gs.dialogueTree.getCurrentNode() && !gs.dialogueTree.getCurrentNode()->choices.empty()) {
            gs.dialogueTree.choose(gs.dialogueUI.getSelectedChoice());
        } else {
            gs.dialogueTree.next();
        }
    }
}

void handleInteract(GameState& gs) {
    if (!isGameplayActionAllowed(gs)) return;

    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
    glm::vec2 playerPos(pPos.x, pPos.y);

    if (gs.dialogueTree.isActive()) {
        if (gs.dialogueUI.isVisible()) {
            gs.dialogueUI.confirm();
            const DialogueNode* currentNode = gs.dialogueTree.getCurrentNode();
            if (currentNode && currentNode->choices.empty()) {
                gs.dialogueTree.next();
            } else if (currentNode) {
                gs.dialogueTree.choose(gs.dialogueUI.getSelectedChoice());
            }
        } else {
            gs.dialogueTree.next();
        }
    } else if (SessionService::tryUseHomeBaseDoor(gs, playerPos)) {
        // Enter/leave the secret base via home POIs.
    } else if (WorldQuery::isCurrentRegion(gs.regionManager, "home_base")) {
        MapRegion* buildRegion = gs.regionManager.getCurrentRegion();
        bool nearBed = buildRegion && WorldQuery::isNearFurniture(
            gs.buildingSystem,
            buildRegion->getTileMap(),
            "simple_bed",
            playerPos,
            2.2f);
        if (nearBed) {
            gs.timeSystem.restUntil(gs.timeSystem.getRestUntilHour());
            gs.emotionSystem.reduceGrievance(
                25.0f + static_cast<float>(gs.buildingSystem.getComfort()) * 0.2f);
            gs.emotionSystem.addChildlikeHeart(
                gs.timeSystem.getRestChildlikeHeartReward() +
                static_cast<float>(gs.buildingSystem.getChildlikeRestoreBonus()));
            if (gs.princess) {
                gs.princess->addAffection(2.0f);
            }
            SessionService::showNotice(gs, "床铺休息 Rest: 童心恢复，委屈下降");
        } else if (gs.princess && gs.princess->canInteract(playerPos, 2.0f)) {
            gs.talkedWithPrincessAtBaseThisFrame = true;
            WeatherType currentWeather = gs.weatherSystem.getCurrentWeather();
            bool rainy = currentWeather == WeatherType::Rain ||
                         currentWeather == WeatherType::HeavyRain;
            if (rainy && gs.timeSystem.isNighttime()) {
                gs.dialogueTree.start("base_rain_night");
                gs.emotionSystem.reduceGrievance(12.0f);
                gs.emotionSystem.addChildlikeHeart(10.0f);
                gs.princess->addAffection(3.0f);
                SessionService::showNotice(gs, "雨夜谈心 Rain talk: 童心 +10");
            } else if (gs.emotionSystem.isLowChildlikeHeart()) {
                gs.dialogueTree.start("base_low_heart");
                gs.emotionSystem.reduceGrievance(8.0f);
                gs.emotionSystem.addChildlikeHeart(16.0f);
                gs.princess->addAffection(2.0f);
                SessionService::showNotice(gs, "低童心安抚 Comfort: 童心 +16");
            } else {
                gs.dialogueTree.start("start");
            }
        } else if (gs.emotionSystem.getState().grievance > 30.0f) {
            gs.isVenting = true;
            gs.ventAnimation.start(playerPos);
            gs.emotionSystem.vent();
            SessionService::showNotice(gs, "宣泄 Vent: 委屈清空");
        }
    } else if (gs.princess && gs.princess->canInteract(playerPos, 2.0f)) {
        gs.dialogueTree.start("start");
    } else if (WorldQuery::isPlayerAtHome(
                   gs.regionManager,
                   gs.homePosition,
                   gs.homeRadius,
                   playerPos) &&
               gs.emotionSystem.getState().grievance > 30.0f) {
        gs.isVenting = true;
        gs.ventAnimation.start(playerPos);
        gs.emotionSystem.vent();
        SessionService::showNotice(gs, "宣泄 Vent: 委屈清空");
    }
}

}  // namespace InteractionInputController
