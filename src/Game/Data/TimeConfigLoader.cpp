#include "Game/Data/TimeConfigLoader.h"

#include "Engine/Scripting/LuaVM.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>

namespace {

TimePeriod periodFromId(const std::string& id) {
    if (id == "Dawn" || id == "dawn" || id == "黎明") return TimePeriod::Dawn;
    if (id == "Morning" || id == "morning" || id == "上午") return TimePeriod::Morning;
    if (id == "Afternoon" || id == "afternoon" || id == "下午") return TimePeriod::Afternoon;
    if (id == "Dusk" || id == "dusk" || id == "黄昏") return TimePeriod::Dusk;
    if (id == "Night" || id == "night" || id == "夜晚") return TimePeriod::Night;
    if (id == "LateNight" || id == "late_night" || id == "latenight" || id == "深夜") {
        return TimePeriod::LateNight;
    }
    return TimePeriod::Morning;
}

}  // namespace

namespace TimeConfigLoader {

bool load(LuaVM& lua, const char* path, const TimeConfig& defaults, TimeConfig& outConfig) {
    sol::protected_function_result result = lua.state().script_file(
        path, sol::script_pass_on_error, sol::load_mode::any);
    if (!result.valid()) {
        sol::error err = result;
        fprintf(stderr, "[TimeSystem] Failed to load %s: %s\n", path, err.what());
        return false;
    }

    sol::table table = result;
    if (!table.valid()) {
        fprintf(stderr, "[TimeSystem] No valid table returned from %s\n", path);
        return false;
    }

    TimeConfig loaded = defaults;

    sol::optional<sol::table> periods = table.get<sol::optional<sol::table>>("periods");
    if (periods) {
        std::vector<TimePeriodRule> rules;
        for (auto& pair : *periods) {
            if (!pair.second.is<sol::table>()) continue;
            sol::table item = pair.second.as<sol::table>();
            TimePeriodRule rule;
            rule.period = periodFromId(item.get_or("id", std::string{}));
            rule.start = std::clamp(item.get_or("start", 0.0f), 0.0f, 24.0f);
            rule.finish = std::clamp(item.get_or("finish", 0.0f), 0.0f, 24.0f);
            rules.push_back(rule);
        }
        if (!rules.empty()) {
            loaded.periodRules = std::move(rules);
        }
    }

    sol::optional<sol::table> recovery = table.get<sol::optional<sol::table>>("baseRecovery");
    if (recovery) {
        loaded.nightBonusStartHour = std::clamp(
            recovery->get_or("nightBonusStart", loaded.nightBonusStartHour), 0.0f, 24.0f);
        loaded.restUntilHour = std::clamp(
            recovery->get_or("restUntilHour", loaded.restUntilHour), 0.0f, 23.99f);
        loaded.restChildlikeHeartReward = std::max(
            0.0f,
            recovery->get_or("childlikeHeartPerRest", loaded.restChildlikeHeartReward));
    }

    outConfig = std::move(loaded);
    return true;
}

}  // namespace TimeConfigLoader
