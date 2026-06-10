#include "Game/Services/SaveGameService.h"

#include "Game/Data/SaveMigration.h"
#include "Game/Data/SaveRepository.h"
#include "Game/Data/SaveSerializer.h"
#include "Game/GameState.h"
#include "Game/Services/SaveApplier.h"
#include "Game/Services/SaveSnapshotBuilder.h"

#include <cstdio>

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
    SaveSerializer::Json jsonData;
    std::string errorMessage;
    if (!SaveRepository::readSave(slot, jsonData, &errorMessage)) {
        if (!errorMessage.empty()) {
            std::fprintf(stderr, "[SaveGameService] Failed to parse %s: %s\n",
                SaveRepository::getSavePath(slot).c_str(),
                errorMessage.c_str());
        }
        gs.ui.menuMessage = "没有可读取的存档 / No save found";
        gs.ui.menuMessageTimer = 3.0f;
        return false;
    }

    SaveData saveData = SaveSerializer::fromJson(jsonData);
    SaveMigration::migrateToCurrent(saveData);
    return applySaveData(gs, saveData);
}

bool hasSave(const std::string& slot) {
    return SaveRepository::hasSave(slot);
}

SaveMeta getSaveMeta(const std::string& slot) {
    SaveMeta meta;
    meta.slot = slot;

    SaveSerializer::Json jsonData;
    if (!SaveRepository::readSave(slot, jsonData)) {
        return meta;
    }

    meta.timestamp = jsonData.value("timestamp", "");
    if (jsonData.contains("player") && jsonData["player"].is_object() &&
        jsonData["player"].contains("progress")) {
        meta.playTime = jsonData["player"]["progress"].value("totalPlayTime", 0.0f);
    }

    if (jsonData.contains("regions") && !jsonData["regions"].empty()) {
        meta.regionName = jsonData["regions"][0].value("name", "");
    }

    return meta;
}

}  // namespace SaveGameService
