#include "Game/AI/EnemyDefinition.h"

#include <algorithm>

namespace {

const std::vector<EnemyDef>& definitions() {
    static const std::vector<EnemyDef> defs = {
        {"faded_dust_cloud", "褪色尘团", EnemyType::Chaser, EnemySpecialEffect::ChildlikeDamage,
            18.0f, 5.0f, 1.8f, 0.26f, glm::vec3(0.48f, 0.50f, 0.55f), 0, 1},
        {"popup_bubble", "弹窗泡泡", EnemyType::Chaser, EnemySpecialEffect::PopupCover,
            26.0f, 7.0f, 2.1f, 0.30f, glm::vec3(0.96f, 0.62f, 0.18f), 1, 3},
        {"payment_button", "充值按钮怪", EnemyType::Shooter, EnemySpecialEffect::ChildlikeDamage,
            22.0f, 6.0f, 1.5f, 0.28f, glm::vec3(0.95f, 0.20f, 0.28f), 1, 4},
        {"close_x_bug", "关闭叉怪", EnemyType::Exploder, EnemySpecialEffect::SpawnSmallPopup,
            16.0f, 8.0f, 3.1f, 0.24f, glm::vec3(0.82f, 0.86f, 0.92f), 0, 2},
        {"scrap_soldier", "废铁小兵", EnemyType::Chaser, EnemySpecialEffect::ScrapGuard,
            42.0f, 10.0f, 1.4f, 0.34f, glm::vec3(0.48f, 0.58f, 0.68f), 2, 5},
        {"scrap_soldier_elite", "废铁小兵精英", EnemyType::Chaser, EnemySpecialEffect::ScrapGuard,
            80.0f, 14.0f, 1.2f, 0.42f, glm::vec3(0.72f, 0.40f, 0.30f), 5, 9}
    };
    return defs;
}

}  // namespace

namespace EnemyDefinitions {

const EnemyDef* find(const std::string& enemyId) {
    const auto& defs = definitions();
    auto it = std::find_if(defs.begin(), defs.end(),
        [&enemyId](const EnemyDef& def) { return def.id == enemyId; });
    return it == defs.end() ? nullptr : &*it;
}

const std::vector<EnemyDef>& all() {
    return definitions();
}

}  // namespace EnemyDefinitions
