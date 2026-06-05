#include "DialogueTree.h"
#include "Engine/Scripting/LuaVM.h"
#include <cstdio>

DialogueTree::DialogueTree() = default;
DialogueTree::~DialogueTree() = default;

bool DialogueTree::loadFromLua(const char* dialogueId) {
    if (!luaVM) {
        fprintf(stderr, "[DialogueTree] No LuaVM bound; call setLuaVM() first.\n");
        return false;
    }

    // 对话树以Lua表形式存储在 assets/scripts/dialogues/<id>.lua
    // 文件格式: return { start = {...}, node1 = {...}, ... }
    std::string path = std::string("assets/scripts/dialogues/") + dialogueId + ".lua";

    // Use the shared LuaVM so global helpers (dist, normalize, ...) and other
    // loaded scripts (abilities, emotion_config) are all in the same state.
    sol::protected_function_result result = luaVM->state().script_file(
        path, sol::script_pass_on_error, sol::load_mode::any);

    if (!result.valid()) {
        sol::error err = result;
        fprintf(stderr, "[DialogueTree] Failed to load %s: %s\n", path.c_str(), err.what());
        return false;
    }

    // 获取返回的表
    sol::table dialogueTable = result;
    if (!dialogueTable.valid()) {
        fprintf(stderr, "[DialogueTree] No valid table returned from: %s\n", path.c_str());
        return false;
    }

    nodes.clear();

    // 遍历表中的每个节点
    for (auto& pair : dialogueTable) {
        // 跳过非字符串键（如数字索引）
        if (!pair.first.is<std::string>()) {
            continue;
        }
        std::string nodeId = pair.first.as<std::string>();
        sol::table nodeTable = pair.second;

        DialogueNode node;
        node.id = nodeId;
        node.speaker = nodeTable.get_or("speaker", std::string(""));
        node.text = nodeTable.get_or("text", std::string(""));
        node.nextNode = nodeTable.get_or("next",
            nodeTable.get_or("nextNode", std::string("")));

        // 解析文本颜色
        sol::optional<sol::table> colorOpt = nodeTable.get<sol::optional<sol::table>>("textColor");
        if (colorOpt) {
            node.textColor = glm::vec3(
                colorOpt->get_or("r", colorOpt->get_or("x", 1.0f)),
                colorOpt->get_or("g", colorOpt->get_or("y", 1.0f)),
                colorOpt->get_or("b", colorOpt->get_or("z", 1.0f))
            );
        }

        // 解析选项
        sol::optional<sol::table> choicesOpt = nodeTable.get<sol::optional<sol::table>>("choices");
        if (choicesOpt) {
            for (auto& choicePair : *choicesOpt) {
                sol::table choiceTable = choicePair.second;
                DialogueChoice choice;
                choice.text = choiceTable.get_or("text", std::string(""));
                choice.nextNode = choiceTable.get_or("next", std::string(""));
                choice.affectionChange = choiceTable.get_or("affectionChange", 0);
                node.choices.push_back(choice);
            }
        }

        nodes[nodeId] = node;
    }

    return !nodes.empty();
}

void DialogueTree::start(const std::string& startNode) {
    enterNode(startNode);
}

const DialogueNode* DialogueTree::getCurrentNode() const {
    auto it = nodes.find(currentNodeId);
    if (it != nodes.end()) {
        return &it->second;
    }
    return nullptr;
}

void DialogueTree::choose(int choiceIndex) {
    const DialogueNode* node = getCurrentNode();
    if (!node || choiceIndex < 0 || choiceIndex >= static_cast<int>(node->choices.size())) {
        return;
    }

    const DialogueChoice& choice = node->choices[choiceIndex];
    lastAffectionChange = choice.affectionChange;

    if (choice.onChoose) {
        choice.onChoose();
    }

    if (!choice.nextNode.empty()) {
        enterNode(choice.nextNode);
    } else {
        end();
    }
}

void DialogueTree::next() {
    const DialogueNode* node = getCurrentNode();
    if (!node) {
        end();
        return;
    }

    // 如果有选项，不调用next（应该用choose）
    if (!node->choices.empty()) {
        return;
    }

    if (!node->nextNode.empty()) {
        enterNode(node->nextNode);
    } else {
        end();
    }
}

void DialogueTree::end() {
    if (!currentNodeId.empty()) {
        const DialogueNode* node = getCurrentNode();
        if (node && onDialogueEnd) {
            onDialogueEnd(*node);
        }
    }
    currentNodeId.clear();
}

void DialogueTree::enterNode(const std::string& nodeId) {
    auto it = nodes.find(nodeId);
    if (it == nodes.end()) {
        fprintf(stderr, "[DialogueTree] Node not found: %s\n", nodeId.c_str());
        end();
        return;
    }

    currentNodeId = nodeId;

    if (onNodeEnter) {
        onNodeEnter(it->second);
    }
}
