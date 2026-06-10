#pragma once

#include "Game/Data/SaveData.h"

#include <nlohmann/json.hpp>
#include <string>

namespace SaveSerializer {

using Json = nlohmann::json;

Json toJson(const SaveData& data);
SaveData fromJson(const Json& data);
std::string currentTimestamp();

}  // namespace SaveSerializer
