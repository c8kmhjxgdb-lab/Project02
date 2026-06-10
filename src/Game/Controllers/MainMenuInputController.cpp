#include "Game/Controllers/MainMenuInputController.h"

#include "Game/GameState.h"
#include "Game/Presentation/MainMenuView.h"
#include "Game/Services/AudioService.h"

namespace {

bool activateMenuSelection(GameState& gs, const InputController::Callbacks& callbacks) {
    return callbacks.activateMainMenuSelection && callbacks.activateMainMenuSelection(gs);
}

}  // namespace

namespace MainMenuInputController {

bool handleKeyDown(GameState& gs,
                   SDL_Scancode scancode,
                   const InputController::Callbacks& callbacks) {
    if (scancode == SDL_SCANCODE_ESCAPE) {
        AudioService::playUiSfx(gs.audioSystem, "cancel");
        return true;
    }
    if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP) {
        gs.ui.menuSelection = (gs.ui.menuSelection + MainMenuView::kMenuItemCount - 1) %
            MainMenuView::kMenuItemCount;
        AudioService::playUiSfx(gs.audioSystem, "navigate");
    } else if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN) {
        gs.ui.menuSelection = (gs.ui.menuSelection + 1) % MainMenuView::kMenuItemCount;
        AudioService::playUiSfx(gs.audioSystem, "navigate");
    } else if (scancode == SDL_SCANCODE_RETURN ||
               scancode == SDL_SCANCODE_KP_ENTER ||
               scancode == SDL_SCANCODE_SPACE) {
        AudioService::playUiSfx(gs.audioSystem, "confirm");
        return activateMenuSelection(gs, callbacks);
    }
    return false;
}

bool handleMouseButtonDown(GameState& gs,
                           const SDL_MouseButtonEvent& buttonEvent,
                           const InputController::Callbacks& callbacks) {
    if (buttonEvent.button != SDL_BUTTON_LEFT) {
        return false;
    }

    int hit = MainMenuView::hitTest(
        static_cast<float>(buttonEvent.x),
        static_cast<float>(buttonEvent.y),
        gs.screenWidth,
        gs.screenHeight);
    if (hit < 0) {
        return false;
    }

    gs.ui.menuSelection = hit;
    AudioService::playUiSfx(gs.audioSystem, "confirm");
    return activateMenuSelection(gs, callbacks);
}

}  // namespace MainMenuInputController
