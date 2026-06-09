#include "Game/Controllers/InputController.h"

#include "Game/Controllers/AbilityInputController.h"
#include "Game/Controllers/BuildingInputController.h"
#include "Game/Controllers/InteractionInputController.h"
#include "Game/Controllers/MainMenuInputController.h"
#include "Game/GameState.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/PlayerInputQuery.h"
#include "Game/Services/SaveGameService.h"
#include "Game/Services/SessionService.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

namespace {

#ifdef DEBUG
WeatherType nextDebugWeather(WeatherType current) {
    switch (current) {
        case WeatherType::Clear:     return WeatherType::Cloudy;
        case WeatherType::Cloudy:    return WeatherType::Rain;
        case WeatherType::Rain:      return WeatherType::HeavyRain;
        case WeatherType::HeavyRain: return WeatherType::Fog;
        case WeatherType::Fog:       return WeatherType::Snow;
        case WeatherType::Snow:      return WeatherType::Clear;
        default:                     return WeatherType::Clear;
    }
}

float nextPeriodStartHour(TimePeriod period) {
    switch (period) {
        case TimePeriod::Dawn:      return 7.0f;
        case TimePeriod::Morning:   return 12.0f;
        case TimePeriod::Afternoon: return 17.0f;
        case TimePeriod::Dusk:      return 19.0f;
        case TimePeriod::Night:     return 23.0f;
        case TimePeriod::LateNight: return 5.0f;
        default:                    return 10.0f;
    }
}

void handleDebugKey(GameState& gs, SDL_Scancode scancode) {
    if (scancode == SDL_SCANCODE_N) {
        b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
        glm::vec2 playerPos(pPos.x, pPos.y);
        glm::vec2 spawnPos(playerPos + PlayerInputQuery::getAimDirection(gs, playerPos) * 3.0f);
        CombatService::spawnEnemy(gs, spawnPos);
    }

    if (scancode == SDL_SCANCODE_H) {
        gs.playerHealth.heal(30.0f);
    }

    if (scancode == SDL_SCANCODE_F6) {
        gs.emotionSystem.reduceChildlikeHeart(150.0f);
    }

    if (scancode == SDL_SCANCODE_F7) {
        gs.emotionSystem.addGrievance(25.0f);
    }

    if (scancode == SDL_SCANCODE_F8) {
        gs.weatherSystem.setWeatherImmediate(
            nextDebugWeather(gs.weatherSystem.getCurrentWeather()), 1.0f);
    }

    if (scancode == SDL_SCANCODE_F10) {
        gs.timeSystem.setHour(nextPeriodStartHour(gs.timeSystem.getPeriod()));
    }
}
#endif

bool handleKeyDown(GameState& gs,
                   SDL_Scancode scancode,
                   const InputController::Callbacks& callbacks) {
    gs.input.setKey(scancode, true);

    if (gs.appMode == AppMode::MainMenu) {
        return MainMenuInputController::handleKeyDown(gs, scancode, callbacks);
    }

    if (scancode == SDL_SCANCODE_ESCAPE) {
        if (gs.buildingSystem.isActive()) {
            gs.buildingSystem.setBuildMode(false);
        } else {
            return true;
        }
    }
    if (scancode == SDL_SCANCODE_1) gs.charExpression = 0;
    if (scancode == SDL_SCANCODE_2) gs.charExpression = 1;
    if (scancode == SDL_SCANCODE_3) gs.charExpression = 2;
    if (scancode == SDL_SCANCODE_EQUALS || scancode == SDL_SCANCODE_KP_PLUS)
        gs.camera.setZoom(gs.camera.zoom / 1.15f);
    if (scancode == SDL_SCANCODE_MINUS || scancode == SDL_SCANCODE_KP_MINUS)
        gs.camera.setZoom(gs.camera.zoom * 1.15f);

    BuildingInputController::handleToggleKey(gs, scancode);
    BuildingInputController::handleKeyDown(gs, scancode);
    AbilityInputController::handleKeyDown(gs, scancode);

    if (scancode == SDL_SCANCODE_E) {
        InteractionInputController::handleInteract(gs);
    }

    InteractionInputController::handleDialogueNavigation(gs, scancode);

#ifdef DEBUG
    handleDebugKey(gs, scancode);
#endif

    if (scancode == SDL_SCANCODE_F5) {
        if (SaveGameService::saveCurrentGame(gs, "autosave")) {
            SessionService::showNotice(gs, "已保存 Saved");
        } else {
            SessionService::showNotice(gs, "保存失败 Save failed");
        }
    }

    if (scancode == SDL_SCANCODE_F9) {
        SaveGameService::loadGameSlot(gs, "autosave");
    }

    return false;
}

bool handleMouseButtonDown(GameState& gs,
                           const SDL_MouseButtonEvent& buttonEvent,
                           const InputController::Callbacks& callbacks) {
    gs.input.mousePos = glm::vec2(static_cast<float>(buttonEvent.x),
                                  static_cast<float>(buttonEvent.y));

    if (gs.appMode == AppMode::MainMenu) {
        return MainMenuInputController::handleMouseButtonDown(gs, buttonEvent, callbacks);
    }

    if (!gs.isDead && !gs.dialogueUI.isVisible() && !gs.toySystem.isMiniCarActive()) {
        if (gs.buildingSystem.isActive()) {
            BuildingInputController::handleMouseButtonDown(gs, buttonEvent.button);
        } else {
            AbilityInputController::handleMouseButtonDown(gs, buttonEvent.button);
        }
    }

    return false;
}

}  // namespace

namespace InputController {

bool handleEvent(GameState& gs, const SDL_Event& e, const Callbacks& callbacks) {
    if (e.type == SDL_KEYDOWN) {
        return handleKeyDown(gs, e.key.keysym.scancode, callbacks);
    }

    if (e.type == SDL_KEYUP) {
        SDL_Scancode scancode = e.key.keysym.scancode;
        gs.input.setKey(scancode, false);
    } else if (e.type == SDL_MOUSEWHEEL) {
        if (BuildingInputController::handleMouseWheel(gs, e.wheel.y)) {
            // Building controller consumed the wheel.
        } else if (e.wheel.y > 0) {
            gs.camera.setZoom(gs.camera.zoom * 1.15f);
        } else {
            gs.camera.setZoom(gs.camera.zoom / 1.15f);
        }
    } else if (e.type == SDL_MOUSEMOTION) {
        gs.input.mousePos = glm::vec2(static_cast<float>(e.motion.x),
                                      static_cast<float>(e.motion.y));
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        return handleMouseButtonDown(gs, e.button, callbacks);
    }

    return false;
}

}  // namespace InputController
