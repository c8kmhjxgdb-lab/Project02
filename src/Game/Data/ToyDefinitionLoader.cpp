#include "Game/Data/ToyDefinitionLoader.h"

#include "Engine/Scripting/LuaVM.h"

#include <cstdio>
#include <string>

namespace ToyDefinitionLoader {

bool load(LuaVM& lua, const char* path, std::vector<ToyDef>& outDefinitions) {
    sol::protected_function_result result = lua.state().script_file(
        path, sol::script_pass_on_error, sol::load_mode::any);
    if (!result.valid()) {
        sol::error err = result;
        fprintf(stderr, "[ToySystem] Failed to load %s: %s\n", path, err.what());
        return false;
    }

    sol::table table = result;
    if (!table.valid()) {
        fprintf(stderr, "[ToySystem] No valid table returned from %s\n", path);
        return false;
    }

    std::vector<ToyDef> loaded;
    for (auto& pair : table) {
        if (!pair.first.is<std::string>() || !pair.second.is<sol::table>()) continue;
        std::string id = pair.first.as<std::string>();
        sol::table item = pair.second.as<sol::table>();

        ToyDef def;
        def.id = id;
        def.name = item.get_or("name", id);
        def.displayFurniture = item.get_or("displayFurniture",
            item.get_or("displayOn", std::string("toy_shelf")));
        def.dailyReward = item.get_or("dailyReward", true);

        sol::optional<sol::table> reward = item.get<sol::optional<sol::table>>("reward");
        if (reward) {
            def.rewardCoins = reward->get_or("coins", def.rewardCoins);
            def.rewardChildlikeHeart = reward->get_or("childlikeHeart", def.rewardChildlikeHeart);
            def.rewardAffection = reward->get_or("affection", def.rewardAffection);
        }
        sol::optional<sol::table> daily = item.get<sol::optional<sol::table>>("dailyRewards");
        if (daily) {
            def.rewardCoins = daily->get_or("coins", def.rewardCoins);
            def.rewardChildlikeHeart = daily->get_or("childlikeHeart", def.rewardChildlikeHeart);
            def.rewardAffection = daily->get_or("affection", def.rewardAffection);
        }

        loaded.push_back(def);
    }

    if (loaded.empty()) {
        fprintf(stderr, "[ToySystem] Toy config is empty: %s\n", path);
        return false;
    }

    outDefinitions = std::move(loaded);
    return true;
}

}  // namespace ToyDefinitionLoader
