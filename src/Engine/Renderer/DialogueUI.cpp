#include "DialogueUI.h"
#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/TextRenderer.h"
#include <algorithm>
#include <cstddef>

namespace {

size_t utf8PrefixBytes(const std::string& text, int codepoints) {
    if (codepoints <= 0) return 0;

    size_t i = 0;
    int count = 0;
    while (i < text.size() && count < codepoints) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        size_t step = 1;
        if ((c & 0x80) == 0) {
            step = 1;
        } else if ((c & 0xE0) == 0xC0) {
            step = 2;
        } else if ((c & 0xF0) == 0xE0) {
            step = 3;
        } else if ((c & 0xF8) == 0xF0) {
            step = 4;
        }
        if (i + step > text.size()) break;
        i += step;
        ++count;
    }
    return i;
}

std::string combineName(const std::string& cn, const std::string& en) {
    if (cn.empty()) return en;
    if (en.empty()) return cn;
    return cn + " / " + en;
}

std::string combineChoice(const std::string& cn, const std::string& en) {
    if (en.empty()) return cn;
    if (cn.empty()) return en;
    return cn + "\n" + en;
}

}  // namespace

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
    state.speakerNameEn = node.speakerEn;
    state.dialogueText = node.text;
    state.dialogueTextEn = node.textEn;
    state.textColor = node.textColor;
    state.typewriterIndex = 0;
    state.typewriterTimer = 0.0f;
    state.selectedChoice = 0;
    state.choiceTexts.clear();
    state.choiceTextsEn.clear();

    // 准备选项文本
    for (const auto& choice : node.choices) {
        state.choiceTexts.push_back(choice.text);
        state.choiceTextsEn.push_back(choice.textEn);
    }
}

void DialogueUI::update(float dt) {
    if (!state.visible) return;

    // 逐字显示
    state.typewriterTimer += dt;
    int charsToShow = static_cast<int>(state.typewriterTimer * typewriterSpeed);
    int new_index = static_cast<int>(utf8PrefixBytes(state.dialogueText, charsToShow));

    if (new_index > state.typewriterIndex) {
        state.typewriterIndex = new_index;
    }
}

void DialogueUI::render(const glm::mat4& orthoProj, int sw, int sh) {
    if (!state.visible) return;
    (void)sh;

    Draw2D::beginFrame(orthoProj);

    // 对话框尺寸
    float padding = 22.0f;
    boxWidth = static_cast<float>(sw) - padding * 4.0f;
    if (boxWidth < 420.0f) boxWidth = 420.0f;
    boxHeight = 158.0f;
    if (!state.choiceTexts.empty()) {
        boxHeight += static_cast<float>(state.choiceTexts.size()) * 50.0f + 14.0f;
    }
    float boxX = (static_cast<float>(sw) - boxWidth) * 0.5f;
    float boxY = 28.0f;

    // 对话框背景（半透明深色矩形）
    Draw2D::drawRectFilled(boxX, boxY, boxWidth, boxHeight, glm::vec3(0.055f, 0.060f, 0.080f), 0.88f);
    Draw2D::drawRectFilled(boxX, boxY + boxHeight - 4.0f, boxWidth, 4.0f, glm::vec3(0.82f, 0.54f, 0.88f), 0.72f);
    Draw2D::drawRect(boxX, boxY, boxWidth, boxHeight, glm::vec3(0.48f, 0.36f, 0.58f), 2.0f, 0.86f);

    // 说话者名字标签
    std::string speaker = combineName(state.speakerName, state.speakerNameEn);
    if (!speaker.empty()) {
        glm::ivec2 nameSize = TextRenderer::measureText(speaker, 18);
        float nameW = static_cast<float>(nameSize.x) + 24.0f;
        float nameH = 30.0f;
        float nameX = boxX + 18.0f;
        float nameY = boxY + boxHeight - nameH - 12.0f;

        Draw2D::drawRectFilled(nameX, nameY, nameW, nameH, glm::vec3(0.4f, 0.3f, 0.5f));
    }

    // 选项列表
    if (!state.choiceTexts.empty() && state.typewriterIndex >= static_cast<int>(state.dialogueText.size())) {
        choiceBoxY = boxY + 18.0f;
        for (size_t i = 0; i < state.choiceTexts.size(); ++i) {
            float optY = choiceBoxY + (state.choiceTexts.size() - 1 - i) * 50.0f;
            bool selected = (i == static_cast<size_t>(state.selectedChoice));
            glm::vec3 optColor = selected ? glm::vec3(0.9f, 0.7f, 0.4f) : glm::vec3(0.7f);

            // 选项背景
            Draw2D::drawRectFilled(boxX + padding, optY, boxWidth - padding * 2, 42.0f,
                selected ? glm::vec3(0.22f, 0.16f, 0.26f) : glm::vec3(0.095f, 0.10f, 0.13f), 0.92f);
            Draw2D::drawRect(boxX + padding, optY, boxWidth - padding * 2, 42.0f,
                optColor, 1.5f, selected ? 0.88f : 0.45f);

            if (selected) {
                Draw2D::drawRectFilled(boxX + padding + 8.0f, optY + 9.0f, 5.0f, 24.0f, optColor, 0.92f);
            }
        }
    }

    Draw2D::endFrame();

    if (!speaker.empty()) {
        TextRenderer::drawText(orthoProj,
            boxX + 30.0f,
            boxY + boxHeight - 37.0f,
            speaker, 18, glm::vec3(1.0f, 0.92f, 1.0f), 0.98f);
    }

    std::string visibleText = state.dialogueText.substr(0, static_cast<size_t>(state.typewriterIndex));
    int textWrap = static_cast<int>(boxWidth - padding * 2.0f);
    float textTop = boxY + boxHeight - (speaker.empty() ? 26.0f : 58.0f);
    TextRenderer::drawTextTopLeft(orthoProj,
        boxX + padding,
        textTop,
        visibleText,
        22,
        state.textColor,
        0.98f,
        textWrap);

    if (!state.dialogueTextEn.empty()) {
        TextRenderer::drawTextTopLeft(orthoProj,
            boxX + padding,
            textTop - 52.0f,
            state.dialogueTextEn,
            17,
            glm::vec3(0.72f, 0.78f, 0.86f),
            0.92f,
            textWrap);
    }

    if (!state.choiceTexts.empty() && state.typewriterIndex >= static_cast<int>(state.dialogueText.size())) {
        for (size_t i = 0; i < state.choiceTexts.size(); ++i) {
            float optY = choiceBoxY + (state.choiceTexts.size() - 1 - i) * 50.0f;
            bool selected = (i == static_cast<size_t>(state.selectedChoice));
            std::string combined = combineChoice(
                state.choiceTexts[i],
                i < state.choiceTextsEn.size() ? state.choiceTextsEn[i] : std::string());
            TextRenderer::drawTextTopLeft(orthoProj,
                boxX + padding + 22.0f,
                optY + 36.0f,
                combined,
                16,
                selected ? glm::vec3(1.0f, 0.86f, 0.54f) : glm::vec3(0.86f, 0.88f, 0.92f),
                0.96f,
                static_cast<int>(boxWidth - padding * 2.0f - 34.0f));
        }
    }
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
