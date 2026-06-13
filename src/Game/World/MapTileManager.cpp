#include "MapTileManager.h"

#include <chrono>

static double getTime() {
    auto now = std::chrono::system_clock::now();
    auto dur = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::duration<double>>(dur).count();
}

void MapTileManager::bind(TileMap& map) {
    tileMap = &map;
}

void MapTileManager::init(TileMap& map, b2WorldId world) {
    bind(map);
    worldId = world;

    // Create physics bodies for all tiles that need them
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            TileType type = map.getTile(x, y);
            if (needsBody(type)) {
                createBodyFor(x, y);
            }
        }
    }
}

bool MapTileManager::setTile(int x, int y, TileType newType) {
    if (!tileMap || !tileMap->isInBounds(x, y)) return false;

    TileType oldType = tileMap->getTile(x, y);
    if (oldType == newType) return true;

    bool oldNeedsBody = needsBody(oldType);
    bool newNeedsBody = needsBody(newType);

    // Update tile data
    tileMap->setTile(x, y, newType);

    // Physics sync
    if (oldNeedsBody && !newNeedsBody) {
        destroyBodyFor(x, y);
    } else if (!oldNeedsBody && newNeedsBody) {
        createBodyFor(x, y);
    }

    // Record modification (for save files)
    if (!isReplaying) {
        modifications.push_back({x, y, oldType, newType, getTime()});
    }

    return true;
}

bool MapTileManager::setTiles(const std::vector<glm::ivec2>& positions, TileType newType) {
    bool anyChanged = false;
    for (const auto& pos : positions) {
        if (setTile(pos.x, pos.y, newType)) {
            anyChanged = true;
        }
    }
    return anyChanged;
}

b2BodyId MapTileManager::getBodyAt(int x, int y) const {
    auto it = tileBodies.find(packCoord(x, y));
    if (it != tileBodies.end()) {
        return it->second;
    }
    return b2_nullBodyId;
}

void MapTileManager::shutdown() {
    for (const auto& pair : tileBodies) {
        if (b2Body_IsValid(pair.second)) {
            b2DestroyBody(pair.second);
        }
    }
    tileBodies.clear();
    worldId = b2_nullWorldId;
}

bool MapTileManager::replayModifications(const std::vector<TileModification>& savedModifications) {
    if (!tileMap) return false;

    bool allApplied = true;
    bool previousReplayState = isReplaying;
    isReplaying = true;
    for (const TileModification& mod : savedModifications) {
        if (!setTile(mod.x, mod.y, mod.newType)) {
            allApplied = false;
        }
    }
    isReplaying = previousReplayState;

    modifications = savedModifications;
    return allApplied;
}

void MapTileManager::createBodyFor(int x, int y) {
    if (!tileMap || !b2World_IsValid(worldId)) return;

    // Check if body already exists
    if (tileBodies.find(packCoord(x, y)) != tileBodies.end()) {
        return;
    }

    glm::vec2 worldPos = tileMap->tileToWorld(x, y);
    b2BodyDef bd = b2DefaultBodyDef();
    bd.position = b2Vec2{worldPos.x, worldPos.y};
    b2BodyId bodyId = b2CreateBody(worldId, &bd);

    // 1x1 tile AABB shape (slightly smaller than tile to avoid gaps)
    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density = 0.0f;
    sd.material.friction = 0.3f;
    b2Polygon shape = b2MakeBox(0.45f, 0.45f);
    b2CreatePolygonShape(bodyId, &sd, &shape);

    tileBodies[packCoord(x, y)] = bodyId;
}

void MapTileManager::destroyBodyFor(int x, int y) {
    auto it = tileBodies.find(packCoord(x, y));
    if (it != tileBodies.end()) {
        b2DestroyBody(it->second);
        tileBodies.erase(it);
    }
}
