#pragma once

#include <glm/vec2.hpp>
#include <cstdint>

/**
 * 装饰物类型
 */
enum class DecorType : uint8_t {
    None = 0,
    Tree = 1,
    Bush = 2,
    Flower = 3,
    TallGrass = 4,
    Rock = 5,
    Stump = 6,
    Count
};

/**
 * 装饰物实例数据
 */
struct Decoration {
    DecorType type;
    uint8_t variant;      // 变种（同类型不同外观）
    uint8_t rotation;     // 旋转角度（0-3，每90度）
    uint8_t scale;        // 缩放（0=小, 1=中, 2=大）
    int16_t tileX;
    int16_t tileY;

    float getScaleFactor() const {
        return scale == 0 ? 0.7f : (scale == 1 ? 1.0f : 1.3f);
    }

    float getRotationRadians() const {
        return static_cast<float>(rotation) * 3.14159265f * 0.5f;
    }
};

/**
 * 判断是否为高大装饰物（需要遮挡排序）
 */
inline bool isTallDecor(DecorType type) {
    return type == DecorType::Tree || type == DecorType::Stump;
}
