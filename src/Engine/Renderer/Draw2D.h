#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

/**
 * Draw2D - Immediate mode 2D rendering API
 *
 * Uses batch rendering: all draw calls are cached to a vertex buffer,
 * then submitted in one glDraw call at endFrame().
 */
namespace Draw2D {

    // Initialize (create VAO/VBO/shaders)
    bool init();

    // Cleanup resources
    void shutdown();

    // Begin a frame (set view-projection matrix)
    void beginFrame(const glm::mat4& viewProj);

    // End frame and submit draw calls
    void endFrame();

    // Clear draw cache
    void clear();

    // ========== Basic Shapes ==========

    // Hollow rectangle (stroke)
    void drawRect(float x, float y, float w, float h, const glm::vec3& color, float thickness = 0.02f, float alpha = 1.0f);

    // Filled rectangle
    void drawRectFilled(float x, float y, float w, float h, const glm::vec3& color, float alpha = 1.0f);

    // Hollow circle (stroke)
    void drawCircle(float cx, float cy, float r, const glm::vec3& color, float thickness = 0.02f, int segments = 32, float alpha = 1.0f);

    // Filled circle.
    // 5th param is ALPHA (0..1) — segments are fixed internally to 32.
    // This is the function that 4 call sites were misusing as alpha (the
    // previous 5th param `int segments` silently truncated to 0, hiding the
    // bug). Renamed to alpha to match the actual call sites' intent.
    void drawCircleFilled(float cx, float cy, float r, const glm::vec3& color, float alpha = 1.0f);

    // Line
    void drawLine(float x1, float y1, float x2, float y2, const glm::vec3& color, float thickness = 0.02f, float alpha = 1.0f);

    // ========== Gradient Shapes ==========

    // Gradient rectangle (four corners, each with own color)
    void drawRectGradient(float x, float y, float w, float h,
                          const glm::vec3& topLeft, const glm::vec3& topRight,
                          const glm::vec3& bottomLeft, const glm::vec3& bottomRight);

    // Gradient circle (inner to outer)
    void drawCircleGradient(float cx, float cy, float r,
                            const glm::vec3& innerColor, const glm::vec3& outerColor,
                            int segments = 32);

    // ========== State Queries ==========

    // Get current batch vertex count
    int getVertexCount();

    // Get maximum vertex capacity
    int getMaxVertices();
}
