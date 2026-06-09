#include "Game/Building/BuildingStats.h"

#include <algorithm>
#include <unordered_set>

namespace {

const FurnitureDef* findDef(const std::vector<FurnitureDef>& definitions, const std::string& defId) {
    auto it = std::find_if(definitions.begin(), definitions.end(),
        [&defId](const FurnitureDef& def) { return def.id == defId; });
    return it == definitions.end() ? nullptr : &*it;
}

}  // namespace

namespace BuildingStats {

int comfort(const std::vector<FurnitureInstance>& instances,
            const std::vector<FurnitureDef>& definitions) {
    int total = 0;
    for (const FurnitureInstance& instance : instances) {
        const FurnitureDef* def = findDef(definitions, instance.defId);
        if (def) total += def->comfort;
    }
    return total;
}

int nostalgiaScore(const std::vector<FurnitureInstance>& instances,
                   const std::vector<FurnitureDef>& definitions) {
    std::unordered_set<std::string> tags;
    int score = 0;
    for (const FurnitureInstance& instance : instances) {
        const FurnitureDef* def = findDef(definitions, instance.defId);
        if (!def) continue;
        score += std::max(0, def->childlikeRestore);
        if (!def->nostalgiaTag.empty() && tags.insert(def->nostalgiaTag).second) {
            score += 2;
        }
    }
    return score;
}

int lightScore(const std::vector<FurnitureInstance>& instances,
               const std::vector<FurnitureDef>& definitions) {
    int score = 0;
    for (const FurnitureInstance& instance : instances) {
        const FurnitureDef* def = findDef(definitions, instance.defId);
        if (!def) continue;
        if (def->lightRadius > 0.0f) {
            score += static_cast<int>(def->lightRadius);
        }
        if (def->nightBonus) {
            score += 1;
        }
    }
    return score;
}

int childlikeRestoreBonus(const std::vector<FurnitureInstance>& instances,
                          const std::vector<FurnitureDef>& definitions) {
    int bonus = 0;
    for (const FurnitureInstance& instance : instances) {
        const FurnitureDef* def = findDef(definitions, instance.defId);
        if (def) {
            bonus += std::max(0, def->childlikeRestore);
        }
    }
    return bonus;
}

float nightLightBonus(float hour, int lightScore) {
    bool night = hour >= 19.0f || hour < 5.0f;
    if (!night) return 0.0f;
    return std::min(0.25f, static_cast<float>(lightScore) * 0.03f);
}

}  // namespace BuildingStats
