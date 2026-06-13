#include "Game/Controllers/InteractionInputController.h"

#include "Game/Building/BuildingSystem.h"
#include "Game/Boss/PopupCrownBoss.h"
#include "Game/Emotion/EmotionSystem.h"
#include "Game/Emotion/VentAnimation.h"
#include "Game/GameState.h"
#include "Game/Inventory/Inventory.h"
#include "Game/Progress/StoryProgress.h"
#include "Game/Services/DialogueService.h"
#include "Game/Services/NoticeService.h"
#include "Game/Services/RegionService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/Social/DialogueTree.h"
#include "Game/Social/Princess.h"
#include "Game/World/RegionManager.h"
#include "Game/World/TimeSystem.h"
#include "Game/World/WeatherSystem.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

namespace {

bool isGameplayActionAllowed(const InteractionInputController::Context& context) {
    return !context.isDead &&
           !context.buildingSystem.isActive() &&
           !context.toySystem.isMiniCarActive();
}

glm::vec2 getPlayerPosition(const InteractionInputController::Context& context) {
    b2Vec2 pos = b2Body_GetPosition(context.playerBodyId);
    return glm::vec2(pos.x, pos.y);
}

void showNotice(InteractionInputController::Context& context,
                const InteractionInputController::Callbacks& callbacks,
                const std::string& notice) {
    if (callbacks.showNotice) {
        callbacks.showNotice(context, notice);
    }
}

DialogueService::NoticeSink makeNoticeSink(InteractionInputController::Context& context,
                                           const InteractionInputController::Callbacks& callbacks) {
    return {
        [&context, &callbacks](const std::string& notice) {
            showNotice(context, callbacks, notice);
        }
    };
}

bool tryHandleStoryPoi(InteractionInputController::Context& context,
                       const InteractionInputController::Callbacks& callbacks,
                       const glm::vec2& playerPos) {
    MapRegion* region = context.regionManager.getCurrentRegion();
    if (!region) return false;

    if (region->getId() == "real_street_prologue" &&
        WorldQuery::isNearPOI(region, "star_candy", playerPos, 1.6f)) {
        if (!context.storyProgress.getFlag("star_candy_collected")) {
            context.storyProgress.setFlag("star_candy_collected", true);
            context.inventory.addItem("old_game_coin", 1);
            context.storyProgress.startChapter("prologue_star_candy");
            showNotice(context, callbacks, "捡到星星糖 Star Candy found");
        }
        return true;
    }

    if (region->getId() == "home_base" &&
        WorldQuery::isNearPOI(region, "pixel_controller_spot", playerPos, 1.6f)) {
        if (context.storyProgress.getFlag("pixel_controller_placed")) {
            return true;
        }
        if (context.inventory.consumeItem("pixel_controller", 1)) {
            context.storyProgress.setFlag("pixel_controller_placed", true);
            context.inventory.addItem("recovery_candy", 2);
            showNotice(context, callbacks, "试玩币机关已点亮");
        }
        return true;
    }

    if (region->getId() == "popup_arcade" &&
        WorldQuery::isNearPOI(region, "tieyi_cage", playerPos, 1.8f)) {
        if (context.storyProgress.getFlag("tieyi_rescued")) {
            return true;
        }
        if (context.storyProgress.getFlag("scrap_elite_defeated")) {
            context.storyProgress.unlockPartner("tieyi");
            context.storyProgress.setFlag("tieyi_rescued", true);
            context.inventory.addItem("color_battery", 1);
            showNotice(context, callbacks, "铁翼恢复了火箭核心");
        }
        return true;
    }

    if (region->getId() == "popup_arcade" &&
        WorldQuery::isNearPOI(region, "arcade_boss_door", playerPos, 1.8f)) {
        if (context.inventory.getItemCount("trial_token") < 3 ||
            !context.storyProgress.getFlag("tieyi_rescued")) {
            return true;
        }
        if (!context.popupCrownBoss.isActive() && !context.popupCrownBoss.isDefeated()) {
            context.popupCrownBoss.start();
            context.storyProgress.startChapter("chapter_1_popup_arcade");
        }

        glm::vec2 arenaPos(0.0f, 0.0f);
        if (WorldQuery::tryGetPOIWorldPosition(region, "popup_crown_arena", arenaPos)) {
            b2Body_SetTransform(
                context.playerBodyId,
                b2Vec2{arenaPos.x, arenaPos.y},
                b2Rot{0});
            b2Body_SetLinearVelocity(context.playerBodyId, b2Vec2_zero);
        }
        return true;
    }

    return false;
}

void applyProloguePrincessProgress(InteractionInputController::Context& context,
                                   const InteractionInputController::Callbacks& callbacks) {
    if (!context.princess) return;
    if (!WorldQuery::isCurrentRegion(context.regionManager, "real_street_prologue")) return;
    if (!context.storyProgress.getFlag("star_candy_collected")) return;
    if (context.storyProgress.getFlag("alya_following")) return;

    context.princess->setFollowing(true);
    context.storyProgress.setFlag("alya_following", true);
    context.storyProgress.completeChapter("prologue_star_candy");
    showNotice(context, callbacks, "艾莉娅加入：前往南侧裂缝回秘密基地");
}

}  // namespace

namespace InteractionInputController {

Context makeContext(GameState& gs) {
    return {
        gs.isDead,
        gs.buildingSystem,
        gs.toySystem,
        gs.dialogueTree,
        gs.dialogueUI,
        gs.regionManager,
        gs.timeSystem,
        gs.emotionSystem,
        gs.weatherSystem,
        gs.ventAnimation,
        gs.inventory,
        gs.storyProgress,
        gs.popupCrownBoss,
        gs.princess,
        gs.playerBodyId,
        gs.homePosition,
        gs.homeRadius,
        gs.isVenting,
        gs.ui.talkedWithPrincessAtBaseThisFrame,
        {
            gs.buildingSystem,
            gs.dialogueTree,
            gs.dialogueUI,
            gs.regionManager,
            gs.timeSystem,
            gs.emotionSystem,
            gs.weatherSystem,
            gs.ventAnimation,
            gs.princess,
            gs.homePosition,
            gs.homeRadius,
            gs.isVenting,
            gs.ui.talkedWithPrincessAtBaseThisFrame
        }
    };
}

Callbacks makeCallbacks(GameState& gs) {
    return {
        [&gs](Context&, const glm::vec2& playerPos) {
            RegionService::DoorContext doorContext = RegionService::makeDoorContext(gs);
            return RegionService::tryUseHomeBaseDoor(doorContext, playerPos);
        },
        [&gs](Context&, const std::string& notice) {
            NoticeService::Context noticeContext = NoticeService::makeContext(gs);
            NoticeService::showNotice(noticeContext, notice);
        }
    };
}

void handleDialogueNavigation(Context& context, SDL_Scancode scancode) {
    DialogueService::handleNavigation(context.dialogue, scancode);
}

void handleInteract(Context& context, const Callbacks& callbacks) {
    if (!isGameplayActionAllowed(context)) return;

    glm::vec2 playerPos = getPlayerPosition(context);

    DialogueService::NoticeSink noticeSink = makeNoticeSink(context, callbacks);

    if (DialogueService::advanceActiveDialogue(context.dialogue)) {
    } else if (tryHandleStoryPoi(context, callbacks, playerPos)) {
    } else if (callbacks.tryUseHomeBaseDoor &&
               callbacks.tryUseHomeBaseDoor(context, playerPos)) {
        // Enter/leave the secret base via home POIs.
    } else if (WorldQuery::isCurrentRegion(context.regionManager, "home_base")) {
        if (DialogueService::tryRestAtBaseBed(context.dialogue, playerPos, noticeSink)) {
            return;
        }
        if (DialogueService::tryStartPrincessDialogue(context.dialogue, playerPos, noticeSink)) {
            applyProloguePrincessProgress(context, callbacks);
            return;
        }
        DialogueService::tryVent(context.dialogue, playerPos, noticeSink);
    } else if (DialogueService::tryStartPrincessDialogue(context.dialogue, playerPos, noticeSink)) {
        applyProloguePrincessProgress(context, callbacks);
        return;
    } else {
        DialogueService::tryVent(context.dialogue, playerPos, noticeSink);
    }
}

}  // namespace InteractionInputController
