#include "App/WindowEvents.h"

#include "Game/GameState.h"

#include <GL/glew.h>

namespace WindowEvents {

void handleWindowEvent(GameState& gs, const SDL_WindowEvent& event) {
    if (event.event != SDL_WINDOWEVENT_RESIZED &&
        event.event != SDL_WINDOWEVENT_SIZE_CHANGED) {
        return;
    }

    int newW = event.data1;
    int newH = event.data2;
    if (newW <= 0 || newH <= 0) return;

    gs.screenWidth = newW;
    gs.screenHeight = newH;
    gs.postProcess.resize(newW, newH);
    glViewport(0, 0, newW, newH);
}

}  // namespace WindowEvents
