#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>

/**
 * 掉落物类型
 */
enum class DropType : uint8_t {
    Coin,      // 星币
    Health,    // 生命药水
    Mana,      // 魔力药水
    Item       // 随机物品
};

/**
 * 掉落物唯一ID
 */
struct DropId {
    int id;
    bool operator==(const DropId& other) const { return id == other.id; }
    bool operator!=(const DropId& other) const { return id != other.id; }
};

inline DropId makeDropId(int i) { return DropId{ i }; }
inline const DropId DROP_NULL = DropId{ -1 };

/**
 * Drop - 掉落物数据结构
 */
struct Drop {
    b2BodyId bodyId;
    DropId id;
    DropType type;
    int value;           // 数量/值
    glm::vec3 color;
    float lifetime;      // 存在时间（自动消失）
    float maxLifetime;
    bool collected;
    float bobTimer;      // 上下浮动动画

    // 拾取特效
    float collectTimer;
    bool collecting;

    // 活跃状态
    bool active;

    Drop()
        : bodyId(b2_nullBodyId), id(DROP_NULL), type(DropType::Coin)
        , value(1), color(1, 1, 0), lifetime(0), maxLifetime(15.0f)
        , collected(false), bobTimer(0), collectTimer(0), collecting(false)
        , active(true) {}
};

/**
 * DropManager - 掉落物管理器
 */
class DropManager {
public:
    DropManager();
    ~DropManager();

    void init();

    // 生成掉落物
    DropId spawn(b2WorldId world, const glm::vec2& pos, DropType type, int value);

    // 更新（自动拾取检测、生命周期）
    void update(float dt, const glm::vec2& playerPos);

    // 获取活跃掉落物列表
    const std::vector<Drop>& getActive() const { return drops; }

    // 玩家拾取
    void collect(DropId id);

    // 强制销毁
    void destroy(DropId id);

    // 清空
    void clear();

    // 设置自动拾取范围
    void setCollectRange(float range) { collectRange = range; }
    float getCollectRange() const { return collectRange; }

    // 获取下一个可用ID
    int getNextId() const { return nextId; }

private:
    std::vector<Drop> drops;
    int nextId;
    float collectRange;

    Drop* find(DropId id);
    const Drop* find(DropId id) const;

    b2BodyId createBody(b2WorldId world, const glm::vec2& pos);
};
