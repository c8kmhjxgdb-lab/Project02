#pragma once

#include <cstdint>
#include <vector>

enum class TimePeriod : uint8_t {
    Dawn,
    Morning,
    Afternoon,
    Dusk,
    Night,
    LateNight
};

struct TimeSnapshot {
    int day = 1;
    float hour = 10.0f;
    TimePeriod period = TimePeriod::Morning;
    bool isNewDay = false;
    bool isHourChanged = false;
};

struct TimePeriodRule {
    TimePeriod period = TimePeriod::Morning;
    float start = 7.0f;
    float finish = 12.0f;
};

struct TimeConfig {
    std::vector<TimePeriodRule> periodRules;
    float nightBonusStartHour = 19.0f;
    float restUntilHour = 7.0f;
    float restChildlikeHeartReward = 20.0f;
};
