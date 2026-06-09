#pragma once

#include "Game/Building/Furniture.h"

#include <vector>

class LuaVM;

namespace FurnitureDefinitionLoader {

bool load(LuaVM& lua, const char* path, std::vector<FurnitureDef>& outDefinitions);

}  // namespace FurnitureDefinitionLoader
