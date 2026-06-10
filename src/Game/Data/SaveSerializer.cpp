#include "Game/Data/SaveSerializer.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>

namespace SaveSerializer {

namespace {

bool readChapterState(const Json& value, ChapterState& outState) {
    if (value.is_number_unsigned()) {
        auto raw = value.get<Json::number_unsigned_t>();
        if (raw > static_cast<Json::number_unsigned_t>(std::numeric_limits<long long>::max())) {
            return false;
        }
        return tryParseChapterState(static_cast<long long>(raw), outState);
    }
    if (value.is_number_integer()) {
        return tryParseChapterState(value.get<Json::number_integer_t>(), outState);
    }
    return false;
}

}  // namespace

std::string currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}

Json toJson(const SaveData& data) {
    Json j;
    j["version"] = data.version;
    j["timestamp"] = data.timestamp;

    j["player"]["regionId"] = data.player.regionId;
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

    j["princess"]["name"] = data.princess.name;
    j["princess"]["position"] = {data.princess.position.x, data.princess.position.y};
    j["princess"]["affection"] = data.princess.affection;
    j["princess"]["following"] = data.princess.following;
    j["princess"]["ultimateCharge"] = data.princess.ultimateCharge;
    j["princess"]["hasBody"] = data.princess.hasBody;

    j["emotion"]["childlikeHeart"] = data.childlikeHeart;
    j["emotion"]["grievance"] = data.grievance;
    j["emotion"]["joy"] = data.joy;
    j["emotion"]["stress"] = data.stress;

    j["environment"]["day"] = data.environment.day;
    j["environment"]["hour"] = data.environment.hour;
    j["environment"]["weather"] = data.environment.weather;
    j["environment"]["weatherIntensity"] = data.environment.weatherIntensity;
    j["environment"]["storyWeatherTag"] = data.environment.storyWeatherTag;

    j["homeBase"]["furniture"] = Json::array();
    for (const FurnitureInstance& furniture : data.homeFurniture) {
        j["homeBase"]["furniture"].push_back({
            {"instanceId", furniture.instanceId},
            {"defId", furniture.defId},
            {"tileX", furniture.tile.x},
            {"tileY", furniture.tile.y},
            {"rotation", furniture.rotation}
        });
    }

    j["inventory"]["coins"] = data.player.coins;
    j["inventory"]["unlockedFurniture"] = data.unlockedFurniture;
    j["inventory"]["furnitureStock"] = Json::array();
    for (const FurnitureStock& stock : data.furnitureStock) {
        j["inventory"]["furnitureStock"].push_back({
            {"defId", stock.defId},
            {"count", stock.count}
        });
    }
    j["inventory"]["items"] = Json::array();
    for (const ItemStack& stack : data.itemStacks) {
        if (stack.itemId.empty() || stack.count <= 0) continue;
        j["inventory"]["items"].push_back({
            {"itemId", stack.itemId},
            {"count", stack.count}
        });
    }

    j["story"]["chapters"] = Json::array();
    for (const ChapterProgressEntry& chapter : data.storyProgress.chapters) {
        if (chapter.chapterId.empty() || !isValidChapterState(chapter.state)) continue;
        j["story"]["chapters"].push_back({
            {"chapterId", chapter.chapterId},
            {"state", static_cast<int>(chapter.state)}
        });
    }
    j["story"]["unlockedPartners"] = Json::array();
    for (const std::string& partnerId : data.storyProgress.unlockedPartners) {
        if (!partnerId.empty()) {
            j["story"]["unlockedPartners"].push_back(partnerId);
        }
    }
    j["story"]["flags"] = Json::array();
    for (const StoryFlagEntry& flag : data.storyProgress.flags) {
        if (flag.flagId.empty()) continue;
        j["story"]["flags"].push_back({
            {"flagId", flag.flagId},
            {"value", flag.value}
        });
    }

    j["toys"]["collected"] = data.toyData.collectedToys;
    j["toys"]["miniCarLastRewardDay"] = data.toyData.miniCarLastRewardDay;
    j["toys"]["miniCarBestTime"] = data.toyData.miniCarBestTime;

    j["quests"]["entries"] = Json::array();
    for (const QuestSaveEntry& quest : data.quests) {
        Json qj;
        qj["id"] = quest.id;
        qj["state"] = static_cast<int>(quest.state);
        qj["rewardClaimed"] = quest.rewardClaimed;
        qj["objectives"] = Json::array();
        for (const QuestObjectiveProgress& objective : quest.objectives) {
            qj["objectives"].push_back({
                {"type", objective.type},
                {"targetId", objective.targetId},
                {"required", objective.required},
                {"current", objective.current}
            });
        }
        j["quests"]["entries"].push_back(qj);
    }

    j["regions"] = Json::array();
    for (const auto& region : data.regions) {
        Json rj;
        rj["id"] = region.id;
        rj["name"] = region.name;
        rj["seed"] = region.seed;
        rj["size"] = {region.size.x, region.size.y};
        rj["tileSize"] = region.tileSize;

        rj["modifications"] = Json::array();
        for (const auto& mod : region.modifications) {
            rj["modifications"].push_back({
                {"x", mod.x},
                {"y", mod.y},
                {"oldType", static_cast<int>(mod.oldType)},
                {"newType", static_cast<int>(mod.newType)},
                {"timestamp", mod.timestamp}
            });
        }

        rj["decorations"] = Json::array();
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

        rj["pois"] = Json::array();
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

SaveData fromJson(const Json& data) {
    SaveData result;

    result.version = data.value("version", 1);
    result.timestamp = data.value("timestamp", "");

    if (data.contains("player")) {
        const Json& p = data["player"];
        result.player.regionId = p.value("regionId", std::string{});
        if (p.contains("position") && p["position"].is_array() && p["position"].size() >= 2) {
            result.player.position = glm::vec2(p["position"][0], p["position"][1]);
        }
        result.player.health = p.value("health", 100.0f);
        result.player.maxHealth = p.value("maxHealth", 100.0f);
        result.player.mana = p.value("mana", 0.0f);
        result.player.maxMana = p.value("maxMana", 0.0f);
        result.player.coins = p.value("coins", 0);

        if (p.contains("progress")) {
            const Json& prog = p["progress"];
            result.player.progress.discoveredRegions = prog.value("discoveredRegions", std::vector<std::string>{});
            result.player.progress.completedQuests = prog.value("completedQuests", std::vector<std::string>{});
            result.player.progress.collectedItems = prog.value("collectedItems", std::vector<std::string>{});
            result.player.progress.totalPlayTime = prog.value("totalPlayTime", 0.0f);
            result.player.progress.maxHealth = prog.value("maxHealth", 100);
            result.player.progress.maxMana = prog.value("maxMana", 0);
        }
    }

    if (data.contains("princess")) {
        const Json& princess = data["princess"];
        result.princess.name = princess.value("name", result.princess.name);
        if (princess.contains("position") && princess["position"].is_array() &&
            princess["position"].size() >= 2) {
            result.princess.position = glm::vec2(princess["position"][0], princess["position"][1]);
        }
        result.princess.affection = princess.value("affection", 0.0f);
        result.princess.following = princess.value("following", false);
        result.princess.ultimateCharge = princess.value("ultimateCharge", 0.0f);
        result.princess.hasBody = princess.value("hasBody", false);
    }

    if (data.contains("emotion")) {
        result.childlikeHeart = data["emotion"].value("childlikeHeart", 950.0f);
        result.grievance = data["emotion"].value("grievance", 0.0f);
        result.joy = data["emotion"].value("joy", 50.0f);
        result.stress = data["emotion"].value("stress", 0.0f);
    }

    if (data.contains("environment")) {
        const Json& env = data["environment"];
        result.environment.day = env.value("day", 1);
        result.environment.hour = env.value("hour", 10.0f);
        result.environment.weather = env.value("weather", std::string("Clear"));
        result.environment.weatherIntensity = env.value("weatherIntensity", 0.0f);
        result.environment.storyWeatherTag = env.value("storyWeatherTag", std::string{});
    }

    if (data.contains("homeBase") && data["homeBase"].contains("furniture")) {
        for (const auto& f : data["homeBase"]["furniture"]) {
            FurnitureInstance instance;
            instance.instanceId = f.value("instanceId", 0);
            instance.defId = f.value("defId", std::string{});
            instance.tile = glm::ivec2(f.value("tileX", 0), f.value("tileY", 0));
            instance.rotation = static_cast<uint8_t>(f.value("rotation", 0));
            if (!instance.defId.empty()) {
                result.homeFurniture.push_back(instance);
            }
        }
    }

    if (data.contains("inventory")) {
        const Json& inv = data["inventory"];
        result.player.coins = inv.value("coins", result.player.coins);
        result.unlockedFurniture = inv.value("unlockedFurniture", std::vector<std::string>{});
        if (inv.contains("furnitureStock")) {
            for (const auto& item : inv["furnitureStock"]) {
                FurnitureStock stock;
                stock.defId = item.value("defId", std::string{});
                stock.count = item.value("count", 0);
                if (!stock.defId.empty() && stock.count > 0) {
                    result.furnitureStock.push_back(stock);
                }
            }
        }
        if (inv.contains("items") && inv["items"].is_array()) {
            for (const auto& item : inv["items"]) {
                if (!item.is_object()) continue;
                if (!item.contains("itemId") || !item["itemId"].is_string()) continue;
                if (!item.contains("count") || !item["count"].is_number_integer()) continue;
                ItemStack stack;
                stack.itemId = item["itemId"].get<std::string>();
                stack.count = item["count"].get<int>();
                if (!stack.itemId.empty() && stack.count > 0) {
                    result.itemStacks.push_back(stack);
                }
            }
        }
    }

    if (data.contains("story")) {
        const Json& story = data["story"];
        if (story.contains("chapters") && story["chapters"].is_array()) {
            for (const auto& chapterJson : story["chapters"]) {
                if (!chapterJson.is_object()) continue;
                if (!chapterJson.contains("chapterId") || !chapterJson["chapterId"].is_string()) continue;
                if (!chapterJson.contains("state")) continue;
                ChapterProgressEntry chapter;
                chapter.chapterId = chapterJson["chapterId"].get<std::string>();
                if (!chapter.chapterId.empty() && readChapterState(chapterJson["state"], chapter.state)) {
                    result.storyProgress.chapters.push_back(chapter);
                }
            }
        }
        if (story.contains("unlockedPartners") && story["unlockedPartners"].is_array()) {
            for (const auto& partnerJson : story["unlockedPartners"]) {
                if (!partnerJson.is_string()) {
                    continue;
                }
                std::string partnerId = partnerJson.get<std::string>();
                if (!partnerId.empty()) {
                    result.storyProgress.unlockedPartners.push_back(partnerId);
                }
            }
        }
        if (story.contains("flags") && story["flags"].is_array()) {
            for (const auto& flagJson : story["flags"]) {
                if (!flagJson.is_object()) continue;
                if (!flagJson.contains("flagId") || !flagJson["flagId"].is_string()) continue;
                if (!flagJson.contains("value") || !flagJson["value"].is_boolean()) continue;
                StoryFlagEntry flag;
                flag.flagId = flagJson["flagId"].get<std::string>();
                flag.value = flagJson["value"].get<bool>();
                if (!flag.flagId.empty()) {
                    result.storyProgress.flags.push_back(flag);
                }
            }
        }
    }

    if (data.contains("toys")) {
        const Json& toys = data["toys"];
        result.toyData.collectedToys = toys.value("collected", std::vector<std::string>{});
        result.toyData.miniCarLastRewardDay = toys.value("miniCarLastRewardDay", 0);
        result.toyData.miniCarBestTime = toys.value("miniCarBestTime", 0.0f);
    }

    if (data.contains("quests") && data["quests"].contains("entries")) {
        for (const auto& q : data["quests"]["entries"]) {
            QuestSaveEntry entry;
            entry.id = q.value("id", std::string{});
            entry.state = static_cast<QuestState>(q.value("state", static_cast<int>(QuestState::Available)));
            entry.rewardClaimed = q.value("rewardClaimed", false);
            if (q.contains("objectives")) {
                for (const auto& obj : q["objectives"]) {
                    QuestObjectiveProgress objective;
                    objective.type = obj.value("type", std::string{});
                    objective.targetId = obj.value("targetId", std::string{});
                    objective.required = obj.value("required", 1);
                    objective.current = obj.value("current", 0);
                    entry.objectives.push_back(objective);
                }
            }
            if (!entry.id.empty()) {
                result.quests.push_back(entry);
            }
        }
    } else if (!result.player.progress.completedQuests.empty()) {
        for (const std::string& questId : result.player.progress.completedQuests) {
            QuestSaveEntry entry;
            entry.id = questId;
            entry.state = QuestState::Rewarded;
            entry.rewardClaimed = true;
            result.quests.push_back(entry);
        }
    }

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

}  // namespace SaveSerializer
