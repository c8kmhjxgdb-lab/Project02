#pragma once

#include "Game/Services/ProgressionUpdateService.h"
#include "Game/Services/RegionUpdateService.h"

struct GameState;

namespace WorldUpdateService {

struct State {
    RegionUpdateService::State regionState;
    ProgressionUpdateService::State progressionState;
};

void update(GameState& gs, float dt, State& state);

}  // namespace WorldUpdateService
