#include "Game/Data/SaveRepository.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace SaveRepository {

std::string getSavePath(const std::string& slot) {
    return "saves/" + slot + ".json";
}

bool writeSave(const std::string& slot, const Json& data) {
    std::string path = getSavePath(slot);
    fs::path dir = fs::path(path).parent_path();
    if (!dir.empty() && !fs::exists(dir)) {
        fs::create_directories(dir);
    }

    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << data.dump(2);
    return true;
}

bool readSave(const std::string& slot, Json& outData, std::string* errorMessage) {
    std::string path = getSavePath(slot);
    std::ifstream file(path);
    if (!file.is_open()) return false;

    try {
        file >> outData;
    } catch (const Json::exception& e) {
        if (errorMessage) {
            *errorMessage = e.what();
        }
        outData = Json{};
        return false;
    }

    return true;
}

bool deleteSave(const std::string& slot) {
    std::string path = getSavePath(slot);
    try {
        if (fs::exists(path)) {
            fs::remove(path);
            return true;
        }
    } catch (...) {
        return false;
    }
    return false;
}

bool hasSave(const std::string& slot) {
    return fs::exists(getSavePath(slot));
}

std::vector<std::string> getSaveSlots() {
    std::vector<std::string> slots;
    try {
        fs::path savesDir("saves");
        if (fs::exists(savesDir) && fs::is_directory(savesDir)) {
            for (const auto& entry : fs::directory_iterator(savesDir)) {
                if (entry.path().extension() == ".json") {
                    slots.push_back(entry.path().stem().string());
                }
            }
        }
    } catch (...) {
        // Ignore filesystem errors; callers treat an empty list as no saves.
    }
    return slots;
}

}  // namespace SaveRepository
