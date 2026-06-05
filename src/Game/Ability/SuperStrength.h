#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>

/**
 * SuperStrength - 超人抓举系统
 *
 * 使用Box2D距离关节实现抓取和投掷功能。
 * 抓取时物体被吸附在玩家前方固定位置，投掷时施加冲量。
 */
class SuperStrength {
public:
    SuperStrength();

    // 尝试抓取最近的物体
    // 返回是否成功抓取
    bool tryGrab(b2WorldId world, b2BodyId playerBody,
                 const glm::vec2& grabDirection, float grabRange = 3.0f);

    // 投掷抓取的物体
    void throwObject(const glm::vec2& throwDirection, float throwForce = 20.0f);

    // 释放物体（不投掷，直接断开）
    void release();

    // 是否正在抓取
    bool isGrabbing() const { return b2Body_IsValid(grabbedBodyId); }

    // 获取被抓物体的位置
    glm::vec2 getGrabbedPosition() const;

    // 获取被抓物体的ID
    b2BodyId getGrabbedBodyId() const { return grabbedBodyId; }

    // 设置最大抓取质量（超过此质量的物体无法抓起）
    void setMaxGrabMass(float mass) { maxGrabMass = mass; }
    float getMaxGrabMass() const { return maxGrabMass; }

    // 设置抓取距离
    void setGrabDistance(float dist) { grabDistance = dist; }
    float getGrabDistance() const { return grabDistance; }

    // 设置抓取目标的碰撞过滤(类别/掩码)
    void setGrabFilter(uint32_t category, uint32_t mask) {
        grabCategoryBits = category;
        grabMaskBits = mask;
    }

private:
    b2WorldId currentWorld;
    b2BodyId playerBodyId;
    b2BodyId grabbedBodyId;
    b2JointId grabJointId;

    float grabDistance;      // 物体保持在玩家前方的距离
    float maxGrabMass;       // 最大抓取质量

    // 过滤：抓取时排除哪些碰撞类别
    // 默认排除墙壁/地形(0x0001) 和 投射物(0x0002),只抓 0x0004 敌人/可拾取物
    uint32_t grabCategoryBits = 0x0004;
    uint32_t grabMaskBits     = 0x0004;
};
