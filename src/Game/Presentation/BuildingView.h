#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class BuildingSystem;
struct TileMap;

namespace BuildingView {

void render(const BuildingSystem& buildingSystem, const TileMap& map, const glm::mat4& viewProj);
void renderPreview(const BuildingSystem& buildingSystem,
                   const TileMap& map,
                   const glm::mat4& viewProj,
                   const glm::vec2& mouseWorld);

}  // namespace BuildingView
