#pragma once

#include "Game/Data/SaveData.h"

struct GameState;

namespace SaveApplier {

void resetSessionState(GameState& gs);

bool applySaveData(GameState& gs, const SaveData& saveData);

}  // namespace SaveApplier
