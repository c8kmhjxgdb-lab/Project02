#pragma once

#include "TileMap.h"

/**
 * TerrainGenerator — 噪声地形生成
 *
 * 使用 Perlin 噪声 + fBm（分形布朗运动）生成连贯地形，
 * 替代纯随机的噪点状地形。
 */
class TerrainGenerator {
public:
    // 使用 Perlin 噪声生成连贯地形
    static void generateCoherent(
        TileMap& map,
        int seed,
        float scale = 0.08f,        // 噪声缩放
        float persistence = 0.5f,   // 持久度
        int octaves = 4,            // 八度数
        float waterLevel = 0.0f     // 水平面高度（噪声值阈值）
    );

    // 生成路径网络（连接关键区域）
    static void generatePaths(TileMap& map, int numPaths = 3, unsigned int seed = 0);

private:
    // 2D Perlin 噪声（基于种子）
    static float perlinNoise(float x, float y, unsigned int seed);

    // 分形布朗运动
    static float fbm(float x, float y, int octaves, float persistence, unsigned int seed);

    // 高度 → 瓦片类型映射
    static TileType heightToTile(float height, float waterLevel);

    // 后处理：平滑边界
    static void smoothEdges(TileMap& map);
};
