#pragma once

#include <string>

struct GameUiState {
    std::string stage7Notice;
    float stage7NoticeTimer = 0.0f;
    bool talkedWithPrincessAtBaseThisFrame = false;

    int menuSelection = 0;
    std::string menuMessage;
    float menuMessageTimer = 0.0f;
};
