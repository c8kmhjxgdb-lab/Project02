#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace SaveRepository {

using Json = nlohmann::json;

std::string getSavePath(const std::string& slot);

bool writeSave(const std::string& slot, const Json& data);
bool readSave(const std::string& slot, Json& outData, std::string* errorMessage = nullptr);
bool deleteSave(const std::string& slot);
bool hasSave(const std::string& slot);
std::vector<std::string> getSaveSlots();

}  // namespace SaveRepository
