#pragma once

#include "Game/Quest/QuestTypes.h"

#include <vector>

class LuaVM;

namespace QuestDefinitionLoader {

bool load(LuaVM& lua, const char* path, std::vector<QuestDef>& outDefinitions);

}  // namespace QuestDefinitionLoader
