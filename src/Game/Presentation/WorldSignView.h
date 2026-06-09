#pragma once

#include "Engine/Camera/Camera2D.h"
#include "Game/World/MapRegion.h"

#include <glm/mat4x4.hpp>
#include <vector>

namespace WorldSignView {

void renderBoards(const TileMap& map,
                  const std::vector<PointOfInterest>& pois,
                  const glm::mat4& viewProj);

void renderText(const TileMap& map,
                const std::vector<PointOfInterest>& pois,
                const Camera2D& camera,
                const glm::mat4& screenProj,
                int screenW,
                int screenH);

}  // namespace WorldSignView
