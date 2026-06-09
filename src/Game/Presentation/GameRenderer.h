#pragma once

#include "Game/World/TileMap.h"

struct GameState;

namespace GameRenderer {

void renderMainMenu(GameState& gs);

void renderWorld(GameState& gs, const TileColors& tileColors, float dt);

}  // namespace GameRenderer
