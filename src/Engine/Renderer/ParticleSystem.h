#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>

/**
 * 粒子类型
 */
enum class ParticleType : uint8_t {
    Point,      // 点状粒子
    Circle,     // 圆形软粒子
    Spark,      // 火花（带重力）
    Trail       // 尾迹粒子
};

/**
 * Particle - 单个粒子
 */
struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec3 color;
    float lifetime;
    float maxLifetime;
    float size;
    ParticleType type;
    bool active;

    // 火花专用
    float gravity;

    Particle()
        : position(0), velocity(0), color(1), lifetime(0), maxLifetime(1.0f)
        , size(0.1f), type(ParticleType::Circle), active(false), gravity(0) {}
};

/**
 * ParticleSystem - 粒子系统
 *
 * 使用对象池管理大量粒子，支持发射、更新和渲染。
 */
class ParticleSystem {
public:
    ParticleSystem(int maxParticles = 1000);
    ~ParticleSystem();

    void init();

    // 发射单个粒子
    void emit(const glm::vec2& pos, const glm::vec2& vel,
              const glm::vec3& color, float lifetime, float size,
              ParticleType type = ParticleType::Circle);

    // 发射一组粒子（爆炸效果）
    void emitBurst(const glm::vec2& pos, int count,
                   const glm::vec3& color, float speed,
                   float lifetime, float size);

    // 发射圆形分布的粒子（光环效果）
    void emitRing(const glm::vec2& pos, int count,
                  const glm::vec3& color, float radius,
                  float lifetime, float size);

    // 更新所有粒子
    void update(float dt);

    // 获取活跃粒子列表
    const std::vector<Particle>& getActive() const { return particles; }

    // 获取活跃粒子数量
    int getActiveCount() const;

    // 清空所有粒子
    void clear();

    // 获取最大粒子数
    int getMaxParticles() const { return maxParticles; }

private:
    std::vector<Particle> particles;
    int maxParticles;
    int firstFree;  // 对象池空闲索引

    Particle* findFree();
};
