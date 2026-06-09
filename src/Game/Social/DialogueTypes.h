#pragma once

#include <functional>
#include <glm/vec3.hpp>
#include <string>
#include <vector>

/**
 * 对话选项
 */
struct DialogueChoice {
    std::string text;           // 选项文本
    std::string textEn;         // 英文选项文本
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
    std::string speakerEn;      // 英文名字
    std::string text;           // 对话内容
    std::string textEn;         // 英文对话内容
    glm::vec3 textColor;        // 文本颜色
    std::vector<DialogueChoice> choices;
    std::string nextNode;       // 如果没有选项，自动跳转
    float displayTime;          // 自动显示时间（0=等待选择）

    DialogueNode() : textColor(1.0f, 1.0f, 1.0f), displayTime(0.0f) {}
};
