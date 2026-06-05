#pragma once

#include <glm/vec3.hpp>
#include <functional>
#include <string>

/**
 * TimeSystem — 游戏时间系统
 *
 * 管理游戏内的日夜循环，提供基于时间的光照计算。
 * 默认时间流速：1 真实秒 = 1 游戏分钟（1 游戏小时 = 60 真实秒）
 */
class TimeSystem {
public:
    // 初始化
    void init(float startHour = 6.0f);  // 默认从早上6点开始

    // 更新
    void update(float dt);

    // 时间控制
    void setHour(float hour);
    void setDaySpeed(float multiplier);  // 1.0 = 真实时间速度
    void pauseDayNightCycle(bool paused);

    // 获取当前时间
    float getHour() const { return currentHour; }
    int getHourInt() const { return static_cast<int>(currentHour); }
    int getMinuteInt() const {
        return static_cast<int>((currentHour - getHourInt()) * 60);
    }
    int getDay() const { return currentDay; }

    // 获取光照颜色（基于时间）
    glm::vec3 getAmbientLight() const;
    glm::vec3 getSkyColor() const;

    // 判断时间段
    bool isDaytime() const { return currentHour >= 6.0f && currentHour < 18.0f; }
    bool isNighttime() const { return !isDaytime(); }
    bool isDawn() const { return currentHour >= 5.0f && currentHour < 7.0f; }
    bool isDusk() const { return currentHour >= 17.0f && currentHour < 19.0f; }

    // 回调
    using TimeCallback = std::function<void(float hour)>;
    void setOnHourChange(TimeCallback cb) { onHourChange = cb; }

    // 获取时间描述字符串
    std::string getTimeString() const;

private:
    float currentHour;
    int currentDay;
    float daySpeed;  // 每小时真实秒数（默认 1.0 = 60真实秒 = 1游戏小时）
    bool paused;

    float lastHourInt;
    TimeCallback onHourChange;

    // 根据时间计算光照
    glm::vec3 computeAmbientLight() const;
};
