#include "DialogueUI.h"
#include "Engine/Renderer/Draw2D.h"
#include <algorithm>

// Default constructor - initialize member variables
DialogueUI::DialogueUI()
    : boxWidth(0)
    , boxHeight(0)
    , choiceBoxY(0)
    , typewriterSpeed(30.0f)
    , maxLineLength(30)
{}

void DialogueUI::begin(const DialogueNode& node) {
    state.visible = true;
    state.speakerName = node.speaker;
    state.dialogueText = node.text;
    state.typewriterIndex = 0;
    state.typewriterTimer = 0.0f;
    state.selectedChoice = 0;
    state.choiceTexts.clear();

    // 准备选项文本
    for (const auto& choice : node.choices) {
        state.choiceTexts.push_back(choice.text);
    }
}

void DialogueUI::update(float dt) {
    if (!state.visible) return;

    // 逐字显示
    state.typewriterTimer += dt;
    int charsToShow = static_cast<int>(state.typewriterTimer * typewriterSpeed);
    int new_index = std::min(charsToShow, static_cast<int>(state.dialogueText.size()));

    if (new_index > state.typewriterIndex) {
        state.typewriterIndex = new_index;
    }
}

void DialogueUI::render(const glm::mat4& orthoProj, int sw, int sh) {
    if (!state.visible) return;

    Draw2D::beginFrame(orthoProj);

    // 对话框尺寸
    float padding = 20.0f;
    float textHeight = 24.0f;
    int lines = (static_cast<int>(state.dialogueText.size()) / maxLineLength) + 1;
    int visibleLines = std::min(lines, 3);  // 最多显示3行
    boxHeight = padding * 2 + textHeight * visibleLines;
    boxWidth = sw - padding * 4;
    float boxX = padding * 2;
    float boxY = sh - boxHeight - padding;

    // 如果有选项，增加高度
    if (!state.choiceTexts.empty()) {
        boxHeight += state.choiceTexts.size() * 35.0f + padding;
        boxY = sh - boxHeight - padding;
    }

    // 对话框背景（半透明深色矩形）
    Draw2D::drawRectFilled(boxX, boxY, boxWidth, boxHeight, glm::vec3(0.1f, 0.1f, 0.15f));
    Draw2D::drawRect(boxX, boxY, boxWidth, boxHeight, glm::vec3(0.4f, 0.3f, 0.5f), 0.02f);

    // 说话者名字标签
    if (!state.speakerName.empty()) {
        float nameW = state.speakerName.size() * 14.0f + 20.0f;
        float nameH = 25.0f;
        float nameX = boxX + 10;
        float nameY = boxY + boxHeight - nameH - 5;

        Draw2D::drawRectFilled(nameX, nameY, nameW, nameH, glm::vec3(0.4f, 0.3f, 0.5f));
        // 注意：这里应该渲染文字，目前用占位符
    }

    // 对话文本区域
    float textX = boxX + padding;
    float textY = boxY + padding + textHeight * (visibleLines - 1);

    // 显示已解锁的文本
    std::string visibleText = state.dialogueText.substr(0, state.typewriterIndex);

    // 简单渲染：用矩形条模拟文字（实际项目中应使用字体渲染）
    int charWidth = 12;
    for (size_t i = 0; i < visibleText.size() && i < 90; ++i) {
        if (visibleText[i] == '\n') continue;
        float cx = textX + (i % maxLineLength) * charWidth;
        float cy = textY - (i / maxLineLength) * textHeight;
        Draw2D::drawRectFilled(cx, cy, charWidth - 2, textHeight - 4, glm::vec3(0.9f));
    }

    // 选项列表
    if (!state.choiceTexts.empty() && state.typewriterIndex >= static_cast<int>(state.dialogueText.size())) {
        choiceBoxY = boxY + padding + textHeight * visibleLines + padding * 0.5f;
        for (size_t i = 0; i < state.choiceTexts.size(); ++i) {
            float optY = choiceBoxY + (state.choiceTexts.size() - 1 - i) * 35.0f;
            bool selected = (i == static_cast<size_t>(state.selectedChoice));
            glm::vec3 optColor = selected ? glm::vec3(0.9f, 0.7f, 0.4f) : glm::vec3(0.7f);

            // 选项背景
            Draw2D::drawRectFilled(boxX + padding, optY, boxWidth - padding * 2, 28.0f,
                selected ? glm::vec3(0.2f, 0.15f, 0.25f) : glm::vec3(0.12f));
            Draw2D::drawRect(boxX + padding, optY, boxWidth - padding * 2, 28.0f,
                optColor, 0.015f);

            // 选项文字占位
            if (selected) {
                Draw2D::drawRectFilled(boxX + padding + 5, optY + 4, 8, 20.0f, optColor);
            }
        }
    }

    Draw2D::endFrame();
}

void DialogueUI::selectChoice(int index) {
    if (index >= 0 && index < static_cast<int>(state.choiceTexts.size())) {
        state.selectedChoice = index;
    }
}

void DialogueUI::navigateUp() {
    if (state.choiceTexts.empty()) return;
    state.selectedChoice--;
    if (state.selectedChoice < 0) {
        state.selectedChoice = static_cast<int>(state.choiceTexts.size()) - 1;
    }
}

void DialogueUI::navigateDown() {
    if (state.choiceTexts.empty()) return;
    state.selectedChoice++;
    if (state.selectedChoice >= static_cast<int>(state.choiceTexts.size())) {
        state.selectedChoice = 0;
    }
}

void DialogueUI::confirm() {
    // 如果文本还在逐字显示，立即显示全部
    if (state.typewriterIndex < static_cast<int>(state.dialogueText.size())) {
        state.typewriterIndex = static_cast<int>(state.dialogueText.size());
        return;
    }

    // 否则确认选择
    // 返回值由外部通过getSelectedChoice()获取
}

std::vector<std::string> DialogueUI::wrapText(const std::string& text, int maxLen) {
    std::vector<std::string> lines;
    std::string currentLine;

    for (char c : text) {
        currentLine += c;
        if (static_cast<int>(currentLine.size()) >= maxLen || c == '\n') {
            lines.push_back(currentLine);
            currentLine.clear();
        }
    }

    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    return lines;
}
