#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

struct FurnitureStock {
    std::string defId;
    int count = 0;
};

struct ItemStack {
    std::string itemId;
    int count = 0;
};

class Inventory {
public:
    void setCoins(int value);
    int getCoins() const { return coins; }
    bool spendCoins(int amount);
    void addCoins(int amount);

    int getItemCount(const std::string& itemId) const;
    void setItemCount(const std::string& itemId, int count);
    void addItem(const std::string& itemId, int count = 1);
    bool consumeItem(const std::string& itemId, int count = 1);
    std::vector<ItemStack> getItemStacks() const;
    void loadItemStacks(const std::vector<ItemStack>& stacks);

    int getFurnitureCount(const std::string& defId) const;
    void setFurnitureCount(const std::string& defId, int count);
    void addFurniture(const std::string& defId, int count = 1);
    bool consumeFurniture(const std::string& defId, int count = 1);
    std::vector<FurnitureStock> getFurnitureStock() const;
    void loadFurnitureStock(const std::vector<FurnitureStock>& stock);

    bool isFurnitureUnlocked(const std::string& defId) const;
    void unlockFurniture(const std::string& defId);
    void unlockFurnitureDefaults();
    std::vector<std::string> getUnlockedFurniture() const;
    void loadUnlockedFurniture(const std::vector<std::string>& unlocked);

private:
    int coins = 0;
    std::unordered_map<std::string, int> itemCounts;
    std::unordered_map<std::string, int> furnitureCounts;
    std::unordered_set<std::string> unlockedFurniture;
};
