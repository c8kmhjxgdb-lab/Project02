#pragma once

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <cstdint>
#include <functional>

/**
 * MiniMap — 小地图渲染器
 *
 * 将地形缩略渲染到一张纹理上，显示在屏幕角落。
 * 关键优化：不每帧更新，而是每秒更新一次（或事件触发）。
 *
 * 使用回调函数获取瓦片类型，避免引擎模块间的循环依赖。
 */
class MiniMap {
public:
    void init(int size = 150);
    void shutdown();

    // 标记需要更新
    void requestUpdate();

    // 按需更新（基于计时器）
    void update(float deltaTime, const glm::vec2& playerPos);

    // Render minimap to screen (orthographic projection)
    // Automatically scales based on screen resolution
    void render(const glm::mat4& orthoProj, int screenWidth, int screenHeight);

    // 强制立即更新
    void forceUpdate(const glm::vec2& playerPos);

    void setVisible(bool v) { m_visible = v; }
    bool isVisible() const { return m_visible; }

    // Set tile accessor callback: returns tile type index (0=grass, 3=water, 4=wall, etc.)
    using TileGetter = std::function<uint8_t(int x, int y)>;
    void setTileGetter(TileGetter getter) { tileGetter = std::move(getter); }

    // Set map dimensions
    void setMapDimensions(int width, int height, float tileSize);

    // Entity marker types for minimap display
    struct EntityMarker {
        glm::vec2 worldPos;
        enum class Type : uint8_t { Player, Princess, Enemy, NPC } type;
    };

    // Set entities to display on minimap (called before update/render)
    void setEntities(const std::vector<EntityMarker>& entities) { m_entities = entities; }

private:
    GLuint texture = 0;
    GLuint vao = 0, vbo = 0;
    GLuint shader = 0;                 // member instead of static-local
    GLint uniformProj = -1;            // (was: lazy-init static inside render())
    GLint uniformRect = -1;
    GLint uniformTex = -1;
    bool shaderReady = false;
    int mapSize = 150;  // 纹理尺寸（正方形）

    std::vector<uint8_t> minimapData; // 每个瓦片的颜色索引

    float updateTimer = 0.0f;
    float updateInterval = 0.2f;
    bool isDirty = false;
    bool m_visible = true;

    // Last update player position
    glm::vec2 lastPlayerPos{ -1e9f, -1e9f };

    // Map dimensions
    int mapWidth = 40;
    int mapHeight = 30;
    float mapTileSize = 1.0f;

    TileGetter tileGetter;

    // Entity markers for display on minimap
    std::vector<EntityMarker> m_entities;

    void updateTexture(const glm::vec2& playerPos);
    glm::ivec2 worldToMinimap(const glm::vec2& worldPos);

    // From tile type index to minimap color index
    static uint8_t tileToColorIndex(uint8_t tileType);
};
