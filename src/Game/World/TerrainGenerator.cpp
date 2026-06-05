#include "TerrainGenerator.h"

#include <cmath>
#include <random>
#include <algorithm>

// Hash-based pseudo-random gradient for Perlin noise
static float hashGrad(int ix, int iy, unsigned int seed) {
    unsigned int n = static_cast<unsigned int>(ix * 374761393 + iy * 668265263 + seed);
    n = (n ^ (n >> 13)) * 1274126177;
    return static_cast<float>(n & 0x7fffffff) / 2147483647.0f * 2.0f - 1.0f;
}

// Smoothstep interpolation
static float smoothstep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float TerrainGenerator::perlinNoise(float x, float y, unsigned int seed) {
    // Grid cell
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - static_cast<float>(ix);
    float fy = y - static_cast<float>(iy);

    // Gradient dot products
    float g00 = hashGrad(ix, iy, seed);
    float g10 = hashGrad(ix + 1, iy, seed);
    float g01 = hashGrad(ix, iy + 1, seed);
    float g11 = hashGrad(ix + 1, iy + 1, seed);

    // Interpolation
    float u = smoothstep(fx);
    float v = smoothstep(fy);

    float nx0 = g00 * (fx - 0.0f) + u * (g10 * (fx - 1.0f) - g00 * (fx - 0.0f));
    float nx1 = g01 * (fx - 0.0f) + u * (g11 * (fx - 1.0f) - g01 * (fx - 0.0f));

    return nx0 + v * (nx1 - nx0);
}

float TerrainGenerator::fbm(float x, float y, int octaves, float persistence, unsigned int seed) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += perlinNoise(x * frequency, y * frequency, seed + i * 1000) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue;
}

TileType TerrainGenerator::heightToTile(float height, float waterLevel) {
    // Height is normalized to roughly [-1, 1] range from fBm
    if (height < waterLevel - 0.3f) return TileType::DeepWater;
    if (height < waterLevel) return TileType::Water;
    if (height < waterLevel + 0.15f) return TileType::Sand;
    if (height < waterLevel + 0.4f) return TileType::Grass;
    if (height < waterLevel + 0.55f) return TileType::Dirt;
    if (height < waterLevel + 0.7f) return TileType::Stone;
    return TileType::Wall;
}

void TerrainGenerator::smoothEdges(TileMap& map) {
    // Simple cellular automata smoothing pass
    std::vector<TileType> oldTiles = map.tiles;
    for (int y = 1; y < map.height - 1; ++y) {
        for (int x = 1; x < map.width - 1; ++x) {
            // Count blocked neighbors
            int blocked = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    TileType t = oldTiles[static_cast<size_t>(y + dy) * map.width + (x + dx)];
                    if (!isWalkable(t)) blocked++;
                }
            }
            TileType center = oldTiles[static_cast<size_t>(y) * map.width + x];
            if (blocked >= 5) {
                if (isWalkable(center) && center != TileType::Water && center != TileType::DeepWater) {
                    map.tiles[static_cast<size_t>(y) * map.width + x] = TileType::Stone;
                }
            } else if (blocked <= 3) {
                if (!isWalkable(center) && center != TileType::Wall) {
                    map.tiles[static_cast<size_t>(y) * map.width + x] = TileType::Grass;
                }
            }
        }
    }
}

void TerrainGenerator::generateCoherent(
    TileMap& map,
    int seed,
    float scale,
    float persistence,
    int octaves,
    float waterLevel
) {
    map.tiles.resize(static_cast<size_t>(map.width) * map.height);

    unsigned int useed = static_cast<unsigned int>(seed);

    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            // Skip borders — always wall
            if (x == 0 || y == 0 || x == map.width - 1 || y == map.height - 1) {
                map.tiles[static_cast<size_t>(y) * map.width + x] = TileType::Wall;
                continue;
            }

            // Sample fBm at this position
            float nx = static_cast<float>(x) * scale;
            float ny = static_cast<float>(y) * scale;
            float height = fbm(nx, ny, octaves, persistence, useed);

            map.tiles[static_cast<size_t>(y) * map.width + x] = heightToTile(height, waterLevel);
        }
    }

    // Smooth transitions
    smoothEdges(map);
}

void TerrainGenerator::generatePaths(TileMap& map, int numPaths, unsigned int seed) {
    std::mt19937 rng(static_cast<unsigned>(seed));

    for (int p = 0; p < numPaths; ++p) {
        // Pick two random points on the map
        std::uniform_int_distribution<int> distX(2, map.width - 3);
        std::uniform_int_distribution<int> distY(2, map.height - 3);

        int x0 = distX(rng);
        int y0 = distY(rng);
        int x1 = distX(rng);
        int y1 = distY(rng);

        // Simple L-shaped path
        int cx = x0;
        int cy = y0;

        while (cx != x1 || cy != y1) {
            if (map.isInBounds(cx, cy)) {
                TileType t = map.getTile(cx, cy);
                if (isWalkable(t) && t != TileType::Water && t != TileType::DeepWater) {
                    map.setTile(cx, cy, TileType::Path);
                }
            }

            // Move toward target
            if (cx != x1 && (rng() % 2 == 0 || cy == y1)) {
                cx += (x1 > cx) ? 1 : -1;
            } else if (cy != y1) {
                cy += (y1 > cy) ? 1 : -1;
            }
        }

        // Also pave the endpoint
        if (map.isInBounds(x1, y1)) {
            TileType t = map.getTile(x1, y1);
            if (isWalkable(t) && t != TileType::Water && t != TileType::DeepWater) {
                map.setTile(x1, y1, TileType::Path);
            }
        }
    }
}
