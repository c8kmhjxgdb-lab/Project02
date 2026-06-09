#include "Game/Data/EmotionConfigLoader.h"

#include "Engine/Scripting/LuaVM.h"

#include <algorithm>
#include <cstdio>

namespace EmotionConfigLoader {

bool load(LuaVM& lua, const char* path, const EmotionConfig& defaults, EmotionConfig& outConfig) {
    sol::protected_function_result result = lua.state().script_file(
        path, sol::script_pass_on_error, sol::load_mode::any);
    if (!result.valid()) {
        sol::error err = result;
        fprintf(stderr, "[EmotionSystem] Failed to load %s: %s\n", path, err.what());
        return false;
    }

    sol::table table = result;
    if (!table.valid()) {
        fprintf(stderr, "[EmotionSystem] No valid table returned from %s\n", path);
        return false;
    }

    EmotionConfig loaded = defaults;
    loaded.hasChildlikeHeartConfig = false;

    sol::optional<sol::table> heart = table.get<sol::optional<sol::table>>("childlikeHeart");
    if (heart) {
        loaded.hasChildlikeHeartConfig = true;
        loaded.childlikeHeartMin = heart->get_or("min", loaded.childlikeHeartMin);
        loaded.childlikeHeartMax = std::max(loaded.childlikeHeartMin + 1.0f,
            heart->get_or("max", loaded.childlikeHeartMax));
        loaded.childlikeHeartLowThreshold = std::clamp(
            heart->get_or("lowThreshold", loaded.childlikeHeartLowThreshold),
            loaded.childlikeHeartMin,
            loaded.childlikeHeartMax);
        loaded.childlikeHeartLowSpeedMultiplier = std::clamp(
            heart->get_or("lowSpeedMultiplier", loaded.childlikeHeartLowSpeedMultiplier),
            0.1f,
            1.0f);
        loaded.initialChildlikeHeart = std::clamp(
            heart->get_or("initial", loaded.initialChildlikeHeart),
            loaded.childlikeHeartMin,
            loaded.childlikeHeartMax);
    }

    sol::optional<sol::table> grievance = table.get<sol::optional<sol::table>>("grievance");
    if (grievance) {
        loaded.grievanceDepressedThreshold = std::clamp(
            grievance->get_or("depressedThreshold", loaded.grievanceDepressedThreshold),
            0.0f,
            100.0f);
        loaded.grievanceExtremeThreshold = std::clamp(
            grievance->get_or("extremeThreshold", loaded.grievanceExtremeThreshold),
            loaded.grievanceDepressedThreshold,
            100.0f);
        loaded.homeGrievanceRecoveryPerMinute = std::max(
            0.0f,
            grievance->get_or("homeRecoveryPerMinute", loaded.homeGrievanceRecoveryPerMinute));
        loaded.ventCooldownDuration = std::max(
            0.0f,
            grievance->get_or("ventCooldown", loaded.ventCooldownDuration));
    }

    outConfig = loaded;
    return true;
}

}  // namespace EmotionConfigLoader
