#include "Furniture.h"

std::vector<FurnitureDef> createDefaultFurnitureDefs() {
    return {
        {"simple_bed", "小木床", FurnitureCategory::Bed, {3, 2}, 30, 8, 6, 1, 0.0f, true, true, "home", "bed",
            {0.50f, 0.33f, 0.22f}, {0.92f, 0.80f, 0.62f}},
        {"writing_desk", "书桌", FurnitureCategory::Work, {2, 2}, 24, 5, 2, 1, 0.0f, false, true, "school", "desk",
            {0.42f, 0.28f, 0.16f}, {0.82f, 0.62f, 0.34f}},
        {"flower_pot", "花盆", FurnitureCategory::Decor, {1, 1}, 12, 3, 1, 2, 0.0f, false, true, "garden", "flower_pot",
            {0.45f, 0.23f, 0.12f}, {0.44f, 0.78f, 0.36f}},
        {"star_lamp", "星愿灯", FurnitureCategory::Light, {1, 1}, 18, 4, 4, 2, 4.0f, true, true, "star", "lamp",
            {0.25f, 0.33f, 0.46f}, {1.00f, 0.88f, 0.36f}},
        {"toy_shelf", "玩具架", FurnitureCategory::Storage, {3, 1}, 28, 6, 3, 1, 0.0f, false, true, "game", "toy_shelf",
            {0.36f, 0.24f, 0.16f}, {0.96f, 0.58f, 0.28f}},
        {"soft_rug", "地毯", FurnitureCategory::Decor, {3, 2}, 10, 4, 1, 0, 0.0f, false, false, "home", "rug",
            {0.60f, 0.22f, 0.24f}, {0.92f, 0.64f, 0.42f}},
        {"childhood_poster", "童年海报", FurnitureCategory::Poster, {2, 1}, 14, 3, 2, 0, 0.0f, false, false, "anime", "poster",
            {0.18f, 0.32f, 0.54f}, {0.98f, 0.82f, 0.28f}},
    };
}
