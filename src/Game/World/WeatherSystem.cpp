#include "WeatherSystem.h"
#include "Engine/Renderer/ParticleSystem.h"
#include "Engine/Camera/Camera2D.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

void WeatherSystem::init(ParticleSystem* particleSys, Camera2D* cam) {
    currentWeather = WeatherType::Clear;
    targetWeather = WeatherType::Clear;
    intensity = 0.0f;
    targetIntensity = 0.0f;
    transitionSpeed = 0.5f;
    weatherTimer = 0.0f;
    changeInterval = 300.0f;
    useRandomWeather = false;
    particleSystem = particleSys;
    camera = cam;
}

void WeatherSystem::update(float dt, const Camera2D& cam) {
    // 天气过渡
    updateTransition(dt);

    // 随机天气变化
    if (useRandomWeather) {
        weatherTimer += dt;
        if (weatherTimer >= changeInterval) {
            weatherTimer = 0.0f;
            targetWeather = chooseRandomWeather();
            targetIntensity = 1.0f;
        }
    }

    // 天气效果
    switch (currentWeather) {
        case WeatherType::Rain:
        case WeatherType::HeavyRain:
            updateRainEffect(dt, cam);
            break;
        case WeatherType::Fog:
            updateFogEffect(dt);
            break;
        case WeatherType::Snow:
            updateSnowEffect(dt, cam);
            break;
        default:
            break;
    }
}

void WeatherSystem::updateTransition(float dt) {
    // 平滑过渡强度
    if (intensity < targetIntensity) {
        intensity += dt * transitionSpeed;
        if (intensity >= targetIntensity) {
            intensity = targetIntensity;
            currentWeather = targetWeather;
        }
    } else if (intensity > targetIntensity) {
        intensity -= dt * transitionSpeed;
        if (intensity <= targetIntensity) {
            intensity = targetIntensity;
        }
    }
}

void WeatherSystem::updateRainEffect(float dt, const Camera2D& cam) {
    if (!particleSystem) return;

    // 雨天粒子
    float emitRate = (currentWeather == WeatherType::HeavyRain) ? 200.0f : 80.0f;
    emitRate *= intensity;

    // 计算视口范围
    int screenWidth = 800, screenHeight = 600;  // 从配置读取或使用默认值
    glm::vec2 topLeft = cam.screenToWorld(0, screenHeight, screenWidth, screenHeight);
    glm::vec2 bottomRight = cam.screenToWorld(screenWidth, 0, screenWidth, screenHeight);
    float viewportWidth = bottomRight.x - topLeft.x;
    float viewportHeight = bottomRight.y - topLeft.y;

    // 积分发射：限制每帧最大发射数
    float maxParticlesPerFrame = 50.0f;
    float particlesToEmit = std::min(emitRate * dt, maxParticlesPerFrame);

    for (int i = 0; i < static_cast<int>(particlesToEmit); ++i) {
        float x = cam.position.x - viewportWidth / 2 +
                  (static_cast<float>(rand() % 1000) / 1000.0f) * viewportWidth;
        float y = cam.position.y + viewportHeight / 2 + 1.0f;

        particleSystem->emit(
            glm::vec2(x, y),
            glm::vec2(0.0f, -8.0f),  // 向下
            glm::vec3(0.6f, 0.7f, 0.9f),
            2.0f,  // lifetime
            0.05f, // size
            ParticleType::Circle
        );
    }

    // 处理小数部分
    rainAccumulator += emitRate * dt - particlesToEmit;
}

void WeatherSystem::updateFogEffect(float) {
    // 雾效果通过光照和能见度参数影响渲染
    // 实际雾效在着色器中通过 uniform 实现
}

void WeatherSystem::updateSnowEffect(float dt, const Camera2D& cam) {
    if (!particleSystem) return;

    float emitRate = 60.0f * intensity;

    int screenWidth = 800, screenHeight = 600;
    glm::vec2 topLeft = cam.screenToWorld(0, screenHeight, screenWidth, screenHeight);
    glm::vec2 bottomRight = cam.screenToWorld(screenWidth, 0, screenWidth, screenHeight);
    float viewportWidth = bottomRight.x - topLeft.x;
    float viewportHeight = bottomRight.y - topLeft.y;

    float maxParticlesPerFrame = 30.0f;
    float particlesToEmit = std::min(emitRate * dt, maxParticlesPerFrame);

    for (int i = 0; i < static_cast<int>(particlesToEmit); ++i) {
        float x = cam.position.x - viewportWidth / 2 +
                  (static_cast<float>(rand() % 1000) / 1000.0f) * viewportWidth;
        float y = cam.position.y + viewportHeight / 2 + 1.0f;

        particleSystem->emit(
            glm::vec2(x, y),
            glm::vec2(static_cast<float>(rand() % 100 - 50) / 100.0f, -2.0f),  // 轻微飘动
            glm::vec3(0.95f, 0.95f, 1.0f),
            3.0f,  // lifetime
            0.08f, // size
            ParticleType::Circle
        );
    }

    snowAccumulator += emitRate * dt - particlesToEmit;
}

void WeatherSystem::setWeather(WeatherType type) {
    targetWeather = type;
    targetIntensity = (type == WeatherType::Clear) ? 0.0f : 1.0f;
    useRandomWeather = false;
}

void WeatherSystem::setRandomWeather(float interval) {
    changeInterval = interval > 0.0f ? interval : 300.0f;
    useRandomWeather = true;
    weatherTimer = 0.0f;
}

void WeatherSystem::clearWeather() {
    setWeather(WeatherType::Clear);
}

float WeatherSystem::getMovementMultiplier() const {
    switch (currentWeather) {
        case WeatherType::Rain:
            return 1.0f - 0.1f * intensity;  // 减速10%
        case WeatherType::HeavyRain:
            return 1.0f - 0.2f * intensity;  // 减速20%
        case WeatherType::Snow:
            return 1.0f - 0.15f * intensity; // 减速15%
        default:
            return 1.0f;
    }
}

float WeatherSystem::getVisibility() const {
    switch (currentWeather) {
        case WeatherType::Fog:
            return 1.0f - 0.5f * intensity;  // 能见度50%
        case WeatherType::HeavyRain:
            return 1.0f - 0.3f * intensity;  // 能见度70%
        default:
            return 1.0f;
    }
}

glm::vec3 WeatherSystem::getLightModifier() const {
    switch (currentWeather) {
        case WeatherType::Clear:
            return glm::vec3(1.0f, 1.0f, 1.0f);
        case WeatherType::Cloudy:
            return glm::vec3(0.8f, 0.8f, 0.8f);
        case WeatherType::Rain:
            return glm::vec3(0.6f, 0.7f, 0.9f);  // 偏冷色调
        case WeatherType::HeavyRain:
            return glm::vec3(0.5f, 0.6f, 0.8f);  // 更冷更暗
        case WeatherType::Fog:
            return glm::vec3(0.5f, 0.5f, 0.5f);  // 灰色
        case WeatherType::Snow:
            return glm::vec3(0.7f, 0.8f, 0.9f);  // 偏冷但明亮
        default:
            return glm::vec3(1.0f, 1.0f, 1.0f);
    }
}

const char* WeatherSystem::getWeatherName(WeatherType type) {
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

WeatherType WeatherSystem::chooseRandomWeather() const {
    // 根据权重随机选择天气
    int roll = rand() % 100;
    if (roll < 40) return WeatherType::Clear;      // 40% 晴天
    if (roll < 60) return WeatherType::Cloudy;     // 20% 多云
    if (roll < 75) return WeatherType::Rain;       // 15% 雨天
    if (roll < 85) return WeatherType::Fog;        // 10% 雾天
    if (roll < 95) return WeatherType::HeavyRain;  // 10% 大雨
    return WeatherType::Snow;                      // 5% 雪天
}

void WeatherSystem::render() {
    // 天气粒子效果通过 ParticleSystem 渲染
    // 这里可以添加额外的天气特效渲染（如全屏雾效）
}
