#include "Draw2D.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <vector>

namespace Draw2D {

    // ========== Internal state ==========

    struct Vertex {
        float x, y;
        float r, g, b, a;
    };

    static GLuint sVAO = 0;
    static GLuint sVBO = 0;
    static GLuint sShaderProg = 0;
    static GLint sUniformViewProj = -1;
    static std::vector<Vertex> sVertices;
    static const int MAX_VERTICES = 65536;
    static bool sInitialized = false;
    static glm::mat4 sCurrentViewProj(1.0f);

    // ========== Shaders ==========

    static const char* VERT_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
uniform mat4 uViewProj;
out vec4 vColor;
void main() {
    gl_Position = uViewProj * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)";

    static const char* FRAG_SHADER = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
void main() {
    FragColor = vColor;
}
)";

    static GLuint compileShader(GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        GLint ok;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            fprintf(stderr, "Draw2D shader compile error: %s\n", log);
            return 0;
        }
        return shader;
    }

    static GLuint createShaderProgram() {
        GLuint vert = compileShader(GL_VERTEX_SHADER, VERT_SHADER);
        GLuint frag = compileShader(GL_FRAGMENT_SHADER, FRAG_SHADER);
        if (!vert || !frag) return 0;
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);
        GLint ok;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetProgramInfoLog(prog, 512, nullptr, log);
            fprintf(stderr, "Draw2D program link error: %s\n", log);
            return 0;
        }
        glDeleteShader(vert);
        glDeleteShader(frag);
        return prog;
    }

    // ========== Public API ==========

    bool init() {
        if (sInitialized) return true;

        sShaderProg = createShaderProgram();
        if (!sShaderProg) {
            fprintf(stderr, "Failed to create Draw2D shader program\n");
            return false;
        }

        sUniformViewProj = glGetUniformLocation(sShaderProg, "uViewProj");

        glGenVertexArrays(1, &sVAO);
        glGenBuffers(1, &sVBO);

        glBindVertexArray(sVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sVBO);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

        // location 0: vec2 aPos
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        // location 1: vec4 aColor (RGBA, includes alpha for translucent shapes)
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        sVertices.reserve(MAX_VERTICES);
        sInitialized = true;
        return true;
    }

    void shutdown() {
        if (!sInitialized) return;
        glDeleteBuffers(1, &sVBO);
        glDeleteVertexArrays(1, &sVAO);
        glDeleteProgram(sShaderProg);
        sVAO = 0;
        sVBO = 0;
        sShaderProg = 0;
        sVertices.clear();
        sInitialized = false;
    }

    void beginFrame(const glm::mat4& viewProj) {
        if (!sInitialized) return;
        sVertices.clear();
        sCurrentViewProj = viewProj;
    }

    void endFrame() {
        if (!sInitialized || sVertices.empty()) return;

        glUseProgram(sShaderProg);
        glUniformMatrix4fv(sUniformViewProj, 1, GL_FALSE, &sCurrentViewProj[0][0]);

        glBindVertexArray(sVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sVertices.size() * sizeof(Vertex), sVertices.data());

        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLint>(sVertices.size()));

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void clear() {
        sVertices.clear();
    }

    // ========== Helpers ==========

    // Single triangle, all 3 vertices share the same color/alpha.
    static void addTriangle(float x1, float y1, float x2, float y2, float x3, float y3,
                            float r, float g, float b, float a) {
        if (sVertices.size() + 3 > static_cast<size_t>(MAX_VERTICES)) {
            fprintf(stderr, "Draw2D: vertex buffer overflow\n");
            return;
        }
        sVertices.push_back({x1, y1, r, g, b, a});
        sVertices.push_back({x2, y2, r, g, b, a});
        sVertices.push_back({x3, y3, r, g, b, a});
    }

    // Quad with 4 corners: bl(0), br(1), tr(2), tl(3)
    // Triangles: (0,1,3) and (1,2,3). All vertices share the same color/alpha.
    static void addQuad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
                        float r, float g, float b, float a) {
        if (sVertices.size() + 6 > static_cast<size_t>(MAX_VERTICES)) {
            fprintf(stderr, "Draw2D: vertex buffer overflow\n");
            return;
        }
        sVertices.push_back({x0, y0, r, g, b, a});
        sVertices.push_back({x1, y1, r, g, b, a});
        sVertices.push_back({x3, y3, r, g, b, a});
        sVertices.push_back({x1, y1, r, g, b, a});
        sVertices.push_back({x2, y2, r, g, b, a});
        sVertices.push_back({x3, y3, r, g, b, a});
    }

    // ========== Draw functions ==========

    void drawRect(float x, float y, float w, float h, const glm::vec3& color, float thickness, float alpha) {
        float t = thickness;
        float r = color.r, g = color.g, b = color.b;

        // Top edge
        addQuad(x, y + h - t, x + w, y + h - t, x + w, y + h, x, y + h,
                r, g, b, alpha);
        // Bottom edge
        addQuad(x, y, x + w, y, x + w, y + t, x, y + t,
                r, g, b, alpha);
        // Left edge
        addQuad(x, y + t, x + t, y + t, x + t, y + h - t, x, y + h - t,
                r, g, b, alpha);
        // Right edge
        addQuad(x + w - t, y + t, x + w, y + t, x + w, y + h - t, x + w - t, y + h - t,
                r, g, b, alpha);
    }

    void drawRectFilled(float x, float y, float w, float h, const glm::vec3& color, float alpha) {
        addQuad(x, y, x + w, y, x + w, y + h, x, y + h,
                color.r, color.g, color.b, alpha);
    }

    void drawCircleFilled(float cx, float cy, float r, const glm::vec3& color, float alpha) {
        const int segments = 32;
        for (int i = 0; i < segments; ++i) {
            float a1 = 2.0f * 3.14159265f * i / segments;
            float a2 = 2.0f * 3.14159265f * (i + 1) / segments;
            float x1 = cx + r * cosf(a1);
            float y1 = cy + r * sinf(a1);
            float x2 = cx + r * cosf(a2);
            float y2 = cy + r * sinf(a2);
            addTriangle(cx, cy, x1, y1, x2, y2,
                        color.r, color.g, color.b, alpha);
        }
    }

    void drawCircle(float cx, float cy, float r, const glm::vec3& color, float thickness, int segments, float alpha) {
        for (int i = 0; i < segments; ++i) {
            float a1 = 2.0f * 3.14159265f * i / segments;
            float a2 = 2.0f * 3.14159265f * (i + 1) / segments;
            float innerR = r - thickness * 0.5f;
            float outerR = r + thickness * 0.5f;
            float ix1 = cx + innerR * cosf(a1), iy1 = cy + innerR * sinf(a1);
            float ix2 = cx + innerR * cosf(a2), iy2 = cy + innerR * sinf(a2);
            float ox1 = cx + outerR * cosf(a1), oy1 = cy + outerR * sinf(a1);
            float ox2 = cx + outerR * cosf(a2), oy2 = cy + outerR * sinf(a2);
            addQuad(ix1, iy1, ox1, oy1, ox2, oy2, ix2, iy2,
                    color.r, color.g, color.b, alpha);
        }
    }

    void drawLine(float x1, float y1, float x2, float y2, const glm::vec3& color, float thickness, float alpha) {
        glm::vec2 dir(x2 - x1, y2 - y1);
        float len = glm::length(dir);
        if (len < 0.0001f) return;
        glm::vec2 normal(-dir.y / len, dir.x / len);
        float hx = normal.x * thickness * 0.5f;
        float hy = normal.y * thickness * 0.5f;
        addQuad(x1 + hx, y1 + hy, x1 - hx, y1 - hy, x2 - hx, y2 - hy, x2 + hx, y2 + hy,
                color.r, color.g, color.b, alpha);
    }

    void drawRectGradient(float x, float y, float w, float h,
                          const glm::vec3& topLeft, const glm::vec3& topRight,
                          const glm::vec3& bottomLeft, const glm::vec3& bottomRight) {
        // Per-vertex colors. We use a different path here because each corner has its own color.
        // We push 6 vertices manually with their own color/alpha (alpha=1.0 for gradients).
        if (sVertices.size() + 6 > static_cast<size_t>(MAX_VERTICES)) {
            fprintf(stderr, "Draw2D: vertex buffer overflow\n");
            return;
        }
        const float a = 1.0f;
        // Triangle 1: bl, br, tl
        sVertices.push_back({x,     y,     bottomLeft.r,  bottomLeft.g,  bottomLeft.b,  a});
        sVertices.push_back({x + w, y,     bottomRight.r, bottomRight.g, bottomRight.b, a});
        sVertices.push_back({x,     y + h, topLeft.r,     topLeft.g,     topLeft.b,     a});
        // Triangle 2: br, tr, tl
        sVertices.push_back({x + w, y,     bottomRight.r, bottomRight.g, bottomRight.b, a});
        sVertices.push_back({x + w, y + h, topRight.r,    topRight.g,    topRight.b,    a});
        sVertices.push_back({x,     y + h, topLeft.r,     topLeft.g,     topLeft.b,     a});
    }

    void drawCircleGradient(float cx, float cy, float r,
                            const glm::vec3& innerColor, const glm::vec3& outerColor,
                            int segments) {
        for (int i = 0; i < segments; ++i) {
            float a1 = 2.0f * 3.14159265f * i / segments;
            float a2 = 2.0f * 3.14159265f * (i + 1) / segments;
            float x1 = cx + r * cosf(a1), y1 = cy + r * sinf(a1);
            float x2 = cx + r * cosf(a2), y2 = cy + r * sinf(a2);
            if (sVertices.size() + 3 > static_cast<size_t>(MAX_VERTICES)) {
                fprintf(stderr, "Draw2D: vertex buffer overflow\n");
                return;
            }
            // Center vertex uses inner color, edge vertices use outer.
            sVertices.push_back({cx, cy, innerColor.r, innerColor.g, innerColor.b, 1.0f});
            sVertices.push_back({x1, y1, outerColor.r, outerColor.g, outerColor.b, 1.0f});
            sVertices.push_back({x2, y2, outerColor.r, outerColor.g, outerColor.b, 1.0f});
        }
    }

    int getVertexCount() {
        return static_cast<int>(sVertices.size());
    }

    int getMaxVertices() {
        return MAX_VERTICES;
    }

}  // namespace Draw2D
