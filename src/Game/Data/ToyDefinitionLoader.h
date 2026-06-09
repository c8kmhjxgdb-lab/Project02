#pragma once

#include "Game/Toy/ToyTypes.h"

#include <vector>

class LuaVM;

namespace ToyDefinitionLoader {

bool load(LuaVM& lua, const char* path, std::vector<ToyDef>& outDefinitions);

}  // namespace ToyDefinitionLoader
