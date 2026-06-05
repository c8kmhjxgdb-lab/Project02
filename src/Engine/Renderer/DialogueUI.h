#pragma once

#include "Game/Social/DialogueTree.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>
#include <vector>

struct DialogueUIState {
    bool visible;
    std::string speakerName;
    std::string dialogueText;
    std::vector<std::string> choiceTexts;
    int selectedChoice;
    float typewriterTimer;
    int typewriterIndex;  // 逐字显示的当前字符索引

    DialogueUIState()
        : visible(false)
        , selectedChoice(0)
        , typewriterTimer(0.0f)
        , typewriterIndex(0)
    {}
};

/**
 * 对话UI渲染器
 *
 * 使用Draw2D立即模式绘制对话框、选项等UI元素。
 */
class DialogueUI {
public:
    DialogueUI();

    void begin(const DialogueNode& node);
    void update(float dt);
    void render(const glm::mat4& orthoProj, int screenWidth, int screenHeight);

    // 输入处理
    void selectChoice(int index);
    void navigateUp();
    void navigateDown();
    void confirm();

    bool isVisible() const { return state.visible; }
    void hide() { state.visible = false; }

    int getSelectedChoice() const { return state.selectedChoice; }

    // 设置逐字显示速度（字符/秒）
    void setTypewriterSpeed(float speed) { typewriterSpeed = speed; }

private:
    DialogueUIState state;
    float boxWidth, boxHeight;
    float choiceBoxY;
    float typewriterSpeed;
    int maxLineLength;

    // 计算文本换行
    std::vector<std::string> wrapText(const std::string& text, int maxLen);
};
