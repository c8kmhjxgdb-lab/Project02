#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

/**
 * BondTechnique - 羁绊技系统
 *
 * 当公主在队伍中且 ultimateCharge 满时，按 G 键释放合体技。
 * 效果：全屏星愿冲击波，对范围内所有敌人造成巨额伤害。
 */
struct BondTechnique {
    std::vector<glm::vec2> waveFronts;   // 扩散波前位置
    float remainingTime = 0.0f;
    float lifetime = 1.0f;
    float radius = 0.0f;
    float maxRadius = 15.0f;
    float damage = 100.0f;
    bool active = false;
    bool damageApplied = false;  // 防止重复伤害

    void reset() {
        waveFronts.clear();
        remainingTime = 0.0f;
        radius = 0.0f;
        active = false;
        damageApplied = false;
    }

    void activate(const glm::vec2& centerPos) {
        reset();
        active = true;
        remainingTime = lifetime;
        waveFronts.push_back(centerPos);
    }
};

class BondTechniqueSystem {
public:
    BondTechniqueSystem();

    // 尝试释放羁绊技
    bool activate(const glm::vec2& centerPos);

    // 更新
    void update(float dt);

    // 是否正在激活
    bool isActive() const { return technique.active; }

    // 获取当前羁绊技数据
    const BondTechnique& getCurrentTechnique() const { return technique; }
    BondTechnique& getCurrentTechnique() { return technique; }

    // 设置伤害
    void setDamage(float dmg) { technique.damage = dmg; }
    float getDamage() const { return technique.damage; }

    // 设置最大半径
    void setMaxRadius(float r) { technique.maxRadius = r; }
    float getMaxRadius() const { return technique.maxRadius; }

    // 冷却
    float getCooldown() const { return cooldown; }
    void setCooldown(float cd) { cooldown = cd; }
    void updateCooldown(float dt) { if (cooldown > 0) cooldown -= dt; }
    bool canActivate() const { return cooldown <= 0.0f; }

private:
    BondTechnique technique;
    float cooldown = 0.0f;
};
