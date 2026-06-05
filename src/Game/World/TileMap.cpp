#include "TileMap.h"

#include <cstdio>
#include <cstring>
#include <random>

TileMap::TileMap(int w, int h, float ts)
    : width(w), height(h), tileSize(ts) {
    tiles.resize(static_cast<size_t>(w) * h, TileType::Grass);
}

bool TileMap::load(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "TileMap: failed to open %s\n", filename);
        return false;
    }

    if (fscanf(f, "%d %d", &width, &height) != 2) {
        fprintf(stderr, "TileMap: invalid header in %s\n", filename);
        fclose(f);
        return false;
    }

    tiles.resize(static_cast<size_t>(width) * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int val;
            if (fscanf(f, "%d", &val) != 1) {
                fprintf(stderr, "TileMap: read error at (%d,%d) in %s\n", x, y, filename);
                fclose(f);
                return false;
            }
            tiles[static_cast<size_t>(y) * width + x] = static_cast<TileType>(val);
        }
    }

    fclose(f);
    return true;
}

void TileMap::generate(int seed) {
    std::mt19937 rng(static_cast<unsigned>(seed));
    std::uniform_int_distribution<int> dist(0, 100);

    tiles.resize(static_cast<size_t>(width) * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = dist(rng);
            TileType type = TileType::Grass;

            if (r < 5) {
                type = TileType::Water;
            } else if (r < 10) {
                type = TileType::Stone;
            } else if (r < 15) {
                type = TileType::Dirt;
            } else if (r < 18) {
                type = TileType::Wall;
            } else if (r < 22) {
                type = TileType::Path;
            }

            // 边界一定是墙
            if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                type = TileType::Wall;
            }

            tiles[static_cast<size_t>(y) * width + x] = type;
        }
    }
}

TileType TileMap::getTile(int x, int y) const {
    if (!isInBounds(x, y)) return TileType::Wall;
    return tiles[static_cast<size_t>(y) * width + x];
}

void TileMap::setTile(int x, int y, TileType type) {
    if (!isInBounds(x, y)) return;
    tiles[static_cast<size_t>(y) * width + x] = type;
}

bool TileMap::isSolid(int x, int y) const {
    if (!isInBounds(x, y)) return true;
    TileType t = getTile(x, y);
    return getTileDef(t).createsPhysicsBody;
}

Passability TileMap::getPassability(int x, int y) const {
    if (!isInBounds(x, y)) return Passability::Blocked;
    return getTileDef(getTile(x, y)).passability;
}

float TileMap::getMovementCost(int x, int y) const {
    if (!isInBounds(x, y)) return 0.0f;
    return getTileDef(getTile(x, y)).movementCost;
}

float TileMap::getDamagePerSecond(int x, int y) const {
    if (!isInBounds(x, y)) return 0.0f;
    return getTileDef(getTile(x, y)).damagePerSecond;
}

glm::ivec2 TileMap::worldToTile(float wx, float wy) const {
    int tx = static_cast<int>(floorf(wx / tileSize));
    int ty = static_cast<int>(floorf(wy / tileSize));
    return glm::ivec2(tx, ty);
}

glm::vec2 TileMap::tileToWorld(int tx, int ty) const {
    return glm::vec2(
        (tx + 0.5f) * tileSize,
        (ty + 0.5f) * tileSize
    );
}

bool TileMap::isInBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

void TileMap::clear() {
    tiles.clear();
    width = 0;
    height = 0;
}
