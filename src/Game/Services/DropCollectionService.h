#pragma once

class Inventory;
class StoryProgress;
class HealthComponent;
struct Drop;
struct GameState;

namespace DropCollectionService {

struct Context {
    Inventory& inventory;
    StoryProgress& storyProgress;
    HealthComponent& playerHealth;
    float& playerMana;
    float& playerMaxMana;
};

Context makeContext(GameState& gs);

void collect(Context& context, const Drop& drop);

}  // namespace DropCollectionService
