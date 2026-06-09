#include "WeatherSystem.h"
#include "Engine/Renderer/ParticleSystem.h"
#include "Engine/Camera/Camera2D.h"
#include "Game/Data/WeatherConfigLoader.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <utility>

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
    currentRegionId = "default";
    indoorContext = false;
    allowParticleEffects = true;
    currentSpecialTag.clear();
    defaultChangeInterval = 300.0f;
    regionRules.clear();
}

bool WeatherSystem::loadConfig(LuaVM& lua, const char* path) {
    WeatherConfig defaults;
    defaults.defaultChangeInterval = defaultChangeInterval;
    defaults.regionRules = regionRules;

    WeatherConfig config;
    if (!WeatherConfigLoader::load(lua, path, defaults, config)) {
        return false;
    }

    defaultChangeInterval = config.defaultChangeInterval;
    if (config.hasRegionRules) {
        regionRules = std::move(config.regionRules);
        setRegionContext(currentRegionId, indoorContext, allowParticleEffects);
    }

    return true;
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
            if (shouldEmitParticles()) updateRainEffect(dt, cam);
            break;
        case WeatherType::Fog:
            updateFogEffect(dt);
            break;
        case WeatherType::Snow:
            if (shouldEmitParticles()) updateSnowEffect(dt, cam);
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
            currentWeather = targetWeather;
        }
    }
}

void WeatherSystem::updateRainEffect(float dt, const Camera2D& cam) {
    if (!particleSystem) return;

    // 雨天粒子
    float emitRate = (currentWeather == WeatherType::HeavyRain) ? 200.0f : 80.0f;
    emitRate *= intensity;

    // 计算视口范围
    float screenWidth = 800.0f, screenHeight = 600.0f;  // 从配置读取或使用默认值
    glm::vec2 topLeft = cam.screenToWorld(0.0f, screenHeight, screenWidth, screenHeight);
    glm::vec2 bottomRight = cam.screenToWorld(screenWidth, 0.0f, screenWidth, screenHeight);
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
    // (rainAccumulator was previously incremented here for fractional particles
    // but never read — removed as dead state.)
}

void WeatherSystem::updateFogEffect(float) {
    // 雾效果通过光照和能见度参数影响渲染
    // 实际雾效在着色器中通过 uniform 实现
}

void WeatherSystem::updateSnowEffect(float dt, const Camera2D& cam) {
    if (!particleSystem) return;

    float emitRate = 60.0f * intensity;

    float screenWidth = 800.0f, screenHeight = 600.0f;
    glm::vec2 topLeft = cam.screenToWorld(0.0f, screenHeight, screenWidth, screenHeight);
    glm::vec2 bottomRight = cam.screenToWorld(screenWidth, 0.0f, screenWidth, screenHeight);
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
    // (snowAccumulator was previously incremented here for fractional particles
    // but never read — removed as dead state.)
}

void WeatherSystem::setWeather(WeatherType type) {
    targetWeather = type;
    targetIntensity = (type == WeatherType::Clear) ? 0.0f : 1.0f;
    if (std::abs(intensity - targetIntensity) < 0.001f) {
        currentWeather = targetWeather;
    }
    useRandomWeather = false;
}

void WeatherSystem::setWeatherImmediate(WeatherType type, float restoredIntensity) {
    currentWeather = type;
    targetWeather = type;
    intensity = (type == WeatherType::Clear) ? 0.0f : std::clamp(restoredIntensity, 0.0f, 1.0f);
    targetIntensity = intensity;
    useRandomWeather = false;
    weatherTimer = 0.0f;
}

void WeatherSystem::setRandomWeather(float interval) {
    changeInterval = interval > 0.0f ? interval : defaultChangeInterval;
    useRandomWeather = true;
    weatherTimer = 0.0f;
}

void WeatherSystem::clearWeather() {
    setWeather(WeatherType::Clear);
}

void WeatherSystem::setRegionContext(const std::string& regionId, bool indoor, bool allowParticles) {
    currentRegionId = regionId.empty() ? "default" : regionId;
    indoorContext = indoor;
    allowParticleEffects = allowParticles;
    currentSpecialTag.clear();

    const WeatherRegionRule* rule = findRegionRule(currentRegionId);
    if (!rule) {
        rule = findRegionRule("default");
    }
    if (rule) {
        indoorContext = rule->indoor;
        allowParticleEffects = rule->allowParticles;
        currentSpecialTag = rule->specialTag;
    }
}

bool WeatherSystem::shouldEmitParticles() const {
    if (indoorContext || !allowParticleEffects) return false;
    return currentWeather == WeatherType::Rain ||
           currentWeather == WeatherType::HeavyRain ||
           currentWeather == WeatherType::Snow;
}

float WeatherSystem::getMovementMultiplier() const {
    if (indoorContext) return 1.0f;

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
    if (indoorContext) return 1.0f;

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
    return WeatherTypes::getWeatherName(type);
}

const char* WeatherSystem::getWeatherId(WeatherType type) {
    return WeatherTypes::getWeatherId(type);
}

WeatherType WeatherSystem::weatherFromId(const std::string& id) {
    return WeatherTypes::weatherFromId(id);
}

WeatherType WeatherSystem::chooseRandomWeather() const {
    const WeatherRegionRule* rule = findRegionRule(currentRegionId);
    if (!rule || rule->weights.empty()) {
        rule = findRegionRule("default");
    }

    if (rule && !rule->weights.empty()) {
        int total = 0;
        for (const auto& entry : rule->weights) {
            total += std::max(0, entry.second);
        }
        if (total > 0) {
            int roll = rand() % total;
            for (const auto& entry : rule->weights) {
                roll -= std::max(0, entry.second);
                if (roll < 0) {
                    return entry.first;
                }
            }
        }
    }

    int roll = rand() % 100;
    if (roll < 40) return WeatherType::Clear;
    if (roll < 60) return WeatherType::Cloudy;
    if (roll < 75) return WeatherType::Rain;
    if (roll < 85) return WeatherType::Fog;
    if (roll < 95) return WeatherType::HeavyRain;
    return WeatherType::Snow;
}

const WeatherRegionRule* WeatherSystem::findRegionRule(const std::string& regionId) const {
    auto it = regionRules.find(regionId);
    return it == regionRules.end() ? nullptr : &it->second;
}
