#include "Game/Building/BuildingPhysicsSync.h"

#include "Game/Building/BuildingPlacementRules.h"

#include <algorithm>

namespace {

const FurnitureDef* findDef(const std::vector<FurnitureDef>& definitions, const std::string& defId) {
    auto it = std::find_if(definitions.begin(), definitions.end(),
        [&defId](const FurnitureDef& def) { return def.id == defId; });
    return it == definitions.end() ? nullptr : &*it;
}

}  // namespace

void BuildingPhysicsSync::init(b2WorldId world) {
    worldId = world;
}

void BuildingPhysicsSync::setTileSize(float size) {
    tileSize = size > 0.0f ? size : 1.0f;
}

void BuildingPhysicsSync::setEnabled(bool value) {
    enabled = value;
}

void BuildingPhysicsSync::createFor(const FurnitureInstance& instance,
                                    const std::vector<FurnitureDef>& definitions) {
    if (!enabled) return;
    if (!b2World_IsValid(worldId)) return;
    if (bodies.find(instance.instanceId) != bodies.end()) return;

    const FurnitureDef* def = findDef(definitions, instance.defId);
    if (!def || !def->blocksMovement) return;

    glm::ivec2 size = BuildingPlacementRules::rotatedSize(*def, instance.rotation);
    float cx = (static_cast<float>(instance.tile.x) + static_cast<float>(size.x) * 0.5f) * tileSize;
    float cy = (static_cast<float>(instance.tile.y) + static_cast<float>(size.y) * 0.5f) * tileSize;

    b2BodyDef bd = b2DefaultBodyDef();
    bd.position = b2Vec2{cx, cy};
    b2BodyId bodyId = b2CreateBody(worldId, &bd);

    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density = 0.0f;
    sd.material.friction = 0.5f;
    b2Polygon shape = b2MakeBox(static_cast<float>(size.x) * tileSize * 0.5f - 0.04f,
                                static_cast<float>(size.y) * tileSize * 0.5f - 0.04f);
    b2CreatePolygonShape(bodyId, &sd, &shape);
    bodies[instance.instanceId] = bodyId;
}

void BuildingPhysicsSync::destroyFor(int instanceId) {
    auto it = bodies.find(instanceId);
    if (it == bodies.end()) return;
    if (b2Body_IsValid(it->second)) {
        b2DestroyBody(it->second);
    }
    bodies.erase(it);
}

void BuildingPhysicsSync::rebuild(const std::vector<FurnitureInstance>& instances,
                                  const std::vector<FurnitureDef>& definitions) {
    clear();
    if (!enabled) return;
    for (const FurnitureInstance& instance : instances) {
        createFor(instance, definitions);
    }
}

void BuildingPhysicsSync::clear() {
    for (const auto& pair : bodies) {
        if (b2Body_IsValid(pair.second)) {
            b2DestroyBody(pair.second);
        }
    }
    bodies.clear();
}
