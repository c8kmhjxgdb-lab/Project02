#include "SaveSystem.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

std::string SaveSystem::getSavePath(const std::string& slot) const {
    // 存档保存在当前目录的 saves/ 子目录下
    return "saves/" + slot + ".json";
}

std::string SaveSystem::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}

bool SaveSystem::saveGame(const std::string& slot,
                         const glm::vec2& playerPos,
                         float playerHealth,
                         float playerMaxHealth,
                         float playerMana,
                         float playerMaxMana,
                         int playerCoins,
                         const PlayerProgress& progress,
                         const RegionManager& regionManager,
                         float grievance) {
    SaveData data;
    data.version = 2;
    data.timestamp = getCurrentTimestamp();

    // 玩家数据
    data.player.position = playerPos;
    data.player.health = playerHealth;
    data.player.maxHealth = playerMaxHealth;
    data.player.mana = playerMana;
    data.player.maxMana = playerMaxMana;
    data.player.coins = playerCoins;
    data.player.progress = progress;

    // 情感状态
    data.grievance = grievance;
    data.joy = 0.0f;  // 暂时未实现joy系统

    // 区域数据
    for (const auto& regionId : regionManager.getDiscoveredRegions()) {
        const MapRegion* region = regionManager.getRegion(regionId);
        if (!region) continue;

        SaveData::RegionData regionData;
        regionData.id = region->getId();
        regionData.name = region->getName();
        regionData.seed = region->getSeed();
        regionData.size = {region->getWidth(), region->getHeight()};
        regionData.tileSize = region->getTileMap().tileSize;

        // 获取修改记录（从 MapTileManager）
        regionData.modifications = region->getTileManager().getModifications();
        regionData.decorModifications = region->getDecorations();
        regionData.pois = region->getPOIs();

        data.regions.push_back(regionData);
    }

    // 序列化并保存
    json jsonData = saveDataToJson(data);

    std::string path = getSavePath(slot);

    // 确保 saves/ 目录存在
    fs::path dir = fs::path(path).parent_path();
    if (!dir.empty() && !fs::exists(dir)) {
        fs::create_directories(dir);
    }

    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << jsonData.dump(2); // 格式化输出
    return true;
}

bool SaveSystem::loadGame(const std::string& slot, SaveData& outData) {
    std::string path = getSavePath(slot);
    std::ifstream file(path);
    if (!file.is_open()) return false;

    json jsonData;
    file >> jsonData;

    outData = jsonToSaveData(jsonData);
    return true;
}

bool SaveSystem::deleteSave(const std::string& slot) {
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

bool SaveSystem::hasSave(const std::string& slot) const {
    return fs::exists(getSavePath(slot));
}

std::vector<std::string> SaveSystem::getSaveSlots() const {
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
        // 忽略错误
    }
    return slots;
}

SaveSystem::SaveMeta SaveSystem::getSaveMeta(const std::string& slot) const {
    SaveMeta meta;
    meta.slot = slot;
    meta.timestamp = "";
    meta.regionName = "";
    meta.playTime = 0.0f;

    std::string path = getSavePath(slot);
    std::ifstream file(path);
    if (!file.is_open()) return meta;

    try {
        json jsonData;
        file >> jsonData;

        meta.timestamp = jsonData.value("timestamp", "");
        meta.playTime = jsonData.value("player", json::object()).value("playTime", 0.0f);

        if (jsonData.contains("regions") && !jsonData["regions"].empty()) {
            meta.regionName = jsonData["regions"][0].value("name", "");
        }
    } catch (...) {
        // 解析失败，返回空meta
    }

    return meta;
}

json SaveSystem::saveDataToJson(const SaveData& data) const {
    json j;
    j["version"] = data.version;
    j["timestamp"] = data.timestamp;

    // 玩家数据
    j["player"]["position"] = {data.player.position.x, data.player.position.y};
    j["player"]["health"] = data.player.health;
    j["player"]["maxHealth"] = data.player.maxHealth;
    j["player"]["mana"] = data.player.mana;
    j["player"]["maxMana"] = data.player.maxMana;
    j["player"]["coins"] = data.player.coins;
    j["player"]["progress"]["discoveredRegions"] = data.player.progress.discoveredRegions;
    j["player"]["progress"]["completedQuests"] = data.player.progress.completedQuests;
    j["player"]["progress"]["collectedItems"] = data.player.progress.collectedItems;
    j["player"]["progress"]["totalPlayTime"] = data.player.progress.totalPlayTime;
    j["player"]["progress"]["maxHealth"] = data.player.progress.maxHealth;
    j["player"]["progress"]["maxMana"] = data.player.progress.maxMana;

    // 情感状态
    j["emotion"]["grievance"] = data.grievance;
    j["emotion"]["joy"] = data.joy;

    // 区域数据
    j["regions"] = json::array();
    for (const auto& region : data.regions) {
        json rj;
        rj["id"] = region.id;
        rj["name"] = region.name;
        rj["seed"] = region.seed;
        rj["size"] = {region.size.x, region.size.y};
        rj["tileSize"] = region.tileSize;

        // 修改记录
        rj["modifications"] = json::array();
        for (const auto& mod : region.modifications) {
            rj["modifications"].push_back({
                {"x", mod.x},
                {"y", mod.y},
                {"oldType", static_cast<int>(mod.oldType)},
                {"newType", static_cast<int>(mod.newType)},
                {"timestamp", mod.timestamp}
            });
        }

        // 装饰物
        rj["decorations"] = json::array();
        for (const auto& decor : region.decorModifications) {
            rj["decorations"].push_back({
                {"type", static_cast<int>(decor.type)},
                {"variant", decor.variant},
                {"rotation", decor.rotation},
                {"scale", decor.scale},
                {"tileX", decor.tileX},
                {"tileY", decor.tileY}
            });
        }

        // POI
        rj["pois"] = json::array();
        for (const auto& poi : region.pois) {
            rj["pois"].push_back({
                {"type", static_cast<int>(poi.type)},
                {"tileX", poi.tilePos.x},
                {"tileY", poi.tilePos.y},
                {"id", poi.id},
                {"displayName", poi.displayName},
                {"metadata", poi.metadata}
            });
        }

        j["regions"].push_back(rj);
    }

    return j;
}

SaveData SaveSystem::jsonToSaveData(const json& data) const {
    SaveData result;

    result.version = data.value("version", 1);
    result.timestamp = data.value("timestamp", "");

    // 玩家数据
    if (data.contains("player")) {
        const json& p = data["player"];
        if (p.contains("position") && p["position"].is_array() && p["position"].size() >= 2) {
            result.player.position = glm::vec2(p["position"][0], p["position"][1]);
        }
        result.player.health = p.value("health", 100.0f);
        result.player.maxHealth = p.value("maxHealth", 100.0f);
        result.player.mana = p.value("mana", 0.0f);
        result.player.maxMana = p.value("maxMana", 0.0f);
        result.player.coins = p.value("coins", 0);

        if (p.contains("progress")) {
            const json& prog = p["progress"];
            result.player.progress.discoveredRegions = prog.value("discoveredRegions", std::vector<std::string>{});
            result.player.progress.completedQuests = prog.value("completedQuests", std::vector<std::string>{});
            result.player.progress.collectedItems = prog.value("collectedItems", std::vector<std::string>{});
            result.player.progress.totalPlayTime = prog.value("totalPlayTime", 0.0f);
            result.player.progress.maxHealth = prog.value("maxHealth", 100);
            result.player.progress.maxMana = prog.value("maxMana", 0);
        }
    }

    // 情感状态
    if (data.contains("emotion")) {
        result.grievance = data["emotion"].value("grievance", 0.0f);
        result.joy = data["emotion"].value("joy", 0.0f);
    }

    // 区域数据
    if (data.contains("regions")) {
        for (const auto& r : data["regions"]) {
            SaveData::RegionData region;
            region.id = r.value("id", "");
            region.name = r.value("name", "");
            region.seed = r.value("seed", 0);

            if (r.contains("size") && r["size"].is_array() && r["size"].size() >= 2) {
                region.size = glm::ivec2(r["size"][0], r["size"][1]);
            }
            region.tileSize = r.value("tileSize", 1.0f);

            // 修改记录
            if (r.contains("modifications")) {
                for (const auto& m : r["modifications"]) {
                    TileModification mod;
                    mod.x = m.value("x", 0);
                    mod.y = m.value("y", 0);
                    mod.oldType = static_cast<TileType>(m.value("oldType", 0));
                    mod.newType = static_cast<TileType>(m.value("newType", 0));
                    mod.timestamp = m.value("timestamp", 0.0);
                    region.modifications.push_back(mod);
                }
            }

            // 装饰物
            if (r.contains("decorations")) {
                for (const auto& d : r["decorations"]) {
                    Decoration decor;
                    decor.type = static_cast<DecorType>(d.value("type", 0));
                    decor.variant = d.value("variant", static_cast<uint8_t>(0));
                    decor.rotation = d.value("rotation", static_cast<uint8_t>(0));
                    decor.scale = d.value("scale", static_cast<uint8_t>(0));
                    decor.tileX = d.value("tileX", static_cast<int16_t>(0));
                    decor.tileY = d.value("tileY", static_cast<int16_t>(0));
                    region.decorModifications.push_back(decor);
                }
            }

            // POI
            if (r.contains("pois")) {
                for (const auto& p : r["pois"]) {
                    PointOfInterest poi;
                    poi.type = static_cast<PointOfInterest::Type>(p.value("type", 0));
                    poi.tilePos = glm::ivec2(p.value("tileX", 0), p.value("tileY", 0));
                    poi.id = p.value("id", "");
                    poi.displayName = p.value("displayName", "");
                    poi.metadata = p.value("metadata", 0);
                    region.pois.push_back(poi);
                }
            }

            result.regions.push_back(region);
        }
    }

    return result;
}
