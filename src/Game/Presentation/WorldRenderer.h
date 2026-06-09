#pragma once

#include "Engine/Renderer/DecorRenderer.h"
#include "Game/World/Decoration.h"
#include "Game/World/TileMap.h"

#include <glm/mat4x4.hpp>
#include <vector>

namespace WorldRenderer {

struct ViewBounds {
    float left = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    float top = 0.0f;
};

void renderTileMap(const TileMap& map,
                   const TileColors& colors,
                   const glm::mat4& viewProj,
                   const ViewBounds& bounds,
                   float animationTime);

void renderLowDecorations(DecorRenderer& renderer,
                          const TileMap& map,
                          const std::vector<Decoration>& decorations,
                          const glm::mat4& viewProj,
                          float dt);

void renderHighDecorations(DecorRenderer& renderer,
                           const TileMap& map,
                           const std::vector<Decoration>& decorations,
                           const glm::mat4& viewProj,
                           float dt);

}  // namespace WorldRenderer
