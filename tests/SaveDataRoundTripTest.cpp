#include "Game/Data/SaveMigration.h"
#include "Game/Data/SaveSerializer.h"
#include "TestSupport.h"

#include <limits>

namespace {

void saveDataRoundTripsNewFields() {
    SaveData data;
    data.version = SaveMigration::kCurrentVersion;
    data.timestamp = "test";
    data.player.regionId = "starter_village";
    data.player.position = glm::vec2(3.0f, 4.0f);
    data.player.health = 80.0f;
    data.player.maxHealth = 100.0f;
    data.player.mana = 25.0f;
    data.player.maxMana = 50.0f;
    data.player.coins = 7;
    data.environment.day = 2;
    data.environment.hour = 18.5f;
    data.environment.weather = "Rain";
    data.itemStacks.push_back({"recovery_candy", 2});
    data.itemStacks.push_back({"pixel_screw", 4});
    data.storyProgress.chapters.push_back({"chapter_1_popup_arcade", ChapterState::Completed});
    data.storyProgress.unlockedPartners.push_back("tieyi");
    data.storyProgress.flags.push_back({"arcade_boss_defeated", true});

    SaveSerializer::Json json = SaveSerializer::toJson(data);
    SaveData restored = SaveSerializer::fromJson(json);
    SaveMigration::migrateToCurrent(restored);

    TestSupport::require(restored.version == SaveMigration::kCurrentVersion, "save version migrates to current");
    TestSupport::require(restored.player.regionId == "starter_village", "player region round trips");
    TestSupport::require(restored.player.position.x == 3.0f, "player x position round trips");
    TestSupport::require(restored.player.position.y == 4.0f, "player y position round trips");
    TestSupport::require(restored.player.coins == 7, "player coins round trips");
    TestSupport::require(restored.environment.day == 2, "environment day round trips");
    TestSupport::require(restored.environment.weather == "Rain", "environment weather round trips");
    TestSupport::require(restored.itemStacks.size() == 2, "item stacks round trip");
    TestSupport::require(restored.itemStacks[0].itemId == "recovery_candy", "item id round trips");
    TestSupport::require(restored.itemStacks[0].count == 2, "item count round trips");
    TestSupport::require(restored.itemStacks[1].itemId == "pixel_screw", "second item id round trips");
    TestSupport::require(restored.itemStacks[1].count == 4, "second item count round trips");
    TestSupport::require(restored.storyProgress.chapters.size() == 1, "chapter progress round trips");
    TestSupport::require(restored.storyProgress.chapters[0].chapterId == "chapter_1_popup_arcade",
        "chapter id round trips");
    TestSupport::require(restored.storyProgress.chapters[0].state == ChapterState::Completed,
        "chapter state round trips");
    TestSupport::require(restored.storyProgress.unlockedPartners.size() == 1,
        "partner progress round trips");
<<<<<<< HEAD
    TestSupport::require(restored.storyProgress.unlockedPartners[0] == "tieyi",
        "partner id round trips");
    TestSupport::require(restored.storyProgress.flags.size() == 1, "story flag round trips");
    TestSupport::require(restored.storyProgress.flags[0].flagId == "arcade_boss_defeated",
        "story flag id round trips");
    TestSupport::require(restored.storyProgress.flags[0].value, "story flag value round trips");
}

void invalidNewFieldsAreIgnored() {
    SaveSerializer::Json json;
    json["inventory"]["items"] = SaveSerializer::Json::array({
        {{"itemId", "recovery_candy"}, {"count", 2}},
        {{"itemId", ""}, {"count", 5}},
        {{"itemId", "pixel_screw"}, {"count", 0}},
        {{"itemId", 17}, {"count", 1}},
        {{"itemId", "bad_count"}, {"count", "x"}},
        "bad item"
    });
    json["story"]["chapters"] = SaveSerializer::Json::array({
        {{"chapterId", "chapter_1_popup_arcade"}, {"state", static_cast<int>(ChapterState::Completed)}},
        {{"chapterId", ""}, {"state", static_cast<int>(ChapterState::Unlocked)}},
        {{"chapterId", "bad_state"}, {"state", 99}},
        {{"chapterId", "wrapped_locked"}, {"state", 256}},
        {{"chapterId", "wrapped_completed"}, {"state", 259}},
        {{"chapterId", "huge_state"}, {"state", 4294967299ULL}},
        {{"chapterId", "too_large_state"}, {"state",
            static_cast<unsigned long long>(std::numeric_limits<long long>::max()) + 1ULL}},
        {{"chapterId", 17}, {"state", static_cast<int>(ChapterState::Unlocked)}},
        {{"chapterId", "bad_type"}, {"state", "completed"}},
        "bad chapter"
    });
    json["story"]["unlockedPartners"] = SaveSerializer::Json::array({"tieyi", "", 17});
    json["story"]["flags"] = SaveSerializer::Json::array({
        {{"flagId", "arcade_boss_defeated"}, {"value", true}},
        {{"flagId", ""}, {"value", true}},
        {{"flagId", 17}, {"value", true}},
        {{"flagId", "bad_value"}, {"value", "true"}},
        "bad flag"
    });

    SaveData restored = SaveSerializer::fromJson(json);

    TestSupport::require(restored.itemStacks.size() == 1, "invalid item stacks are ignored");
    TestSupport::require(restored.itemStacks[0].itemId == "recovery_candy", "valid item remains");
    TestSupport::require(restored.itemStacks[0].count == 2, "valid item count remains");
    TestSupport::require(restored.storyProgress.chapters.size() == 1, "invalid chapters are ignored");
    TestSupport::require(restored.storyProgress.chapters[0].chapterId == "chapter_1_popup_arcade",
        "valid chapter remains");
    TestSupport::require(restored.storyProgress.chapters[0].state == ChapterState::Completed,
        "valid chapter state remains");
    TestSupport::require(restored.storyProgress.unlockedPartners.size() == 1,
        "invalid partners are ignored");
    TestSupport::require(restored.storyProgress.unlockedPartners[0] == "tieyi", "valid partner remains");
    TestSupport::require(restored.storyProgress.flags.size() == 1, "invalid story flags are ignored");
    TestSupport::require(restored.storyProgress.flags[0].flagId == "arcade_boss_defeated",
        "valid flag remains");
    TestSupport::require(restored.storyProgress.flags[0].value, "valid flag value remains");
}

void invalidNewFieldContainersAreIgnored() {
    SaveSerializer::Json json;
    json["inventory"]["items"] = "bad items";
    json["story"]["chapters"] = "bad chapters";
    json["story"]["unlockedPartners"] = 42;
    json["story"]["flags"] = "bad flags";

    SaveData restored = SaveSerializer::fromJson(json);

    TestSupport::require(restored.itemStacks.empty(), "invalid item container ignored");
    TestSupport::require(restored.storyProgress.chapters.empty(), "invalid chapter container ignored");
    TestSupport::require(restored.storyProgress.unlockedPartners.empty(), "invalid partner container ignored");
    TestSupport::require(restored.storyProgress.flags.empty(), "invalid flag container ignored");
}

void serializerSkipsInvalidNewFieldsOnWrite() {
    SaveData data;
    data.itemStacks.push_back({"recovery_candy", 2});
    data.itemStacks.push_back({"", 4});
    data.itemStacks.push_back({"pixel_screw", 0});
    data.storyProgress.chapters.push_back({"chapter_1_popup_arcade", ChapterState::Completed});
    data.storyProgress.chapters.push_back({"", ChapterState::Unlocked});
    data.storyProgress.chapters.push_back({"bad_state", static_cast<ChapterState>(99)});
    data.storyProgress.unlockedPartners.push_back("tieyi");
    data.storyProgress.unlockedPartners.push_back("");
    data.storyProgress.flags.push_back({"arcade_boss_defeated", true});
    data.storyProgress.flags.push_back({"", true});

    SaveSerializer::Json json = SaveSerializer::toJson(data);

    TestSupport::require(json["inventory"]["items"].size() == 1, "invalid item stacks skipped on write");
    TestSupport::require(json["story"]["chapters"].size() == 1, "invalid chapters skipped on write");
    TestSupport::require(json["story"]["chapters"][0]["chapterId"] == "chapter_1_popup_arcade",
        "valid chapter is written");
    TestSupport::require(json["story"]["unlockedPartners"].size() == 1,
        "invalid partners skipped on write");
    TestSupport::require(json["story"]["flags"].size() == 1, "invalid flags skipped on write");
}

}  // namespace

int main() {
    saveDataRoundTripsNewFields();
    invalidNewFieldsAreIgnored();
    invalidNewFieldContainersAreIgnored();
    serializerSkipsInvalidNewFieldsOnWrite();
=======
    TestSupport::require(restored.storyProgress.flags.size() == 1, "story flag round trips");
>>>>>>> origin/main
    return 0;
}
