#pragma once

#include "Game/Building/Furniture.h"

#include <box2d/box2d.h>
#include <unordered_map>
#include <vector>

class BuildingPhysicsSync {
public:
    void init(b2WorldId world);
    void setTileSize(float size);
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }

    void createFor(const FurnitureInstance& instance,
                   const std::vector<FurnitureDef>& definitions);
    void destroyFor(int instanceId);
    void rebuild(const std::vector<FurnitureInstance>& instances,
                 const std::vector<FurnitureDef>& definitions);
    void clear();

private:
    b2WorldId worldId = b2_nullWorldId;
    std::unordered_map<int, b2BodyId> bodies;
    bool enabled = true;
    float tileSize = 1.0f;
};
