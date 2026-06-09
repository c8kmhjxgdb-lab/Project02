#pragma once

struct GameState;

namespace PlayerLifecycleService {

void updateAlive(GameState& gs, float dt);
void updateDeathRespawn(GameState& gs, float dt);
void updateWhileDead(GameState& gs, float dt);

}  // namespace PlayerLifecycleService
