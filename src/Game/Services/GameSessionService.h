#pragma once

struct GameState;

namespace GameSessionService {

struct MenuActivationResult {
    bool quit = false;
    bool enterPlaying = false;
};

bool initializeWorld(GameState& gs);

void startNewGame(GameState& gs);

MenuActivationResult activateMainMenuSelection(GameState& gs);

}  // namespace GameSessionService
