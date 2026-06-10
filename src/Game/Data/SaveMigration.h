#pragma once

#include "Game/Data/SaveData.h"

namespace SaveMigration {

inline constexpr int kCurrentVersion = 3;

void migrateToCurrent(SaveData& data);

}  // namespace SaveMigration
