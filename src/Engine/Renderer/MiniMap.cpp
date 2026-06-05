#include "MiniMap.h"

#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include "Draw2D.h"

void MiniMap::init(int size) {
    mapSize = size;
    minimapData.resize(static_cast<size_t>(size) * size, 0);

    // Create texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, mapSize, mapSize, 0, GL_RED, GL_UNSIGNED_BYTE, minimapData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create quad VBO (screen-space, bottom-right corner)
    float quadVerts[] = {
        // pos (0-1)     // tex coord
        0.0f, 0.0f,      0.0f, 1.0f,
        1.0f, 0.0f,      1.0f, 1.0f,
        1.0f, 1.0f,      1.0f, 0.0f,
        0.0f, 0.0f,      0.0f, 1.0f,
        1.0f, 1.0f,      1.0f, 0.0f,
        0.0f, 1.0f,      0.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    isDirty = true;
}

void MiniMap::shutdown() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (texture) glDeleteTextures(1, &texture);
    if (shader) glDeleteProgram(shader);
    vao = vbo = texture = shader = 0;
    uniformProj = uniformRect = uniformTex = -1;
    shaderReady = false;
}

void MiniMap::setMapDimensions(int width, int height, float tileSize) {
    mapWidth = width;
    mapHeight = height;
    mapTileSize = tileSize;
}

void MiniMap::requestUpdate() {
    isDirty = true;
}

void MiniMap::update(float deltaTime, const glm::vec2& playerPos) {
    if (!m_visible) return;

    updateTimer += deltaTime;
    if (updateTimer >= updateInterval) {
        updateTimer = 0.0f;

        // Only update if player moved significantly
        float dist = glm::distance(playerPos, lastPlayerPos);
        if (dist > 2.0f || isDirty) {
            forceUpdate(playerPos);
            lastPlayerPos = playerPos;
            isDirty = false;
        }
    }
}

uint8_t MiniMap::tileToColorIndex(uint8_t tileType) {
    // Match TileType enum values from TileMap.h
    // Grass=0, Dirt=1, Stone=2, Water=3, Wall=4, Path=5, Sand=6, Lava=8, DeepWater=9, ...
    switch (tileType) {
        case 0:  return 80;   // Grass
        case 1:  return 120;  // Dirt
        case 2:  return 160;  // Stone
        case 3:  return 50;   // Water
        case 4:  return 200;  // Wall
        case 5:  return 150;  // Path
        case 6:  return 180;  // Sand
        case 7:  return 250;  // Snow
        case 8:  return 220;  // Lava
        case 9:  return 30;   // DeepWater
        case 10: return 140;  // Bridge
        case 11: return 190;  // Door
        case 12: return 240;  // Portal
        default: return 100;
    }
}

glm::ivec2 MiniMap::worldToMinimap(const glm::vec2& worldPos) {
    int mx = static_cast<int>(worldPos.x / (mapWidth * mapTileSize) * mapSize);
    int my = static_cast<int>(worldPos.y / (mapHeight * mapTileSize) * mapSize);
    mx = mx < 0 ? 0 : (mx >= mapSize ? mapSize - 1 : mx);
    my = my < 0 ? 0 : (my >= mapSize ? mapSize - 1 : my);
    return glm::ivec2(mx, my);
}

void MiniMap::forceUpdate(const glm::vec2& playerPos) {
    if (!tileGetter) return;

    // Clear to black
    std::memset(minimapData.data(), 0, minimapData.size());

    // Draw tiles
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            uint8_t type = tileGetter(x, y);
            uint8_t color = tileToColorIndex(type);

            int mx = static_cast<int>(static_cast<float>(x) / mapWidth * mapSize);
            int my = static_cast<int>(static_cast<float>(y) / mapHeight * mapSize);
            mx = mx < 0 ? 0 : (mx >= mapSize ? mapSize - 1 : mx);
            my = my < 0 ? 0 : (my >= mapSize ? mapSize - 1 : my);

            minimapData[static_cast<size_t>(my) * mapSize + mx] = color;
        }
    }

    // Draw player position (bright white)
    glm::ivec2 pp = worldToMinimap(playerPos);
    if (pp.x >= 0 && pp.x < mapSize && pp.y >= 0 && pp.y < mapSize) {
        minimapData[static_cast<size_t>(pp.y) * mapSize + pp.x] = 255;
        if (pp.x + 1 < mapSize) minimapData[pp.y * mapSize + pp.x + 1] = 255;
        if (pp.y + 1 < mapSize) minimapData[(pp.y + 1) * mapSize + pp.x] = 255;
    }

    // Upload to GPU
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mapSize, mapSize, GL_RED, GL_UNSIGNED_BYTE, minimapData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void MiniMap::render(const glm::mat4& orthoProj, int screenWidth, int /*screenHeight*/) {
    if (!m_visible || !texture) return;

    const int margin = 10;
    const int size = 120;

    float x0 = static_cast<float>(screenWidth - size - margin);
    float y0 = static_cast<float>(margin);

    if (!shaderReady) {
        const char* vs = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
uniform mat4 uProj;
uniform vec4 uRect;
out vec2 vTexCoord;
void main() {
    vec2 pos = uRect.xy + aPos * uRect.zw;
    gl_Position = uProj * vec4(pos, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";
        const char* fs = R"(
#version 330 core
in vec2 vTexCoord;
uniform sampler2D uTex;
out vec4 FragColor;
void main() {
    float r = texture(uTex, vTexCoord).r;
    vec3 color;
    if (r < 0.15) color = vec3(0.15, 0.15, 0.2);
    else if (r < 0.3) color = vec3(0.08, 0.3, 0.5);
    else if (r < 0.5) color = vec3(0.2, 0.55, 0.18);
    else if (r < 0.65) color = vec3(0.5, 0.38, 0.15);
    else if (r < 0.85) color = vec3(0.55, 0.55, 0.5);
    else color = vec3(0.95, 0.95, 0.3);
    FragColor = vec4(color, 0.85);
}
)";

        GLuint vsObj = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vsObj, 1, &vs, nullptr);
        glCompileShader(vsObj);
        GLuint fsObj = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fsObj, 1, &fs, nullptr);
        glCompileShader(fsObj);

        shader = glCreateProgram();
        glAttachShader(shader, vsObj);
        glAttachShader(shader, fsObj);
        glLinkProgram(shader);
        glDeleteShader(vsObj);
        glDeleteShader(fsObj);

        uniformProj = glGetUniformLocation(shader, "uProj");
        uniformRect = glGetUniformLocation(shader, "uRect");
        uniformTex = glGetUniformLocation(shader, "uTex");
        shaderReady = (shader != 0);
    }

    if (!shaderReady) return;

    glUseProgram(shader);
    glUniformMatrix4fv(uniformProj, 1, GL_FALSE, &orthoProj[0][0]);
    glUniform4f(uniformRect, x0, y0, size, size);
    glUniform1i(uniformTex, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Draw border
    Draw2D::beginFrame(orthoProj);
    Draw2D::drawRect(x0 - 2, y0 - 2, size + 4, size + 4, glm::vec3(0.0f), 0.005f);
    Draw2D::endFrame();

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
