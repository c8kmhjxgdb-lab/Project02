#include "TextRenderer.h"

#include <GL/glew.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace TextRenderer {
namespace {

struct CachedText {
    GLuint texture = 0;
    int width = 1;
    int height = 1;
};

struct Vertex {
    float x, y;
    float u, v;
};

GLuint sVAO = 0;
GLuint sVBO = 0;
GLuint sShader = 0;
GLint sUniformViewProj = -1;
GLint sUniformColor = -1;
GLint sUniformTexture = -1;
bool sInitialized = false;
std::unordered_map<std::string, CachedText> sCache;

const char* VERT_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
uniform mat4 uViewProj;
out vec2 vUV;
void main() {
    gl_Position = uViewProj * vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
)";

const char* FRAG_SHADER = R"(
#version 330 core
in vec2 vUV;
uniform sampler2D uTexture;
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    float a = texture(uTexture, vUV).a;
    FragColor = vec4(uColor.rgb, uColor.a * a);
}
)";

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        fprintf(stderr, "TextRenderer shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createShaderProgram() {
    GLuint vert = compileShader(GL_VERTEX_SHADER, VERT_SHADER);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, FRAG_SHADER);
    if (!vert || !frag) {
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        fprintf(stderr, "TextRenderer program link error: %s\n", log);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

std::string makeKey(const std::string& text, int fontSize, int wrapWidth) {
    return text + "\x1f" + std::to_string(fontSize) + "\x1f" + std::to_string(wrapWidth);
}

#ifdef _WIN32
std::wstring utf8ToWide(const std::string& text) {
    if (text.empty()) return L"";

    int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                  text.data(), static_cast<int>(text.size()),
                                  nullptr, 0);
    DWORD flags = MB_ERR_INVALID_CHARS;
    if (len <= 0) {
        flags = 0;
        len = MultiByteToWideChar(CP_UTF8, flags,
                                  text.data(), static_cast<int>(text.size()),
                                  nullptr, 0);
    }
    if (len <= 0) return L"";

    std::wstring wide(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, flags,
                        text.data(), static_cast<int>(text.size()),
                        wide.data(), len);
    return wide;
}

HFONT createUIFont(int fontSize) {
    return CreateFontW(
        -fontSize,
        0, 0, 0,
        FW_SEMIBOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Microsoft YaHei UI");
}

CachedText rasterizeText(const std::string& utf8Text, int fontSize, int wrapWidth) {
    CachedText result;
    std::wstring text = utf8ToWide(utf8Text);
    if (text.empty()) return result;

    HDC screenDC = GetDC(nullptr);
    HDC hdc = CreateCompatibleDC(screenDC);
    ReleaseDC(nullptr, screenDC);
    if (!hdc) return result;

    HFONT font = createUIFont(fontSize);
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));

    UINT flags = DT_NOPREFIX | DT_EDITCONTROL;
    RECT measureRect{0, 0, wrapWidth > 0 ? wrapWidth : 4096, 0};
    if (wrapWidth > 0) flags |= DT_WORDBREAK;
    DrawTextW(hdc, text.c_str(), -1, &measureRect, flags | DT_CALCRECT);

    int measuredWidth = static_cast<int>(measureRect.right - measureRect.left);
    int measuredHeight = static_cast<int>(measureRect.bottom - measureRect.top);
    int width = std::max(1, measuredWidth + 4);
    int height = std::max(1, measuredHeight + 2);
    if (wrapWidth > 0) width = std::max(1, std::min(width, wrapWidth + 4));

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap || !bits) {
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        DeleteDC(hdc);
        return result;
    }

    HGDIOBJ oldBitmap = SelectObject(hdc, bitmap);
    std::memset(bits, 0, static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

    RECT drawRect{2, 0, width - 2, height};
    DrawTextW(hdc, text.c_str(), -1, &drawRect, flags);

    auto* pixels = static_cast<uint8_t*>(bits);
    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
    for (size_t i = 0; i < pixelCount; ++i) {
        uint8_t* px = pixels + i * 4;
        uint8_t alpha = std::max(px[0], std::max(px[1], px[2]));
        px[0] = 255;
        px[1] = 255;
        px[2] = 255;
        px[3] = alpha;
    }

    glGenTextures(1, &result.texture);
    glBindTexture(GL_TEXTURE_2D, result.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, bits);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    result.width = width;
    result.height = height;

    SelectObject(hdc, oldBitmap);
    SelectObject(hdc, oldFont);
    DeleteObject(bitmap);
    DeleteObject(font);
    DeleteDC(hdc);
    return result;
}
#else
CachedText rasterizeText(const std::string&, int, int) {
    return {};
}
#endif

CachedText& getTextTexture(const std::string& text, int fontSize, int wrapWidth) {
    std::string key = makeKey(text, fontSize, wrapWidth);
    auto it = sCache.find(key);
    if (it != sCache.end()) return it->second;

    CachedText rasterized = rasterizeText(text, fontSize, wrapWidth);
    auto inserted = sCache.emplace(std::move(key), rasterized);
    return inserted.first->second;
}

}  // namespace

bool init() {
    if (sInitialized) return true;

    sShader = createShaderProgram();
    if (!sShader) return false;

    sUniformViewProj = glGetUniformLocation(sShader, "uViewProj");
    sUniformColor = glGetUniformLocation(sShader, "uColor");
    sUniformTexture = glGetUniformLocation(sShader, "uTexture");

    glGenVertexArrays(1, &sVAO);
    glGenBuffers(1, &sVBO);

    glBindVertexArray(sVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    sInitialized = true;
    return true;
}

void shutdown() {
    for (auto& pair : sCache) {
        if (pair.second.texture) {
            glDeleteTextures(1, &pair.second.texture);
        }
    }
    sCache.clear();

    if (sVBO) glDeleteBuffers(1, &sVBO);
    if (sVAO) glDeleteVertexArrays(1, &sVAO);
    if (sShader) glDeleteProgram(sShader);
    sVBO = 0;
    sVAO = 0;
    sShader = 0;
    sInitialized = false;
}

glm::ivec2 measureText(const std::string& utf8Text, int fontSize, int wrapWidth) {
    if (utf8Text.empty()) return glm::ivec2(0, 0);
    CachedText& cached = getTextTexture(utf8Text, fontSize, wrapWidth);
    return glm::ivec2(cached.width, cached.height);
}

void drawText(const glm::mat4& viewProj,
              float x, float y,
              const std::string& utf8Text,
              int fontSize,
              const glm::vec3& color,
              float alpha,
              int wrapWidth) {
    if (!sInitialized || utf8Text.empty() || alpha <= 0.0f) return;

    CachedText& cached = getTextTexture(utf8Text, fontSize, wrapWidth);
    if (!cached.texture) return;

    float w = static_cast<float>(cached.width);
    float h = static_cast<float>(cached.height);
    Vertex verts[4] = {
        {x,     y,     0.0f, 1.0f},
        {x + w, y,     1.0f, 1.0f},
        {x + w, y + h, 1.0f, 0.0f},
        {x,     y + h, 0.0f, 0.0f},
    };

    glUseProgram(sShader);
    glUniformMatrix4fv(sUniformViewProj, 1, GL_FALSE, &viewProj[0][0]);
    glUniform4f(sUniformColor, color.r, color.g, color.b, alpha);
    glUniform1i(sUniformTexture, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cached.texture);

    glBindVertexArray(sVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void drawTextTopLeft(const glm::mat4& viewProj,
                     float x, float topY,
                     const std::string& utf8Text,
                     int fontSize,
                     const glm::vec3& color,
                     float alpha,
                     int wrapWidth) {
    glm::ivec2 size = measureText(utf8Text, fontSize, wrapWidth);
    drawText(viewProj, x, topY - static_cast<float>(size.y),
             utf8Text, fontSize, color, alpha, wrapWidth);
}

void drawTextCentered(const glm::mat4& viewProj,
                      float centerX, float centerY,
                      const std::string& utf8Text,
                      int fontSize,
                      const glm::vec3& color,
                      float alpha,
                      int wrapWidth) {
    glm::ivec2 size = measureText(utf8Text, fontSize, wrapWidth);
    drawText(viewProj,
             centerX - static_cast<float>(size.x) * 0.5f,
             centerY - static_cast<float>(size.y) * 0.5f,
             utf8Text, fontSize, color, alpha, wrapWidth);
}

}  // namespace TextRenderer
