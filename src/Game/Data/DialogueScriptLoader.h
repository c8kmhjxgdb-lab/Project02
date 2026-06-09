#pragma once

#include "Game/Social/DialogueTypes.h"

#include <string>
#include <unordered_map>

class LuaVM;

namespace DialogueScriptLoader {

bool load(LuaVM& luaVM,
          const std::string& path,
          std::unordered_map<std::string, DialogueNode>& outNodes);

}  // namespace DialogueScriptLoader
