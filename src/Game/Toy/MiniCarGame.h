#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

struct MiniCarResult {
    bool finished = false;
    float finishTime = 0.0f;
};

class MiniCarGame {
public:
    void start();
    void stop();
    bool isActive() const { return active; }
    float getTimer() const { return timer; }

    MiniCarResult update(float dt, bool up, bool down, bool left, bool right);
    void render(const glm::mat4& viewProj) const;

private:
    bool active = false;
    bool finishReached = false;
    glm::vec2 carPos{0.0f, 0.0f};
    glm::vec2 carVel{0.0f, 0.0f};
    float timer = 0.0f;
};
