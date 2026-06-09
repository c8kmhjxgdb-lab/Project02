#include "Game/Controllers/MainMenuInputController.h"

#include "Game/GameState.h"
#include "Game/Presentation/MainMenuView.h"

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
        return true;
    }
    if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP) {
        gs.menuSelection = (gs.menuSelection + MainMenuView::kMenuItemCount - 1) %
            MainMenuView::kMenuItemCount;
    } else if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN) {
        gs.menuSelection = (gs.menuSelection + 1) % MainMenuView::kMenuItemCount;
    } else if (scancode == SDL_SCANCODE_RETURN ||
               scancode == SDL_SCANCODE_KP_ENTER ||
               scancode == SDL_SCANCODE_SPACE) {
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

    gs.menuSelection = hit;
    return activateMenuSelection(gs, callbacks);
}

}  // namespace MainMenuInputController
