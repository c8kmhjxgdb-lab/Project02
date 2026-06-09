#pragma once

#include "Game/World/WeatherTypes.h"

#include <glm/vec3.hpp>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

class ParticleSystem;
struct Camera2D;  // Forward declare as struct to match Camera2D.h
class LuaVM;

/**
 * WeatherSystem — 天气系统
 *
 * 管理天气效果切换、粒子效果和光照修正。
 * 支持与日夜循环系统配合，产生动态的环境效果。
 */
class WeatherSystem {
public:
    // 初始化（注入依赖）
    void init(ParticleSystem* particleSys, Camera2D* camera);
    bool loadConfig(LuaVM& lua, const char* path);

    void update(float dt, const Camera2D& camera);

    // 天气控制
    void setWeather(WeatherType type);
    void setWeatherImmediate(WeatherType type, float restoredIntensity = 1.0f);
    void setRandomWeather(float changeInterval = 300.0f);  // 每N秒随机变化
    void clearWeather();
    void setRegionContext(const std::string& regionId, bool indoor, bool allowParticles = true);

    // 获取当前天气
    WeatherType getCurrentWeather() const { return currentWeather; }
    bool isIndoorContext() const { return indoorContext; }
    const std::string& getCurrentRegionId() const { return currentRegionId; }
    const std::string& getCurrentSpecialTag() const { return currentSpecialTag; }
    float getDefaultChangeInterval() const { return defaultChangeInterval; }

    // 天气效果强度（0-1）
    float getIntensity() const { return intensity; }
    bool shouldEmitParticles() const;

    // 获取天气对移动的影响
    float getMovementMultiplier() const;

    // 获取天气对视线的影响
    float getVisibility() const;

    // 获取天气对光照的影响（返回 RGB 系数）
    glm::vec3 getLightModifier() const;

    // 获取天气名称
    static const char* getWeatherName(WeatherType type);
    static const char* getWeatherId(WeatherType type);
    static WeatherType weatherFromId(const std::string& id);

private:
    WeatherType currentWeather;
    WeatherType targetWeather;
    float intensity;           // 当前强度 0-1
    float targetIntensity;
    float transitionSpeed;     // 过渡速度

    float weatherTimer;
    float changeInterval;
    bool useRandomWeather;

    ParticleSystem* particleSystem;
    Camera2D* camera;  // 非拥有指针，仅用于获取视口信息
    std::string currentRegionId;
    bool indoorContext;
    bool allowParticleEffects;
    std::string currentSpecialTag;
    float defaultChangeInterval;

    std::unordered_map<std::string, WeatherRegionRule> regionRules;

    // 天气过渡
    void updateTransition(float dt);

    // 天气效果
    void updateRainEffect(float dt, const Camera2D& cam);
    void updateFogEffect(float dt);
    void updateSnowEffect(float dt, const Camera2D& cam);

    // 随机天气选择
    WeatherType chooseRandomWeather() const;
    const WeatherRegionRule* findRegionRule(const std::string& regionId) const;
};
