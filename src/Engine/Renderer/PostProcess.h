#pragma once

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

/**
 * 后处理渲染管线
 *
 * 将场景渲染到FBO纹理，然后施加后处理效果（暗角、色彩调整等）。
 */
class PostProcess {
public:
    PostProcess();
    ~PostProcess();

    // 初始化FBO和全屏四边形
    bool init(int width, int height);

    // 显式释放GL资源
    void shutdown();

    // 窗口大小改变时重置FBO
    void resize(int width, int height);

    // 开始渲染到FBO
    void beginRender();

    // 结束FBO渲染
    void endRender();

    // 设置后处理参数
    void setVignetteIntensity(float intensity) { vignetteIntensity = intensity; }

    // 绘制后处理结果到屏幕（内部会设置uniform）
    void draw(GLuint shaderProgram);

    // 获取屏幕纹理ID（供其他系统使用）
    GLuint getScreenTexture() const { return colorTexture; }

    // FBO是否有效
    bool isValid() const { return fbo != 0; }

private:
    GLuint fbo;
    GLuint colorTexture;
    GLuint quadVAO, quadVBO;
    int width, height;
    float vignetteIntensity;
};
