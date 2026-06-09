#pragma once

#include "Game/Building/Furniture.h"

#include <vector>

namespace BuildingStats {

int comfort(const std::vector<FurnitureInstance>& instances,
            const std::vector<FurnitureDef>& definitions);

int nostalgiaScore(const std::vector<FurnitureInstance>& instances,
                   const std::vector<FurnitureDef>& definitions);

int lightScore(const std::vector<FurnitureInstance>& instances,
               const std::vector<FurnitureDef>& definitions);

int childlikeRestoreBonus(const std::vector<FurnitureInstance>& instances,
                          const std::vector<FurnitureDef>& definitions);

float nightLightBonus(float hour, int lightScore);

}  // namespace BuildingStats
