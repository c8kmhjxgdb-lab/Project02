#pragma once

#include "Game/World/TimeTypes.h"

#include <glm/vec3.hpp>
#include <functional>
#include <string>
#include <vector>

class LuaVM;

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
    bool loadConfig(LuaVM& lua, const char* path);

    // 更新
    void update(float dt);

    // 时间控制
    void setHour(float hour);
    void setDay(int day);
    void setDaySpeed(float multiplier);  // 1.0 = 真实时间速度
    void pauseDayNightCycle(bool shouldPause);
    void restUntil(float hour);

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
    bool isDaytime() const;
    bool isNighttime() const;
    bool isDawn() const;
    bool isDusk() const;
    TimePeriod getPeriod() const;
    TimeSnapshot getSnapshot() const;
    bool consumeHourChangedFlag();
    bool consumeNewDayFlag();
    float getRestUntilHour() const { return restUntilHour; }
    float getRestChildlikeHeartReward() const { return restChildlikeHeartReward; }
    float getNightBonusStartHour() const { return nightBonusStartHour; }

    // 回调
    using TimeCallback = std::function<void(float hour)>;
    void setOnHourChange(TimeCallback cb) { onHourChange = cb; }

    // 获取时间描述字符串
    std::string getTimeString() const;
    static const char* getPeriodName(TimePeriod period);

private:
    float currentHour;
    int currentDay;
    float daySpeed;  // 每小时真实秒数（默认 1.0 = 60真实秒 = 1游戏小时）
    bool paused;

    float lastHourInt;
    bool hourChangedFlag = false;
    bool newDayFlag = false;
    TimeCallback onHourChange;
    std::vector<TimePeriodRule> periodRules;
    float nightBonusStartHour = 19.0f;
    float restUntilHour = 7.0f;
    float restChildlikeHeartReward = 20.0f;

    // 根据时间计算光照
    glm::vec3 computeAmbientLight() const;
    void resetDefaultPeriodRules();
    static bool hourInRange(float hour, float start, float finish);
};
