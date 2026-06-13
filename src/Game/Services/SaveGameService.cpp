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
    if (jsonData.contains("emotion") && jsonData["emotion"].is_object()) {
        meta.childlikeHeart = jsonData["emotion"].value("childlikeHeart", 0.0f);
    }
    if (jsonData.contains("story") && jsonData["story"].is_object() &&
        jsonData["story"].contains("unlockedPartners") &&
        jsonData["story"]["unlockedPartners"].is_array()) {
        for (const auto& partnerJson : jsonData["story"]["unlockedPartners"]) {
            if (partnerJson.is_string() && !partnerJson.get<std::string>().empty()) {
                ++meta.rescuedPartners;
            }
        }
    }

    std::string currentRegionId;
    if (jsonData.contains("player") && jsonData["player"].is_object()) {
        currentRegionId = jsonData["player"].value("regionId", std::string{});
    }
    if (jsonData.contains("regions") && jsonData["regions"].is_array()) {
        for (const auto& regionJson : jsonData["regions"]) {
            if (!regionJson.is_object()) continue;
            if (!currentRegionId.empty() &&
                regionJson.value("id", std::string{}) == currentRegionId) {
                meta.regionName = regionJson.value("name", std::string{});
                break;
            }
            if (meta.regionName.empty()) {
                meta.regionName = regionJson.value("name", std::string{});
            }
        }
    }

    return meta;
}

}  // namespace SaveGameService
