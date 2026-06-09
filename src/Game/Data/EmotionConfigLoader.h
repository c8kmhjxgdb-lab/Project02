#pragma once

#include "Game/Emotion/EmotionConfig.h"

class LuaVM;

namespace EmotionConfigLoader {

bool load(LuaVM& lua, const char* path, const EmotionConfig& defaults, EmotionConfig& outConfig);

}  // namespace EmotionConfigLoader
