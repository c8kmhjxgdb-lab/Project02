#pragma once

#include "Game/Data/SaveData.h"

#include <nlohmann/json.hpp>

namespace SaveSerializer {

using Json = nlohmann::json;

Json toJson(const SaveData& data);
SaveData fromJson(const Json& data);

}  // namespace SaveSerializer
