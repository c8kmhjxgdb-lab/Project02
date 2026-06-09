#include "PathfindingSystem.h"
#include <algorithm>
#include <cmath>

bool PathfindingSystem::isWalkable(const TileMap& map, const glm::ivec2& pos,
                                   const PathfindingConfig& config) const {
    if (!map.isInBounds(pos.x, pos.y)) return false;

    TileType tile = map.getTile(pos.x, pos.y);
    Passability pass = getTileDef(tile).passability;

    if (pass == Passability::Blocked) return false;
    if (config.avoidWater && (pass == Passability::Water)) return false;
    if (config.avoidLava && pass == Passability::Lava) return false;

    return true;
}

std::vector<glm::ivec2> PathfindingSystem::getWalkableNeighbors(
    const TileMap& map, const glm::ivec2& pos, bool diagonal) const {

    std::vector<glm::ivec2> neighbors;
    static const glm::ivec2 cardinalDirs[] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };
    static const glm::ivec2 diagonalDirs[] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (const auto& dir : cardinalDirs) {
        glm::ivec2 neighbor = pos + dir;
        if (map.isInBounds(neighbor.x, neighbor.y)) {
            neighbors.push_back(neighbor);
        }
    }

    if (diagonal) {
        for (const auto& dir : diagonalDirs) {
            glm::ivec2 neighbor = pos + dir;
            if (map.isInBounds(neighbor.x, neighbor.y)) {
                neighbors.push_back(neighbor);
            }
        }
    }

    return neighbors;
}

float PathfindingSystem::heuristic(const glm::ivec2& a, const glm::ivec2& b,
                                   bool diagonal) const {
    float dx = static_cast<float>(std::abs(a.x - b.x));
    float dy = static_cast<float>(std::abs(a.y - b.y));

    if (diagonal) {
        // 对角距离（Chebyshev距离）
        return dx + dy + (std::sqrt(2.0f) - 2.0f) * std::min(dx, dy);
    }
    // 曼哈顿距离
    return dx + dy;
}

PathResult PathfindingSystem::findPath(const TileMap& map,
                                       const glm::ivec2& start,
                                       const glm::ivec2& end,
                                       const PathfindingConfig& config) {
    PathResult result;

    if (!map.isInBounds(start.x, start.y) ||
        !map.isInBounds(end.x, end.y)) {
        return result;
    }

    if (!isWalkable(map, end, config)) {
        // 目标不可达，尝试找最近的可到达点
        return findPathToNearest(map, start, {end}, config);
    }

    // 清空状态
    openList = {};
    closedSet.clear();
    nodeMap.clear();

    // 初始化起点
    PathNode startNode;
    startNode.position = start;
    startNode.gCost = 0.0f;
    startNode.hCost = heuristic(start, end, config.allowDiagonal);
    startNode.parentPos = glm::ivec2(-1, -1);  // 起点标记

    openList.push(startNode);
    nodeMap[packCoord(start)] = startNode;

    int iterations = 0;
    const int maxIterations = 50000;  // 对 60x60 地图足够（最多 3600 个节点）

    while (!openList.empty() && iterations < maxIterations) {
        ++iterations;

        // 取出fCost最小的节点
        PathNode current = openList.top();
        openList.pop();

        // 到达终点
        if (current.position == end) {
            // 回溯路径（通过坐标链）
            glm::ivec2 pos = end;
            while (pos.x != -1 && pos.y != -1) {
                result.path.insert(result.path.begin(), pos);
                auto it = nodeMap.find(packCoord(pos));
                if (it == nodeMap.end()) break;
                result.totalCost += it->second.gCost;
                pos = it->second.parentPos;
            }
            result.found = true;
            return result;
        }

        closedSet.insert(packCoord(current.position));

        // 遍历邻居
        auto neighbors = getWalkableNeighbors(map, current.position,
                                              config.allowDiagonal);
        for (const auto& neighborPos : neighbors) {
            uint64_t neighborKey = packCoord(neighborPos);

            if (closedSet.count(neighborKey)) continue;
            if (!isWalkable(map, neighborPos, config)) continue;

            float moveCost = 1.0f;
            if (config.allowDiagonal &&
                (neighborPos.x != current.position.x &&
                 neighborPos.y != current.position.y)) {
                moveCost = 1.414f; // sqrt(2)
            }

            // 地形代价
            TileType tile = map.getTile(neighborPos.x, neighborPos.y);
            const TileDef& def = getTileDef(tile);
            moveCost *= def.movementCost;

            float newGCost = current.gCost + moveCost;

            auto it = nodeMap.find(neighborKey);
            if (it == nodeMap.end() || newGCost < it->second.gCost) {
                // 新节点或更优路径
                PathNode newNode;
                newNode.position = neighborPos;
                newNode.gCost = newGCost;
                newNode.hCost = heuristic(neighborPos, end, config.allowDiagonal);
                newNode.parentPos = current.position;  // 存储父节点坐标，非指针

                nodeMap[neighborKey] = newNode;
                openList.push(newNode);
            }
        }
    }

    // 未找到路径
    return result;
}

PathResult PathfindingSystem::findPathToNearest(const TileMap& map,
                                                const glm::ivec2& start,
                                                const std::vector<glm::ivec2>& targets,
                                                const PathfindingConfig& config) {
    // 尝试找到每个目标的路径，返回最短的
    PathResult bestResult;
    float bestCost = std::numeric_limits<float>::max();

    for (const auto& target : targets) {
        if (!map.isInBounds(target.x, target.y)) continue;

        // 如果目标本身不可达，找最近的可通行邻居
        if (!isWalkable(map, target, config)) {
            auto neighbors = getWalkableNeighbors(map, target, config.allowDiagonal);
            std::vector<glm::ivec2> walkableNeighbors;
            for (const auto& n : neighbors) {
                if (isWalkable(map, n, config)) {
                    walkableNeighbors.push_back(n);
                }
            }
            if (walkableNeighbors.empty()) continue;

            // 找离原目标最近的邻居
            glm::ivec2 nearest = walkableNeighbors[0];
            float minDist = static_cast<float>(std::abs(nearest.x - target.x) +
                                               std::abs(nearest.y - target.y));
            for (size_t i = 1; i < walkableNeighbors.size(); ++i) {
                float dist = static_cast<float>(std::abs(walkableNeighbors[i].x - target.x) +
                                                std::abs(walkableNeighbors[i].y - target.y));
                if (dist < minDist) {
                    minDist = dist;
                    nearest = walkableNeighbors[i];
                }
            }

            PathResult result = findPath(map, start, nearest, config);
            if (result.found && result.totalCost < bestCost) {
                bestCost = result.totalCost;
                bestResult = result;
                // 在路径末尾添加原目标（虽然不可达）
                bestResult.path.push_back(target);
            }
        } else {
            PathResult result = findPath(map, start, target, config);
            if (result.found && result.totalCost < bestCost) {
                bestCost = result.totalCost;
                bestResult = result;
            }
        }
    }

    return bestResult;
}

bool PathfindingSystem::hasLineOfSight(const TileMap& map,
                                       const glm::ivec2& a,
                                       const glm::ivec2& b) const {
    // 使用Bresenham算法检查视线路径上的所有瓦片
    int dx = std::abs(b.x - a.x);
    int dy = std::abs(b.y - a.y);
    int sx = (a.x < b.x) ? 1 : -1;
    int sy = (a.y < b.y) ? 1 : -1;
    int err = dx - dy;

    int x = a.x, y = a.y;
    while (true) {
        if (!map.isInBounds(x, y)) return false;
        TileType tile = map.getTile(x, y);
        const TileDef& def = getTileDef(tile);
        if (def.passability == Passability::Blocked) return false;

        if (x == b.x && y == b.y) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }

    return true;
}
