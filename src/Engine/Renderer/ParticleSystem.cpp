#include "ParticleSystem.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

ParticleSystem::ParticleSystem(int maxP)
    : maxParticles(maxP), firstFree(0) {
    particles.resize(static_cast<size_t>(maxParticles));
}

ParticleSystem::~ParticleSystem() {
    clear();
}

void ParticleSystem::init() {
    particles.resize(static_cast<size_t>(maxParticles));
    firstFree = 0;
}

Particle* ParticleSystem::findFree() {
    // 从上次位置开始查找
    int start = firstFree;
    for (int i = start; i < maxParticles; ++i) {
        if (!particles[static_cast<size_t>(i)].active) {
            firstFree = i + 1;
            return &particles[static_cast<size_t>(i)];
        }
    }
    // 回绕到开头
    for (int i = 0; i < start; ++i) {
        if (!particles[static_cast<size_t>(i)].active) {
            firstFree = i + 1;
            return &particles[static_cast<size_t>(i)];
        }
    }
    return nullptr;  // 粒子池已满
}

void ParticleSystem::emit(const glm::vec2& pos, const glm::vec2& vel,
                          const glm::vec3& color, float lifetime, float size,
                          ParticleType type) {
    Particle* p = findFree();
    if (!p) return;

    p->position = pos;
    p->velocity = vel;
    p->color = color;
    p->maxLifetime = lifetime;
    p->lifetime = lifetime;
    p->size = size;
    p->type = type;
    p->active = true;
    p->gravity = 0;

    if (type == ParticleType::Spark) {
        p->gravity = -9.8f;  // 向下的重力（屏幕空间中向下为负Y）
    }
}

void ParticleSystem::emitBurst(const glm::vec2& pos, int count,
                               const glm::vec3& color, float speed,
                               float lifetime, float size) {
    for (int i = 0; i < count; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(count)) * 6.28318f;
        // 添加随机偏移
        angle += (std::rand() % 100) / 100.0f * 0.3f;

        glm::vec2 vel(std::cos(angle) * speed, std::sin(angle) * speed);
        emit(pos, vel, color, lifetime * (0.5f + (std::rand() % 100) / 200.0f),
             size * (0.5f + (std::rand() % 100) / 200.0f), ParticleType::Spark);
    }
}

void ParticleSystem::emitRing(const glm::vec2& pos, int count,
                              const glm::vec3& color, float radius,
                              float lifetime, float size) {
    for (int i = 0; i < count; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(count)) * 6.28318f;
        glm::vec2 pPos(pos.x + std::cos(angle) * radius,
                       pos.y + std::sin(angle) * radius);
        glm::vec2 vel(std::cos(angle) * radius * 2.0f,
                      std::sin(angle) * radius * 2.0f);
        emit(pPos, vel, color, lifetime, size, ParticleType::Circle);
    }
}

void ParticleSystem::update(float dt) {
    for (auto& p : particles) {
        if (!p.active) continue;

        // 更新生命周期
        p.lifetime -= dt;
        if (p.lifetime <= 0.0f) {
            p.active = false;
            continue;
        }

        // 更新位置
        p.position += p.velocity * dt;

        // 重力
        if (p.gravity != 0) {
            p.velocity.y += p.gravity * dt;
        }

        // 阻力（轻量空气阻力）
        p.velocity *= 0.98f;
    }
}

int ParticleSystem::getActiveCount() const {
    int count = 0;
    for (const auto& p : particles) {
        if (p.active) count++;
    }
    return count;
}

void ParticleSystem::clear() {
    for (auto& p : particles) {
        p.active = false;
    }
    firstFree = 0;
}
