#pragma once

#include <string>

enum class GameMenuPage {
    Quest,
    Character,
    Inventory,
    Partners,
    System
};

struct GameUiState {
    std::string stage7Notice;
    float stage7NoticeTimer = 0.0f;
    bool talkedWithPrincessAtBaseThisFrame = false;

    int menuSelection = 0;
    std::string menuMessage;
    float menuMessageTimer = 0.0f;

    bool gameMenuOpen = false;
    GameMenuPage gameMenuPage = GameMenuPage::Quest;
    int inventorySelection = 0;
    int questSelection = 0;
};
