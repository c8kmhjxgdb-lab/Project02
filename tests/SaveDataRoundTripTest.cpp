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
    return 0;
}
