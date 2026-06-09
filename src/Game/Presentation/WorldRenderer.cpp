#include "Game/Presentation/WorldRenderer.h"

#include "Engine/Renderer/Draw2D.h"

#include <algorithm>
#include <cmath>

namespace {

float hash01(int x, int y, int salt = 0) {
    unsigned int n = static_cast<unsigned int>(x * 73856093u) ^
                     static_cast<unsigned int>(y * 19349663u) ^
                     static_cast<unsigned int>(salt * 83492791u);
    n ^= n >> 13;
    n *= 1274126177u;
    return static_cast<float>(n & 0x00ffffffu) / static_cast<float>(0x01000000u);
}

bool isWaterSurface(TileType type) {
    return type == TileType::Water || type == TileType::DeepWater;
}

bool sameTileSurface(TileType a, TileType b) {
    if (a == b) return true;
    if (isWaterSurface(a) && isWaterSurface(b)) return true;
    return false;
}

glm::vec3 tintColor(const glm::vec3& color, float amount) {
    return color + (glm::vec3(1.0f) - color) * amount;
}

glm::vec3 shadeColor(const glm::vec3& color, float amount) {
    return color * (1.0f - amount);
}

}  // namespace

namespace WorldRenderer {

void renderTileMap(const TileMap& map,
                   const TileColors& colors,
                   const glm::mat4& viewProj,
                   const ViewBounds& bounds,
                   float animationTime) {
    if (map.width <= 0 || map.height <= 0) return;

    glm::ivec2 tl = map.worldToTile(bounds.left - 1.0f, bounds.bottom - 1.0f);
    glm::ivec2 br = map.worldToTile(bounds.right + 1.0f, bounds.top + 1.0f);

    int tx0 = std::max(0, tl.x);
    int ty0 = std::max(0, tl.y);
    int tx1 = std::min(map.width - 1, br.x);
    int ty1 = std::min(map.height - 1, br.y);
    float ts = map.tileSize;

    Draw2D::beginFrame(viewProj);

    for (int y = ty0; y <= ty1; ++y) {
        for (int x = tx0; x <= tx1; ++x) {
            TileType type = map.getTile(x, y);
            glm::vec3 base = colors.get(type);
            float wx = x * ts;
            float wy = y * ts;
            float noise = hash01(x, y, 17);
            float noise2 = hash01(x, y, 41);
            float shade = 0.92f + noise * 0.14f;
            glm::vec3 color = base * shade;

            if (type == TileType::Water || type == TileType::DeepWater) {
                glm::vec3 deep = shadeColor(base, type == TileType::DeepWater ? 0.35f : 0.18f);
                glm::vec3 crest = tintColor(base, 0.25f);
                Draw2D::drawRectGradient(wx, wy, ts, ts, crest, tintColor(base, 0.12f), deep, deep);

                float waveY = wy + ts * (0.35f + noise * 0.35f);
                Draw2D::drawLine(wx + ts * 0.16f, waveY,
                                 wx + ts * 0.84f, waveY + (noise2 - 0.5f) * ts * 0.12f,
                                 tintColor(base, 0.55f), 0.025f, 0.42f);
                if (noise2 > 0.55f) {
                    Draw2D::drawLine(wx + ts * 0.10f, wy + ts * 0.70f,
                                     wx + ts * 0.52f, wy + ts * 0.66f,
                                     tintColor(base, 0.45f), 0.018f, 0.32f);
                }
            } else if (type == TileType::Wall || type == TileType::Stone || type == TileType::Door) {
                glm::vec3 face = tintColor(base, 0.08f);
                glm::vec3 side = shadeColor(base, 0.22f);
                Draw2D::drawRectFilled(wx, wy, ts, ts, side);
                Draw2D::drawRectFilled(wx + ts * 0.06f, wy + ts * 0.12f,
                                       ts * 0.88f, ts * 0.78f, face);
                Draw2D::drawLine(wx + ts * 0.08f, wy + ts * 0.78f,
                                 wx + ts * 0.92f, wy + ts * 0.78f,
                                 tintColor(base, 0.18f), 0.018f, 0.55f);
                if (noise > 0.45f) {
                    Draw2D::drawLine(wx + ts * 0.20f, wy + ts * 0.34f,
                                     wx + ts * 0.58f, wy + ts * 0.42f,
                                     shadeColor(base, 0.36f), 0.018f, 0.45f);
                }
            } else if (type == TileType::Path || type == TileType::Dirt ||
                       type == TileType::Sand || type == TileType::Snow) {
                Draw2D::drawRectFilled(wx, wy, ts, ts, color);
                Draw2D::drawRectFilled(wx, wy + ts * 0.52f, ts, ts * 0.48f,
                                       tintColor(color, type == TileType::Snow ? 0.10f : 0.05f), 0.28f);
                Draw2D::drawCircleFilled(wx + ts * (0.24f + noise * 0.18f),
                                         wy + ts * (0.26f + noise2 * 0.42f),
                                         ts * 0.035f, shadeColor(base, 0.22f), 0.42f);
                if (noise2 > 0.62f) {
                    Draw2D::drawCircleFilled(wx + ts * 0.70f, wy + ts * 0.34f,
                                             ts * 0.025f, shadeColor(base, 0.16f), 0.35f);
                }
            } else if (type == TileType::Bridge) {
                Draw2D::drawRectFilled(wx, wy, ts, ts, color);
                Draw2D::drawLine(wx + ts * 0.18f, wy, wx + ts * 0.18f, wy + ts,
                                 shadeColor(base, 0.32f), 0.018f, 0.65f);
                Draw2D::drawLine(wx + ts * 0.50f, wy, wx + ts * 0.50f, wy + ts,
                                 shadeColor(base, 0.28f), 0.018f, 0.65f);
                Draw2D::drawLine(wx + ts * 0.82f, wy, wx + ts * 0.82f, wy + ts,
                                 shadeColor(base, 0.32f), 0.018f, 0.65f);
            } else if (type == TileType::Lava) {
                Draw2D::drawRectGradient(wx, wy, ts, ts,
                    tintColor(base, 0.16f), base, shadeColor(base, 0.35f), shadeColor(base, 0.20f));
                Draw2D::drawLine(wx + ts * 0.12f, wy + ts * (0.30f + noise * 0.20f),
                                 wx + ts * 0.88f, wy + ts * (0.56f + noise2 * 0.18f),
                                 glm::vec3(1.0f, 0.72f, 0.12f), 0.030f, 0.55f);
            } else if (type == TileType::Portal) {
                Draw2D::drawRectFilled(wx, wy, ts, ts, shadeColor(base, 0.25f));
                Draw2D::drawCircleFilled(wx + ts * 0.5f, wy + ts * 0.5f,
                                         ts * (0.28f + 0.05f * std::sin(animationTime * 3.0f + noise)),
                                         tintColor(base, 0.35f), 0.55f);
                Draw2D::drawCircle(wx + ts * 0.5f, wy + ts * 0.5f, ts * 0.38f,
                                   tintColor(base, 0.55f), 0.025f, 28, 0.7f);
            } else {
                Draw2D::drawRectFilled(wx, wy, ts, ts, color);
                if (noise > 0.38f) {
                    glm::vec3 blade = shadeColor(base, 0.20f);
                    Draw2D::drawLine(wx + ts * (0.20f + noise * 0.25f), wy + ts * 0.18f,
                                     wx + ts * (0.24f + noise * 0.25f), wy + ts * 0.36f,
                                     blade, 0.016f, 0.55f);
                    Draw2D::drawLine(wx + ts * (0.62f + noise2 * 0.16f), wy + ts * 0.28f,
                                     wx + ts * (0.58f + noise2 * 0.16f), wy + ts * 0.48f,
                                     tintColor(base, 0.10f), 0.014f, 0.45f);
                }
            }

            glm::vec3 edgeColor = shadeColor(base, 0.30f);
            if (!map.isInBounds(x - 1, y) || !sameTileSurface(type, map.getTile(x - 1, y))) {
                Draw2D::drawLine(wx, wy, wx, wy + ts, edgeColor, 0.012f, 0.45f);
            }
            if (!map.isInBounds(x + 1, y) || !sameTileSurface(type, map.getTile(x + 1, y))) {
                Draw2D::drawLine(wx + ts, wy, wx + ts, wy + ts, edgeColor, 0.012f, 0.45f);
            }
            if (!map.isInBounds(x, y - 1) || !sameTileSurface(type, map.getTile(x, y - 1))) {
                Draw2D::drawLine(wx, wy, wx + ts, wy, edgeColor, 0.012f, 0.45f);
            }
            if (!map.isInBounds(x, y + 1) || !sameTileSurface(type, map.getTile(x, y + 1))) {
                Draw2D::drawLine(wx, wy + ts, wx + ts, wy + ts, edgeColor, 0.012f, 0.45f);
            }
        }
    }

    Draw2D::endFrame();
}

void renderLowDecorations(DecorRenderer& renderer,
                          const TileMap& map,
                          const std::vector<Decoration>& decorations,
                          const glm::mat4& viewProj,
                          float dt) {
    if (!renderer.isInitialized()) return;

    renderer.beginFrame(viewProj, dt);
    for (const auto& decor : decorations) {
        if (decor.type == DecorType::None) continue;
        if (decor.type == DecorType::Flower ||
            decor.type == DecorType::TallGrass ||
            decor.type == DecorType::Rock) {
            glm::vec2 worldPos = map.tileToWorld(decor.tileX, decor.tileY);
            renderer.addDecor(worldPos,
                              decor.type,
                              decor.variant,
                              decor.getRotationRadians(),
                              decor.getScaleFactor());
        }
    }
    renderer.endFrame();
}

void renderHighDecorations(DecorRenderer& renderer,
                           const TileMap& map,
                           const std::vector<Decoration>& decorations,
                           const glm::mat4& viewProj,
                           float dt) {
    if (!renderer.isInitialized()) return;

    renderer.beginFrame(viewProj, dt);
    for (const auto& decor : decorations) {
        if (decor.type == DecorType::None) continue;
        if (isTallDecor(decor.type)) {
            glm::vec2 worldPos = map.tileToWorld(decor.tileX, decor.tileY);
            renderer.addDecor(worldPos,
                              decor.type,
                              decor.variant,
                              decor.getRotationRadians(),
                              decor.getScaleFactor());
        }
    }
    renderer.endFrame();
}

}  // namespace WorldRenderer
