#pragma once

class LuaVM;

namespace LuaConfigRepository {

const char* abilitiesPath();
const char* emotionLegacyPath();
const char* timeEventsPath();
const char* weatherConfigPath();
const char* childhoodConfigPath();
const char* furniturePath();
const char* toysPath();
const char* questsPath();

bool loadSharedRuntimeScripts(LuaVM& lua);

}  // namespace LuaConfigRepository
