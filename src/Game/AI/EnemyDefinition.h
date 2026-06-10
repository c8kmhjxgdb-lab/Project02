#pragma once

#include "Game/AI/Enemy.h"

#include <glm/vec3.hpp>

#include <string>
#include <vector>

enum class EnemySpecialEffect : uint8_t {
    None,
    PopupCover,
    ChildlikeDamage,
    SpawnSmallPopup,
    ScrapGuard
};

struct EnemyDef {
    std::string id;
    std::string displayName;
    EnemyType baseType = EnemyType::Chaser;
    EnemySpecialEffect special = EnemySpecialEffect::None;
    float maxHealth = 30.0f;
    float damage = 8.0f;
    float speed = 2.0f;
    float radius = 0.3f;
    glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
    int coinDropMin = 1;
    int coinDropMax = 3;
};

namespace EnemyDefinitions {

const EnemyDef* find(const std::string& enemyId);
const std::vector<EnemyDef>& all();

}  // namespace EnemyDefinitions
