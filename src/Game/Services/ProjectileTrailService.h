#pragma once

class ParticleSystem;
class ProjectileManager;
struct GameState;

namespace ProjectileTrailService {

struct Context {
    ProjectileManager& projectileManager;
    ParticleSystem& particleSystem;
    float& charTime;
};

Context makeContext(GameState& gs);

void emitTrails(Context& context);

}  // namespace ProjectileTrailService
