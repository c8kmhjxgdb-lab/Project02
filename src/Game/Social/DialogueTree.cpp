#include "DialogueTree.h"

#include "Game/Data/DialogueScriptLoader.h"

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
    return DialogueScriptLoader::load(*luaVM, path, nodes);
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
