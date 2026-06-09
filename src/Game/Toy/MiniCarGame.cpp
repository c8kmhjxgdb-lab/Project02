#include "MiniCarGame.h"

#include "Engine/Renderer/Draw2D.h"

#include <algorithm>
#include <cmath>

void MiniCarGame::start() {
    active = true;
    finishReached = false;
    carPos = glm::vec2(5.0f, 4.0f);
    carVel = glm::vec2(0.0f);
    timer = 0.0f;
}

void MiniCarGame::stop() {
    active = false;
    carVel = glm::vec2(0.0f);
}

MiniCarResult MiniCarGame::update(float dt, bool up, bool down, bool left, bool right) {
    MiniCarResult result;
    if (!active) return result;

    timer += dt;
    glm::vec2 accel(0.0f);
    if (up) accel.y += 1.0f;
    if (down) accel.y -= 1.0f;
    if (left) accel.x -= 1.0f;
    if (right) accel.x += 1.0f;

    float lenSq = accel.x * accel.x + accel.y * accel.y;
    if (lenSq > 0.001f) {
        accel /= std::sqrt(lenSq);
    }

    carVel += accel * (7.0f * dt);
    carVel *= std::max(0.0f, 1.0f - 2.2f * dt);
    float speedSq = carVel.x * carVel.x + carVel.y * carVel.y;
    constexpr float maxSpeed = 3.0f;
    if (speedSq > maxSpeed * maxSpeed) {
        carVel = carVel / std::sqrt(speedSq) * maxSpeed;
    }

    carPos += carVel * dt;
    carPos.x = std::clamp(carPos.x, 3.0f, 15.0f);
    carPos.y = std::clamp(carPos.y, 3.0f, 10.0f);

    bool atFinish = carPos.x >= 13.4f && carPos.y >= 8.4f;
    if (atFinish && !finishReached) {
        finishReached = true;
        active = false;
        result.finished = true;
        result.finishTime = timer;
    }

    return result;
}

void MiniCarGame::render(const glm::mat4& viewProj) const {
    if (!active) return;

    Draw2D::beginFrame(viewProj);
    Draw2D::drawRectFilled(3.0f, 3.0f, 12.0f, 7.0f, glm::vec3(0.08f, 0.10f, 0.12f), 0.34f);
    Draw2D::drawRect(3.0f, 3.0f, 12.0f, 7.0f, glm::vec3(0.95f, 0.75f, 0.28f), 0.045f, 0.7f);

    Draw2D::drawLine(4.0f, 4.0f, 14.0f, 4.0f, glm::vec3(0.95f, 0.95f, 0.86f), 0.04f, 0.75f);
    Draw2D::drawLine(14.0f, 4.0f, 14.0f, 9.0f, glm::vec3(0.95f, 0.95f, 0.86f), 0.04f, 0.75f);
    Draw2D::drawLine(14.0f, 9.0f, 5.0f, 9.0f, glm::vec3(0.95f, 0.95f, 0.86f), 0.04f, 0.75f);

    Draw2D::drawRectFilled(13.25f, 8.25f, 1.35f, 1.15f, glm::vec3(0.20f, 0.70f, 0.32f), 0.45f);
    Draw2D::drawRect(13.25f, 8.25f, 1.35f, 1.15f, glm::vec3(0.74f, 1.0f, 0.60f), 0.035f, 0.9f);

    Draw2D::drawRectFilled(carPos.x - 0.24f, carPos.y - 0.14f, 0.48f, 0.28f,
                           glm::vec3(0.88f, 0.18f, 0.18f), 0.96f);
    Draw2D::drawCircleFilled(carPos.x - 0.16f, carPos.y - 0.17f, 0.07f,
                             glm::vec3(0.04f, 0.04f, 0.04f), 0.95f);
    Draw2D::drawCircleFilled(carPos.x + 0.16f, carPos.y - 0.17f, 0.07f,
                             glm::vec3(0.04f, 0.04f, 0.04f), 0.95f);
    Draw2D::endFrame();
}
