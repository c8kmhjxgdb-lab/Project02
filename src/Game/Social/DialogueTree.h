#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <glm/vec3.hpp>

// Forward declaration — DialogueTree only stores a non-owning pointer.
class LuaVM;

/**
 * 对话选项
 */
struct DialogueChoice {
    std::string text;           // 选项文本
    std::string nextNode;       // 跳转到的节点ID
    int affectionChange;        // 好感度变化
    std::function<void()> onChoose; // 选择后的回调
};

/**
 * 对话节点
 */
struct DialogueNode {
    std::string id;
    std::string speaker;        // 说话者名字
    std::string text;           // 对话内容
    glm::vec3 textColor;        // 文本颜色
    std::vector<DialogueChoice> choices;
    std::string nextNode;       // 如果没有选项，自动跳转
    float displayTime;          // 自动显示时间（0=等待选择）

    DialogueNode() : textColor(1.0f, 1.0f, 1.0f), displayTime(0.0f) {}
};

/**
 * 对话树管理器
 */
class DialogueTree {
public:
    DialogueTree();
    ~DialogueTree();

    // Bind to the shared LuaVM. Must be called before loadFromLua().
    // All dialogue files are loaded into the same Lua state as abilities/config
    // so that global helpers (dist, normalize, ...) are available inside dialogue scripts.
    void setLuaVM(LuaVM* vm) { luaVM = vm; }

    // 从Lua加载对话树
    bool loadFromLua(const char* dialogueId);

    // 开始对话
    void start(const std::string& startNode);

    // 获取当前节点
    const DialogueNode* getCurrentNode() const;

    // 选择选项
    void choose(int choiceIndex);

    // 继续到下一节点
    void next();

    // 是否正在对话中
    bool isActive() const { return !currentNodeId.empty(); }

    // 结束对话
    void end();

    // 对话事件回调
    using DialogueCallback = std::function<void(const DialogueNode&)>;
    void setOnNodeEnter(DialogueCallback cb) { onNodeEnter = std::move(cb); }
    void setOnDialogueEnd(DialogueCallback cb) { onDialogueEnd = std::move(cb); }

    // 获取好感度变化（用于外部处理）
    int getLastAffectionChange() const { return lastAffectionChange; }

private:
    std::unordered_map<std::string, DialogueNode> nodes;
    std::string currentNodeId;
    int lastAffectionChange;

    DialogueCallback onNodeEnter;
    DialogueCallback onDialogueEnd;

    // Shared with the rest of the game. Not owned.
    LuaVM* luaVM = nullptr;

    void enterNode(const std::string& nodeId);
};
