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

    // Create quad VBO (screen-space)
    float quadVerts[] = {
        // pos (0-1)     // tex coord
        0.0f, 0.0f,      0.0f, 0.0f,
        1.0f, 0.0f,      1.0f, 0.0f,
        1.0f, 1.0f,      1.0f, 1.0f,
        0.0f, 0.0f,      0.0f, 0.0f,
        1.0f, 1.0f,      1.0f, 1.0f,
        0.0f, 1.0f,      0.0f, 1.0f,
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
        if (dist > 0.35f || isDirty) {
            forceUpdate(playerPos);
            lastPlayerPos = playerPos;
            isDirty = false;
        }
    }
}

uint8_t MiniMap::tileToColorIndex(uint8_t tileType) {
    // Match TileType enum values from TileMap.h
    // Color values mapped to shader thresholds in MiniMap::render():
    //   r < 0.12  → deep water (dark blue)   → values 0-30
    //   r < 0.24  → water (blue)             → values 31-61
    //   r < 0.40  → grass (green)            → values 62-102
    //   r < 0.52  → dirt/sand (brown/yellow) → values 103-132
    //   r < 0.68  → stone/path (gray)        → values 133-173
    //   r < 0.80  → wall/door (dark gray)    → values 174-204
    //   r >= 0.80 → lava/snow/portal (bright)→ values 205-255
    switch (tileType) {
        case 0:  return 85;   // Grass → green
        case 1:  return 115;  // Dirt → brown-yellow
        case 2:  return 155;  // Stone → gray
        case 3:  return 55;   // Water → blue
        case 4:  return 185;  // Wall → dark gray
        case 5:  return 145;  // Path → light gray
        case 6:  return 125;  // Sand → yellow-brown
        case 7:  return 230;  // Snow → bright white-yellow
        case 8:  return 220;  // Lava → bright orange-red
        case 9:  return 20;   // DeepWater → very dark blue
        case 10: return 140;  // Bridge → gray
        case 11: return 180;  // Door → dark gray
        case 12: return 240;  // Portal → bright purple-white
        default: return 0;    // Unknown → black (unexplored)
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
    (void)playerPos; // Player position is now handled via entity markers
    if (!tileGetter) return;
    if (mapWidth <= 0 || mapHeight <= 0) return;

    // Fill every minimap texture pixel by sampling the source map. The old
    // code wrote one texture pixel per map tile, leaving most of the texture
    // black when mapSize was larger than mapWidth/mapHeight.
    for (int my = 0; my < mapSize; ++my) {
        int tileY = static_cast<int>(static_cast<float>(my) / mapSize * mapHeight);
        tileY = tileY < 0 ? 0 : (tileY >= mapHeight ? mapHeight - 1 : tileY);
        for (int mx = 0; mx < mapSize; ++mx) {
            int tileX = static_cast<int>(static_cast<float>(mx) / mapSize * mapWidth);
            tileX = tileX < 0 ? 0 : (tileX >= mapWidth ? mapWidth - 1 : tileX);
            minimapData[static_cast<size_t>(my) * mapSize + mx] =
                tileToColorIndex(tileGetter(tileX, tileY));
        }
    }

    // Upload to GPU
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mapSize, mapSize, GL_RED, GL_UNSIGNED_BYTE, minimapData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void MiniMap::render(const glm::mat4& orthoProj, int screenWidth, int screenHeight) {
    if (!m_visible || !texture) return;

    const int margin = 16;
    int size = static_cast<int>(screenHeight * 0.13f);
    size = size < 92 ? 92 : (size > 142 ? 142 : size);

    // Position: top-right corner
    float x0 = static_cast<float>(screenWidth - size - margin);
    float y0 = static_cast<float>(screenHeight - size - margin);
    if (x0 < margin) x0 = static_cast<float>(margin);
    if (y0 < margin) y0 = static_cast<float>(margin);
    float sizeF = static_cast<float>(size);

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
    if (r < 0.02) {
        // Black — unexplored / void
        color = vec3(0.05, 0.05, 0.08);
    } else if (r < 0.12) {
        // Deep water — dark blue
        color = vec3(0.06, 0.15, 0.35);
    } else if (r < 0.24) {
        // Water — blue
        color = vec3(0.1, 0.3, 0.55);
    } else if (r < 0.40) {
        // Grass — green
        color = vec3(0.18, 0.5, 0.15);
    } else if (r < 0.52) {
        // Dirt / Sand — brown / yellow-brown
        color = vec3(0.5, 0.38, 0.2);
    } else if (r < 0.68) {
        // Stone / Path — gray
        color = vec3(0.5, 0.5, 0.48);
    } else if (r < 0.80) {
        // Wall / Door — dark gray
        color = vec3(0.35, 0.32, 0.35);
    } else {
        // Lava / Snow / Portal — bright
        color = vec3(0.85, 0.7, 0.3);
    }
    FragColor = vec4(color, 0.78);
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

    Draw2D::beginFrame(orthoProj);
    Draw2D::drawRectFilled(x0 - 6.0f, y0 - 6.0f, sizeF + 12.0f, sizeF + 12.0f,
                           glm::vec3(0.02f, 0.025f, 0.03f), 0.34f);
    Draw2D::endFrame();

    glUseProgram(shader);
    glUniformMatrix4fv(uniformProj, 1, GL_FALSE, &orthoProj[0][0]);
    glUniform4f(uniformRect, x0, y0, sizeF, sizeF);
    glUniform1i(uniformTex, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    auto clamp01Local = [](float v) {
        return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    };

    auto toScreen = [&](const glm::vec2& worldPos) {
        float mapWorldW = mapWidth * mapTileSize;
        float mapWorldH = mapHeight * mapTileSize;
        float nx = mapWorldW > 0.0f ? clamp01Local(worldPos.x / mapWorldW) : 0.0f;
        float ny = mapWorldH > 0.0f ? clamp01Local(worldPos.y / mapWorldH) : 0.0f;
        return glm::vec2(x0 + nx * sizeF, y0 + ny * sizeF);
    };

    Draw2D::beginFrame(orthoProj);
    Draw2D::drawRect(x0 - 2.0f, y0 - 2.0f, sizeF + 4.0f, sizeF + 4.0f,
                     glm::vec3(0.72f, 0.82f, 0.78f), 2.0f, 0.75f);
    Draw2D::drawRect(x0, y0, sizeF, sizeF, glm::vec3(0.0f), 1.0f, 0.38f);

    for (const auto& entity : m_entities) {
        glm::vec2 p = toScreen(entity.worldPos);
        glm::vec3 color(1.0f);
        float radius = 3.0f;
        switch (entity.type) {
            case EntityMarker::Type::Player:
                color = glm::vec3(1.0f, 0.95f, 0.72f);
                radius = 4.5f;
                break;
            case EntityMarker::Type::Princess:
                color = glm::vec3(1.0f, 0.48f, 0.78f);
                radius = 3.6f;
                break;
            case EntityMarker::Type::Enemy:
                color = glm::vec3(1.0f, 0.25f, 0.18f);
                radius = 3.2f;
                break;
            case EntityMarker::Type::NPC:
                color = glm::vec3(0.94f, 0.78f, 0.28f);
                radius = 3.2f;
                break;
        }
        Draw2D::drawCircleFilled(p.x, p.y, radius, color, 0.92f);
        Draw2D::drawCircle(p.x, p.y, radius + 2.0f, glm::vec3(0.0f), 1.0f, 16, 0.45f);
    }
    Draw2D::endFrame();
}
