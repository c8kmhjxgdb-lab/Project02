#include "Inventory.h"

#include <algorithm>
#include <array>

void Inventory::setCoins(int value) {
    coins = std::max(0, value);
}

bool Inventory::spendCoins(int amount) {
    if (amount <= 0) return true;
    if (coins < amount) return false;
    coins -= amount;
    return true;
}

void Inventory::addCoins(int amount) {
    coins = std::max(0, coins + amount);
}

int Inventory::getFurnitureCount(const std::string& defId) const {
    auto it = furnitureCounts.find(defId);
    return it == furnitureCounts.end() ? 0 : it->second;
}

void Inventory::setFurnitureCount(const std::string& defId, int count) {
    if (defId.empty()) return;
    if (count <= 0) {
        furnitureCounts.erase(defId);
    } else {
        furnitureCounts[defId] = count;
    }
}

void Inventory::addFurniture(const std::string& defId, int count) {
    if (defId.empty() || count <= 0) return;
    unlockFurniture(defId);
    furnitureCounts[defId] = getFurnitureCount(defId) + count;
}

bool Inventory::consumeFurniture(const std::string& defId, int count) {
    if (defId.empty() || count <= 0) return false;
    int current = getFurnitureCount(defId);
    if (current < count) return false;
    setFurnitureCount(defId, current - count);
    return true;
}

std::vector<FurnitureStock> Inventory::getFurnitureStock() const {
    std::vector<FurnitureStock> stock;
    stock.reserve(furnitureCounts.size());
    for (const auto& pair : furnitureCounts) {
        if (pair.second > 0) {
            stock.push_back({pair.first, pair.second});
        }
    }
    return stock;
}

void Inventory::loadFurnitureStock(const std::vector<FurnitureStock>& stock) {
    furnitureCounts.clear();
    for (const FurnitureStock& item : stock) {
        setFurnitureCount(item.defId, item.count);
        if (item.count > 0) {
            unlockFurniture(item.defId);
        }
    }
}

bool Inventory::isFurnitureUnlocked(const std::string& defId) const {
    return unlockedFurniture.find(defId) != unlockedFurniture.end();
}

void Inventory::unlockFurniture(const std::string& defId) {
    if (!defId.empty()) {
        unlockedFurniture.insert(defId);
    }
}

void Inventory::unlockFurnitureDefaults() {
    static constexpr std::array<const char*, 4> defaults = {
        "simple_bed",
        "writing_desk",
        "star_lamp",
        "soft_rug",
    };
    for (const char* defId : defaults) {
        unlockFurniture(defId);
    }
}

std::vector<std::string> Inventory::getUnlockedFurniture() const {
    std::vector<std::string> result;
    result.reserve(unlockedFurniture.size());
    for (const std::string& defId : unlockedFurniture) {
        result.push_back(defId);
    }
    std::sort(result.begin(), result.end());
    return result;
}

void Inventory::loadUnlockedFurniture(const std::vector<std::string>& unlocked) {
    unlockedFurniture.clear();
    for (const std::string& defId : unlocked) {
        unlockFurniture(defId);
    }
    unlockFurnitureDefaults();
    for (const auto& pair : furnitureCounts) {
        if (pair.second > 0) {
            unlockFurniture(pair.first);
        }
    }
}
