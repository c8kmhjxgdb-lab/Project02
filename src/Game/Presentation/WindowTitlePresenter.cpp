#include "Game/Presentation/WindowTitlePresenter.h"

#include "Game/GameState.h"

#include <cstdio>

namespace WindowTitlePresenter {

void update(SDL_Window* window, const GameState& gs, State& state) {
    ++state.frameCount;

    Uint32 now = SDL_GetTicks();
    if (now - state.lastUpdateTicks < 1000) {
        return;
    }

    char title[256];
    const EmotionState& emo = gs.emotionSystem.getState();
    const MapRegion* titleRegion = gs.regionManager.getCurrentRegion();
    snprintf(title, sizeof(title),
        "Starchild 2D | Stage 7 Base | FPS:%d | HP:%d/%d | Heart:%d | Grief:%d | Comfort:%d | %s %s | Region:%s | E:Interact",
        state.frameCount,
        static_cast<int>(gs.playerHealth.getCurrentHealth()),
        static_cast<int>(gs.playerHealth.getMaxHealth()),
        static_cast<int>(emo.childlikeHeart),
        static_cast<int>(emo.grievance),
        gs.buildingSystem.getComfort(),
        gs.timeSystem.getTimeString().c_str(),
        WeatherSystem::getWeatherName(gs.weatherSystem.getCurrentWeather()),
        titleRegion ? titleRegion->getId().c_str() : "-");
    SDL_SetWindowTitle(window, title);

    state.frameCount = 0;
    state.lastUpdateTicks = now;
}

}  // namespace WindowTitlePresenter
