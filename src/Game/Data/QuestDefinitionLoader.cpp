#include "Game/Data/QuestDefinitionLoader.h"

#include "Engine/Scripting/LuaVM.h"

#include <cstdio>
#include <string>
#include <utility>

namespace QuestDefinitionLoader {

bool load(LuaVM& lua, const char* path, std::vector<QuestDef>& outDefinitions) {
    sol::protected_function_result result = lua.state().script_file(
        path, sol::script_pass_on_error, sol::load_mode::any);
    if (!result.valid()) {
        sol::error err = result;
        fprintf(stderr, "[QuestSystem] Failed to load %s: %s\n", path, err.what());
        return false;
    }

    sol::table table = result;
    if (!table.valid()) {
        fprintf(stderr, "[QuestSystem] No valid table returned from %s\n", path);
        return false;
    }

    std::vector<QuestDef> loaded;
    for (auto& pair : table) {
        if (!pair.first.is<std::string>() || !pair.second.is<sol::table>()) continue;

        QuestDef quest;
        quest.id = pair.first.as<std::string>();
        sol::table item = pair.second.as<sol::table>();
        quest.name = item.get_or("name", quest.id);
        quest.requiresLowChildlikeHeart = item.get_or("requiresLowChildlikeHeart", false);
        quest.requiresNight = item.get_or("requiresNight", false);
        quest.requiresTalk = item.get_or("requiresTalk", false);
        quest.updateOutsideHomeBase = item.get_or("updateOutsideHomeBase", quest.updateOutsideHomeBase);
        std::string requiredWeather = item.get_or("requiresWeather", std::string{});
        quest.requiresRainy = requiredWeather == "rain" || requiredWeather == "Rain";

        sol::optional<sol::table> requires = item.get<sol::optional<sol::table>>("requires");
        if (requires) {
            for (auto& requiredPair : *requires) {
                if (!requiredPair.second.is<std::string>()) continue;
                std::string required = requiredPair.second.as<std::string>();
                if (required == "mini_car") {
                    quest.requiresMiniCar = true;
                } else {
                    quest.requiredFurniture.push_back(required);
                }
            }
        }

        sol::optional<sol::table> objectives = item.get<sol::optional<sol::table>>("objectives");
        if (objectives) {
            for (auto& objectivePair : *objectives) {
                if (!objectivePair.second.is<sol::table>()) continue;
                sol::table objectiveTable = objectivePair.second.as<sol::table>();

                QuestObjectiveDef objective;
                objective.type = objectiveTable.get_or("type", std::string{});
                objective.targetId = objectiveTable.get_or("target", std::string{});
                if (objective.targetId.empty()) {
                    objective.targetId = objectiveTable.get_or("targetId", std::string{});
                }
                objective.required = objectiveTable.get_or("required", 0);

                if (!objective.type.empty() && !objective.targetId.empty() && objective.required > 0) {
                    quest.objectives.push_back(objective);
                }
            }
        }

        quest.reward.questId = quest.id;
        quest.reward.completed = true;
        sol::optional<sol::table> reward = item.get<sol::optional<sol::table>>("reward");
        if (reward) {
            quest.reward.coins = reward->get_or("coins", 0);
            quest.reward.childlikeHeart = reward->get_or("childlikeHeart", 0.0f);
            quest.reward.reduceGrievance = reward->get_or("reduceGrievance", 0.0f);
            quest.reward.affection = reward->get_or("affection", 0.0f);
            quest.reward.unlockFurniture = reward->get_or("unlockFurniture", std::string{});

            sol::optional<sol::table> itemRewards =
                reward->get<sol::optional<sol::table>>("itemRewards");
            if (itemRewards) {
                for (auto& itemRewardPair : *itemRewards) {
                    if (!itemRewardPair.second.is<sol::table>()) continue;
                    sol::table itemRewardTable = itemRewardPair.second.as<sol::table>();

                    QuestItemReward itemReward;
                    itemReward.itemId = itemRewardTable.get_or("item", std::string{});
                    if (itemReward.itemId.empty()) {
                        itemReward.itemId = itemRewardTable.get_or("itemId", std::string{});
                    }
                    itemReward.count = itemRewardTable.get_or("count", 0);

                    if (!itemReward.itemId.empty() && itemReward.count > 0) {
                        quest.reward.itemRewards.push_back(itemReward);
                    }
                }
            }
        }

        loaded.push_back(quest);
    }

    if (loaded.empty()) {
        fprintf(stderr, "[QuestSystem] Quest config is empty: %s\n", path);
        return false;
    }

    outDefinitions = std::move(loaded);
    return true;
}

}  // namespace QuestDefinitionLoader
