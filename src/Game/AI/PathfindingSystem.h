#pragma once

#include "Game/World/TileMap.h"
#include <glm/geometric.hpp>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

/**
 * A* 寻路节点
 */
struct PathNode {
    glm::ivec2 position;
    float gCost;      // 从起点到当前节点的实际代价
    float hCost;      // 从当前节点到终点的启发式估计代价
    float fCost() const { return gCost + hCost; }
    glm::ivec2 parentPos;  // 父节点坐标（-1,-1 表示起点）

    bool operator>(const PathNode& other) const {
        return fCost() > other.fCost();
    }
};

/**
 * 寻路结果
 */
struct PathResult {
    std::vector<glm::ivec2> path;
    bool found;
    float totalCost;

    PathResult() : found(false), totalCost(0.0f) {}

    // 获取路径上的下一个移动方向
    glm::vec2 getNextDirection(const glm::ivec2& currentPos, float tileSize) const {
        if (path.size() < 2) return glm::vec2(0.0f);

        // 找到当前位在路径中的索引
        for (size_t i = 0; i < path.size() - 1; ++i) {
            if (path[i] == currentPos) {
                glm::vec2 next = glm::vec2(path[i + 1]) * tileSize + glm::vec2(tileSize * 0.5f);
                glm::vec2 current = glm::vec2(currentPos) * tileSize + glm::vec2(tileSize * 0.5f);
                return glm::normalize(next - current);
            }
        }
        return glm::vec2(0.0f);
    }

    // 简化路径（使用 Douglas-Peucker 算法移除共线点）
    PathResult simplify(float epsilon = 0.5f) const {
        if (!found || path.size() <= 2) return *this;

        PathResult result;
        result.found = true;
        result.totalCost = totalCost;

        // 简化的 Douglas-Peucker
        std::vector<glm::ivec2> simplified;
        simplified.push_back(path.front());

        size_t i = 1;
        while (i < path.size() - 1) {
            // 找到偏离当前方向最远的点
            glm::ivec2 dir = path[i] - simplified.back();
            size_t furthest = i;
            float maxDist = 0.0f;

            while (i < path.size() - 1) {
                glm::ivec2 newDir = path[i + 1] - simplified.back();
                // 如果方向变化超过阈值，停止
                float cross = std::abs(dir.x * newDir.y - dir.y * newDir.x);
                float len = std::sqrt(static_cast<float>(dir.x * dir.x + dir.y * dir.y));
                if (len > 0) cross /= len;

                if (cross > epsilon) break;

                dir = newDir;
                furthest = i + 1;
                ++i;
            }

            simplified.push_back(path[furthest]);
            i = furthest + 1;
        }

        simplified.push_back(path.back());
        result.path = std::move(simplified);
        return result;
    }
};

/**
 * 寻路配置
 */
struct PathfindingConfig {
    bool avoidWater = true;      // 避免水域
    bool avoidLava = true;       // 避免熔岩
    bool allowDiagonal = false;  // 允许对角移动
    float maxPathLength = 200.0f; // 最大路径长度
};

/**
 * PathfindingSystem — A* 寻路系统
 *
 * 为敌人AI提供路径规划功能，支持地形代价、障碍物回避。
 */
class PathfindingSystem {
public:
    // 寻找路径
    PathResult findPath(const TileMap& map,
                       const glm::ivec2& start,
                       const glm::ivec2& end,
                       const PathfindingConfig& config = {});

    // 批量寻路（多个目标，返回最近的可到达点）
    PathResult findPathToNearest(const TileMap& map,
                                const glm::ivec2& start,
                                const std::vector<glm::ivec2>& targets,
                                const PathfindingConfig& config = {});

    // 射线投射（视线检查）
    bool hasLineOfSight(const TileMap& map,
                       const glm::ivec2& a,
                       const glm::ivec2& b) const;

    // 获取可达的邻居瓦片
    std::vector<glm::ivec2> getWalkableNeighbors(const TileMap& map,
                                                 const glm::ivec2& pos,
                                                 bool diagonal = false) const;

    // 距离启发式（曼哈顿或欧几里得）
    float heuristic(const glm::ivec2& a, const glm::ivec2& b,
                   bool diagonal = false) const;

private:
    // 开放列表（优先队列）
    std::priority_queue<PathNode, std::vector<PathNode>,
                       std::greater<PathNode>> openList;

    // 关闭列表（已访问）
    std::unordered_set<uint64_t> closedSet;

    // 节点映射（坐标 → 节点数据）
    std::unordered_map<uint64_t, PathNode> nodeMap;

    // 编码坐标为单一键
    static uint64_t packCoord(const glm::ivec2& pos) {
        return (static_cast<uint64_t>(pos.x) << 32) |
               static_cast<uint32_t>(pos.y);
    }

    // 判断瓦片是否可通行
    bool isWalkable(const TileMap& map, const glm::ivec2& pos,
                   const PathfindingConfig& config) const;
};
