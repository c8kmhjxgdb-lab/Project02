#pragma once

#include <glm/vec2.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <random>

/**
 * 数学工具函数
 */
namespace Math {

    // 角度转弧度
    inline float degToRad(float degrees) {
        return degrees * glm::pi<float>() / 180.0f;
    }

    // 弧度转角度
    inline float radToDeg(float radians) {
        return radians * 180.0f / glm::pi<float>();
    }

    // 向量归一化（安全版本，避免除零）
    inline glm::vec2 normalize(const glm::vec2& v) {
        float len = glm::length(v);
        if (len < 0.0001f) return glm::vec2(1, 0);
        return v / len;
    }

    // 计算从点A指向点B的方向
    inline glm::vec2 directionTo(const glm::vec2& from, const glm::vec2& to) {
        return normalize(to - from);
    }

    // 计算两点间距离
    inline float distance(const glm::vec2& a, const glm::vec2& b) {
        return glm::distance(a, b);
    }

    // 线性插值
    inline float lerp(float a, float b, float t) {
        return a + (b - a) * glm::clamp(t, 0.0f, 1.0f);
    }

    // 向量线性插值
    inline glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float t) {
        return glm::mix(a, b, glm::clamp(t, 0.0f, 1.0f));
    }

    // 限制值在范围内
    inline float clamp(float value, float minVal, float maxVal) {
        return glm::clamp(value, minVal, maxVal);
    }

    // 限制向量长度
    inline glm::vec2 clampLength(const glm::vec2& v, float maxLength) {
        float len = glm::length(v);
        if (len > maxLength) {
            return v * (maxLength / len);
        }
        return v;
    }

    // 旋转向量
    inline glm::vec2 rotate(const glm::vec2& v, float angle) {
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        return glm::vec2(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
    }

    // 计算向量的角度（弧度）
    inline float angle(const glm::vec2& v) {
        return std::atan2(v.y, v.x);
    }

    // 两个向量之间的角度差
    inline float angleBetween(const glm::vec2& a, const glm::vec2& b) {
        return std::atan2(b.y, b.x) - std::atan2(a.y, a.x);
    }

    // 点到直线的最近点
    inline glm::vec2 closestPointOnLine(const glm::vec2& lineStart,
                                         const glm::vec2& lineEnd,
                                         const glm::vec2& point) {
        glm::vec2 lineDir = lineEnd - lineStart;
        float lineLen = glm::length(lineDir);
        if (lineLen < 0.0001f) return lineStart;

        lineDir /= lineLen;
        glm::vec2 toPoint = point - lineStart;
        float t = glm::dot(toPoint, lineDir);
        t = glm::clamp(t, 0.0f, lineLen);

        return lineStart + lineDir * t;
    }

    // 圆与圆的碰撞检测
    inline bool circleCircle(const glm::vec2& c1, float r1,
                             const glm::vec2& c2, float r2) {
        float dist = glm::distance(c1, c2);
        return dist < (r1 + r2);
    }

    // 点是否在圆内
    inline bool pointInCircle(const glm::vec2& point, const glm::vec2& center, float radius) {
        return glm::distance(point, center) < radius;
    }

    // 点是否在矩形内
    inline bool pointInRect(const glm::vec2& point, const glm::vec2& min, const glm::vec2& max) {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y;
    }

    // 矩形与矩形碰撞
    inline bool rectRect(const glm::vec2& min1, const glm::vec2& max1,
                         const glm::vec2& min2, const glm::vec2& max2) {
        return min1.x <= max2.x && max1.x >= min2.x &&
               min1.y <= max2.y && max1.y >= min2.y;
    }

    // 简单的伪随机数生成（确定性）
    inline float random(float minVal, float maxVal, unsigned int seed) {
        std::mt19937 gen(seed);
        std::uniform_real_distribution<float> dist(minVal, maxVal);
        return dist(gen);
    }

    // 整数随机
    inline int randomInt(int minVal, int maxVal, unsigned int seed) {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(minVal, maxVal);
        return dist(gen);
    }

    // 从种子生成可重复的浮点随机数（不使用std::mt19937，更轻量）
    inline float hashRandom(unsigned int seed) {
        unsigned int x = seed;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return static_cast<float>(x) / static_cast<float>(0xFFFFFFFFu);
    }

    // 2D噪声（简单的值噪声）
    inline float noise2D(const glm::vec2& p, unsigned int seed = 0) {
        glm::ivec2 i = glm::floor(p);
        glm::vec2 f = glm::fract(p);

        // 四个角的哈希值
        float a = hashRandom(static_cast<unsigned int>(i.x * 374761393 + i.y * 668265263 + seed));
        float b = hashRandom(static_cast<unsigned int>((i.x + 1) * 374761393 + i.y * 668265263 + seed));
        float c = hashRandom(static_cast<unsigned int>(i.x * 374761393 + (i.y + 1) * 668265263 + seed));
        float d = hashRandom(static_cast<unsigned int>((i.x + 1) * 374761393 + (i.y + 1) * 668265263 + seed));

        // 平滑插值
        glm::vec2 u = f * f * (3.0f - 2.0f * f);

        return glm::mix(
            glm::mix(a, b, u.x),
            glm::mix(c, d, u.x),
            u.y
        );
    }

}  // namespace Math
