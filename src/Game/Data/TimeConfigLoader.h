#pragma once

#include "Game/World/TimeTypes.h"

class LuaVM;

namespace TimeConfigLoader {

bool load(LuaVM& lua, const char* path, const TimeConfig& defaults, TimeConfig& outConfig);

}  // namespace TimeConfigLoader
