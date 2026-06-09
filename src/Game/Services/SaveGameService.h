#pragma once

#include "Game/Data/SaveData.h"
#include "Game/GameState.h"

#include <string>

namespace SaveGameService {

void resetSessionState(GameState& gs);

bool saveCurrentGame(GameState& gs, const std::string& slot);

bool applySaveData(GameState& gs, const SaveData& saveData);

bool loadGameSlot(GameState& gs, const std::string& slot);

}  // namespace SaveGameService
