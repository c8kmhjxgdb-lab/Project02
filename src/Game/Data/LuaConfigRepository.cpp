#include "Game/Data/LuaConfigRepository.h"

#include "Engine/Scripting/LuaVM.h"

namespace LuaConfigRepository {

const char* abilitiesPath() { return "assets/scripts/abilities.lua"; }
const char* emotionLegacyPath() { return "assets/scripts/emotion_config.lua"; }
const char* timeEventsPath() { return "assets/scripts/time_events.lua"; }
const char* weatherConfigPath() { return "assets/scripts/weather_config.lua"; }
const char* childhoodConfigPath() { return "assets/scripts/childhood_config.lua"; }
const char* furniturePath() { return "assets/scripts/furniture.lua"; }
const char* toysPath() { return "assets/scripts/toys.lua"; }
const char* questsPath() { return "assets/scripts/quests.lua"; }

bool loadSharedRuntimeScripts(LuaVM& lua) {
    bool ok = true;
    ok = lua.loadFile(abilitiesPath()) && ok;
    ok = lua.loadFile(emotionLegacyPath()) && ok;
    return ok;
}

}  // namespace LuaConfigRepository
