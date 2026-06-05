#include "DecorRenderer.h"

#include <cstdio>
#include <cstring>
#include "Game/World/Decoration.h"
#include "Utils/ShaderUtils.h"

void DecorRenderer::init() {
    shader = createShaderProgram("assets/shaders/decor.vert", "assets/shaders/decor.frag");
    if (!shader) {
        fprintf(stderr, "DecorRenderer: failed to create shader program\n");
        return;
    }

    uniformViewProj = glGetUniformLocation(shader, "uViewProj");
    uniformTime = glGetUniformLocation(shader, "uTime");

    // Unit quad VBO (-0.5 to 0.5)
    float quadVerts[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Instance buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // Instance attributes (location 1-4)
    // location 1: position (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(DecorInstance), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    // location 2: rotation (float)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(DecorInstance), reinterpret_cast<void*>(offsetof(DecorInstance, rotation)));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    // location 3: scale (float)
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(DecorInstance), reinterpret_cast<void*>(offsetof(DecorInstance, scale)));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    // location 4: type (int)
    glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, sizeof(DecorInstance), reinterpret_cast<void*>(offsetof(DecorInstance, type)));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    // location 5: variant (int)
    glVertexAttribPointer(5, 1, GL_INT, GL_FALSE, sizeof(DecorInstance), reinterpret_cast<void*>(offsetof(DecorInstance, variant)));
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);
}

void DecorRenderer::shutdown() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (instanceVBO) glDeleteBuffers(1, &instanceVBO);
    if (shader) glDeleteProgram(shader);
    vao = vbo = instanceVBO = shader = 0;
}

void DecorRenderer::beginFrame(const glm::mat4& viewProj) {
    currentTime += 0.016f; // approximate dt
    glUseProgram(shader);
    glUniformMatrix4fv(uniformViewProj, 1, GL_FALSE, &viewProj[0][0]);
    glUniform1f(uniformTime, currentTime);
}

void DecorRenderer::addDecor(const glm::vec2& pos, DecorType type, int variant,
                              float rotation, float scale) {
    DecorInstance inst;
    inst.position = pos;
    inst.rotation = rotation;
    inst.scale = scale;
    inst.type = static_cast<int>(type);
    inst.variant = variant;
    inst.padding = 0.0f;
    instances.push_back(inst);
}

void DecorRenderer::endFrame() {
    if (instances.empty()) {
        glUseProgram(0);
        return;
    }

    // Upload instance data
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(DecorInstance),
                 instances.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(instances.size()));
    glBindVertexArray(0);

    glUseProgram(0);
    instances.clear();
}
