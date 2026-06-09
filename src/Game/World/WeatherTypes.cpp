#include "Game/World/WeatherTypes.h"

#include <cctype>

namespace WeatherTypes {

const char* getWeatherName(WeatherType type) {
    switch (type) {
        case WeatherType::Clear:      return "晴天";
        case WeatherType::Cloudy:     return "多云";
        case WeatherType::Rain:       return "雨天";
        case WeatherType::HeavyRain:  return "大雨";
        case WeatherType::Fog:        return "雾天";
        case WeatherType::Snow:       return "雪天";
        default:                      return "未知";
    }
}

const char* getWeatherId(WeatherType type) {
    switch (type) {
        case WeatherType::Clear:      return "Clear";
        case WeatherType::Cloudy:     return "Cloudy";
        case WeatherType::Rain:       return "Rain";
        case WeatherType::HeavyRain:  return "HeavyRain";
        case WeatherType::Fog:        return "Fog";
        case WeatherType::Snow:       return "Snow";
        default:                      return "Clear";
    }
}

WeatherType weatherFromId(const std::string& id) {
    std::string normalized;
    normalized.reserve(id.size());
    for (char ch : id) {
        if (ch != '_' && ch != '-' && ch != ' ') {
            normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
    }

    if (normalized == "cloudy" || id == "多云") return WeatherType::Cloudy;
    if (normalized == "rain" || id == "雨天") return WeatherType::Rain;
    if (normalized == "heavyrain" || id == "大雨") return WeatherType::HeavyRain;
    if (normalized == "fog" || id == "雾天") return WeatherType::Fog;
    if (normalized == "snow" || id == "雪天") return WeatherType::Snow;
    return WeatherType::Clear;
}

}  // namespace WeatherTypes
