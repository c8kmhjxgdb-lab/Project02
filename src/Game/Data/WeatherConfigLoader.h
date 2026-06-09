#pragma once

#include "Game/World/WeatherTypes.h"

class LuaVM;

namespace WeatherConfigLoader {

bool load(LuaVM& lua, const char* path, const WeatherConfig& defaults, WeatherConfig& outConfig);

}  // namespace WeatherConfigLoader
