#pragma once

#include "Decoration.h"
#include "TileMap.h"
#include <vector>

/**
 * 在地图上生成装饰物
 * @param map 瓦片地图
 * @param decors 输出装饰物列表
 * @param seed 随机种子
 * @param density 密度（0.0-1.0，每个可装饰瓦片的装饰概率）
 */
void generateDecorations(const TileMap& map, std::vector<Decoration>& decors,
                         int seed, float density = 0.15f);
