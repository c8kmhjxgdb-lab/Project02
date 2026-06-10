# Starchild Nostalgia Rework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first production vertical slice for the nostalgia rework: prologue, secret base loop, in-game menu, inventory, quest log, childlike-heart skill tiers, Popup Arcade, Tieyi rescue, Six-Yuan Crown boss, and save/load for that progress.

**Architecture:** Extend the current C++17 architecture instead of replacing it. Keep `RegionManager`, `QuestSystem`, `EmotionSystem`, `Inventory`, save services, combat services, and `PresentationModelBuilder`; add small focused units for item catalog, story progress, tiered skills, menu views, chapter content, and boss logic.

**Tech Stack:** C++17, CMake, SDL2/OpenGL, Box2D, Lua/sol2, nlohmann_json, existing lightweight executable tests in `tests/`.

---

## Scope Check

The design document covers the full game, but this implementation plan covers only the first playable vertical slice. Chapters 2-8, hidden chapter, finale, full sprite pipeline, full companion AI, equipment, and full shop economy remain outside this plan.

## File Structure

Create:

- `src/Game/Inventory/ItemCatalog.h`
- `src/Game/Inventory/ItemCatalog.cpp`
- `src/Game/Progress/StoryProgress.h`
- `src/Game/Progress/StoryProgress.cpp`
- `src/Game/Ability/ChildlikeSkillProfile.h`
- `src/Game/Ability/ChildlikeSkillProfile.cpp`
- `src/Game/AI/EnemyDefinition.h`
- `src/Game/AI/EnemyDefinition.cpp`
- `src/Game/Boss/BossController.h`
- `src/Game/Boss/BossController.cpp`
- `src/Game/Boss/PopupCrownBoss.h`
- `src/Game/Boss/PopupCrownBoss.cpp`
- `src/Game/Presentation/GameMenuView.h`
- `src/Game/Presentation/GameMenuView.cpp`
- `src/Game/Presentation/QuestLogView.h`
- `src/Game/Presentation/QuestLogView.cpp`
- `src/Game/Presentation/CharacterPanelView.h`
- `src/Game/Presentation/CharacterPanelView.cpp`
- `src/Game/Presentation/InventoryView.h`
- `src/Game/Presentation/InventoryView.cpp`
- `src/Game/Presentation/PixelActorView.h`
- `src/Game/Presentation/PixelActorView.cpp`
- `tests/InventoryItemTest.cpp`
- `tests/StoryProgressTest.cpp`
- `tests/EmotionTierTest.cpp`
- `tests/ChildlikeSkillProfileTest.cpp`
- `tests/QuestObjectiveFlowTest.cpp`
- `tests/RegionFactoryNostalgiaTest.cpp`
- `tests/EnemyDefinitionTest.cpp`
- `tests/PopupCrownBossTest.cpp`

Modify:

- `CMakeLists.txt`
- `src/Game/GameState.h`
- `src/Game/State/GameUiState.h`
- `src/Game/Inventory/Inventory.h`
- `src/Game/Inventory/Inventory.cpp`
- `src/Game/Data/SaveData.h`
- `src/Game/Data/SaveMigration.h`
- `src/Game/Data/SaveMigration.cpp`
- `src/Game/Data/SaveSerializer.cpp`
- `src/Game/Services/SaveSnapshotBuilder.cpp`
- `src/Game/Services/SaveApplier.cpp`
- `src/Game/Emotion/EmotionSystem.h`
- `src/Game/Emotion/EmotionSystem.cpp`
- `src/Game/Quest/QuestTypes.h`
- `src/Game/Quest/QuestSystem.h`
- `src/Game/Quest/QuestSystem.cpp`
- `src/Game/Data/QuestDefinitionLoader.cpp`
- `assets/scripts/quests.lua`
- `src/Game/Services/ProgressionUpdateService.h`
- `src/Game/Services/ProgressionUpdateService.cpp`
- `src/Game/Ability/Projectile.h`
- `src/Game/Ability/Projectile.cpp`
- `src/Game/Services/CombatService.h`
- `src/Game/Services/CombatService.cpp`
- `src/Game/Services/CombatCollisionService.h`
- `src/Game/Services/CombatCollisionService.cpp`
- `src/Game/Services/WorldCombatUpdateService.h`
- `src/Game/Services/WorldCombatUpdateService.cpp`
- `src/Game/Presentation/HudView.h`
- `src/Game/Presentation/HudView.cpp`
- `src/Game/Presentation/PresentationModelBuilder.h`
- `src/Game/Presentation/PresentationModelBuilder.cpp`
- `src/Game/Presentation/GameRenderer.cpp`
- `src/Game/Controllers/InputController.cpp`
- `src/Game/World/RegionFactory.cpp`
- `src/Game/Services/RegionService.cpp`
- `src/Game/Services/EnemySpawnService.cpp`
- `src/Game/Services/WorldUpdateService.cpp`

---

## Task 1: 通用物品背包与物品目录

**Files:**

- Create: `src/Game/Inventory/ItemCatalog.h`
- Create: `src/Game/Inventory/ItemCatalog.cpp`
- Modify: `src/Game/Inventory/Inventory.h`
- Modify: `src/Game/Inventory/Inventory.cpp`
- Create: `tests/InventoryItemTest.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing inventory item test**

Create `tests/InventoryItemTest.cpp`:

```cpp
#include "Game/Inventory/Inventory.h"
#include "Game/Inventory/ItemCatalog.h"
#include "TestSupport.h"

#include <vector>

namespace {

void itemStacksClampAndSort() {
    Inventory inv;
    inv.addItem("recovery_candy", 2);
    inv.addItem("pixel_screw", 5);
    inv.addItem("recovery_candy", 3);
    inv.addItem("", 9);
    inv.addItem("old_button", -2);

    TestSupport::require(inv.getItemCount("recovery_candy") == 5, "recovery candy stacks");
    TestSupport::require(inv.getItemCount("pixel_screw") == 5, "pixel screw stacks");
    TestSupport::require(inv.getItemCount("old_button") == 0, "negative item count ignored");

    std::vector<ItemStack> stacks = inv.getItemStacks();
    TestSupport::require(stacks.size() == 2, "only positive stacks are saved");
    TestSupport::require(stacks[0].itemId == "pixel_screw", "item stacks sort by id");
    TestSupport::require(stacks[1].itemId == "recovery_candy", "item stacks sort by id second");
}

void consumeItemsRespectsCounts() {
    Inventory inv;
    inv.addItem("trial_token", 3);

    TestSupport::require(inv.consumeItem("trial_token", 2), "consume available items succeeds");
    TestSupport::require(inv.getItemCount("trial_token") == 1, "consume subtracts count");
    TestSupport::require(!inv.consumeItem("trial_token", 2), "consume too many fails");
    TestSupport::require(inv.getItemCount("trial_token") == 1, "failed consume preserves count");
}

void loadItemStacksRestoresOnlyValidEntries() {
    Inventory inv;
    inv.loadItemStacks({
        {"color_battery", 2},
        {"", 5},
        {"pixel_screw", 0},
        {"old_button", 4}
    });

    TestSupport::require(inv.getItemCount("color_battery") == 2, "color battery loads");
    TestSupport::require(inv.getItemCount("old_button") == 4, "old button loads");
    TestSupport::require(inv.getItemCount("pixel_screw") == 0, "zero count does not load");
}

void itemCatalogKnowsFirstChapterItems() {
    const ItemDef* candy = ItemCatalog::find("recovery_candy");
    const ItemDef* token = ItemCatalog::find("trial_token");
    const ItemDef* relic = ItemCatalog::find("pixel_controller");

    TestSupport::require(candy != nullptr, "recovery candy definition exists");
    TestSupport::require(candy->category == ItemCategory::Consumable, "recovery candy category");
    TestSupport::require(candy->childlikeHeartDelta == 30.0f, "recovery candy restores heart");
    TestSupport::require(token != nullptr && token->category == ItemCategory::Story, "trial token is story item");
    TestSupport::require(relic != nullptr && relic->category == ItemCategory::Relic, "pixel controller is relic");
}

}  // namespace

int main() {
    itemStacksClampAndSort();
    consumeItemsRespectsCounts();
    loadItemStacksRestoresOnlyValidEntries();
    itemCatalogKnowsFirstChapterItems();
    return 0;
}
```

- [ ] **Step 2: Register and run the failing test**

Add to `CMakeLists.txt` inside `if(BUILD_TESTING)`:

```cmake
    add_executable(InventoryItemTest
        tests/InventoryItemTest.cpp
    )
    configure_starchild_target(InventoryItemTest)
    target_link_libraries(InventoryItemTest PRIVATE StarchildGame)
    add_test(NAME InventoryItemTest COMMAND InventoryItemTest)
```

Run:

```powershell
cmake --build build --config Release --target InventoryItemTest
```

Expected: build fails because `ItemCatalog.h`, `ItemStack`, and item methods do not exist.

- [ ] **Step 3: Add item catalog types**

Create `src/Game/Inventory/ItemCatalog.h`:

```cpp
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
```

Create `src/Game/Inventory/ItemCatalog.cpp`:

```cpp
#include "Game/Inventory/ItemCatalog.h"

#include <algorithm>
#include <array>

namespace {

const std::vector<ItemDef>& definitions() {
    static const std::vector<ItemDef> defs = {
        {"recovery_candy", "恢复糖", ItemCategory::Consumable,
            "小卖部味道的星星糖，恢复 30 点童心。", true, true, 0.0f, 30.0f, 0.0f},
        {"color_battery", "彩色电池", ItemCategory::Consumable,
            "让灰掉的世界重新亮一点，恢复 60 点童心并降低 10 点委屈。", true, true, 0.0f, 60.0f, -10.0f},
        {"trial_token", "试玩币", ItemCategory::Story,
            "弹窗游乐厅的试玩币，集齐三枚可开启 Boss 门。", false, false, 0.0f, 0.0f, 0.0f},
        {"pixel_screw", "像素螺丝", ItemCategory::Material,
            "从废铁工厂里找到的发光螺丝。", false, true, 0.0f, 0.0f, 0.0f},
        {"old_button", "旧按键", ItemCategory::Material,
            "旧游戏机上掉下来的按键。", false, true, 0.0f, 0.0f, 0.0f},
        {"pixel_controller", "像素手柄", ItemCategory::Relic,
            "第一章信物，安放到秘密基地后解锁试玩币训练机关。", false, false, 0.0f, 0.0f, 0.0f},
        {"no_pay_victory_sticker", "未充值也能赢贴纸", ItemCategory::HiddenCollectible,
            "没有被诱导按钮带跑的证明。", false, false, 0.0f, 0.0f, 0.0f},
        {"half_melody_arcade", "半截旋律：游乐厅", ItemCategory::HiddenCollectible,
            "隐藏章回声音乐厅的旋律碎片之一。", false, false, 0.0f, 0.0f, 0.0f}
    };
    return defs;
}

}  // namespace

namespace ItemCatalog {

const ItemDef* find(const std::string& itemId) {
    const auto& defs = definitions();
    auto it = std::find_if(defs.begin(), defs.end(),
        [&itemId](const ItemDef& def) { return def.id == itemId; });
    return it == defs.end() ? nullptr : &*it;
}

const std::vector<ItemDef>& all() {
    return definitions();
}

}  // namespace ItemCatalog
```

- [ ] **Step 4: Extend inventory**

Add to `Inventory.h`:

```cpp
struct ItemStack {
    std::string itemId;
    int count = 0;
};
```

Add public methods:

```cpp
int getItemCount(const std::string& itemId) const;
void setItemCount(const std::string& itemId, int count);
void addItem(const std::string& itemId, int count = 1);
bool consumeItem(const std::string& itemId, int count = 1);
std::vector<ItemStack> getItemStacks() const;
void loadItemStacks(const std::vector<ItemStack>& stacks);
```

Add private field:

```cpp
std::unordered_map<std::string, int> itemCounts;
```

Implement in `Inventory.cpp`:

```cpp
int Inventory::getItemCount(const std::string& itemId) const {
    auto it = itemCounts.find(itemId);
    return it == itemCounts.end() ? 0 : it->second;
}

void Inventory::setItemCount(const std::string& itemId, int count) {
    if (itemId.empty()) return;
    if (count <= 0) {
        itemCounts.erase(itemId);
    } else {
        itemCounts[itemId] = count;
    }
}

void Inventory::addItem(const std::string& itemId, int count) {
    if (itemId.empty() || count <= 0) return;
    setItemCount(itemId, getItemCount(itemId) + count);
}

bool Inventory::consumeItem(const std::string& itemId, int count) {
    if (itemId.empty() || count <= 0) return false;
    int current = getItemCount(itemId);
    if (current < count) return false;
    setItemCount(itemId, current - count);
    return true;
}

std::vector<ItemStack> Inventory::getItemStacks() const {
    std::vector<ItemStack> stacks;
    stacks.reserve(itemCounts.size());
    for (const auto& pair : itemCounts) {
        if (pair.second > 0) {
            stacks.push_back({pair.first, pair.second});
        }
    }
    std::sort(stacks.begin(), stacks.end(),
        [](const ItemStack& a, const ItemStack& b) { return a.itemId < b.itemId; });
    return stacks;
}

void Inventory::loadItemStacks(const std::vector<ItemStack>& stacks) {
    itemCounts.clear();
    for (const ItemStack& stack : stacks) {
        setItemCount(stack.itemId, stack.count);
    }
}
```

- [ ] **Step 5: Add new sources to CMake**

Add `src/Game/Inventory/ItemCatalog.cpp` to the `StarchildGame` source list.

- [ ] **Step 6: Run test**

Run:

```powershell
cmake --build build --config Release --target InventoryItemTest
build\Release\InventoryItemTest.exe
```

Expected: executable returns `0`.

- [ ] **Step 7: Commit**

```powershell
git add CMakeLists.txt src/Game/Inventory/ItemCatalog.h src/Game/Inventory/ItemCatalog.cpp src/Game/Inventory/Inventory.h src/Game/Inventory/Inventory.cpp tests/InventoryItemTest.cpp
git commit -m "阶段1-扩展通用背包物品"
```

---

## Task 2: 剧情进度与存档迁移

**Files:**

- Create: `src/Game/Progress/StoryProgress.h`
- Create: `src/Game/Progress/StoryProgress.cpp`
- Modify: `src/Game/GameState.h`
- Modify: `src/Game/Data/SaveData.h`
- Modify: `src/Game/Data/SaveMigration.h`
- Modify: `src/Game/Data/SaveMigration.cpp`
- Modify: `src/Game/Data/SaveSerializer.cpp`
- Modify: `src/Game/Services/SaveSnapshotBuilder.cpp`
- Modify: `src/Game/Services/SaveApplier.cpp`
- Create: `tests/StoryProgressTest.cpp`
- Modify: `tests/SaveDataRoundTripTest.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write story progress test**

Create `tests/StoryProgressTest.cpp`:

```cpp
#include "Game/Progress/StoryProgress.h"
#include "TestSupport.h"

namespace {

void chapterStatesAdvanceMonotonically() {
    StoryProgress progress;
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::Locked,
        "chapter 1 starts locked before explicit unlock");

    progress.unlockChapter("chapter_1_popup_arcade");
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::Unlocked,
        "chapter unlocks");

    progress.startChapter("chapter_1_popup_arcade");
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::InProgress,
        "chapter starts");

    progress.completeChapter("chapter_1_popup_arcade");
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::Completed,
        "chapter completes");

    progress.startChapter("chapter_1_popup_arcade");
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::Completed,
        "completed chapter does not downgrade");
}

void partnerAndFlagsPersistInSnapshot() {
    StoryProgress progress;
    progress.unlockPartner("tieyi");
    progress.setFlag("arcade_boss_defeated", true);
    progress.setFlag("bad_ending_risk", false);

    StoryProgressSnapshot snapshot = progress.getSnapshot();

    StoryProgress restored;
    restored.loadSnapshot(snapshot);

    TestSupport::require(restored.isPartnerUnlocked("tieyi"), "partner unlock restores");
    TestSupport::require(restored.getFlag("arcade_boss_defeated"), "true flag restores");
    TestSupport::require(!restored.getFlag("bad_ending_risk"), "false flag restores");
}

}  // namespace

int main() {
    chapterStatesAdvanceMonotonically();
    partnerAndFlagsPersistInSnapshot();
    return 0;
}
```

- [ ] **Step 2: Extend save round-trip test**

Add to `tests/SaveDataRoundTripTest.cpp` before serialization:

```cpp
    data.itemStacks.push_back({"recovery_candy", 2});
    data.itemStacks.push_back({"pixel_screw", 4});
    data.storyProgress.chapters.push_back({"chapter_1_popup_arcade", ChapterState::Completed});
    data.storyProgress.unlockedPartners.push_back("tieyi");
    data.storyProgress.flags.push_back({"arcade_boss_defeated", true});
```

Add after restore:

```cpp
    TestSupport::require(restored.itemStacks.size() == 2, "item stacks round trip");
    TestSupport::require(restored.itemStacks[0].itemId == "recovery_candy", "item id round trips");
    TestSupport::require(restored.itemStacks[0].count == 2, "item count round trips");
    TestSupport::require(restored.storyProgress.chapters.size() == 1, "chapter progress round trips");
    TestSupport::require(restored.storyProgress.chapters[0].state == ChapterState::Completed,
        "chapter state round trips");
    TestSupport::require(restored.storyProgress.unlockedPartners.size() == 1,
        "partner progress round trips");
    TestSupport::require(restored.storyProgress.flags.size() == 1, "story flag round trips");
```

- [ ] **Step 3: Register and run failing tests**

Add `StoryProgressTest` to `CMakeLists.txt`:

```cmake
    add_executable(StoryProgressTest
        tests/StoryProgressTest.cpp
    )
    configure_starchild_target(StoryProgressTest)
    target_link_libraries(StoryProgressTest PRIVATE StarchildGame)
    add_test(NAME StoryProgressTest COMMAND StoryProgressTest)
```

Run:

```powershell
cmake --build build --config Release --target StoryProgressTest
```

Expected: build fails because `StoryProgress` does not exist.

- [ ] **Step 4: Implement story progress types**

Create `src/Game/Progress/StoryProgress.h`:

```cpp
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class ChapterState : uint8_t {
    Locked,
    Unlocked,
    InProgress,
    Completed
};

struct ChapterProgressEntry {
    std::string chapterId;
    ChapterState state = ChapterState::Locked;
};

struct StoryFlagEntry {
    std::string flagId;
    bool value = false;
};

struct StoryProgressSnapshot {
    std::vector<ChapterProgressEntry> chapters;
    std::vector<std::string> unlockedPartners;
    std::vector<StoryFlagEntry> flags;
};

class StoryProgress {
public:
    ChapterState getChapterState(const std::string& chapterId) const;
    void unlockChapter(const std::string& chapterId);
    void startChapter(const std::string& chapterId);
    void completeChapter(const std::string& chapterId);

    bool isPartnerUnlocked(const std::string& partnerId) const;
    void unlockPartner(const std::string& partnerId);

    bool getFlag(const std::string& flagId) const;
    void setFlag(const std::string& flagId, bool value);

    StoryProgressSnapshot getSnapshot() const;
    void loadSnapshot(const StoryProgressSnapshot& snapshot);

private:
    std::unordered_map<std::string, ChapterState> chapters;
    std::unordered_set<std::string> partners;
    std::unordered_map<std::string, bool> flags;
};
```

Create `src/Game/Progress/StoryProgress.cpp`:

```cpp
#include "Game/Progress/StoryProgress.h"

#include <algorithm>

namespace {

bool isKnownStateAdvance(ChapterState current, ChapterState next) {
    return static_cast<int>(next) > static_cast<int>(current);
}

}  // namespace

ChapterState StoryProgress::getChapterState(const std::string& chapterId) const {
    auto it = chapters.find(chapterId);
    return it == chapters.end() ? ChapterState::Locked : it->second;
}

void StoryProgress::unlockChapter(const std::string& chapterId) {
    if (chapterId.empty()) return;
    ChapterState current = getChapterState(chapterId);
    if (isKnownStateAdvance(current, ChapterState::Unlocked)) {
        chapters[chapterId] = ChapterState::Unlocked;
    }
}

void StoryProgress::startChapter(const std::string& chapterId) {
    if (chapterId.empty()) return;
    ChapterState current = getChapterState(chapterId);
    if (isKnownStateAdvance(current, ChapterState::InProgress)) {
        chapters[chapterId] = ChapterState::InProgress;
    }
}

void StoryProgress::completeChapter(const std::string& chapterId) {
    if (chapterId.empty()) return;
    chapters[chapterId] = ChapterState::Completed;
}

bool StoryProgress::isPartnerUnlocked(const std::string& partnerId) const {
    return partners.find(partnerId) != partners.end();
}

void StoryProgress::unlockPartner(const std::string& partnerId) {
    if (!partnerId.empty()) {
        partners.insert(partnerId);
    }
}

bool StoryProgress::getFlag(const std::string& flagId) const {
    auto it = flags.find(flagId);
    return it != flags.end() && it->second;
}

void StoryProgress::setFlag(const std::string& flagId, bool value) {
    if (!flagId.empty()) {
        flags[flagId] = value;
    }
}

StoryProgressSnapshot StoryProgress::getSnapshot() const {
    StoryProgressSnapshot snapshot;
    for (const auto& pair : chapters) {
        snapshot.chapters.push_back({pair.first, pair.second});
    }
    std::sort(snapshot.chapters.begin(), snapshot.chapters.end(),
        [](const ChapterProgressEntry& a, const ChapterProgressEntry& b) {
            return a.chapterId < b.chapterId;
        });

    snapshot.unlockedPartners.assign(partners.begin(), partners.end());
    std::sort(snapshot.unlockedPartners.begin(), snapshot.unlockedPartners.end());

    for (const auto& pair : flags) {
        snapshot.flags.push_back({pair.first, pair.second});
    }
    std::sort(snapshot.flags.begin(), snapshot.flags.end(),
        [](const StoryFlagEntry& a, const StoryFlagEntry& b) {
            return a.flagId < b.flagId;
        });
    return snapshot;
}

void StoryProgress::loadSnapshot(const StoryProgressSnapshot& snapshot) {
    chapters.clear();
    partners.clear();
    flags.clear();

    for (const ChapterProgressEntry& chapter : snapshot.chapters) {
        if (!chapter.chapterId.empty()) {
            chapters[chapter.chapterId] = chapter.state;
        }
    }
    for (const std::string& partner : snapshot.unlockedPartners) {
        unlockPartner(partner);
    }
    for (const StoryFlagEntry& flag : snapshot.flags) {
        setFlag(flag.flagId, flag.value);
    }
}
```

- [ ] **Step 5: Add fields to GameState and SaveData**

In `GameState.h`, include:

```cpp
#include "Game/Progress/StoryProgress.h"
```

Add field:

```cpp
StoryProgress storyProgress;
```

In `SaveData.h`, include:

```cpp
#include "Game/Progress/StoryProgress.h"
```

Add fields:

```cpp
std::vector<ItemStack> itemStacks;
StoryProgressSnapshot storyProgress;
```

- [ ] **Step 6: Bump save version and serialize new fields**

Set `SaveMigration::kCurrentVersion` to `4`.

In `SaveSerializer::toJson`, add:

```cpp
    j["inventory"]["items"] = Json::array();
    for (const ItemStack& stack : data.itemStacks) {
        j["inventory"]["items"].push_back({
            {"itemId", stack.itemId},
            {"count", stack.count}
        });
    }

    j["story"]["chapters"] = Json::array();
    for (const ChapterProgressEntry& chapter : data.storyProgress.chapters) {
        j["story"]["chapters"].push_back({
            {"chapterId", chapter.chapterId},
            {"state", static_cast<int>(chapter.state)}
        });
    }
    j["story"]["unlockedPartners"] = data.storyProgress.unlockedPartners;
    j["story"]["flags"] = Json::array();
    for (const StoryFlagEntry& flag : data.storyProgress.flags) {
        j["story"]["flags"].push_back({
            {"flagId", flag.flagId},
            {"value", flag.value}
        });
    }
```

In `SaveSerializer::fromJson`, read these fields under the existing `inventory` and new `story` sections.

- [ ] **Step 7: Wire snapshot builder and applier**

In `SaveSnapshotBuilder.cpp`, after furniture stock:

```cpp
    data.itemStacks = gs.inventory.getItemStacks();
    data.storyProgress = gs.storyProgress.getSnapshot();
```

In `SaveApplier.cpp`, after furniture stock load:

```cpp
    gs.inventory.loadItemStacks(saveData.itemStacks);
    gs.storyProgress.loadSnapshot(saveData.storyProgress);
```

- [ ] **Step 8: Add CMake source**

Add `src/Game/Progress/StoryProgress.cpp` to `StarchildGame`.

- [ ] **Step 9: Run tests**

Run:

```powershell
cmake --build build --config Release --target StoryProgressTest
build\Release\StoryProgressTest.exe
cmake --build build --config Release --target SaveDataRoundTripTest
build\Release\SaveDataRoundTripTest.exe
```

Expected: both return `0`.

- [ ] **Step 10: Commit**

```powershell
git add CMakeLists.txt src/Game/GameState.h src/Game/Progress/StoryProgress.h src/Game/Progress/StoryProgress.cpp src/Game/Data/SaveData.h src/Game/Data/SaveMigration.h src/Game/Data/SaveMigration.cpp src/Game/Data/SaveSerializer.cpp src/Game/Services/SaveSnapshotBuilder.cpp src/Game/Services/SaveApplier.cpp tests/StoryProgressTest.cpp tests/SaveDataRoundTripTest.cpp
git commit -m "阶段1-保存剧情进度和背包物品"
```

---

## Task 3: 童心档位 helper 与技能参数表

**Files:**

- Modify: `src/Game/Emotion/EmotionSystem.h`
- Modify: `src/Game/Emotion/EmotionSystem.cpp`
- Create: `src/Game/Ability/ChildlikeSkillProfile.h`
- Create: `src/Game/Ability/ChildlikeSkillProfile.cpp`
- Create: `tests/EmotionTierTest.cpp`
- Create: `tests/ChildlikeSkillProfileTest.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write tier tests**

Create `tests/EmotionTierTest.cpp`:

```cpp
#include "Game/Emotion/EmotionSystem.h"
#include "TestSupport.h"

int main() {
    EmotionSystem emotion;

    emotion.setChildlikeHeart(0.0f);
    TestSupport::require(emotion.getChildlikeHeartTier() == ChildlikeHeartTier::Faded,
        "0 heart is faded");
    TestSupport::require(!emotion.canSeeHiddenPickups(), "faded cannot see hidden pickups");

    emotion.setChildlikeHeart(200.0f);
    TestSupport::require(emotion.getChildlikeHeartTier() == ChildlikeHeartTier::Normal,
        "200 heart is normal");

    emotion.setChildlikeHeart(500.0f);
    TestSupport::require(emotion.getChildlikeHeartTier() == ChildlikeHeartTier::Vivid,
        "500 heart is vivid");
    TestSupport::require(emotion.canSeeHiddenPickups(), "vivid can see hidden pickups");

    emotion.setChildlikeHeart(800.0f);
    TestSupport::require(emotion.getChildlikeHeartTier() == ChildlikeHeartTier::Radiant,
        "800 heart is radiant");
    TestSupport::require(emotion.getChildlikeHeartTierName() == std::string("绚烂"),
        "radiant tier name");
    return 0;
}
```

Create `tests/ChildlikeSkillProfileTest.cpp`:

```cpp
#include "Game/Ability/ChildlikeSkillProfile.h"
#include "TestSupport.h"

int main() {
    SkillTierProfile faded = ChildlikeSkillProfile::forTier(ChildlikeHeartTier::Faded);
    SkillTierProfile normal = ChildlikeSkillProfile::forTier(ChildlikeHeartTier::Normal);
    SkillTierProfile vivid = ChildlikeSkillProfile::forTier(ChildlikeHeartTier::Vivid);
    SkillTierProfile radiant = ChildlikeSkillProfile::forTier(ChildlikeHeartTier::Radiant);

    TestSupport::require(faded.fireName == std::string("打火机"), "faded fire name");
    TestSupport::require(normal.fireName == std::string("火球术"), "normal fire name");
    TestSupport::require(vivid.fireDamageMultiplier > normal.fireDamageMultiplier,
        "vivid fire stronger than normal");
    TestSupport::require(radiant.lightningMaxChains > vivid.lightningMaxChains,
        "radiant lightning chains increase");
    TestSupport::require(radiant.movementDistance > faded.movementDistance,
        "radiant movement distance increase");
    return 0;
}
```

- [ ] **Step 2: Register and run failing tests**

Add `EmotionTierTest` and `ChildlikeSkillProfileTest` to `CMakeLists.txt`, linking both with `StarchildGame`.

Run:

```powershell
cmake --build build --config Release --target EmotionTierTest
```

Expected: build fails because tier APIs do not exist.

- [ ] **Step 3: Add tier enum and methods**

In `EmotionSystem.h`, add:

```cpp
#include <string>

enum class ChildlikeHeartTier : uint8_t {
    Faded,
    Normal,
    Vivid,
    Radiant
};
```

Add public methods:

```cpp
ChildlikeHeartTier getChildlikeHeartTier() const;
std::string getChildlikeHeartTierName() const;
bool canSeeHiddenPickups() const;
```

In `EmotionSystem.cpp`, implement:

```cpp
ChildlikeHeartTier EmotionSystem::getChildlikeHeartTier() const {
    if (state.childlikeHeart >= 800.0f) return ChildlikeHeartTier::Radiant;
    if (state.childlikeHeart >= 500.0f) return ChildlikeHeartTier::Vivid;
    if (state.childlikeHeart >= 200.0f) return ChildlikeHeartTier::Normal;
    return ChildlikeHeartTier::Faded;
}

std::string EmotionSystem::getChildlikeHeartTierName() const {
    switch (getChildlikeHeartTier()) {
        case ChildlikeHeartTier::Faded: return "失色";
        case ChildlikeHeartTier::Normal: return "寻常";
        case ChildlikeHeartTier::Vivid: return "鲜活";
        case ChildlikeHeartTier::Radiant: return "绚烂";
        default: return "?";
    }
}

bool EmotionSystem::canSeeHiddenPickups() const {
    ChildlikeHeartTier tier = getChildlikeHeartTier();
    return tier == ChildlikeHeartTier::Vivid || tier == ChildlikeHeartTier::Radiant;
}
```

- [ ] **Step 4: Add skill profile**

Create `src/Game/Ability/ChildlikeSkillProfile.h`:

```cpp
#pragma once

#include "Game/Emotion/EmotionSystem.h"

#include <string>

struct SkillTierProfile {
    std::string fireName;
    std::string iceName;
    std::string lightningName;
    std::string shieldName;
    std::string movementName;
    float fireDamageMultiplier = 1.0f;
    float iceDamageMultiplier = 1.0f;
    float projectileSpeedMultiplier = 1.0f;
    float projectileRadiusMultiplier = 1.0f;
    float iceSlowMultiplier = 0.4f;
    int lightningMaxChains = 3;
    float shieldDamageReduction = 0.4f;
    float movementDistance = 6.0f;
    float movementCooldown = 2.5f;
};

namespace ChildlikeSkillProfile {

SkillTierProfile forTier(ChildlikeHeartTier tier);

}  // namespace ChildlikeSkillProfile
```

Create `src/Game/Ability/ChildlikeSkillProfile.cpp`:

```cpp
#include "Game/Ability/ChildlikeSkillProfile.h"

namespace ChildlikeSkillProfile {

SkillTierProfile forTier(ChildlikeHeartTier tier) {
    switch (tier) {
        case ChildlikeHeartTier::Faded:
            return {"打火机", "冷风", "静电", "抱头", "快步",
                0.6f, 0.5f, 0.75f, 0.85f, 0.75f, 1, 0.20f, 3.0f, 3.0f};
        case ChildlikeHeartTier::Normal:
            return {"火球术", "冰锥刺", "闪电链", "护盾", "飞行",
                1.0f, 1.0f, 1.0f, 1.0f, 0.55f, 3, 0.40f, 6.0f, 2.5f};
        case ChildlikeHeartTier::Vivid:
            return {"流星坠", "霜花爆裂", "雷暴交响", "星辉屏障", "星翼滑翔",
                1.4f, 1.5f, 1.18f, 1.2f, 0.35f, 5, 0.60f, 10.0f, 2.0f};
        case ChildlikeHeartTier::Radiant:
            return {"星愿焰火", "童心冻结", "童心共鸣雷", "童心圣域", "星愿翱翔",
                2.0f, 2.2f, 1.35f, 1.45f, 0.20f, 8, 0.80f, 15.0f, 1.5f};
        default:
            return forTier(ChildlikeHeartTier::Normal);
    }
}

}  // namespace ChildlikeSkillProfile
```

- [ ] **Step 5: Add CMake source and tests**

Add `src/Game/Ability/ChildlikeSkillProfile.cpp` to `StarchildGame`. Add both new tests to `if(BUILD_TESTING)`.

- [ ] **Step 6: Run tests**

Run:

```powershell
cmake --build build --config Release --target EmotionTierTest
build\Release\EmotionTierTest.exe
cmake --build build --config Release --target ChildlikeSkillProfileTest
build\Release\ChildlikeSkillProfileTest.exe
```

Expected: both return `0`.

- [ ] **Step 7: Commit**

```powershell
git add CMakeLists.txt src/Game/Emotion/EmotionSystem.h src/Game/Emotion/EmotionSystem.cpp src/Game/Ability/ChildlikeSkillProfile.h src/Game/Ability/ChildlikeSkillProfile.cpp tests/EmotionTierTest.cpp tests/ChildlikeSkillProfileTest.cpp
git commit -m "阶段1-加入童心档位技能参数"
```

---

## Task 4: 目标列表任务系统

**Files:**

- Modify: `src/Game/Quest/QuestTypes.h`
- Modify: `src/Game/Quest/QuestSystem.h`
- Modify: `src/Game/Quest/QuestSystem.cpp`
- Modify: `src/Game/Data/QuestDefinitionLoader.cpp`
- Modify: `assets/scripts/quests.lua`
- Modify: `src/Game/Services/ProgressionUpdateService.h`
- Modify: `src/Game/Services/ProgressionUpdateService.cpp`
- Create: `tests/QuestObjectiveFlowTest.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write objective flow test**

Create `tests/QuestObjectiveFlowTest.cpp`:

```cpp
#include "Game/Quest/QuestSystem.h"
#include "TestSupport.h"

namespace {

QuestDef makeCollectQuest() {
    QuestDef quest;
    quest.id = "arcade_trial_tokens";
    quest.name = "找回试玩币";
    quest.objectives.push_back({"collect", "trial_token", 3});
    quest.objectives.push_back({"defeat", "popup_bubble", 5});
    quest.reward.questId = quest.id;
    quest.reward.completed = true;
    quest.reward.coins = 20;
    quest.reward.itemRewards.push_back({"old_button", 2});
    return quest;
}

void objectiveQuestTracksProgressAndRewardsOnce() {
    QuestSystem quests;
    quests.initWithDefinitions({makeCollectQuest()});

    QuestSnapshot snapshot;
    snapshot.facts.push_back({"collect", "trial_token", 2});
    snapshot.facts.push_back({"defeat", "popup_bubble", 5});

    auto first = quests.update(snapshot);
    TestSupport::require(first.empty(), "partial objective quest does not reward");
    TestSupport::require(quests.getQuestState("arcade_trial_tokens") == QuestState::Active,
        "partial objective quest becomes active");

    snapshot.facts[0].count = 3;
    auto second = quests.update(snapshot);
    auto third = quests.update(snapshot);

    TestSupport::require(second.size() == 1, "completed objective quest rewards");
    TestSupport::require(second.front().coins == 20, "objective quest coin reward");
    TestSupport::require(second.front().itemRewards.size() == 1, "objective quest item reward");
    TestSupport::require(second.front().itemRewards[0].itemId == "old_button", "objective item id");
    TestSupport::require(third.empty(), "objective quest rewards once");
}

}  // namespace

int main() {
    objectiveQuestTracksProgressAndRewardsOnce();
    return 0;
}
```

- [ ] **Step 2: Register and run failing test**

Add `QuestObjectiveFlowTest` to `CMakeLists.txt`.

Run:

```powershell
cmake --build build --config Release --target QuestObjectiveFlowTest
```

Expected: build fails because objective definitions and item rewards do not exist.

- [ ] **Step 3: Extend quest types**

In `QuestTypes.h`, add:

```cpp
struct QuestFact {
    std::string type;
    std::string targetId;
    int count = 0;
};

struct QuestObjectiveDef {
    std::string type;
    std::string targetId;
    int required = 1;
};

struct QuestItemReward {
    std::string itemId;
    int count = 0;
};
```

Add to `QuestSnapshot`:

```cpp
std::string currentRegionId;
std::vector<QuestFact> facts;
```

Add to `QuestReward`:

```cpp
std::vector<QuestItemReward> itemRewards;
float maxChildlikeHeart = 0.0f;
std::string storyFlag;
```

Add to `QuestDef`:

```cpp
std::vector<QuestObjectiveDef> objectives;
bool updateOutsideHomeBase = true;
```

- [ ] **Step 4: Add definition injection for tests**

In `QuestSystem.h`, add:

```cpp
void initWithDefinitions(const std::vector<QuestDef>& questDefs);
```

In `QuestSystem.cpp`:

```cpp
void QuestSystem::initWithDefinitions(const std::vector<QuestDef>& questDefs) {
    completedQuests.clear();
    definitions = questDefs;
    questEntries.clear();
    for (const QuestDef& quest : definitions) {
        ensureEntry(quest);
    }
}
```

- [ ] **Step 5: Update satisfaction and objective progress**

Update `isSatisfied`:

```cpp
    if (!quest.objectives.empty()) {
        for (const QuestObjectiveDef& objective : quest.objectives) {
            int current = 0;
            for (const QuestFact& fact : snapshot.facts) {
                if (fact.type == objective.type && fact.targetId == objective.targetId) {
                    current = std::max(current, fact.count);
                }
            }
            if (current < objective.required) return false;
        }
    }
```

Keep existing furniture/weather checks so current Stage 7 tests continue to pass.

Update `buildObjectives` to append objective-list progress:

```cpp
    for (const QuestObjectiveDef& objectiveDef : quest.objectives) {
        int current = 0;
        for (const QuestFact& fact : snapshot.facts) {
            if (fact.type == objectiveDef.type && fact.targetId == objectiveDef.targetId) {
                current = std::max(current, fact.count);
            }
        }
        objectives.push_back({
            objectiveDef.type,
            objectiveDef.targetId,
            objectiveDef.required,
            current
        });
    }
```

- [ ] **Step 6: Allow non-home quests**

Replace the early return in `QuestSystem::update`:

```cpp
    if (!snapshot.inHomeBase) return rewards;
```

with per-quest gating:

```cpp
        if (!snapshot.inHomeBase && !quest.updateOutsideHomeBase) continue;
```

Set legacy default base quests to `updateOutsideHomeBase = false` in `createDefaultQuestDefs`.

- [ ] **Step 7: Apply item rewards**

In `ProgressionUpdateService::applyQuestReward`, after coins:

```cpp
    for (const QuestItemReward& itemReward : reward.itemRewards) {
        context.inventory.addItem(itemReward.itemId, itemReward.count);
    }
```

Do not apply `storyFlag` or `maxChildlikeHeart` in this task. Task 9 adds `StoryProgress& storyProgress` to `ProgressionUpdateService::Context` and applies those fields when pickup and chapter progression facts are wired.

- [ ] **Step 8: Update Lua loader**

Teach `QuestDefinitionLoader.cpp` to read:

```lua
objectives = {
  { type = "collect", target = "trial_token", required = 3 },
  { type = "defeat", target = "popup_bubble", required = 5 },
}
```

and:

```lua
reward = {
  itemRewards = {
    { item = "old_button", count = 2 }
  }
}
```

- [ ] **Step 9: Replace first-round quests config**

Update `assets/scripts/quests.lua` to include the first-round quest chain from the design:

```lua
return {
  prologue_star_candy = {
    name = "捡到星星糖",
    objectives = {
      { type = "interact", target = "star_candy", required = 1 },
      { type = "enter_region", target = "real_street_prologue", required = 1 },
    },
    reward = { itemRewards = { { item = "old_game_coin", count = 1 } } }
  },
  arcade_trial_tokens = {
    name = "找回试玩币",
    objectives = {
      { type = "collect", target = "trial_token", required = 3 },
      { type = "defeat", target = "popup_bubble", required = 5 },
    },
    reward = {
      coins = 20,
      itemRewards = { { item = "old_button", count = 2 } }
    }
  }
}
```

Keep the existing base furniture quests in the file until the vertical slice uses the new quest chain completely.

- [ ] **Step 10: Run quest tests**

Run:

```powershell
cmake --build build --config Release --target QuestSystemFlowTest
build\Release\QuestSystemFlowTest.exe
cmake --build build --config Release --target QuestObjectiveFlowTest
build\Release\QuestObjectiveFlowTest.exe
```

Expected: both return `0`.

- [ ] **Step 11: Commit**

```powershell
git add CMakeLists.txt src/Game/Quest/QuestTypes.h src/Game/Quest/QuestSystem.h src/Game/Quest/QuestSystem.cpp src/Game/Data/QuestDefinitionLoader.cpp assets/scripts/quests.lua src/Game/Services/ProgressionUpdateService.h src/Game/Services/ProgressionUpdateService.cpp tests/QuestObjectiveFlowTest.cpp
git commit -m "阶段1-扩展任务目标系统"
```

---

## Task 5: 童心档位接入战斗技能

**Files:**

- Modify: `src/Game/Ability/Projectile.h`
- Modify: `src/Game/Ability/Projectile.cpp`
- Modify: `src/Game/Ability/Lightning.h`
- Modify: `src/Game/Ability/Lightning.cpp`
- Modify: `src/Game/Services/CombatService.h`
- Modify: `src/Game/Services/CombatService.cpp`
- Modify: `src/Game/Services/CombatCollisionService.h`
- Modify: `src/Game/Services/CombatCollisionService.cpp`
- Modify: `src/Game/Controllers/AbilityInputController.cpp`
- Modify: `tests/CombatCastServiceTest.cpp`

- [ ] **Step 1: Extend combat cast test**

In `tests/CombatCastServiceTest.cpp`, add `ChildlikeHeartTier tier = ChildlikeHeartTier::Normal;` to `Fixture`, include `Game/Emotion/EmotionSystem.h`, and add `tier` to `CastContext`.

Add test:

```cpp
void radiantTierIncreasesProjectileDamageAndRadius() {
    Fixture fixture;
    fixture.tier = ChildlikeHeartTier::Radiant;
    CombatService::CastContext context = fixture.makeCastContext();

    bool cast = CombatService::tryCastProjectile(
        context,
        ProjectileType::Fireball,
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f));

    TestSupport::require(cast, "radiant fireball cast succeeds");
    const auto& projectiles = fixture.projectileManager.getActive();
    TestSupport::require(projectiles.size() == 1, "radiant fireball creates projectile");
    TestSupport::require(projectiles.front().damage == 50.0f, "radiant fireball doubles base damage");
    TestSupport::require(projectiles.front().radius > 0.17f, "radiant fireball radius grows");
}
```

Call it from `main()`.

- [ ] **Step 2: Run failing combat test**

Run:

```powershell
cmake --build build --config Release --target CombatCastServiceTest
```

Expected: build fails because `CastContext` lacks tier data.

- [ ] **Step 3: Add tier to cast context**

In `CombatService.h`, include `Game/Emotion/EmotionSystem.h` and add:

```cpp
ChildlikeHeartTier childlikeTier = ChildlikeHeartTier::Normal;
```

to `CastContext`.

In `CombatService::makeCastContext`, set:

```cpp
gs.emotionSystem.getChildlikeHeartTier()
```

Update all test construction sites to pass a tier.

- [ ] **Step 4: Apply skill profile in projectile casting**

In `CombatService.cpp`, include:

```cpp
#include "Game/Ability/ChildlikeSkillProfile.h"
```

Use profile:

```cpp
    SkillTierProfile profile = ChildlikeSkillProfile::forTier(context.childlikeTier);
    float damage = 25.0f * profile.fireDamageMultiplier;
    float speed = 18.0f * profile.projectileSpeedMultiplier;
    float radiusScale = profile.projectileRadiusMultiplier;

    if (type == ProjectileType::IceSpike) {
        damage = 20.0f * profile.iceDamageMultiplier;
        speed = 14.0f * profile.projectileSpeedMultiplier;
        particleColor = glm::vec3(0.4f, 0.7f, 1.0f);
    }
```

Add a new `radiusScale` argument to `ProjectileManager::fire`.

- [ ] **Step 5: Extend projectile creation**

Change signature in `Projectile.h`:

```cpp
ProjectileId fire(b2WorldId world, const glm::vec2& pos,
                  const glm::vec2& dir, ProjectileType type,
                  float damage, float speed, b2BodyId owner,
                  float radiusScale = 1.0f);
```

In `Projectile.cpp`, after type radius is assigned:

```cpp
proj.radius *= std::max(0.1f, radiusScale);
```

The Box2D shape radius remains the base collision radius in this task. Later tuning can align visual and physical radius after gameplay validation.

- [ ] **Step 6: Apply tier to lightning**

In `tryCastLightning`, use profile:

```cpp
SkillTierProfile profile = ChildlikeSkillProfile::forTier(context.childlikeTier);
int maxChains = profile.lightningMaxChains;
```

Loop to `maxChains` instead of `context.lightning.getMaxChains()`.

- [ ] **Step 7: Apply tier to ice slow**

Add `ChildlikeHeartTier childlikeTier` to `CombatCollisionService::Context`, set it in `makeContext`, and in ice hit:

```cpp
SkillTierProfile profile = ChildlikeSkillProfile::forTier(context.childlikeTier);
const_cast<Enemy*>(enemy)->applySlow(3.0f, profile.iceSlowMultiplier);
```

- [ ] **Step 8: Run combat tests**

Run:

```powershell
cmake --build build --config Release --target CombatCastServiceTest
build\Release\CombatCastServiceTest.exe
cmake --build build --config Release --target CombatCollisionServiceTest
build\Release\CombatCollisionServiceTest.exe
```

Expected: both return `0`.

- [ ] **Step 9: Commit**

```powershell
git add src/Game/Ability/Projectile.h src/Game/Ability/Projectile.cpp src/Game/Ability/Lightning.h src/Game/Ability/Lightning.cpp src/Game/Services/CombatService.h src/Game/Services/CombatService.cpp src/Game/Services/CombatCollisionService.h src/Game/Services/CombatCollisionService.cpp src/Game/Controllers/AbilityInputController.cpp tests/CombatCastServiceTest.cpp
git commit -m "阶段1-接入童心档位战斗参数"
```

---

## Task 6: HUD 与游戏内菜单状态

**Files:**

- Modify: `src/Game/State/GameUiState.h`
- Modify: `src/Game/Presentation/HudView.h`
- Modify: `src/Game/Presentation/HudView.cpp`
- Modify: `src/Game/Presentation/PresentationModelBuilder.h`
- Modify: `src/Game/Presentation/PresentationModelBuilder.cpp`
- Create: `src/Game/Presentation/GameMenuView.h`
- Create: `src/Game/Presentation/GameMenuView.cpp`
- Create: `src/Game/Presentation/QuestLogView.h`
- Create: `src/Game/Presentation/QuestLogView.cpp`
- Create: `src/Game/Presentation/CharacterPanelView.h`
- Create: `src/Game/Presentation/CharacterPanelView.cpp`
- Create: `src/Game/Presentation/InventoryView.h`
- Create: `src/Game/Presentation/InventoryView.cpp`
- Modify: `src/Game/Presentation/GameRenderer.cpp`
- Modify: `src/Game/Controllers/InputController.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Add UI state**

In `GameUiState.h`, add:

```cpp
enum class GameMenuPage {
    Quest,
    Character,
    Inventory,
    Partners,
    System
};

bool gameMenuOpen = false;
GameMenuPage gameMenuPage = GameMenuPage::Quest;
int inventorySelection = 0;
int questSelection = 0;
```

- [ ] **Step 2: Toggle menu in input**

In `InputController.cpp`, update `handleKeyDown`:

```cpp
    if (scancode == SDL_SCANCODE_TAB) {
        gs.ui.gameMenuOpen = !gs.ui.gameMenuOpen;
        AudioService::playUiSfx(gs.audioSystem, gs.ui.gameMenuOpen ? "confirm" : "cancel");
        return true;
    }

    if (scancode == SDL_SCANCODE_ESCAPE) {
        if (gs.ui.gameMenuOpen) {
            gs.ui.gameMenuOpen = false;
            AudioService::playUiSfx(gs.audioSystem, "cancel");
            return true;
        }
        ...
    }
```

When menu is open, ignore gameplay actions in `AbilityInputController` by adding this field to `AbilityInputController::Context`:

```cpp
bool gameMenuOpen = false;
```

Set it in `AbilityInputController::makeContext` from `gs.ui.gameMenuOpen`, and update `isGameplayActionAllowed`:

```cpp
return !context.isDead &&
       !context.gameMenuOpen &&
       !context.buildingSystem.isActive() &&
       !context.toySystem.isMiniCarActive();
```

- [ ] **Step 3: Extend HUD model**

In `HudView::Model`, add:

```cpp
std::string childlikeTierName;
std::string trackedQuestText;
std::string fireSkillName;
std::string iceSkillName;
std::string lightningSkillName;
std::string shieldSkillName;
std::string movementSkillName;
```

In `PresentationModelBuilder::buildHudModel`, set:

```cpp
SkillTierProfile profile = ChildlikeSkillProfile::forTier(gs.emotionSystem.getChildlikeHeartTier());
model.childlikeTierName = gs.emotionSystem.getChildlikeHeartTierName();
model.fireSkillName = profile.fireName;
model.iceSkillName = profile.iceName;
model.lightningSkillName = profile.lightningName;
model.shieldSkillName = profile.shieldName;
model.movementSkillName = profile.movementName;
model.trackedQuestText = gs.questSystem.getTrackedQuestText();
```

Add `QuestSystem::getTrackedQuestText()` returning the first active quest with first incomplete objective formatted as `任务名: target current/required`.

- [ ] **Step 4: Render HUD text**

In `HudView.cpp`, render the tier and tracked quest:

```cpp
    TextRenderer::drawText(uiProj, 24.0f, screenH - 154.0f,
        "童心档位: " + model.childlikeTierName,
        15, glm::vec3(0.86f, 0.92f, 1.0f), 0.95f);

    if (!model.trackedQuestText.empty()) {
        TextRenderer::drawText(uiProj, screenW - 360.0f, screenH - 82.0f,
            model.trackedQuestText,
            15, glm::vec3(1.0f, 0.88f, 0.48f), 0.95f);
    }
```

Render skill names below slots with 10-12 px text. Use short names already provided by `SkillTierProfile`.

- [ ] **Step 5: Create menu view shell**

Create `GameMenuView.h`:

```cpp
#pragma once

#include "Game/State/GameUiState.h"

#include <string>
#include <vector>

namespace GameMenuView {

struct Model {
    bool open = false;
    GameMenuPage page = GameMenuPage::Quest;
    std::vector<std::string> questLines;
    std::vector<std::string> characterLines;
    std::vector<std::string> inventoryLines;
    std::vector<std::string> partnerLines;
    std::vector<std::string> systemLines;
};

void render(const Model& model, int screenW, int screenH);

}  // namespace GameMenuView
```

Create `GameMenuView.cpp` to draw a full-screen translucent panel, left tabs, and lines from the active page using `Draw2D` and `TextRenderer`.

Use this implementation for `src/Game/Presentation/GameMenuView.cpp`:

```cpp
#include "Game/Presentation/GameMenuView.h"

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/TextRenderer.h"

#include <algorithm>
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

namespace {

const char* pageName(GameMenuPage page) {
    switch (page) {
        case GameMenuPage::Quest: return "任务";
        case GameMenuPage::Character: return "属性";
        case GameMenuPage::Inventory: return "背包";
        case GameMenuPage::Partners: return "伙伴";
        case GameMenuPage::System: return "系统";
        default: return "?";
    }
}

const std::vector<std::string>& activeLines(const GameMenuView::Model& model) {
    switch (model.page) {
        case GameMenuPage::Quest: return model.questLines;
        case GameMenuPage::Character: return model.characterLines;
        case GameMenuPage::Inventory: return model.inventoryLines;
        case GameMenuPage::Partners: return model.partnerLines;
        case GameMenuPage::System: return model.systemLines;
        default: return model.systemLines;
    }
}

}  // namespace

namespace GameMenuView {

void render(const Model& model, int screenW, int screenH) {
    if (!model.open) return;

    float w = static_cast<float>(std::max(screenW, 800));
    float h = static_cast<float>(std::max(screenH, 600));
    glm::mat4 uiProj = glm::ortho(0.0f, w, 0.0f, h);

    Draw2D::beginFrame(uiProj);
    Draw2D::drawRectFilled(0.0f, 0.0f, w, h, glm::vec3(0.02f, 0.025f, 0.035f), 0.78f);

    float panelX = 72.0f;
    float panelY = 64.0f;
    float panelW = w - 144.0f;
    float panelH = h - 128.0f;
    Draw2D::drawRectFilled(panelX, panelY, panelW, panelH, glm::vec3(0.035f, 0.045f, 0.060f), 0.94f);
    Draw2D::drawRect(panelX, panelY, panelW, panelH, glm::vec3(0.28f, 0.55f, 0.88f), 2.0f, 0.88f);
    Draw2D::drawRectFilled(panelX, panelY + panelH - 5.0f, panelW, 5.0f, glm::vec3(1.0f, 0.76f, 0.28f), 0.9f);

    std::array<GameMenuPage, 5> pages = {
        GameMenuPage::Quest,
        GameMenuPage::Character,
        GameMenuPage::Inventory,
        GameMenuPage::Partners,
        GameMenuPage::System
    };

    float tabX = panelX + 24.0f;
    float tabY = panelY + panelH - 70.0f;
    for (GameMenuPage page : pages) {
        bool selected = page == model.page;
        glm::vec3 color = selected ? glm::vec3(0.20f, 0.45f, 0.78f) : glm::vec3(0.07f, 0.09f, 0.12f);
        Draw2D::drawRectFilled(tabX, tabY, 112.0f, 34.0f, color, selected ? 0.98f : 0.74f);
        Draw2D::drawRect(tabX, tabY, 112.0f, 34.0f,
            selected ? glm::vec3(0.82f, 0.92f, 1.0f) : glm::vec3(0.24f, 0.30f, 0.36f),
            1.5f, 0.85f);
        tabX += 124.0f;
    }
    Draw2D::endFrame();

    TextRenderer::drawText(uiProj, panelX + 26.0f, panelY + panelH - 42.0f,
        "星愿菜单", 24, glm::vec3(1.0f, 0.88f, 0.48f), 0.98f);

    tabX = panelX + 36.0f;
    for (GameMenuPage page : pages) {
        TextRenderer::drawText(uiProj, tabX, tabY + 9.0f,
            pageName(page), 16,
            page == model.page ? glm::vec3(1.0f) : glm::vec3(0.74f, 0.82f, 0.88f),
            0.98f);
        tabX += 124.0f;
    }

    float lineY = panelY + panelH - 118.0f;
    const std::vector<std::string>& lines = activeLines(model);
    for (const std::string& line : lines) {
        TextRenderer::drawText(uiProj, panelX + 36.0f, lineY,
            line, 17, glm::vec3(0.90f, 0.96f, 1.0f), 0.96f);
        lineY -= 26.0f;
        if (lineY < panelY + 34.0f) break;
    }
}

}  // namespace GameMenuView
```

Create minimal page-specific files so the CMake entries compile and can be expanded later:

`src/Game/Presentation/QuestLogView.h`

```cpp
#pragma once
namespace QuestLogView { void reservedForQuestLogView(); }
```

`src/Game/Presentation/QuestLogView.cpp`

```cpp
#include "Game/Presentation/QuestLogView.h"
namespace QuestLogView { void reservedForQuestLogView() {} }
```

`src/Game/Presentation/CharacterPanelView.h`

```cpp
#pragma once
namespace CharacterPanelView { void reservedForCharacterPanelView(); }
```

`src/Game/Presentation/CharacterPanelView.cpp`

```cpp
#include "Game/Presentation/CharacterPanelView.h"
namespace CharacterPanelView { void reservedForCharacterPanelView() {} }
```

`src/Game/Presentation/InventoryView.h`

```cpp
#pragma once
namespace InventoryView { void reservedForInventoryView(); }
```

`src/Game/Presentation/InventoryView.cpp`

```cpp
#include "Game/Presentation/InventoryView.h"
namespace InventoryView { void reservedForInventoryView() {} }
```

- [ ] **Step 6: Build menu model**

Add to `PresentationModelBuilder.h`:

```cpp
GameMenuView::Model buildGameMenuModel(const GameState& gs);
```

In `PresentationModelBuilder.cpp`, fill:

- Quest lines from `gs.questSystem.getSaveData()`.
- Character lines from health, childlike heart, tier, grievance, speed.
- Inventory lines from `gs.inventory.getItemStacks()` and `ItemCatalog::find`.
- Partner lines for Alya and Tieyi based on `gs.storyProgress`.
- System lines: `F5 保存`, `F9 读取`, `Esc/Tab 返回`.

- [ ] **Step 7: Render menu in game renderer**

In `GameRenderer.cpp`, after HUD rendering, call:

```cpp
GameMenuView::render(PresentationModelBuilder::buildGameMenuModel(gs),
    gs.screenWidth, gs.screenHeight);
```

Use the current render architecture location where `HudView::render` is called.

- [ ] **Step 8: Add CMake sources**

Add new presentation `.cpp` files to `StarchildGame`.

- [ ] **Step 9: Build**

Run:

```powershell
cmake --build build --config Release
```

Expected: build succeeds.

- [ ] **Step 10: Manual check**

Run from `build`:

```powershell
cd build
.\Release\Starchild2D.exe
```

Manual expected:

- `Tab` opens and closes menu.
- `Esc` closes menu before other escape behavior.
- HUD shows childlike tier and skill names.
- Gameplay attack keys do not fire while menu is open.

- [ ] **Step 11: Commit**

```powershell
git add CMakeLists.txt src/Game/State/GameUiState.h src/Game/Presentation/HudView.h src/Game/Presentation/HudView.cpp src/Game/Presentation/PresentationModelBuilder.h src/Game/Presentation/PresentationModelBuilder.cpp src/Game/Presentation/GameMenuView.h src/Game/Presentation/GameMenuView.cpp src/Game/Presentation/QuestLogView.h src/Game/Presentation/QuestLogView.cpp src/Game/Presentation/CharacterPanelView.h src/Game/Presentation/CharacterPanelView.cpp src/Game/Presentation/InventoryView.h src/Game/Presentation/InventoryView.cpp src/Game/Presentation/GameRenderer.cpp src/Game/Controllers/InputController.cpp
git commit -m "阶段2-加入游戏内菜单和童心HUD"
```

---

## Task 7: 正式区域 ID 与序章/基地/第一章地图

**Files:**

- Modify: `src/Game/World/RegionFactory.cpp`
- Modify: `src/Game/Services/RegionService.cpp`
- Create: `tests/RegionFactoryNostalgiaTest.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write region test**

Create `tests/RegionFactoryNostalgiaTest.cpp`:

```cpp
#include "Game/World/RegionFactory.h"
#include "TestSupport.h"

namespace {

bool hasPoi(const MapRegion& region, const std::string& id) {
    for (const PointOfInterest& poi : region.getPOIs()) {
        if (poi.id == id) return true;
    }
    return false;
}

bool hasConnectionTo(const MapRegion& region, const std::string& targetId) {
    for (const MapConnection& connection : region.getConnections()) {
        if (connection.targetRegionId == targetId) return true;
    }
    return false;
}

}  // namespace

int main() {
    auto prologue = RegionFactory::createRegion("real_street_prologue");
    TestSupport::require(prologue->getName() == "小卖部门口", "prologue region name");
    TestSupport::require(hasPoi(*prologue, "star_candy"), "prologue has star candy POI");
    TestSupport::require(hasConnectionTo(*prologue, "home_base"), "prologue connects to home base");

    auto base = RegionFactory::createRegion("home_base");
    TestSupport::require(hasPoi(*base, "base_map_table"), "home base has map table");
    TestSupport::require(hasPoi(*base, "save_bed"), "home base has save bed");
    TestSupport::require(hasConnectionTo(*base, "popup_arcade"), "home base connects to popup arcade");

    auto arcade = RegionFactory::createRegion("popup_arcade");
    TestSupport::require(arcade->getName() == "弹窗游乐厅", "arcade region name");
    TestSupport::require(hasPoi(*arcade, "arcade_boss_door"), "arcade has boss door");
    TestSupport::require(hasPoi(*arcade, "tieyi_cage"), "arcade has Tieyi cage");
    return 0;
}
```

- [ ] **Step 2: Register and run failing test**

Add `RegionFactoryNostalgiaTest` to `CMakeLists.txt`.

Run:

```powershell
cmake --build build --config Release --target RegionFactoryNostalgiaTest
```

Expected: test fails because region IDs and POIs are not configured.

- [ ] **Step 3: Add region specs**

In `RegionFactory.cpp`, update `getRegionSpec`:

```cpp
    if (regionId == "real_street_prologue") {
        return {"小卖部门口", RegionType::Overworld, 36, 28};
    }
    if (regionId == "home_base") {
        return {"秘密基地", RegionType::Indoor, 24, 18};
    }
    if (regionId == "popup_arcade") {
        return {"弹窗游乐厅", RegionType::Dungeon, 60, 60};
    }
```

Add seeds for these IDs.

- [ ] **Step 4: Configure prologue**

Add `configureRealStreetPrologue(MapRegion& region)`:

- Lay a path from shop entrance to crack.
- Add POIs:
  - `star_candy`
  - `shopkeeper_shadow`
  - `childhood_crack`
- Add connection to `home_base`.

Use `TileType::Path`, `TileType::Grass`, `TileType::Stone`, and `TileType::Portal`.

- [ ] **Step 5: Expand home base**

Update `configureHomeBase`:

- Add POIs:
  - `base_exit`
  - `base_map_table`
  - `save_bed`
  - `pixel_controller_spot`
  - `arcade_gate`
- Add connection from base to `popup_arcade`.
- Keep existing furniture floor and build area.

- [ ] **Step 6: Configure popup arcade**

Add `configurePopupArcade(MapRegion& region)`:

- South entrance hall.
- Central trial machine area.
- East scrap factory.
- North boss arena.
- POIs:
  - `trapped_player_shadow`
  - `popup_vendor`
  - `trial_token_1`
  - `trial_token_2`
  - `trial_token_3`
  - `tieyi_cage`
  - `gray_bureau_notice`
  - `arcade_boss_door`
  - `popup_crown_arena`
  - `base_return_gate`
- Add connection back to `home_base`.

- [ ] **Step 7: Update region service doors**

In `RegionService::tryUseHomeBaseDoor`, change old `starter_village` transitions to:

- `real_street_prologue` to `home_base`.
- `home_base` `base_exit` to `real_street_prologue`.
- `home_base` `arcade_gate` to `popup_arcade`.
- `popup_arcade` `base_return_gate` to `home_base`.

Keep `starter_village` support for old saves until the new game start point changes.

- [ ] **Step 8: Run region tests**

Run:

```powershell
cmake --build build --config Release --target RegionFactoryNostalgiaTest
build\Release\RegionFactoryNostalgiaTest.exe
cmake --build build --config Release --target RegionServiceDoorTest
build\Release\RegionServiceDoorTest.exe
```

Expected: both return `0`.

- [ ] **Step 9: Commit**

```powershell
git add CMakeLists.txt src/Game/World/RegionFactory.cpp src/Game/Services/RegionService.cpp tests/RegionFactoryNostalgiaTest.cpp
git commit -m "阶段3-加入序章基地和弹窗游乐厅区域"
```

---

## Task 8: 第一章敌人定义与固定刷怪

**Files:**

- Create: `src/Game/AI/EnemyDefinition.h`
- Create: `src/Game/AI/EnemyDefinition.cpp`
- Modify: `src/Game/AI/Enemy.h`
- Modify: `src/Game/AI/Enemy.cpp`
- Modify: `src/Game/Services/EnemySpawnService.cpp`
- Create: `tests/EnemyDefinitionTest.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write enemy definition test**

Create `tests/EnemyDefinitionTest.cpp`:

```cpp
#include "Game/AI/EnemyDefinition.h"
#include "TestSupport.h"

int main() {
    const EnemyDef* bubble = EnemyDefinitions::find("popup_bubble");
    const EnemyDef* button = EnemyDefinitions::find("payment_button");
    const EnemyDef* closeX = EnemyDefinitions::find("close_x_bug");
    const EnemyDef* scrap = EnemyDefinitions::find("scrap_soldier_elite");

    TestSupport::require(bubble != nullptr, "popup bubble exists");
    TestSupport::require(bubble->baseType == EnemyType::Chaser, "popup bubble is chaser");
    TestSupport::require(bubble->special == EnemySpecialEffect::PopupCover, "popup bubble covers");
    TestSupport::require(button != nullptr && button->baseType == EnemyType::Shooter, "payment button is shooter");
    TestSupport::require(closeX != nullptr && closeX->baseType == EnemyType::Exploder, "close x is exploder");
    TestSupport::require(scrap != nullptr && scrap->maxHealth > bubble->maxHealth, "elite is tougher");
    return 0;
}
```

- [ ] **Step 2: Register and run failing test**

Add `EnemyDefinitionTest` to `CMakeLists.txt`.

Run:

```powershell
cmake --build build --config Release --target EnemyDefinitionTest
```

Expected: build fails because `EnemyDefinition.h` does not exist.

- [ ] **Step 3: Add enemy definition types**

Create `EnemyDefinition.h`:

```cpp
#pragma once

#include "Game/AI/Enemy.h"

#include <glm/vec3.hpp>
#include <string>
#include <vector>

enum class EnemySpecialEffect {
    None,
    PopupCover,
    ChildlikeDamage,
    SpawnSmallPopup,
    ScrapGuard
};

struct EnemyDef {
    std::string id;
    std::string displayName;
    EnemyType baseType = EnemyType::Chaser;
    EnemySpecialEffect special = EnemySpecialEffect::None;
    float maxHealth = 30.0f;
    float damage = 8.0f;
    float speed = 2.0f;
    float radius = 0.3f;
    glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
    int coinDropMin = 1;
    int coinDropMax = 3;
};

namespace EnemyDefinitions {

const EnemyDef* find(const std::string& enemyId);
const std::vector<EnemyDef>& all();

}  // namespace EnemyDefinitions
```

Create `src/Game/AI/EnemyDefinition.cpp`:

```cpp
#include "Game/AI/EnemyDefinition.h"

#include <algorithm>

namespace {

const std::vector<EnemyDef>& definitions() {
    static const std::vector<EnemyDef> defs = {
        {"faded_dust_cloud", "褪色尘团", EnemyType::Chaser, EnemySpecialEffect::ChildlikeDamage,
            18.0f, 5.0f, 1.8f, 0.26f, glm::vec3(0.48f, 0.50f, 0.55f), 0, 1},
        {"popup_bubble", "弹窗泡泡", EnemyType::Chaser, EnemySpecialEffect::PopupCover,
            26.0f, 7.0f, 2.1f, 0.30f, glm::vec3(0.96f, 0.62f, 0.18f), 1, 3},
        {"payment_button", "充值按钮怪", EnemyType::Shooter, EnemySpecialEffect::ChildlikeDamage,
            22.0f, 6.0f, 1.5f, 0.28f, glm::vec3(0.95f, 0.20f, 0.28f), 1, 4},
        {"close_x_bug", "关闭叉怪", EnemyType::Exploder, EnemySpecialEffect::SpawnSmallPopup,
            16.0f, 8.0f, 3.1f, 0.24f, glm::vec3(0.82f, 0.86f, 0.92f), 0, 2},
        {"scrap_soldier", "废铁小兵", EnemyType::Chaser, EnemySpecialEffect::ScrapGuard,
            42.0f, 10.0f, 1.4f, 0.34f, glm::vec3(0.48f, 0.58f, 0.68f), 2, 5},
        {"scrap_soldier_elite", "废铁小兵精英", EnemyType::Chaser, EnemySpecialEffect::ScrapGuard,
            80.0f, 14.0f, 1.2f, 0.42f, glm::vec3(0.72f, 0.40f, 0.30f), 5, 9}
    };
    return defs;
}

}  // namespace

namespace EnemyDefinitions {

const EnemyDef* find(const std::string& enemyId) {
    const auto& defs = definitions();
    auto it = std::find_if(defs.begin(), defs.end(),
        [&enemyId](const EnemyDef& def) { return def.id == enemyId; });
    return it == defs.end() ? nullptr : &*it;
}

const std::vector<EnemyDef>& all() {
    return definitions();
}

}  // namespace EnemyDefinitions
```

- [ ] **Step 4: Add definition ID to Enemy**

In `Enemy.h`, add:

```cpp
std::string definitionId;
EnemySpecialEffect specialEffect = EnemySpecialEffect::None;
```

To avoid include cycles, either forward-declare `EnemySpecialEffect` by moving it to a small `EnemyTypes.h`, or store `uint8_t specialEffect`. Prefer moving shared enums into `EnemyDefinition.h` and include it only in `.cpp` files; in `Enemy` store:

```cpp
uint8_t specialEffect = 0;
```

Use conversion helpers in `EnemyDefinition.cpp`.

- [ ] **Step 5: Add spawn by definition**

Add to `EnemyManager`:

```cpp
EnemyId spawnByDefinition(b2WorldId world, const glm::vec2& pos, const std::string& enemyDefId);
```

Implementation:

- Find `EnemyDef`.
- Call existing `spawn(world, pos, def.baseType)`.
- Override stats, color, radius, coin drops, definitionId, special effect.

- [ ] **Step 6: Fixed spawns for popup arcade**

In `EnemySpawnService.cpp`, when current region is `popup_arcade`, do not use random timer spawns. Use `StoryProgress` flag `popup_arcade_spawns_created` to create deterministic spawns once per save:

- Spawn `popup_bubble` at hall and trial machine points.
- Spawn `payment_button` in trial area and corridor.
- Spawn `close_x_bug` in corridor.
- Spawn `scrap_soldier` and `scrap_soldier_elite` in scrap factory.

After creating the fixed spawns, set:

```cpp
gs.storyProgress.setFlag("popup_arcade_spawns_created", true);
```

If the flag is already true, skip fixed spawns.

- [ ] **Step 7: Run tests and build**

Run:

```powershell
cmake --build build --config Release --target EnemyDefinitionTest
build\Release\EnemyDefinitionTest.exe
cmake --build build --config Release
```

Expected: test returns `0`, full build succeeds.

- [ ] **Step 8: Commit**

```powershell
git add CMakeLists.txt src/Game/AI/EnemyDefinition.h src/Game/AI/EnemyDefinition.cpp src/Game/AI/Enemy.h src/Game/AI/Enemy.cpp src/Game/Services/EnemySpawnService.cpp tests/EnemyDefinitionTest.cpp
git commit -m "阶段4-加入第一章敌人定义"
```

---

## Task 9: 章节拾取物、任务 facts 与奖励应用

**Files:**

- Modify: `src/Game/Drop.h`
- Modify: `src/Game/Drop.cpp`
- Modify: `src/Game/Services/ProgressionUpdateService.h`
- Modify: `src/Game/Services/ProgressionUpdateService.cpp`
- Modify: `src/Game/Services/WorldQuery.h`
- Modify: `src/Game/Services/WorldQuery.cpp`
- Modify: `src/Game/Services/WorldUpdateService.cpp`
- Modify: `src/Game/Services/SaveSnapshotBuilder.cpp`
- Modify: `src/Game/Services/SaveApplier.cpp`
- Modify: `tests/SaveApplierIntegrationTest.cpp`

- [ ] **Step 1: Extend drops to carry item IDs**

In `Drop.h`, add:

```cpp
std::string itemId;
```

Add overload:

```cpp
DropId spawnItem(b2WorldId world, const glm::vec2& pos, const std::string& itemId, int count);
```

In `Drop.cpp`, implement item drops using `DropType::Item`, value as count, and item color from category:

- Consumable: cyan/gold.
- Material: gray/blue.
- Story/relic: gold.
- Hidden collectible: purple/gold.

- [ ] **Step 2: Update collect callback**

Where `DropManager::setCollectCallback` is configured, route item drops:

```cpp
if (drop.type == DropType::Item && !drop.itemId.empty()) {
    gs.inventory.addItem(drop.itemId, std::max(1, drop.value));
    gs.storyProgress.setFlag("collected_" + drop.itemId, true);
}
```

Keep existing coin/health/mana behavior.

- [ ] **Step 3: Build quest facts**

In `ProgressionUpdateService::buildQuestSnapshot`, append facts:

```cpp
questSnapshot.currentRegionId = currentRegionId;
questSnapshot.facts.push_back({"enter_region", currentRegionId, 1});
for (const ItemStack& stack : context.inventory.getItemStacks()) {
    questSnapshot.facts.push_back({"collect", stack.itemId, stack.count});
}
if (context.storyProgress.isPartnerUnlocked("tieyi")) {
    questSnapshot.facts.push_back({"interact", "tieyi_cage", 1});
}
if (context.storyProgress.getFlag("popup_crown_defeated")) {
    questSnapshot.facts.push_back({"clear_boss", "popup_crown", 1});
}
```

Add `StoryProgress& storyProgress` to `ProgressionUpdateService::Context`.

- [ ] **Step 4: Spawn chapter pickups**

Add deterministic pickup spawning for `popup_arcade`:

- `trial_token` x3 at POIs.
- `recovery_candy` x4.
- `color_battery` x2.
- `half_melody_arcade` only if `EmotionSystem::canSeeHiddenPickups()` and not collected.

Use story flags `collected_trial_token_1`, `collected_trial_token_2`, and similar per pickup POI to prevent respawn after saving and loading.

- [ ] **Step 5: Apply quest reward items and story flags**

In `ProgressionUpdateService::applyQuestReward`, after item rewards:

```cpp
if (!reward.storyFlag.empty()) {
    context.storyProgress.setFlag(reward.storyFlag, true);
}
if (reward.maxChildlikeHeart > 0.0f) {
    context.storyProgress.setFlag("max_childlike_bonus_" + reward.questId, true);
}
```

First-round max-heart can be represented by a story flag. Numeric max-heart expansion can be added when `EmotionSystem` exposes max-heart mutators.

- [ ] **Step 6: Integration save check**

Extend `SaveApplierIntegrationTest.cpp` to create a save with `trial_token` and `tieyi` unlocked, apply it, and assert:

```cpp
TestSupport::require(gs.inventory.getItemCount("trial_token") == 3, "trial tokens restore");
TestSupport::require(gs.storyProgress.isPartnerUnlocked("tieyi"), "Tieyi restore");
```

- [ ] **Step 7: Run tests**

Run:

```powershell
cmake --build build --config Release --target SaveApplierIntegrationTest
build\Release\SaveApplierIntegrationTest.exe
cmake --build build --config Release --target QuestObjectiveFlowTest
build\Release\QuestObjectiveFlowTest.exe
```

Expected: both return `0`.

- [ ] **Step 8: Commit**

```powershell
git add src/Game/Drop.h src/Game/Drop.cpp src/Game/Services/ProgressionUpdateService.h src/Game/Services/ProgressionUpdateService.cpp src/Game/Services/WorldQuery.h src/Game/Services/WorldQuery.cpp src/Game/Services/WorldUpdateService.cpp src/Game/Services/SaveSnapshotBuilder.cpp src/Game/Services/SaveApplier.cpp tests/SaveApplierIntegrationTest.cpp
git commit -m "阶段4-接入章节拾取和任务进度"
```

---

## Task 10: BossController 与六元冠冕逻辑

**Files:**

- Create: `src/Game/Boss/BossController.h`
- Create: `src/Game/Boss/BossController.cpp`
- Create: `src/Game/Boss/PopupCrownBoss.h`
- Create: `src/Game/Boss/PopupCrownBoss.cpp`
- Modify: `src/Game/GameState.h`
- Modify: `src/Game/Services/WorldCombatUpdateService.h`
- Modify: `src/Game/Services/WorldCombatUpdateService.cpp`
- Create: `tests/PopupCrownBossTest.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write PopupCrownBoss test**

Create `tests/PopupCrownBossTest.cpp`:

```cpp
#include "Game/Boss/PopupCrownBoss.h"
#include "TestSupport.h"

void phasesFollowHealthThresholds() {
    PopupCrownBoss boss;
    boss.start();

    TestSupport::require(boss.getPhase() == PopupCrownPhase::WelcomePopup, "starts in welcome phase");
    boss.applyDamage(31.0f);
    TestSupport::require(boss.getPhase() == PopupCrownPhase::LimitedOffer, "enters limited offer below 70");
    boss.applyDamage(36.0f);
    TestSupport::require(boss.getPhase() == PopupCrownPhase::SmallWindowWorld, "enters small window below 35");
}

void rewardsReflectTemptationAndHeart() {
    PopupCrownBoss cleanBoss;
    cleanBoss.start();
    cleanBoss.applyDamage(1000.0f);
    BossReward cleanReward = cleanBoss.buildReward(650.0f);

    TestSupport::require(cleanReward.defeated, "clean boss defeated");
    TestSupport::require(cleanReward.itemRewards.size() >= 1, "clean boss grants item rewards");
    TestSupport::require(cleanReward.hiddenSticker, "clean high heart grants sticker");

    PopupCrownBoss temptedBoss;
    temptedBoss.start();
    temptedBoss.recordTemptationUse();
    temptedBoss.applyDamage(1000.0f);
    BossReward temptedReward = temptedBoss.buildReward(650.0f);

    TestSupport::require(!temptedReward.hiddenSticker, "temptation blocks sticker");
}

int main() {
    phasesFollowHealthThresholds();
    rewardsReflectTemptationAndHeart();
    return 0;
}
```

- [ ] **Step 2: Register and run failing test**

Add `PopupCrownBossTest` to `CMakeLists.txt`.

Run:

```powershell
cmake --build build --config Release --target PopupCrownBossTest
```

Expected: build fails because boss files do not exist.

- [ ] **Step 3: Add BossController types**

Create `BossController.h`:

```cpp
#pragma once

#include "Game/Quest/QuestTypes.h"

#include <string>
#include <vector>

struct BossReward {
    bool defeated = false;
    int coins = 0;
    float childlikeHeart = 0.0f;
    std::vector<QuestItemReward> itemRewards;
    bool hiddenSticker = false;
    std::string storyFlag;
};

class BossController {
public:
    virtual ~BossController() = default;
    virtual void start() = 0;
    virtual void update(float dt) = 0;
    virtual void applyDamage(float amount) = 0;
    virtual bool isActive() const = 0;
    virtual bool isDefeated() const = 0;
};
```

`BossController.cpp` can include only the header to satisfy the CMake source list.

- [ ] **Step 4: Implement PopupCrownBoss**

Create `PopupCrownBoss.h`:

```cpp
#pragma once

#include "Game/Boss/BossController.h"

enum class PopupCrownPhase {
    Inactive,
    WelcomePopup,
    LimitedOffer,
    SmallWindowWorld,
    Defeated
};

class PopupCrownBoss final : public BossController {
public:
    void start() override;
    void update(float dt) override;
    void applyDamage(float amount) override;
    bool isActive() const override;
    bool isDefeated() const override;

    PopupCrownPhase getPhase() const { return phase; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    void recordTemptationUse();
    BossReward buildReward(float childlikeHeartAtClear) const;

private:
    float maxHealth = 100.0f;
    float health = 0.0f;
    float phaseTimer = 0.0f;
    int temptationUses = 0;
    PopupCrownPhase phase = PopupCrownPhase::Inactive;

    void refreshPhase();
};
```

Create `PopupCrownBoss.cpp`:

```cpp
#include "Game/Boss/PopupCrownBoss.h"

#include <algorithm>

void PopupCrownBoss::start() {
    health = maxHealth;
    phaseTimer = 0.0f;
    temptationUses = 0;
    phase = PopupCrownPhase::WelcomePopup;
}

void PopupCrownBoss::update(float dt) {
    if (!isActive()) return;
    phaseTimer += dt;
}

void PopupCrownBoss::applyDamage(float amount) {
    if (!isActive() || amount <= 0.0f) return;
    health = std::max(0.0f, health - amount);
    refreshPhase();
}

bool PopupCrownBoss::isActive() const {
    return phase != PopupCrownPhase::Inactive && phase != PopupCrownPhase::Defeated;
}

bool PopupCrownBoss::isDefeated() const {
    return phase == PopupCrownPhase::Defeated;
}

void PopupCrownBoss::recordTemptationUse() {
    if (isActive()) {
        ++temptationUses;
    }
}

BossReward PopupCrownBoss::buildReward(float childlikeHeartAtClear) const {
    BossReward reward;
    reward.defeated = isDefeated();
    if (!reward.defeated) return reward;

    reward.coins = 60;
    reward.childlikeHeart = 120.0f;
    reward.itemRewards.push_back({"pixel_controller", 1});
    reward.itemRewards.push_back({"pixel_screw", 4});
    reward.itemRewards.push_back({"old_button", 2});
    reward.hiddenSticker = temptationUses == 0 && childlikeHeartAtClear >= 500.0f;
    if (reward.hiddenSticker) {
        reward.itemRewards.push_back({"no_pay_victory_sticker", 1});
    }
    reward.storyFlag = "popup_crown_defeated";
    return reward;
}

void PopupCrownBoss::refreshPhase() {
    if (health <= 0.0f) {
        phase = PopupCrownPhase::Defeated;
        return;
    }
    float pct = maxHealth > 0.0f ? health / maxHealth : 0.0f;
    if (pct <= 0.35f) {
        phase = PopupCrownPhase::SmallWindowWorld;
    } else if (pct <= 0.70f) {
        phase = PopupCrownPhase::LimitedOffer;
    } else {
        phase = PopupCrownPhase::WelcomePopup;
    }
}
```

- [ ] **Step 5: Add boss to GameState and update service**

In `GameState.h`, include `PopupCrownBoss.h` and add:

```cpp
PopupCrownBoss popupCrownBoss;
```

In `WorldCombatUpdateService::Context`, add `PopupCrownBoss& popupCrownBoss;`.

In `makeContext`, pass `gs.popupCrownBoss`.

In `updateAlive`, call:

```cpp
context.popupCrownBoss.update(dt);
```

- [ ] **Step 6: Apply reward on boss defeat**

Add a small reward application function in `ProgressionUpdateService` or a dedicated boss progression service:

```cpp
if (context.popupCrownBoss.isDefeated() &&
    !context.storyProgress.getFlag("popup_crown_rewarded")) {
    BossReward reward = context.popupCrownBoss.buildReward(
        context.emotionSystem.getState().childlikeHeart);
    context.inventory.addCoins(reward.coins);
    context.emotionSystem.addChildlikeHeart(reward.childlikeHeart);
    for (const QuestItemReward& itemReward : reward.itemRewards) {
        context.inventory.addItem(itemReward.itemId, itemReward.count);
    }
    context.storyProgress.setFlag(reward.storyFlag, true);
    context.storyProgress.setFlag("popup_crown_rewarded", true);
    context.storyProgress.completeChapter("chapter_1_popup_arcade");
    context.storyProgress.unlockPartner("tieyi");
}
```

- [ ] **Step 7: Add CMake sources**

Add boss `.cpp` files to `StarchildGame` and test registration.

- [ ] **Step 8: Run tests**

Run:

```powershell
cmake --build build --config Release --target PopupCrownBossTest
build\Release\PopupCrownBossTest.exe
cmake --build build --config Release
```

Expected: test returns `0`, full build succeeds.

- [ ] **Step 9: Commit**

```powershell
git add CMakeLists.txt src/Game/Boss/BossController.h src/Game/Boss/BossController.cpp src/Game/Boss/PopupCrownBoss.h src/Game/Boss/PopupCrownBoss.cpp src/Game/GameState.h src/Game/Services/WorldCombatUpdateService.h src/Game/Services/WorldCombatUpdateService.cpp tests/PopupCrownBossTest.cpp
git commit -m "阶段5-加入六元冠冕Boss逻辑"
```

---

## Task 11: 第一章剧情推进与基地回报

**Files:**

- Modify: `src/Game/Services/ProgressionUpdateService.h`
- Modify: `src/Game/Services/ProgressionUpdateService.cpp`
- Modify: `src/Game/Services/RegionService.cpp`
- Modify: `src/Game/Presentation/PresentationModelBuilder.cpp`
- Modify: `src/Game/Presentation/GameMenuView.cpp`
- Modify: `assets/scripts/dialogues/first_meeting.lua`
- Modify: `assets/scripts/dialogues/daily_greetings.lua`

- [ ] **Step 1: Unlock initial story state**

On new game initialization, set:

```cpp
gs.storyProgress.unlockChapter("prologue_star_candy");
gs.storyProgress.unlockChapter("chapter_1_popup_arcade");
```

Set the starting region to `real_street_prologue` in new game setup and default `SaveData::PlayerData`.

- [ ] **Step 2: Prologue interaction flags**

When interacting with `star_candy`, set:

```cpp
gs.storyProgress.setFlag("star_candy_collected", true);
gs.inventory.addItem("old_game_coin", 1);
gs.storyProgress.startChapter("prologue_star_candy");
```

When first talking to Alya after defeating tutorial enemies:

```cpp
gs.princess->setFollowing(true);
gs.storyProgress.setFlag("alya_following", true);
gs.storyProgress.completeChapter("prologue_star_candy");
```

- [ ] **Step 3: Tieyi rescue**

When interacting with `tieyi_cage` after the scrap elite is defeated:

```cpp
gs.storyProgress.unlockPartner("tieyi");
gs.storyProgress.setFlag("tieyi_rescued", true);
gs.inventory.addItem("color_battery", 1);
```

Add notice text: `铁翼恢复了火箭核心`.

- [ ] **Step 4: Boss door gating**

Boss door opens when:

```cpp
gs.inventory.getItemCount("trial_token") >= 3 &&
gs.storyProgress.getFlag("tieyi_rescued")
```

Transition into arena starts `gs.popupCrownBoss.start()` and sets `chapter_1_popup_arcade` to `InProgress`.

- [ ] **Step 5: Pixel Controller base placement**

In `home_base`, if player interacts with `pixel_controller_spot` and owns `pixel_controller`:

```cpp
gs.inventory.consumeItem("pixel_controller", 1);
gs.storyProgress.setFlag("pixel_controller_placed", true);
gs.inventory.addItem("recovery_candy", 2);
```

Display `试玩币机关已点亮`.

- [ ] **Step 6: Update partner page**

In menu model partner lines:

```cpp
if (gs.storyProgress.isPartnerUnlocked("tieyi")) {
    model.partnerLines.push_back("铁翼：已入队 / 火箭破盾支援");
} else {
    model.partnerLines.push_back("铁翼：未救出");
}
```

- [ ] **Step 7: Build and manual route**

Run:

```powershell
cmake --build build --config Release
```

Manual route:

1. Start new game.
2. Interact with star candy.
3. Talk to Alya.
4. Enter base.
5. Enter Popup Arcade.
6. Collect three trial tokens.
7. Rescue Tieyi.
8. Start boss.
9. Defeat boss using debug damage hook or real combat once integrated.
10. Return to base and place Pixel Controller.

Expected:

- Inventory changes at each step.
- Quest facts update.
- Partner page shows Tieyi after rescue.
- Base placement consumes Pixel Controller and sets the story flag.

- [ ] **Step 8: Commit**

```powershell
git add src/Game/Services/ProgressionUpdateService.h src/Game/Services/ProgressionUpdateService.cpp src/Game/Services/RegionService.cpp src/Game/Presentation/PresentationModelBuilder.cpp src/Game/Presentation/GameMenuView.cpp assets/scripts/dialogues/first_meeting.lua assets/scripts/dialogues/daily_greetings.lua
git commit -m "阶段5-串联第一章剧情推进"
```

---

## Task 12: 程序化发光像素表现

**Files:**

- Create: `src/Game/Presentation/PixelActorView.h`
- Create: `src/Game/Presentation/PixelActorView.cpp`
- Modify: `src/Game/Presentation/EntityView.cpp`
- Modify: `src/Game/Presentation/WorldRenderer.cpp`
- Modify: `src/Game/Presentation/GameRenderer.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Create PixelActorView API**

Create `PixelActorView.h`:

```cpp
#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace PixelActorView {

enum class ActorKind {
    Xingyuan,
    Alya,
    Tieyi,
    PopupBubble,
    PaymentButton,
    CloseXBug,
    ScrapSoldier,
    PopupCrown
};

struct Model {
    ActorKind kind = ActorKind::Xingyuan;
    glm::vec2 position{0.0f, 0.0f};
    float animationTime = 0.0f;
    float glow = 0.0f;
    bool faded = false;
};

void render(const Model& model, const glm::mat4& viewProj);

}  // namespace PixelActorView
```

- [ ] **Step 2: Implement pixel drawing**

Create `src/Game/Presentation/PixelActorView.cpp`:

```cpp
#include "Game/Presentation/PixelActorView.h"

#include "Engine/Renderer/Draw2D.h"

#include <algorithm>
#include <glm/vec3.hpp>

namespace {

glm::vec3 fadedMix(const glm::vec3& c, bool faded) {
    if (!faded) return c;
    float gray = (c.r + c.g + c.b) / 3.0f;
    return glm::vec3(gray * 0.72f);
}

void rect(float x, float y, float w, float h, const glm::vec3& c, bool faded, float a = 0.95f) {
    Draw2D::drawRectFilled(x, y, w, h, fadedMix(c, faded), a);
}

void drawGlow(const PixelActorView::Model& model, float radius) {
    if (model.glow <= 0.01f) return;
    Draw2D::drawCircle(model.position.x, model.position.y, radius,
        glm::vec3(0.42f, 0.82f, 1.0f), 0.05f, 28, std::min(0.55f, model.glow));
}

void drawXingyuan(const PixelActorView::Model& model) {
    float x = model.position.x;
    float y = model.position.y;
    drawGlow(model, 0.72f);
    rect(x - 0.18f, y - 0.44f, 0.16f, 0.30f, glm::vec3(0.05f, 0.07f, 0.10f), model.faded);
    rect(x + 0.02f, y - 0.44f, 0.16f, 0.30f, glm::vec3(0.05f, 0.07f, 0.10f), model.faded);
    rect(x - 0.28f, y - 0.14f, 0.56f, 0.48f, glm::vec3(0.03f, 0.05f, 0.10f), model.faded);
    rect(x - 0.12f, y - 0.08f, 0.24f, 0.30f, glm::vec3(0.88f, 0.92f, 0.95f), model.faded);
    rect(x - 0.24f, y + 0.34f, 0.48f, 0.26f, glm::vec3(0.08f, 0.08f, 0.09f), model.faded);
    Draw2D::drawCircleFilled(x, y + 0.46f, 0.18f, fadedMix(glm::vec3(0.95f, 0.72f, 0.58f), model.faded), 0.98f);
    Draw2D::drawCircleFilled(x + 0.24f, y + 0.02f, 0.055f, fadedMix(glm::vec3(0.26f, 0.78f, 1.0f), model.faded), 0.95f);
}

void drawAlya(const PixelActorView::Model& model) {
    float x = model.position.x;
    float y = model.position.y;
    drawGlow(model, 0.78f);
    rect(x - 0.28f, y - 0.38f, 0.56f, 0.58f, glm::vec3(0.90f, 0.94f, 1.0f), model.faded);
    rect(x - 0.34f, y - 0.02f, 0.68f, 0.18f, glm::vec3(0.45f, 0.70f, 1.0f), model.faded);
    Draw2D::drawCircleFilled(x, y + 0.42f, 0.20f, fadedMix(glm::vec3(0.96f, 0.76f, 0.60f), model.faded), 0.98f);
    rect(x - 0.26f, y + 0.46f, 0.52f, 0.18f, glm::vec3(1.0f, 0.76f, 0.18f), model.faded);
    Draw2D::drawCircleFilled(x, y + 0.03f, 0.055f, fadedMix(glm::vec3(0.70f, 0.95f, 1.0f), model.faded), 0.96f);
}

void drawTieyi(const PixelActorView::Model& model) {
    float x = model.position.x;
    float y = model.position.y;
    drawGlow(model, 0.62f);
    rect(x - 0.24f, y - 0.28f, 0.48f, 0.46f, glm::vec3(0.80f, 0.12f, 0.16f), model.faded);
    rect(x - 0.18f, y - 0.48f, 0.14f, 0.20f, glm::vec3(0.18f, 0.32f, 0.78f), model.faded);
    rect(x + 0.04f, y - 0.48f, 0.14f, 0.20f, glm::vec3(0.18f, 0.32f, 0.78f), model.faded);
    rect(x - 0.22f, y + 0.18f, 0.44f, 0.26f, glm::vec3(0.10f, 0.16f, 0.25f), model.faded);
    Draw2D::drawCircleFilled(x, y - 0.02f, 0.07f, fadedMix(glm::vec3(1.0f, 0.78f, 0.18f), model.faded), 0.96f);
    rect(x - 0.36f, y - 0.16f, 0.12f, 0.26f, glm::vec3(0.18f, 0.40f, 0.95f), model.faded);
    rect(x + 0.24f, y - 0.16f, 0.12f, 0.26f, glm::vec3(0.18f, 0.40f, 0.95f), model.faded);
}

void drawSimpleEnemy(const PixelActorView::Model& model, const glm::vec3& color) {
    float x = model.position.x;
    float y = model.position.y;
    drawGlow(model, 0.45f);
    Draw2D::drawCircleFilled(x, y, 0.24f, fadedMix(color, model.faded), 0.94f);
    rect(x - 0.09f, y + 0.04f, 0.06f, 0.06f, glm::vec3(0.05f), model.faded);
    rect(x + 0.03f, y + 0.04f, 0.06f, 0.06f, glm::vec3(0.05f), model.faded);
}

void drawPopupCrown(const PixelActorView::Model& model) {
    float x = model.position.x;
    float y = model.position.y;
    drawGlow(model, 1.30f);
    rect(x - 0.72f, y - 0.32f, 1.44f, 0.60f, glm::vec3(0.95f, 0.68f, 0.12f), model.faded);
    rect(x - 0.68f, y + 0.26f, 0.24f, 0.34f, glm::vec3(1.0f, 0.84f, 0.20f), model.faded);
    rect(x - 0.12f, y + 0.26f, 0.24f, 0.46f, glm::vec3(1.0f, 0.84f, 0.20f), model.faded);
    rect(x + 0.44f, y + 0.26f, 0.24f, 0.34f, glm::vec3(1.0f, 0.84f, 0.20f), model.faded);
    rect(x - 0.40f, y - 0.08f, 0.18f, 0.18f, glm::vec3(0.12f, 0.12f, 0.12f), model.faded);
    rect(x + 0.22f, y - 0.08f, 0.18f, 0.18f, glm::vec3(0.12f, 0.12f, 0.12f), model.faded);
    rect(x - 0.20f, y - 0.24f, 0.40f, 0.08f, glm::vec3(0.18f, 0.10f, 0.06f), model.faded);
}

}  // namespace

namespace PixelActorView {

void render(const Model& model, const glm::mat4& viewProj) {
    Draw2D::beginFrame(viewProj);
    switch (model.kind) {
        case ActorKind::Xingyuan: drawXingyuan(model); break;
        case ActorKind::Alya: drawAlya(model); break;
        case ActorKind::Tieyi: drawTieyi(model); break;
        case ActorKind::PopupBubble: drawSimpleEnemy(model, glm::vec3(0.96f, 0.62f, 0.18f)); break;
        case ActorKind::PaymentButton: drawSimpleEnemy(model, glm::vec3(0.95f, 0.20f, 0.28f)); break;
        case ActorKind::CloseXBug: drawSimpleEnemy(model, glm::vec3(0.82f, 0.86f, 0.92f)); break;
        case ActorKind::ScrapSoldier: drawSimpleEnemy(model, glm::vec3(0.52f, 0.58f, 0.66f)); break;
        case ActorKind::PopupCrown: drawPopupCrown(model); break;
    }
    Draw2D::endFrame();
}

}  // namespace PixelActorView
```

- [ ] **Step 3: Replace or layer player/NPC rendering**

Keep existing shader character rendering available, but add pixel actor rendering after it for first-round visual direction. If both conflict visually, gate shader rendering behind a bool in `GameUiState`:

```cpp
bool usePixelActors = true;
```

Default true.

- [ ] **Step 4: Render first chapter enemies**

Map `Enemy.definitionId` to `PixelActorView::ActorKind` in `EntityView.cpp`:

- `popup_bubble` -> `PopupBubble`
- `payment_button` -> `PaymentButton`
- `close_x_bug` -> `CloseXBug`
- `scrap_soldier` and elite -> `ScrapSoldier`

- [ ] **Step 5: Render boss**

If `gs.popupCrownBoss.isActive()`, render `PopupCrown` at the fixed arena center for the first slice:

```cpp
PixelActorView::render({
    PixelActorView::ActorKind::PopupCrown,
    glm::vec2(30.0f, 50.0f),
    gs.charTime,
    0.75f,
    gs.emotionSystem.getChildlikeHeartTier() == ChildlikeHeartTier::Faded
}, gs.camera.getViewProjection());
```

- [ ] **Step 6: Build and visual check**

Run:

```powershell
cmake --build build --config Release
cd build
.\Release\Starchild2D.exe
```

Manual expected:

- Player reads as black-blue pixel character.
- Alya reads as gold-blue-white character.
- First chapter enemies are visually distinct.
- Boss reads as a large glowing crown.

- [ ] **Step 7: Commit**

```powershell
git add CMakeLists.txt src/Game/Presentation/PixelActorView.h src/Game/Presentation/PixelActorView.cpp src/Game/Presentation/EntityView.cpp src/Game/Presentation/WorldRenderer.cpp src/Game/Presentation/GameRenderer.cpp
git commit -m "阶段6-加入发光像素临时表现"
```

---

## Task 13: 主菜单与新游戏入口

**Files:**

- Modify: `src/Game/Presentation/MainMenuView.h`
- Modify: `src/Game/Presentation/MainMenuView.cpp`
- Modify: `src/Game/Presentation/PresentationModelBuilder.cpp`
- Modify: `src/Game/Scenes/MainMenuScene.cpp`
- Modify: `src/Game/Services/SaveGameService.cpp`
- Modify: `src/Game/Data/SaveData.h`

- [ ] **Step 1: Update menu model**

Extend `MainMenuView::Model`:

```cpp
float childlikeHeart = 0.0f;
int rescuedPartners = 0;
```

Populate from save meta in `SaveGameService::getSaveMeta`. Add fields to `SaveMeta`:

```cpp
float childlikeHeart = 0.0f;
int rescuedPartners = 0;
```

- [ ] **Step 2: Render nostalgia title**

In `MainMenuView.cpp`, change title text to:

```cpp
"星愿之子"
"我会长大，但不交出童心"
```

Show continue save details:

- timestamp.
- region.
- childlike heart.
- rescued partners.

- [ ] **Step 3: New game starts at prologue**

Change default `SaveData::PlayerData`:

```cpp
std::string regionId = "real_street_prologue";
glm::vec2 position = glm::vec2(8.0f, 12.0f);
```

Ensure new game initialization transitions to `real_street_prologue` and initializes story progress.

- [ ] **Step 4: Build and manual check**

Run:

```powershell
cmake --build build --config Release
cd build
.\Release\Starchild2D.exe
```

Manual expected:

- Main menu title and subtitle match the new theme.
- New Game starts in `real_street_prologue`.
- Continue shows save metadata when `autosave` exists.

- [ ] **Step 5: Commit**

```powershell
git add src/Game/Presentation/MainMenuView.h src/Game/Presentation/MainMenuView.cpp src/Game/Presentation/PresentationModelBuilder.cpp src/Game/Scenes/MainMenuScene.cpp src/Game/Services/SaveGameService.cpp src/Game/Data/SaveData.h
git commit -m "阶段3-更新主菜单和新游戏入口"
```

---

## Task 14: 回归测试与第一轮手动验收

**Files:**

- Modify only files needed to fix issues found in verification.

- [ ] **Step 1: Full build**

Run:

```powershell
cmake --build build --config Release
```

Expected: build succeeds.

- [ ] **Step 2: CTest**

Run:

```powershell
ctest -C Release --test-dir build --output-on-failure
```

Expected: all registered tests pass.

- [ ] **Step 3: Manual vertical slice**

Run:

```powershell
cd build
.\Release\Starchild2D.exe
```

Manual checklist:

- Main menu opens with new theme.
- New game enters `real_street_prologue`.
- Star candy interaction grants `old_game_coin`.
- Alya follows after prologue step.
- Base map table and save bed are visible/interactable.
- `Tab` opens menu.
- Quest page shows first-round quest progress.
- Character page shows childlike tier.
- Inventory page shows item stacks.
- Popup Arcade loads from base.
- Trial tokens can be collected.
- Popup enemies spawn and behave distinctly.
- Tieyi cage interaction unlocks Tieyi after scrap guard condition.
- Boss starts after gate condition.
- Boss changes phases at expected health thresholds.
- Boss reward grants Pixel Controller.
- Pixel Controller can be placed in base.
- Save with `F5`, exit, continue, and verify progress persists.

- [ ] **Step 4: Fix regressions**

For each failure:

1. Reproduce with the smallest command or route.
2. Fix the local file responsible.
3. Re-run the specific failing test.
4. Re-run full build or CTest if shared code changed.

- [ ] **Step 5: Commit regression fixes when needed**

```powershell
git status --short
git commit -m "阶段6-完成第一章垂直切片回归"
```

Only run the commit command if Step 4 changed files. Stage the concrete files listed by `git status --short` before committing; do not stage unrelated files such as `.claude/worktrees/`.
