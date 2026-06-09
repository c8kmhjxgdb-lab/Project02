#include "Game/Services/SaveGameService.h"

#include "Game/Services/SaveApplier.h"
#include "Game/Services/SaveSnapshotBuilder.h"

namespace SaveGameService {

void resetSessionState(GameState& gs) {
    SaveApplier::resetSessionState(gs);
}

bool saveCurrentGame(GameState& gs, const std::string& slot) {
    return SaveSnapshotBuilder::saveCurrentGame(gs, slot);
}

bool applySaveData(GameState& gs, const SaveData& saveData) {
    return SaveApplier::applySaveData(gs, saveData);
}

bool loadGameSlot(GameState& gs, const std::string& slot) {
    SaveData saveData;
    if (!gs.saveSystem.loadGame(slot, saveData)) {
        gs.menuMessage = "没有可读取的存档 / No save found";
        gs.menuMessageTimer = 3.0f;
        return false;
    }
    return applySaveData(gs, saveData);
}

}  // namespace SaveGameService
