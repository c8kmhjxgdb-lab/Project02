#pragma once

#include <string>

struct GameState;

namespace SaveSnapshotBuilder {

bool saveCurrentGame(GameState& gs, const std::string& slot);

}  // namespace SaveSnapshotBuilder
