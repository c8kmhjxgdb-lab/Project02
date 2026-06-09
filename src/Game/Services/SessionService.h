#pragma once

#include "Game/GameState.h"

#include <glm/vec2.hpp>
#include <string>

namespace SessionService {

void showNotice(GameState& gs, const std::string& notice);

void refreshWeatherRegionContext(GameState& gs);

void clearTransientCombat(GameState& gs);

void refreshRegionGameplayContext(GameState& gs);

bool tryUseHomeBaseDoor(GameState& gs, const glm::vec2& playerPos);

}  // namespace SessionService
