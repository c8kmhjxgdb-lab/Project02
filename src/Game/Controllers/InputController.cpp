#include "Game/Controllers/InputController.h"

#include "Game/Controllers/AbilityInputController.h"
#include "Game/Controllers/BuildingInputController.h"
#include "Game/Controllers/InteractionInputController.h"
#include "Game/GameState.h"
#include "Game/Services/AudioService.h"
#include "Game/Services/CombatService.h"
#include "Game/Services/NoticeService.h"
#include "Game/Services/PlayerInputQuery.h"
#include "Game/Services/SaveGameService.h"

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
        CombatService::SpawnContext spawnContext = CombatService::makeSpawnContext(gs);
        CombatService::spawnEnemy(spawnContext, spawnPos);
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
                   SDL_Scancode scancode) {
    gs.input.setKey(scancode, true);

    if (scancode == SDL_SCANCODE_TAB) {
        gs.ui.gameMenuOpen = !gs.ui.gameMenuOpen;
        AudioService::playUiSfx(gs.audioSystem, gs.ui.gameMenuOpen ? "confirm" : "cancel");
        return true;
    }

    if (scancode == SDL_SCANCODE_ESCAPE) {
        if (gs.ui.gameMenuOpen) {
            gs.ui.gameMenuOpen = false;
            AudioService::playUiSfx(gs.audioSystem, "cancel");
            return true;
        } else if (gs.buildingSystem.isActive()) {
            AudioService::playUiSfx(gs.audioSystem, "cancel");
            gs.buildingSystem.setBuildMode(false);
        } else {
            AudioService::playUiSfx(gs.audioSystem, "cancel");
            return true;
        }
    }

    if (gs.ui.gameMenuOpen) {
        if (scancode == SDL_SCANCODE_1) gs.ui.gameMenuPage = GameMenuPage::Quest;
        if (scancode == SDL_SCANCODE_2) gs.ui.gameMenuPage = GameMenuPage::Character;
        if (scancode == SDL_SCANCODE_3) gs.ui.gameMenuPage = GameMenuPage::Inventory;
        if (scancode == SDL_SCANCODE_4) gs.ui.gameMenuPage = GameMenuPage::Partners;
        if (scancode == SDL_SCANCODE_5) gs.ui.gameMenuPage = GameMenuPage::System;
        return true;
    }

    if (scancode == SDL_SCANCODE_1) gs.charExpression = 0;
    if (scancode == SDL_SCANCODE_2) gs.charExpression = 1;
    if (scancode == SDL_SCANCODE_3) gs.charExpression = 2;
    if (scancode == SDL_SCANCODE_EQUALS || scancode == SDL_SCANCODE_KP_PLUS)
        gs.camera.setZoom(gs.camera.zoom / 1.15f);
    if (scancode == SDL_SCANCODE_MINUS || scancode == SDL_SCANCODE_KP_MINUS)
        gs.camera.setZoom(gs.camera.zoom * 1.15f);

    BuildingInputController::Context buildingContext = BuildingInputController::makeContext(gs);
    BuildingInputController::Callbacks buildingCallbacks = BuildingInputController::makeCallbacks(gs);
    BuildingInputController::handleToggleKey(buildingContext, scancode);
    BuildingInputController::handleKeyDown(buildingContext, scancode, buildingCallbacks);
    AbilityInputController::Context abilityContext = AbilityInputController::makeContext(gs);
    AbilityInputController::Callbacks abilityCallbacks = AbilityInputController::makeCallbacks(gs);
    AbilityInputController::handleKeyDown(abilityContext, scancode, abilityCallbacks);

    InteractionInputController::Context interactionContext =
        InteractionInputController::makeContext(gs);

    if (scancode == SDL_SCANCODE_E) {
        InteractionInputController::Callbacks interactionCallbacks =
            InteractionInputController::makeCallbacks(gs);
        InteractionInputController::handleInteract(interactionContext, interactionCallbacks);
    }

    InteractionInputController::handleDialogueNavigation(interactionContext, scancode);

#ifdef DEBUG
    handleDebugKey(gs, scancode);
#endif

    if (scancode == SDL_SCANCODE_F5) {
        NoticeService::Context noticeContext = NoticeService::makeContext(gs);
        AudioService::playUiSfx(gs.audioSystem, "confirm");
        if (SaveGameService::saveCurrentGame(gs, "autosave")) {
            NoticeService::showNotice(noticeContext, "已保存 Saved");
        } else {
            NoticeService::showNotice(noticeContext, "保存失败 Save failed");
        }
    }

    if (scancode == SDL_SCANCODE_F9) {
        AudioService::playUiSfx(gs.audioSystem, "confirm");
        SaveGameService::loadGameSlot(gs, "autosave");
    }

    return false;
}

bool handleMouseButtonDown(GameState& gs,
                           const SDL_MouseButtonEvent& buttonEvent) {
    gs.input.mousePos = glm::vec2(static_cast<float>(buttonEvent.x),
                                  static_cast<float>(buttonEvent.y));

    if (gs.ui.gameMenuOpen) {
        return true;
    }

    if (!gs.isDead && !gs.dialogueUI.isVisible() && !gs.toySystem.isMiniCarActive()) {
        if (gs.buildingSystem.isActive()) {
            BuildingInputController::Context buildingContext = BuildingInputController::makeContext(gs);
            BuildingInputController::Callbacks buildingCallbacks =
                BuildingInputController::makeCallbacks(gs);
            BuildingInputController::handleMouseButtonDown(
                buildingContext,
                buttonEvent.button,
                buildingCallbacks);
        } else {
            AbilityInputController::Context abilityContext = AbilityInputController::makeContext(gs);
            AbilityInputController::Callbacks abilityCallbacks = AbilityInputController::makeCallbacks(gs);
            AbilityInputController::handleMouseButtonDown(
                abilityContext,
                buttonEvent.button,
                abilityCallbacks);
        }
    }

    return false;
}

}  // namespace

namespace InputController {

bool handleEvent(GameState& gs, const SDL_Event& e, const Callbacks& callbacks) {
    (void)callbacks;

    if (e.type == SDL_KEYDOWN) {
        return handleKeyDown(gs, e.key.keysym.scancode);
    }

    if (e.type == SDL_KEYUP) {
        SDL_Scancode scancode = e.key.keysym.scancode;
        gs.input.setKey(scancode, false);
    } else if (e.type == SDL_MOUSEWHEEL) {
        if (gs.ui.gameMenuOpen) {
            return true;
        }
        BuildingInputController::Context buildingContext = BuildingInputController::makeContext(gs);
        if (BuildingInputController::handleMouseWheel(buildingContext, e.wheel.y)) {
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
        return handleMouseButtonDown(gs, e.button);
    }

    return false;
}

}  // namespace InputController
