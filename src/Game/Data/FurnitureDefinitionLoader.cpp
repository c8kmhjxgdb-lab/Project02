#include "Game/Data/FurnitureDefinitionLoader.h"

#include "Engine/Scripting/LuaVM.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace {

FurnitureCategory furnitureCategoryFromString(const std::string& value) {
    if (value == "Bed" || value == "bed") return FurnitureCategory::Bed;
    if (value == "Work" || value == "Table" || value == "Desk" ||
        value == "work" || value == "table" || value == "desk") {
        return FurnitureCategory::Work;
    }
    if (value == "Light" || value == "light") return FurnitureCategory::Light;
    if (value == "Storage" || value == "ToyDisplay" || value == "Shelf" ||
        value == "storage" || value == "toy_display" || value == "shelf") {
        return FurnitureCategory::Storage;
    }
    if (value == "Poster" || value == "poster") return FurnitureCategory::Poster;
    return FurnitureCategory::Decor;
}

glm::vec3 colorForDrawStyle(const std::string& style, bool accent) {
    if (style == "bed") return accent ? glm::vec3(0.92f, 0.80f, 0.62f) : glm::vec3(0.50f, 0.33f, 0.22f);
    if (style == "desk") return accent ? glm::vec3(0.82f, 0.62f, 0.34f) : glm::vec3(0.42f, 0.28f, 0.16f);
    if (style == "flower_pot") return accent ? glm::vec3(0.44f, 0.78f, 0.36f) : glm::vec3(0.45f, 0.23f, 0.12f);
    if (style == "lamp") return accent ? glm::vec3(1.00f, 0.88f, 0.36f) : glm::vec3(0.25f, 0.33f, 0.46f);
    if (style == "toy_shelf") return accent ? glm::vec3(0.96f, 0.58f, 0.28f) : glm::vec3(0.36f, 0.24f, 0.16f);
    if (style == "rug") return accent ? glm::vec3(0.92f, 0.64f, 0.42f) : glm::vec3(0.60f, 0.22f, 0.24f);
    if (style == "poster") return accent ? glm::vec3(0.98f, 0.82f, 0.28f) : glm::vec3(0.18f, 0.32f, 0.54f);
    return accent ? glm::vec3(0.95f, 0.82f, 0.42f) : glm::vec3(0.7f, 0.55f, 0.38f);
}

void assignDefaultDrawStyle(const std::string& id, FurnitureDef& def) {
    if (!def.drawStyle.empty()) return;

    if (id == "simple_bed") def.drawStyle = "bed";
    else if (id == "writing_desk") def.drawStyle = "desk";
    else if (id == "flower_pot") def.drawStyle = "flower_pot";
    else if (id == "star_lamp") def.drawStyle = "lamp";
    else if (id == "toy_shelf") def.drawStyle = "toy_shelf";
    else if (id == "soft_rug") def.drawStyle = "rug";
    else if (id == "childhood_poster") def.drawStyle = "poster";
}

void sortDefinitions(std::vector<FurnitureDef>& definitions) {
    const std::vector<std::string> preferredOrder = {
        "simple_bed",
        "writing_desk",
        "flower_pot",
        "star_lamp",
        "toy_shelf",
        "soft_rug",
        "childhood_poster",
    };

    std::stable_sort(definitions.begin(), definitions.end(),
        [&preferredOrder](const FurnitureDef& a, const FurnitureDef& b) {
            auto ai = std::find(preferredOrder.begin(), preferredOrder.end(), a.id);
            auto bi = std::find(preferredOrder.begin(), preferredOrder.end(), b.id);
            if (ai != preferredOrder.end() || bi != preferredOrder.end()) {
                return ai < bi;
            }
            return a.id < b.id;
        });
}

}  // namespace

namespace FurnitureDefinitionLoader {

bool load(LuaVM& lua, const char* path, std::vector<FurnitureDef>& outDefinitions) {
    sol::protected_function_result result = lua.state().script_file(
        path, sol::script_pass_on_error, sol::load_mode::any);
    if (!result.valid()) {
        sol::error err = result;
        fprintf(stderr, "[BuildingSystem] Failed to load %s: %s\n", path, err.what());
        return false;
    }

    sol::table table = result;
    if (!table.valid()) {
        fprintf(stderr, "[BuildingSystem] No valid table returned from %s\n", path);
        return false;
    }

    std::vector<FurnitureDef> loaded;
    for (auto& pair : table) {
        if (!pair.first.is<std::string>()) continue;
        if (!pair.second.is<sol::table>()) continue;

        std::string id = pair.first.as<std::string>();
        sol::table item = pair.second.as<sol::table>();
        FurnitureDef def;
        def.id = id;
        def.name = item.get_or("name", id);
        def.category = furnitureCategoryFromString(item.get_or("category", std::string{}));
        def.price = item.get_or("price", 0);
        def.comfort = item.get_or("comfort", 0);
        def.childlikeRestore = item.get_or("childlikeHeartRestore",
            item.get_or("childlikeRestore", 0));
        def.affectionBonus = item.get_or("affectionBonus", 0);
        def.lightRadius = item.get_or("lightRadius", 0.0f);
        def.nightBonus = item.get_or("nightBonus", false);
        def.blocksMovement = item.get_or("blocksMovement", false);
        def.nostalgiaTag = item.get_or("nostalgiaTag", std::string{});
        def.drawStyle = item.get_or("drawStyle", std::string{});

        sol::optional<sol::table> size = item.get<sol::optional<sol::table>>("size");
        if (size) {
            def.size.x = std::max(1, size->get_or(1, 1));
            def.size.y = std::max(1, size->get_or(2, 1));
        }

        assignDefaultDrawStyle(id, def);
        def.color = colorForDrawStyle(def.drawStyle, false);
        def.accentColor = colorForDrawStyle(def.drawStyle, true);
        loaded.push_back(def);
    }

    if (loaded.empty()) {
        fprintf(stderr, "[BuildingSystem] Furniture config is empty: %s\n", path);
        return false;
    }

    sortDefinitions(loaded);
    outDefinitions = std::move(loaded);
    return true;
}

}  // namespace FurnitureDefinitionLoader
