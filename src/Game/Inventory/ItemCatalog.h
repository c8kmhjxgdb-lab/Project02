#pragma once

#include <string>
#include <vector>

enum class ItemCategory {
    Consumable,
    Material,
    Story,
    ToyFurniture,
    Relic,
    HiddenCollectible
};

struct ItemDef {
    std::string id;
    std::string name;
    ItemCategory category = ItemCategory::Material;
    std::string description;
    bool usable = false;
    bool discardable = true;
    float healthDelta = 0.0f;
    float childlikeHeartDelta = 0.0f;
    float grievanceDelta = 0.0f;
};

namespace ItemCatalog {

const ItemDef* find(const std::string& itemId);
const std::vector<ItemDef>& all();

}  // namespace ItemCatalog
