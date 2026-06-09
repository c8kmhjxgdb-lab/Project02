#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>
#include <cstdint>

enum class FurnitureCategory : uint8_t {
    Bed,
    Work,
    Decor,
    Light,
    Storage,
    Poster
};

struct FurnitureDef {
    std::string id;
    std::string name;
    FurnitureCategory category = FurnitureCategory::Decor;
    glm::ivec2 size{1, 1};
    int price = 0;
    int comfort = 0;
    int childlikeRestore = 0;
    int affectionBonus = 0;
    float lightRadius = 0.0f;
    bool nightBonus = false;
    bool blocksMovement = false;
    std::string nostalgiaTag;
    std::string drawStyle;
    glm::vec3 color{0.7f, 0.55f, 0.38f};
    glm::vec3 accentColor{0.95f, 0.82f, 0.42f};
};

struct FurnitureInstance {
    int instanceId = 0;
    std::string defId;
    glm::ivec2 tile{0, 0};
    uint8_t rotation = 0;
};

std::vector<FurnitureDef> createDefaultFurnitureDefs();
