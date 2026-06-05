#include "Camera2D.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>

glm::mat4 Camera2D::getViewMatrix() const {
    glm::mat4 view = glm::mat4(1.0f);
    // 逆变序：先缩放逆，再旋转逆，再平移逆
    view = glm::scale(view, glm::vec3(1.0f / zoom, 1.0f / zoom, 1.0f));
    view = glm::rotate(view, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::translate(view, glm::vec3(-position.x, -position.y, 0.0f));
    return view;
}

glm::mat4 Camera2D::getProjectionMatrix(float screenWidth, float screenHeight) const {
    // 正交投影：将屏幕空间映射到 [-1, 1]
    // 注意：OpenGL屏幕坐标原点在左下角
    float aspect = screenWidth / screenHeight;
    float orthoWidth = 1.0f;   // 归一化单位宽度
    float orthoHeight = 1.0f;
    if (aspect >= 1.0f) {
        orthoWidth = aspect;
    } else {
        orthoHeight = 1.0f / aspect;
    }
    return glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -1.0f, 1.0f);
}

glm::mat4 Camera2D::getViewProjMatrix(float screenWidth, float screenHeight) const {
    return getProjectionMatrix(screenWidth, screenHeight) * getViewMatrix();
}

glm::vec2 Camera2D::screenToWorld(float sx, float sy, float screenWidth, float screenHeight) const {
    // 屏幕坐标 -> NDC -> 应用逆变换
    float ndcX = (2.0f * sx / screenWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * sy / screenHeight);  // Y翻转

    // 应用投影逆
    float aspect = screenWidth / screenHeight;
    float worldX = ndcX, worldY = ndcY;
    if (aspect >= 1.0f) {
        worldX *= aspect;
    } else {
        worldY *= 1.0f / aspect;
    }

    // 应用视图逆（旋转+缩放+平移）
    glm::vec2 p(worldX, worldY);
    if (rotation != 0.0f) {
        p = glm::rotate(p, rotation);
    }
    p *= zoom;
    p += position;

    return p;
}

glm::vec2 Camera2D::worldToScreen(float wx, float wy, float screenWidth, float screenHeight) const {
    glm::vec2 p = glm::vec2(wx, wy) - position;

    if (rotation != 0.0f) {
        p = glm::rotate(p, -rotation);
    }
    p *= (1.0f / zoom);

    // 应用投影
    float aspect = screenWidth / screenHeight;
    float ndcX = p.x, ndcY = p.y;
    if (aspect >= 1.0f) {
        ndcX /= aspect;
    } else {
        ndcY *= aspect;
    }

    float sx = (ndcX + 1.0f) * 0.5f * screenWidth;
    float sy = (1.0f - ndcY) * 0.5f * screenHeight;

    return glm::vec2(sx, sy);
}

void Camera2D::smoothFollow(const glm::vec2& target, float dt, float speed) {
    // 指数衰减平滑跟随（帧率无关）
    // t = 1 - exp(-speed * dt) 保证不同帧率下收敛行为一致
    float t = 1.0f - std::exp(-speed * dt);
    t = std::min(1.0f, t);
    position = glm::mix(position, target, t);
}

void Camera2D::setZoom(float z, float minZoom, float maxZoom) {
    zoom = std::clamp(z, minZoom, maxZoom);
}

void Camera2D::getViewportBounds(float screenWidth, float screenHeight,
                                  float& left, float& right, float& bottom, float& top) const {
    // 获取摄像机视口在世界空间中的边界
    float halfW = screenWidth * 0.5f / zoom;
    float halfH = screenHeight * 0.5f / zoom;

    if (rotation == 0.0f) {
        left = position.x - halfW;
        right = position.x + halfW;
        bottom = position.y - halfH;
        top = position.y + halfH;
    } else {
        // 带旋转时需要计算四个角
        float cosA = cosf(-rotation);
        float sinA = sinf(-rotation);
        float hw = halfW, hh = halfH;

        // 四个角在摄像机空间
        glm::vec2 corners[4] = {
            glm::vec2(-hw, -hh),
            glm::vec2( hw, -hh),
            glm::vec2( hw,  hh),
            glm::vec2(-hw,  hh)
        };

        left = right = bottom = top = 0;
        for (int i = 0; i < 4; ++i) {
            glm::vec2 worldCorner = position + glm::vec2(
                corners[i].x * cosA - corners[i].y * sinA,
                corners[i].x * sinA + corners[i].y * cosA
            );
            if (i == 0 || worldCorner.x < left) left = worldCorner.x;
            if (i == 0 || worldCorner.x > right) right = worldCorner.x;
            if (i == 0 || worldCorner.y < bottom) bottom = worldCorner.y;
            if (i == 0 || worldCorner.y > top) top = worldCorner.y;
        }
    }
}
