#pragma once

#include "Game/Data/SaveData.h"
#include "Game/GameState.h"

#include <string>

namespace SaveGameService {

void resetSessionState(GameState& gs);

bool saveCurrentGame(GameState& gs, const std::string& slot);

bool applySaveData(GameState& gs, const SaveData& saveData);

bool loadGameSlot(GameState& gs, const std::string& slot);

bool hasSave(const std::string& slot);

struct SaveMeta {
    std::string slot;
    std::string timestamp;
    std::string regionName;
    float playTime = 0.0f;
    float childlikeHeart = 0.0f;
    int rescuedPartners = 0;
};

SaveMeta getSaveMeta(const std::string& slot);

}  // namespace SaveGameService
