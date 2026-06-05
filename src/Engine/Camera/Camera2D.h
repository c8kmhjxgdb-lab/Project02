#pragma once

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

/**
 * Camera2D - 2D摄像机
 *
 * 支持位置、缩放、旋转，提供屏幕/世界坐标转换。
 * 使用正交投影。
 */
struct Camera2D {
    glm::vec2 position{0.0f, 0.0f};
    float zoom = 1.0f;        // >1 放大, <1 缩小
    float rotation = 0.0f;    // 弧度，逆时针为正

    // 获取视图矩阵（平移+旋转+缩放的逆变换）
    glm::mat4 getViewMatrix() const;

    // 获取投影矩阵（正交投影）
    glm::mat4 getProjectionMatrix(float screenWidth, float screenHeight) const;

    // 获取完整的视图投影矩阵
    glm::mat4 getViewProjMatrix(float screenWidth, float screenHeight) const;

    // 屏幕坐标转世界坐标
    glm::vec2 screenToWorld(float sx, float sy, float screenWidth, float screenHeight) const;

    // 世界坐标转屏幕坐标
    glm::vec2 worldToScreen(float wx, float wy, float screenWidth, float screenHeight) const;

    // 平滑移动到目标位置（线性插值）
    void smoothFollow(const glm::vec2& target, float dt, float speed = 5.0f);

    // 设置缩放（限制范围）
    void setZoom(float z, float minZoom = 0.1f, float maxZoom = 100.0f);

    // 获取视口边界（世界坐标）
    void getViewportBounds(float screenWidth, float screenHeight,
                           float& left, float& right, float& bottom, float& top) const;
};
