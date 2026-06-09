#include "Game/Data/DialogueScriptLoader.h"

#include "Engine/Scripting/LuaVM.h"

#include <cstdio>

namespace DialogueScriptLoader {

bool load(LuaVM& luaVM,
          const std::string& path,
          std::unordered_map<std::string, DialogueNode>& outNodes) {
    sol::protected_function_result result = luaVM.state().script_file(
        path, sol::script_pass_on_error, sol::load_mode::any);

    if (!result.valid()) {
        sol::error err = result;
        fprintf(stderr, "[DialogueTree] Failed to load %s: %s\n", path.c_str(), err.what());
        return false;
    }

    sol::table dialogueTable = result;
    if (!dialogueTable.valid()) {
        fprintf(stderr, "[DialogueTree] No valid table returned from: %s\n", path.c_str());
        return false;
    }

    std::unordered_map<std::string, DialogueNode> loaded;
    for (auto& pair : dialogueTable) {
        if (!pair.first.is<std::string>()) {
            continue;
        }
        if (!pair.second.is<sol::table>()) {
            continue;
        }

        std::string nodeId = pair.first.as<std::string>();
        sol::table nodeTable = pair.second.as<sol::table>();

        DialogueNode node;
        node.id = nodeId;
        node.speaker = nodeTable.get_or("speaker", std::string(""));
        node.speakerEn = nodeTable.get_or("speakerEn", std::string(""));
        node.text = nodeTable.get_or("text", std::string(""));
        node.textEn = nodeTable.get_or("textEn", std::string(""));
        node.nextNode = nodeTable.get_or("next",
            nodeTable.get_or("nextNode", std::string("")));

        sol::optional<sol::table> colorOpt = nodeTable.get<sol::optional<sol::table>>("textColor");
        if (colorOpt) {
            node.textColor = glm::vec3(
                colorOpt->get_or("r", colorOpt->get_or("x", 1.0f)),
                colorOpt->get_or("g", colorOpt->get_or("y", 1.0f)),
                colorOpt->get_or("b", colorOpt->get_or("z", 1.0f))
            );
        }

        sol::optional<sol::table> choicesOpt = nodeTable.get<sol::optional<sol::table>>("choices");
        if (choicesOpt) {
            for (auto& choicePair : *choicesOpt) {
                if (!choicePair.second.is<sol::table>()) {
                    continue;
                }
                sol::table choiceTable = choicePair.second.as<sol::table>();
                DialogueChoice choice;
                choice.text = choiceTable.get_or("text", std::string(""));
                choice.textEn = choiceTable.get_or("textEn", std::string(""));
                choice.nextNode = choiceTable.get_or("next", std::string(""));
                choice.affectionChange = choiceTable.get_or("affectionChange", 0);
                node.choices.push_back(choice);
            }
        }

        loaded[nodeId] = node;
    }

    if (loaded.empty()) {
        return false;
    }

    outNodes = std::move(loaded);
    return true;
}

}  // namespace DialogueScriptLoader
