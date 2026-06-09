#include "Game/Data/WeatherConfigLoader.h"

#include "Engine/Scripting/LuaVM.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>

namespace WeatherConfigLoader {

bool load(LuaVM& lua, const char* path, const WeatherConfig& defaults, WeatherConfig& outConfig) {
    sol::protected_function_result result = lua.state().script_file(
        path, sol::script_pass_on_error, sol::load_mode::any);
    if (!result.valid()) {
        sol::error err = result;
        fprintf(stderr, "[WeatherSystem] Failed to load %s: %s\n", path, err.what());
        return false;
    }

    sol::table table = result;
    if (!table.valid()) {
        fprintf(stderr, "[WeatherSystem] No valid table returned from %s\n", path);
        return false;
    }

    WeatherConfig loaded = defaults;
    loaded.defaultChangeInterval = std::max(1.0f,
        table.get_or("defaultInterval", loaded.defaultChangeInterval));
    loaded.hasRegionRules = false;

    sol::optional<sol::table> regions = table.get<sol::optional<sol::table>>("regions");
    if (regions) {
        std::unordered_map<std::string, WeatherRegionRule> regionRules;
        for (auto& regionPair : *regions) {
            if (!regionPair.first.is<std::string>() || !regionPair.second.is<sol::table>()) {
                continue;
            }

            WeatherRegionRule rule;
            std::string regionId = regionPair.first.as<std::string>();
            sol::table regionTable = regionPair.second.as<sol::table>();
            rule.indoor = regionTable.get_or("indoor", false);
            rule.allowParticles = regionTable.get_or("allowParticles", !rule.indoor);
            rule.specialTag = regionTable.get_or("specialTag", std::string{});

            sol::optional<sol::table> weights = regionTable.get<sol::optional<sol::table>>("weights");
            if (weights) {
                for (auto& weightPair : *weights) {
                    if (!weightPair.first.is<std::string>() || !weightPair.second.is<int>()) {
                        continue;
                    }
                    int weight = std::max(0, weightPair.second.as<int>());
                    if (weight <= 0) continue;
                    rule.weights.push_back({
                        WeatherTypes::weatherFromId(weightPair.first.as<std::string>()),
                        weight
                    });
                }
            }

            regionRules[regionId] = std::move(rule);
        }

        if (!regionRules.empty()) {
            loaded.regionRules = std::move(regionRules);
            loaded.hasRegionRules = true;
        }
    }

    outConfig = std::move(loaded);
    return true;
}

}  // namespace WeatherConfigLoader
