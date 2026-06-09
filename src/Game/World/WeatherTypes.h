#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * 天气类型
 */
enum class WeatherType : uint8_t {
    Clear,      // 晴天
    Cloudy,     // 多云
    Rain,       // 雨天
    HeavyRain,  // 大雨
    Fog,        // 雾天
    Snow        // 雪天
};

struct WeatherRegionRule {
    bool indoor = false;
    bool allowParticles = true;
    std::string specialTag;
    std::vector<std::pair<WeatherType, int>> weights;
};

struct WeatherConfig {
    float defaultChangeInterval = 300.0f;
    std::unordered_map<std::string, WeatherRegionRule> regionRules;
    bool hasRegionRules = false;
};

namespace WeatherTypes {

const char* getWeatherName(WeatherType type);
const char* getWeatherId(WeatherType type);
WeatherType weatherFromId(const std::string& id);

}  // namespace WeatherTypes
