#include "Game/Controllers/InteractionInputController.h"

#include "Game/Building/BuildingSystem.h"
#include "Game/Emotion/EmotionSystem.h"
#include "Game/Emotion/VentAnimation.h"
#include "Game/GameState.h"
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
    } else if (callbacks.tryUseHomeBaseDoor &&
               callbacks.tryUseHomeBaseDoor(context, playerPos)) {
        // Enter/leave the secret base via home POIs.
    } else if (WorldQuery::isCurrentRegion(context.regionManager, "home_base")) {
        if (DialogueService::tryRestAtBaseBed(context.dialogue, playerPos, noticeSink)) {
            return;
        }
        if (DialogueService::tryStartPrincessDialogue(context.dialogue, playerPos, noticeSink)) {
            return;
        }
        DialogueService::tryVent(context.dialogue, playerPos, noticeSink);
    } else if (DialogueService::tryStartPrincessDialogue(context.dialogue, playerPos, noticeSink)) {
        return;
    } else {
        DialogueService::tryVent(context.dialogue, playerPos, noticeSink);
    }
}

}  // namespace InteractionInputController
