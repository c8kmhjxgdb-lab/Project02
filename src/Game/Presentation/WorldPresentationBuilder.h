#pragma once

#include "Game/Presentation/GameRenderer.h"

struct GameState;

namespace WorldPresentationBuilder {

GameRenderer::WorldRenderContext buildRenderContext(GameState& gs);

}  // namespace WorldPresentationBuilder
