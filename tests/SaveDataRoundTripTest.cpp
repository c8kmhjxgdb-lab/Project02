#include "Game/Data/SaveMigration.h"
#include "Game/Data/SaveSerializer.h"
#include "TestSupport.h"

int main() {
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
    TestSupport::require(restored.storyProgress.chapters.size() == 1, "chapter progress round trips");
    TestSupport::require(restored.storyProgress.chapters[0].state == ChapterState::Completed,
        "chapter state round trips");
    TestSupport::require(restored.storyProgress.unlockedPartners.size() == 1,
        "partner progress round trips");
    TestSupport::require(restored.storyProgress.flags.size() == 1, "story flag round trips");
    return 0;
}
