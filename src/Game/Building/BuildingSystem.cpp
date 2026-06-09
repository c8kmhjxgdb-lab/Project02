#include "BuildingSystem.h"

#include "Game/Building/BuildingPlacementRules.h"
#include "Game/Building/BuildingStats.h"
#include "Game/Data/FurnitureDefinitionLoader.h"

#include <algorithm>
#include <utility>

BuildingSystem::BuildingSystem() = default;

BuildingSystem::~BuildingSystem() {
    shutdown();
}

void BuildingSystem::init(b2WorldId world) {
    physicsSync.init(world);
    definitions = createDefaultFurnitureDefs();
    buildableRegionId = "home_base";
}

bool BuildingSystem::loadDefinitions(LuaVM& lua, const char* path) {
    std::vector<FurnitureDef> loaded;
    if (!FurnitureDefinitionLoader::load(lua, path, loaded)) {
        return false;
    }

    definitions = std::move(loaded);
    selectedIndex = std::min(selectedIndex, definitions.size() - 1);
    return true;
}

void BuildingSystem::shutdown() {
    physicsSync.clear();
    instances.clear();
}

void BuildingSystem::setBuildableRegion(const std::string& regionId) {
    buildableRegionId = regionId;
}

void BuildingSystem::setTileSize(float size) {
    physicsSync.setTileSize(size);
    rebuildPhysics();
}

void BuildingSystem::setBuildMode(bool enabled) {
    buildMode = enabled;
    if (!buildMode) {
        cancelMove();
    }
}

void BuildingSystem::toggleBuildMode() {
    buildMode = !buildMode;
    if (!buildMode) {
        cancelMove();
    }
}

bool BuildingSystem::isBuildableHere(const std::string& regionId) const {
    return regionId == buildableRegionId;
}

void BuildingSystem::setSelectedDef(const std::string& defId) {
    for (size_t i = 0; i < definitions.size(); ++i) {
        if (definitions[i].id == defId) {
            selectedIndex = i;
            return;
        }
    }
}

void BuildingSystem::selectNextDef(int direction) {
    if (definitions.empty()) return;
    int count = static_cast<int>(definitions.size());
    int next = static_cast<int>(selectedIndex) + (direction >= 0 ? 1 : -1);
    next = (next % count + count) % count;
    selectedIndex = static_cast<size_t>(next);
}

void BuildingSystem::rotatePreview(int direction) {
    int next = static_cast<int>(previewRotation) + (direction >= 0 ? 1 : -1);
    previewRotation = static_cast<uint8_t>((next % 4 + 4) % 4);
}

const FurnitureDef* BuildingSystem::getSelectedDef() const {
    if (definitions.empty()) return nullptr;
    return &definitions[selectedIndex % definitions.size()];
}

const FurnitureDef* BuildingSystem::getPreviewDef() const {
    if (movingInstanceId != 0) {
        return findDef(movingBackup.defId);
    }
    return getSelectedDef();
}

int BuildingSystem::getSelectedPrice() const {
    const FurnitureDef* def = getSelectedDef();
    return def ? def->price : 0;
}

const FurnitureDef* BuildingSystem::findDef(const std::string& defId) const {
    auto it = std::find_if(definitions.begin(), definitions.end(),
        [&defId](const FurnitureDef& def) { return def.id == defId; });
    return it == definitions.end() ? nullptr : &*it;
}

bool BuildingSystem::canPlace(const TileMap& map, const glm::ivec2& tile, uint8_t rotation) const {
    const FurnitureDef* def = getSelectedDef();
    if (!def) return false;

    return BuildingPlacementRules::canPlace(*def, map, tile, rotation, instances, definitions);
}

bool BuildingSystem::canPlacePreview(const TileMap& map, const glm::ivec2& tile) const {
    const FurnitureDef* def = getPreviewDef();
    if (!def) return false;

    return movingInstanceId != 0
        ? BuildingPlacementRules::canPlace(*def, map, tile, previewRotation, instances, definitions, movingInstanceId)
        : canPlace(map, tile, previewRotation);
}

bool BuildingSystem::placeSelected(const TileMap& map, const glm::ivec2& tile) {
    const FurnitureDef* def = getSelectedDef();
    if (!def || !canPlace(map, tile, previewRotation)) return false;

    FurnitureInstance instance;
    instance.instanceId = nextInstanceId++;
    instance.defId = def->id;
    instance.tile = tile;
    instance.rotation = previewRotation;
    instances.push_back(instance);
    physicsSync.createFor(instance, definitions);
    return true;
}

bool BuildingSystem::beginMoveAt(const glm::ivec2& tile) {
    if (movingInstanceId != 0) {
        cancelMove();
    }

    for (FurnitureInstance& instance : instances) {
        const FurnitureDef* def = findDef(instance.defId);
        if (!def) continue;

        if (BuildingPlacementRules::containsTile(instance, *def, tile)) {
            movingInstanceId = instance.instanceId;
            movingBackup = instance;
            previewRotation = instance.rotation;
            physicsSync.destroyFor(instance.instanceId);
            return true;
        }
    }

    return false;
}

bool BuildingSystem::confirmMove(const TileMap& map, const glm::ivec2& tile) {
    if (movingInstanceId == 0) return false;

    auto it = std::find_if(instances.begin(), instances.end(),
        [this](const FurnitureInstance& instance) {
            return instance.instanceId == movingInstanceId;
        });
    if (it == instances.end()) {
        movingInstanceId = 0;
        return false;
    }

    const FurnitureDef* def = findDef(it->defId);
    if (!def || !BuildingPlacementRules::canPlace(*def, map, tile, previewRotation, instances, definitions, movingInstanceId)) {
        return false;
    }

    it->tile = tile;
    it->rotation = previewRotation;
    physicsSync.createFor(*it, definitions);
    movingInstanceId = 0;
    return true;
}

void BuildingSystem::cancelMove() {
    if (movingInstanceId == 0) return;

    auto it = std::find_if(instances.begin(), instances.end(),
        [this](const FurnitureInstance& instance) {
            return instance.instanceId == movingInstanceId;
    });
    if (it != instances.end()) {
        *it = movingBackup;
        physicsSync.createFor(*it, definitions);
    }

    movingInstanceId = 0;
}

bool BuildingSystem::removeAt(const glm::ivec2& tile, std::string* removedDefId) {
    if (movingInstanceId != 0) return false;

    for (auto it = instances.begin(); it != instances.end(); ++it) {
        const FurnitureDef* def = findDef(it->defId);
        if (!def) continue;
        if (BuildingPlacementRules::containsTile(*it, *def, tile)) {
            if (removedDefId) {
                *removedDefId = it->defId;
            }
            physicsSync.destroyFor(it->instanceId);
            instances.erase(it);
            return true;
        }
    }
    return false;
}

int BuildingSystem::getComfort() const {
    return BuildingStats::comfort(instances, definitions);
}

int BuildingSystem::getNostalgiaScore() const {
    return BuildingStats::nostalgiaScore(instances, definitions);
}

int BuildingSystem::getLightScore() const {
    return BuildingStats::lightScore(instances, definitions);
}

int BuildingSystem::getChildlikeRestoreBonus() const {
    return BuildingStats::childlikeRestoreBonus(instances, definitions);
}

float BuildingSystem::getNightLightBonus(float hour) const {
    return BuildingStats::nightLightBonus(hour, getLightScore());
}

void BuildingSystem::loadInstances(const std::vector<FurnitureInstance>& savedInstances) {
    physicsSync.clear();
    instances = savedInstances;
    nextInstanceId = 1;
    for (const FurnitureInstance& instance : instances) {
        nextInstanceId = std::max(nextInstanceId, instance.instanceId + 1);
    }
    rebuildPhysics();
}

void BuildingSystem::setPhysicsEnabled(bool enabled) {
    if (physicsSync.isEnabled() == enabled) return;
    physicsSync.setEnabled(enabled);
    rebuildPhysics();
}

void BuildingSystem::rebuildPhysics() {
    physicsSync.rebuild(instances, definitions);
}

void BuildingSystem::clearInstances() {
    physicsSync.clear();
    instances.clear();
    nextInstanceId = 1;
}

glm::ivec2 BuildingSystem::getPreviewTile(const TileMap& map, const glm::vec2& worldPos) const {
    const FurnitureDef* def = getPreviewDef();
    glm::ivec2 tile = map.worldToTile(worldPos.x, worldPos.y);
    if (!def) return tile;

    glm::ivec2 size = BuildingPlacementRules::rotatedSize(*def, previewRotation);
    tile.x -= size.x / 2;
    tile.y -= size.y / 2;
    return tile;
}
