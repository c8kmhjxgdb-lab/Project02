#pragma once

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

// Forward declarations for DecorType
enum class DecorType : uint8_t;

/**
 * DecorRenderer — 装饰物批处理渲染器
 *
 * 使用实例化渲染（instanced rendering）批量绘制装饰物。
 * 装饰物使用 SDF 着色器渲染，支持风吹动画。
 */
class DecorRenderer {
public:
    void init();
    void shutdown();

    void beginFrame(const glm::mat4& viewProj);
    void addDecor(const glm::vec2& pos, DecorType type, int variant,
                  float rotation = 0.0f, float scale = 1.0f);
    void endFrame();

    void clear() { instances.clear(); }

    bool isInitialized() const { return shader != 0; }

private:
    GLuint shader = 0;
    GLuint vao = 0, vbo = 0, instanceVBO = 0;

    GLint uniformViewProj = -1;
    GLint uniformTime = -1;

    struct DecorInstance {
        glm::vec2 position;
        float rotation;
        float scale;
        int type;
        int variant;
        float padding;
    };
    std::vector<DecorInstance> instances;

    float currentTime = 0.0f;
};
