#include "Game/Services/DropCollectionService.h"

#include "Game/Drop.h"
#include "Game/GameState.h"
#include "Game/Health.h"
#include "Game/Inventory/Inventory.h"
#include "Game/Progress/StoryProgress.h"

#include <algorithm>

namespace DropCollectionService {

Context makeContext(GameState& gs) {
    return {
        gs.inventory,
        gs.storyProgress,
        gs.playerHealth,
        gs.playerMana,
        gs.playerMaxMana
    };
}

void collect(Context& context, const Drop& drop) {
    switch (drop.type) {
    case DropType::Coin:
        context.inventory.addCoins(std::max(0, drop.value));
        break;
    case DropType::Health:
        context.playerHealth.heal(static_cast<float>(std::max(0, drop.value)));
        break;
    case DropType::Mana:
        context.playerMana = std::clamp(
            context.playerMana + static_cast<float>(std::max(0, drop.value)),
            0.0f,
            context.playerMaxMana);
        break;
    case DropType::Item:
        if (!drop.itemId.empty()) {
            context.inventory.addItem(drop.itemId, std::max(1, drop.value));
            context.storyProgress.setFlag("collected_" + drop.itemId, true);
        }
        if (!drop.collectionFlag.empty()) {
            context.storyProgress.setFlag(drop.collectionFlag, true);
        }
        break;
    }
}

}  // namespace DropCollectionService
