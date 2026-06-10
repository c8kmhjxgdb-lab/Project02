#pragma once

#include "Game/State/GameUiState.h"

#include <string>
#include <vector>

namespace GameMenuView {

struct Model {
    bool open = false;
    GameMenuPage page = GameMenuPage::Quest;
    std::vector<std::string> questLines;
    std::vector<std::string> characterLines;
    std::vector<std::string> inventoryLines;
    std::vector<std::string> partnerLines;
    std::vector<std::string> systemLines;
};

void render(const Model& model, int screenW, int screenH);

}  // namespace GameMenuView
