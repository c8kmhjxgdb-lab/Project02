#include "Game/Data/SaveMigration.h"

namespace SaveMigration {

void migrateToCurrent(SaveData& data) {
    if (data.version <= 0) {
        data.version = 1;
    }

    // Current saves are version 4. Older versions are still structurally
    // compatible after SaveSerializer defaulting, so this is the migration
    // checkpoint for future format changes.
    data.version = kCurrentVersion;
}

}  // namespace SaveMigration
