#pragma once

#include <box2d/box2d.h>

class BondTechniqueSystem;
class EnemyManager;
class Lightning;
class Shield;
struct GameState;

namespace AbilityUpdateService {

struct Context {
    Shield& shield;
    Lightning& lightning;
    BondTechniqueSystem& bondTechnique;
    EnemyManager& enemyManager;
    b2BodyId playerBodyId;
};

Context makeContext(GameState& gs);

void update(Context& context, float dt);

}  // namespace AbilityUpdateService
