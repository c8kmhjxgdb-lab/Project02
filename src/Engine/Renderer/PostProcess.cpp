#include "PostProcess.h"
#include <cstdio>

PostProcess::PostProcess()
    : fbo(0)
    , colorTexture(0)
    , quadVAO(0)
    , quadVBO(0)
    , width(0)
    , height(0)
    , vignetteIntensity(0.0f)
{}

PostProcess::~PostProcess() {
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (colorTexture) glDeleteTextures(1, &colorTexture);
    if (fbo) glDeleteFramebuffers(1, &fbo);
}

bool PostProcess::init(int w, int h) {
    width = w;
    height = h;

    // 1. 创建颜色纹理（渲染目标）
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 2. 创建FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "[PostProcess] FBO incomplete\n");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. 创建全屏四边形（NDC空间 -1~1）
    float quadVerts[] = {
        // position     // texcoords
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,   0.0f, 1.0f,
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return true;
}

void PostProcess::resize(int w, int h) {
    if (w == width && h == height) return;

    // 删除旧纹理和FBO
    glDeleteTextures(1, &colorTexture);
    glDeleteFramebuffers(1, &fbo);
    colorTexture = 0;
    fbo = 0;

    // 更新尺寸
    width = w;
    height = h;

    // 重新创建纹理
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 重新创建FBO并检查完整性
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "[PostProcess] FBO incomplete after resize to %dx%d\n", w, h);
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &colorTexture);
        fbo = 0;
        colorTexture = 0;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcess::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

void PostProcess::endRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}

void PostProcess::draw(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    // 设置uniform
    GLint locVignette = glGetUniformLocation(shaderProgram, "uVignetteIntensity");
    if (locVignette >= 0) {
        glUniform1f(locVignette, vignetteIntensity);
    }

    GLint locTexture = glGetUniformLocation(shaderProgram, "uScreenTexture");
    if (locTexture >= 0) {
        glUniform1i(locTexture, 0);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}
