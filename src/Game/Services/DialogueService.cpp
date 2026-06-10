#include "Game/Services/DialogueService.h"

#include "Engine/Renderer/DialogueUI.h"
#include "Game/Building/BuildingSystem.h"
#include "Game/Emotion/EmotionSystem.h"
#include "Game/Emotion/VentAnimation.h"
#include "Game/Services/WorldQuery.h"
#include "Game/Social/DialogueTree.h"
#include "Game/Social/Princess.h"
#include "Game/World/RegionManager.h"
#include "Game/World/TimeSystem.h"
#include "Game/World/WeatherSystem.h"

#include <SDL2/SDL_scancode.h>

namespace DialogueService {

void showNotice(const NoticeSink& noticeSink, const std::string& notice) {
    if (noticeSink.show) {
        noticeSink.show(notice);
    }
}

void handleNavigation(Context& context, SDL_Scancode scancode) {
    if (!context.dialogueUI.isVisible()) return;

    if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP) {
        context.dialogueUI.navigateUp();
    } else if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN) {
        context.dialogueUI.navigateDown();
    } else if (scancode == SDL_SCANCODE_J || scancode == SDL_SCANCODE_SPACE) {
        context.dialogueUI.confirm();
        const DialogueNode* node = context.dialogueTree.getCurrentNode();
        if (node && !node->choices.empty()) {
            context.dialogueTree.choose(context.dialogueUI.getSelectedChoice());
        } else {
            context.dialogueTree.next();
        }
    }
}

bool advanceActiveDialogue(Context& context) {
    if (!context.dialogueTree.isActive()) {
        return false;
    }

    if (context.dialogueUI.isVisible()) {
        context.dialogueUI.confirm();
        const DialogueNode* currentNode = context.dialogueTree.getCurrentNode();
        if (currentNode && currentNode->choices.empty()) {
            context.dialogueTree.next();
        } else if (currentNode) {
            context.dialogueTree.choose(context.dialogueUI.getSelectedChoice());
        }
    } else {
        context.dialogueTree.next();
    }
    return true;
}

bool tryRestAtBaseBed(Context& context,
                      const glm::vec2& playerPos,
                      const NoticeSink& noticeSink) {
    if (!WorldQuery::isCurrentRegion(context.regionManager, "home_base")) {
        return false;
    }

    MapRegion* buildRegion = context.regionManager.getCurrentRegion();
    bool nearBed = buildRegion && WorldQuery::isNearFurniture(
        context.buildingSystem,
        buildRegion->getTileMap(),
        "simple_bed",
        playerPos,
        2.2f);
    if (!nearBed) {
        return false;
    }

    context.timeSystem.restUntil(context.timeSystem.getRestUntilHour());
    context.emotionSystem.reduceGrievance(
        25.0f + static_cast<float>(context.buildingSystem.getComfort()) * 0.2f);
    context.emotionSystem.addChildlikeHeart(
        context.timeSystem.getRestChildlikeHeartReward() +
        static_cast<float>(context.buildingSystem.getChildlikeRestoreBonus()));
    if (context.princess) {
        context.princess->addAffection(2.0f);
    }
    showNotice(noticeSink, "床铺休息 Rest: 童心恢复，委屈下降");
    return true;
}

bool tryStartPrincessDialogue(Context& context,
                              const glm::vec2& playerPos,
                              const NoticeSink& noticeSink) {
    if (!context.princess || !context.princess->canInteract(playerPos, 2.0f)) {
        return false;
    }

    if (WorldQuery::isCurrentRegion(context.regionManager, "home_base")) {
        context.talkedWithPrincessAtBaseThisFrame = true;
        WeatherType currentWeather = context.weatherSystem.getCurrentWeather();
        bool rainy = currentWeather == WeatherType::Rain ||
                     currentWeather == WeatherType::HeavyRain;
        if (rainy && context.timeSystem.isNighttime()) {
            context.dialogueTree.start("base_rain_night");
            context.emotionSystem.reduceGrievance(12.0f);
            context.emotionSystem.addChildlikeHeart(10.0f);
            context.princess->addAffection(3.0f);
            showNotice(noticeSink, "雨夜谈心 Rain talk: 童心 +10");
            return true;
        }
        if (context.emotionSystem.isLowChildlikeHeart()) {
            context.dialogueTree.start("base_low_heart");
            context.emotionSystem.reduceGrievance(8.0f);
            context.emotionSystem.addChildlikeHeart(16.0f);
            context.princess->addAffection(2.0f);
            showNotice(noticeSink, "低童心安抚 Comfort: 童心 +16");
            return true;
        }
    }

    context.dialogueTree.start("start");
    return true;
}

bool tryVent(Context& context,
             const glm::vec2& playerPos,
             const NoticeSink& noticeSink) {
    if (context.emotionSystem.getState().grievance <= 30.0f) {
        return false;
    }

    bool canVent = WorldQuery::isCurrentRegion(context.regionManager, "home_base") ||
        WorldQuery::isPlayerAtHome(
            context.regionManager,
            context.homePosition,
            context.homeRadius,
            playerPos);
    if (!canVent) {
        return false;
    }

    context.isVenting = true;
    context.ventAnimation.start(playerPos);
    context.emotionSystem.vent();
    showNotice(noticeSink, "宣泄 Vent: 委屈清空");
    return true;
}

}  // namespace DialogueService
