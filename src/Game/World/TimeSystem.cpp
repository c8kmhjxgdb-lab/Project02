#include "TimeSystem.h"

#include "Game/Data/TimeConfigLoader.h"

#include <glm/glm.hpp>
#include <sstream>
#include <iomanip>
#include <algorithm>

void TimeSystem::init(float startHour) {
    currentHour = startHour;
    currentDay = 1;
    daySpeed = 1.0f;  // 默认：60真实秒 = 1游戏小时
    paused = false;
    lastHourInt = static_cast<float>(static_cast<int>(startHour));
    hourChangedFlag = false;
    newDayFlag = false;
    resetDefaultPeriodRules();
}

void TimeSystem::resetDefaultPeriodRules() {
    periodRules = {
        {TimePeriod::Dawn, 5.0f, 7.0f},
        {TimePeriod::Morning, 7.0f, 12.0f},
        {TimePeriod::Afternoon, 12.0f, 17.0f},
        {TimePeriod::Dusk, 17.0f, 19.0f},
        {TimePeriod::Night, 19.0f, 23.0f},
        {TimePeriod::LateNight, 23.0f, 5.0f},
    };
}

bool TimeSystem::loadConfig(LuaVM& lua, const char* path) {
    TimeConfig defaults;
    defaults.periodRules = periodRules;
    defaults.nightBonusStartHour = nightBonusStartHour;
    defaults.restUntilHour = restUntilHour;
    defaults.restChildlikeHeartReward = restChildlikeHeartReward;

    TimeConfig config;
    if (!TimeConfigLoader::load(lua, path, defaults, config)) {
        return false;
    }

    periodRules = std::move(config.periodRules);
    nightBonusStartHour = config.nightBonusStartHour;
    restUntilHour = config.restUntilHour;
    restChildlikeHeartReward = config.restChildlikeHeartReward;
    return true;
}

void TimeSystem::update(float dt) {
    if (paused) return;

    // 更新小时（daySpeed = 1.0 意味着 60 真实秒 = 1 游戏小时）
    currentHour += dt / (60.0f / daySpeed);

    // 日期循环
    while (currentHour >= 24.0f) {
        currentHour -= 24.0f;
        currentDay++;
        newDayFlag = true;
    }

    // 整点回调
    int currentHourIntValue = getHourInt();
    if (static_cast<int>(currentHourIntValue) != static_cast<int>(lastHourInt)) {
        lastHourInt = static_cast<float>(currentHourIntValue);
        hourChangedFlag = true;
        if (onHourChange) onHourChange(currentHour);
    }
}

void TimeSystem::setHour(float hour) {
    if (hour < 0.0f || hour >= 24.0f) return;
    currentHour = hour;
    lastHourInt = static_cast<float>(static_cast<int>(hour));
}

void TimeSystem::setDay(int day) {
    currentDay = day > 0 ? day : 1;
}

void TimeSystem::setDaySpeed(float multiplier) {
    daySpeed = multiplier > 0.0f ? multiplier : 0.0f;
}

void TimeSystem::pauseDayNightCycle(bool shouldPause) {
    paused = shouldPause;
}

void TimeSystem::restUntil(float hour) {
    if (hour < 0.0f || hour >= 24.0f) return;

    if (hour <= currentHour) {
        currentDay++;
        newDayFlag = true;
    }
    currentHour = hour;
    lastHourInt = static_cast<float>(static_cast<int>(hour));
    hourChangedFlag = true;
    if (onHourChange) onHourChange(currentHour);
}

bool TimeSystem::hourInRange(float hour, float start, float finish) {
    if (start < finish) {
        return hour >= start && hour < finish;
    }
    if (start > finish) {
        return hour >= start || hour < finish;
    }
    return true;
}

bool TimeSystem::isDaytime() const {
    TimePeriod period = getPeriod();
    return period == TimePeriod::Dawn ||
           period == TimePeriod::Morning ||
           period == TimePeriod::Afternoon ||
           period == TimePeriod::Dusk;
}

bool TimeSystem::isNighttime() const {
    TimePeriod period = getPeriod();
    return period == TimePeriod::Night || period == TimePeriod::LateNight;
}

bool TimeSystem::isDawn() const {
    return getPeriod() == TimePeriod::Dawn;
}

bool TimeSystem::isDusk() const {
    return getPeriod() == TimePeriod::Dusk;
}

glm::vec3 TimeSystem::computeAmbientLight() const {
    // 根据时间插值光照颜色
    float h = currentHour;

    if (h >= 6.0f && h < 12.0f) {
        // 早晨：暗黄 -> 亮白
        float t = (h - 6.0f) / 6.0f;
        return glm::mix(glm::vec3(0.8f, 0.6f, 0.4f),
                       glm::vec3(1.0f, 0.98f, 0.9f), t);
    } else if (h >= 12.0f && h < 17.0f) {
        // 白天：亮白 -> 金黄
        float t = (h - 12.0f) / 5.0f;
        return glm::mix(glm::vec3(1.0f, 0.98f, 0.9f),
                       glm::vec3(1.0f, 0.9f, 0.6f), t);
    } else if (h >= 17.0f && h < 20.0f) {
        // 黄昏：金黄 -> 暗红
        float t = (h - 17.0f) / 3.0f;
        return glm::mix(glm::vec3(1.0f, 0.9f, 0.6f),
                       glm::vec3(0.6f, 0.3f, 0.2f), t);
    } else if (h >= 20.0f && h < 23.0f) {
        // 夜晚过渡：暗红 -> 深蓝
        float t = (h - 20.0f) / 3.0f;
        return glm::mix(glm::vec3(0.6f, 0.3f, 0.2f),
                       glm::vec3(0.1f, 0.1f, 0.3f), t);
    } else {
        // 深夜：深蓝
        return glm::vec3(0.1f, 0.1f, 0.3f);
    }
}

glm::vec3 TimeSystem::getAmbientLight() const {
    return computeAmbientLight();
}

glm::vec3 TimeSystem::getSkyColor() const {
    float h = currentHour;

    if (h >= 5.0f && h < 7.0f) {
        // 黎明：橙红渐变
        float t = (h - 5.0f) / 2.0f;
        return glm::mix(glm::vec3(0.1f, 0.1f, 0.3f),
                       glm::vec3(1.0f, 0.5f, 0.3f), t);
    } else if (h >= 7.0f && h < 17.0f) {
        // 白天：蓝天
        return glm::vec3(0.4f, 0.6f, 0.9f);
    } else if (h >= 17.0f && h < 19.0f) {
        // 黄昏：橙红
        float t = (h - 17.0f) / 2.0f;
        return glm::mix(glm::vec3(0.4f, 0.6f, 0.9f),
                       glm::vec3(1.0f, 0.4f, 0.2f), t);
    } else if (h >= 19.0f && h < 21.0f) {
        // 傍晚：紫红
        float t = (h - 19.0f) / 2.0f;
        return glm::mix(glm::vec3(1.0f, 0.4f, 0.2f),
                       glm::vec3(0.2f, 0.1f, 0.4f), t);
    } else {
        // 夜晚：深蓝黑
        return glm::vec3(0.05f, 0.05f, 0.15f);
    }
}

TimePeriod TimeSystem::getPeriod() const {
    for (const TimePeriodRule& rule : periodRules) {
        if (hourInRange(currentHour, rule.start, rule.finish)) {
            return rule.period;
        }
    }
    return TimePeriod::LateNight;
}

TimeSnapshot TimeSystem::getSnapshot() const {
    TimeSnapshot snapshot;
    snapshot.day = currentDay;
    snapshot.hour = currentHour;
    snapshot.period = getPeriod();
    snapshot.isNewDay = newDayFlag;
    snapshot.isHourChanged = hourChangedFlag;
    return snapshot;
}

bool TimeSystem::consumeHourChangedFlag() {
    bool value = hourChangedFlag;
    hourChangedFlag = false;
    return value;
}

bool TimeSystem::consumeNewDayFlag() {
    bool value = newDayFlag;
    newDayFlag = false;
    return value;
}

std::string TimeSystem::getTimeString() const {
    int hour = getHourInt();
    int minute = getMinuteInt();
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hour << ":"
       << std::setfill('0') << std::setw(2) << minute;
    return ss.str();
}

const char* TimeSystem::getPeriodName(TimePeriod period) {
    switch (period) {
        case TimePeriod::Dawn:      return "黎明";
        case TimePeriod::Morning:   return "上午";
        case TimePeriod::Afternoon: return "下午";
        case TimePeriod::Dusk:      return "黄昏";
        case TimePeriod::Night:     return "夜晚";
        case TimePeriod::LateNight: return "深夜";
        default:                    return "未知";
    }
}
