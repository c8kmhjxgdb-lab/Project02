#include "DecorationGen.h"

#include <random>
#include <algorithm>

void generateDecorations(const TileMap& map, std::vector<Decoration>& decors,
                         int seed, float density) {
    std::mt19937 rng(static_cast<unsigned>(seed));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Discrete buckets for each attribute. variant/rotation use 4 buckets
    // (0..3), scale uses 3 buckets (0..2). The previous code did
    // `static_cast<uint8_t>(dist(rng) * 4)` which could yield 4 when
    // dist(rng) is exactly 1.0 — clamp to the valid range.
    constexpr uint8_t VARIANT_BUCKETS = 4;
    constexpr uint8_t ROTATION_BUCKETS = 4;
    constexpr uint8_t SCALE_BUCKETS = 3;

    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            if (dist(rng) > density) continue;

            TileType tile = map.getTile(x, y);
            // Only place decorations on walkable ground tiles
            if (tile != TileType::Grass && tile != TileType::Dirt && tile != TileType::Sand
                && tile != TileType::Path) {
                continue;
            }

            // Don't place on borders
            if (x == 0 || y == 0 || x == map.width - 1 || y == map.height - 1) {
                continue;
            }

            Decoration decor;
            decor.tileX = static_cast<int16_t>(x);
            decor.tileY = static_cast<int16_t>(y);

            // Choose decoration type based on ground type
            float r = dist(rng);
            if (tile == TileType::Grass) {
                if (r < 0.30f) decor.type = DecorType::Tree;
                else if (r < 0.55f) decor.type = DecorType::Bush;
                else if (r < 0.80f) decor.type = DecorType::Flower;
                else if (r < 0.92f) decor.type = DecorType::TallGrass;
                else decor.type = DecorType::Stump;
            } else if (tile == TileType::Sand) {
                decor.type = DecorType::Rock;
            } else if (tile == TileType::Path) {
                // Paths rarely get decorations
                if (dist(rng) < 0.05f) {
                    decor.type = DecorType::Rock;
                } else {
                    continue;
                }
            } else {
                decor.type = DecorType::Rock;
            }

            decor.variant = static_cast<uint8_t>(std::min<int>(VARIANT_BUCKETS - 1,
                static_cast<int>(dist(rng) * VARIANT_BUCKETS)));
            decor.rotation = static_cast<uint8_t>(std::min<int>(ROTATION_BUCKETS - 1,
                static_cast<int>(dist(rng) * ROTATION_BUCKETS)));
            decor.scale = static_cast<uint8_t>(std::min<int>(SCALE_BUCKETS - 1,
                static_cast<int>(dist(rng) * SCALE_BUCKETS)));

            decors.push_back(decor);
        }
    }
}
