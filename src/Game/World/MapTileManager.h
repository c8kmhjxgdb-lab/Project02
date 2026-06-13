#pragma once

#include "TileMap.h"
#include <box2d/box2d.h>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <vector>

/**
 * 瓦片修改记录（用于存档）
 */
struct TileModification {
    int x, y;
    TileType oldType, newType;
    double timestamp;
};

/**
 * MapTileManager — 统一瓦片管理与物理同步
 *
 * 所有瓦片修改必须通过此类进行，以确保：
 * 1. Box2D 物理刚体自动同步
 * 2. 修改记录可用于存档
 * 3. 批量操作的事务性
 */
class MapTileManager {
public:
    // 仅绑定地图数据，不创建物理刚体。用于缓存/读档中的非当前区域。
    void bind(TileMap& map);

    // 初始化：从 TileMap 创建所有必要的物理刚体
    void init(TileMap& map, b2WorldId world);

    // 安全修改瓦片类型（自动同步物理）
    bool setTile(int x, int y, TileType newType);

    // 批量修改（事务性）
    bool setTiles(const std::vector<glm::ivec2>& positions, TileType newType);

    // 查询：获取某位置的 Box2D 刚体
    b2BodyId getBodyAt(int x, int y) const;

    // 查询：当前管理的物理刚体总数（用于回归测试，验证 dungeon 类
    // 大地图不会一次性塞入数千个 Box2D 静态刚体导致 buildPhysics 卡顿）
    size_t getBodyCount() const { return tileBodies.size(); }

    // 清理：释放所有物理资源
    void shutdown();

    // 获取修改记录（用于存档）
    const std::vector<TileModification>& getModifications() const { return modifications; }
    void clearModifications() { modifications.clear(); }
    bool replayModifications(const std::vector<TileModification>& savedModifications);

    // 加载时临时禁用修改记录
    void setReplayingMode(bool replaying) { isReplaying = replaying; }

private:
    TileMap* tileMap = nullptr;
    b2WorldId worldId = b2_nullWorldId;

    // 瓦片坐标 → Box2D 刚体ID 映射
    std::unordered_map<uint64_t, b2BodyId> tileBodies;

    std::vector<TileModification> modifications;
    bool isReplaying = false;

    // 编码瓦片坐标为单一 uint64 键
    static uint64_t packCoord(int x, int y) {
        return (static_cast<uint64_t>(x) << 32) | static_cast<uint32_t>(y);
    }

    // 为单个瓦片创建/移除物理刚体
    void createBodyFor(int x, int y);
    void destroyBodyFor(int x, int y);

    // 判断瓦片类型是否需要物理刚体
    static bool needsBody(TileType type) {
        return getTileDef(type).createsPhysicsBody;
    }
};
