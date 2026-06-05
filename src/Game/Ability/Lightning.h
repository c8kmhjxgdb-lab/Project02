#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

/**
 * Lightning - 雷电链式技能
 *
 * 简单的雷电链数据结构，存储折线路径点用于渲染。
 * 实际的敌人查找和伤害由 main.cpp 中的游戏逻辑处理。
 */
struct LightningChain {
    std::vector<glm::vec2> points;     // 折线路径点（从玩家到各目标）
    std::vector<float> damageValues;   // 每个跳点的伤害
    float remainingTime = 0.0f;
    float lifetime = 0.5f;             // 视觉持续时间
    float baseDamage = 30.0f;
    int maxChains = 5;                 // 最多跳跃5次
    float chainRange = 4.0f;           // 每次跳跃的最大距离
    float damageDecay = 0.75f;         // 每次跳跃伤害衰减系数
    bool active = false;

    void reset() {
        points.clear();
        damageValues.clear();
        remainingTime = 0.0f;
        active = false;
    }

    void start(const glm::vec2& startPos) {
        reset();
        active = true;
        remainingTime = lifetime;
        points.push_back(startPos);
    }

    void addHit(const glm::vec2& hitPos, float damage) {
        points.push_back(hitPos);
        damageValues.push_back(damage);
    }
};

class Lightning {
public:
    Lightning();

    // 开始一次新的雷电（只初始化数据结构）
    void begin(const glm::vec2& startPos);

    // 添加一个击中点
    void addHit(const glm::vec2& hitPos, float damage);

    // 结束雷电
    void end();

    // 更新雷电状态
    void update(float dt);

    // 是否正在激活（视觉效果）
    bool isActive() const { return currentChain.active; }

    // 获取当前雷电链
    const LightningChain& getCurrentChain() const { return currentChain; }

    // 设置参数
    void setMaxChains(int max) { currentChain.maxChains = max; }
    int getMaxChains() const { return currentChain.maxChains; }

    void setChainRange(float range) { currentChain.chainRange = range; }
    float getChainRange() const { return currentChain.chainRange; }

    void setBaseDamage(float dmg) { currentChain.baseDamage = dmg; }
    float getBaseDamage() const { return currentChain.baseDamage; }

    // 获取冷却时间
    float getCooldown() const { return cooldown; }
    void setCooldown(float cd) { cooldown = cd; }
    void updateCooldown(float dt) { if (cooldown > 0) cooldown -= dt; }
    bool canCast() const { return cooldown <= 0.0f; }

    // 获取魔力消耗
    float getManaCost() const { return manaCost; }
    void setManaCost(float cost) { manaCost = cost; }

private:
    LightningChain currentChain;
    float baseDamage = 30.0f;
    float cooldown = 0.0f;
    float manaCost = 25.0f;
};
