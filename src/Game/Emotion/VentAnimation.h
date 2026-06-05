#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

/**
 * 蒙头哭动画状态
 */
class VentAnimation {
public:
    // 开始动画
    void start(const glm::vec2& position);

    // 更新
    void update(float dt);

    // 是否正在进行中
    bool isActive() const { return active; }

    // 获取动画进度（0-1）
    float getProgress() const { return active ? timer / duration : 0.0f; }

    // 获取颤抖幅度
    float getShakeAmount() const;

    // 获取位置
    glm::vec2 getPosition() const { return position; }

    // 强制结束
    void stop();

private:
    bool active;
    float timer;
    float duration;  // 3秒
    glm::vec2 position;
};
