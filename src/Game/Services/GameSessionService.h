#pragma once

struct GameState;

namespace GameSessionService {

bool initializeWorld(GameState& gs);

void startNewGame(GameState& gs);

bool activateMainMenuSelection(GameState& gs);

}  // namespace GameSessionService
