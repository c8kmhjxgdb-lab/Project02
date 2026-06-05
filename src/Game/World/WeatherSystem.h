#pragma once

#include <glm/vec3.hpp>
#include <random>

class ParticleSystem;
struct Camera2D;  // Forward declare as struct to match Camera2D.h

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

    void update(float dt, const Camera2D& camera);
    void render();

    // 天气控制
    void setWeather(WeatherType type);
    void setRandomWeather(float changeInterval = 300.0f);  // 每N秒随机变化
    void clearWeather();

    // 获取当前天气
    WeatherType getCurrentWeather() const { return currentWeather; }

    // 天气效果强度（0-1）
    float getIntensity() const { return intensity; }

    // 获取天气对移动的影响
    float getMovementMultiplier() const;

    // 获取天气对视线的影响
    float getVisibility() const;

    // 获取天气对光照的影响（返回 RGB 系数）
    glm::vec3 getLightModifier() const;

    // 获取天气名称
    static const char* getWeatherName(WeatherType type);

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

    // 粒子发射积分累积器
    float rainAccumulator = 0.0f;
    float snowAccumulator = 0.0f;

    // 天气过渡
    void updateTransition(float dt);

    // 天气效果
    void updateRainEffect(float dt, const Camera2D& cam);
    void updateFogEffect(float dt);
    void updateSnowEffect(float dt, const Camera2D& cam);

    // 随机天气选择
    WeatherType chooseRandomWeather() const;
};
