#pragma once

#include "BuildingPhysicsSync.h"
#include "Furniture.h"
#include "Game/World/TileMap.h"

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <vector>

class LuaVM;

class BuildingSystem {
public:
    BuildingSystem();
    ~BuildingSystem();

    void init(b2WorldId world);
    bool loadDefinitions(LuaVM& lua, const char* path);
    void shutdown();

    void setBuildableRegion(const std::string& regionId);
    void setTileSize(float size);
    bool isActive() const { return buildMode; }
    void setBuildMode(bool enabled);
    void toggleBuildMode();

    bool isBuildableHere(const std::string& regionId) const;
    void setSelectedDef(const std::string& defId);
    void selectNextDef(int direction);
    void rotatePreview(int direction);
    const FurnitureDef* getSelectedDef() const;
    const FurnitureDef* getPreviewDef() const;
    int getSelectedPrice() const;
    const std::vector<FurnitureDef>& getDefinitions() const { return definitions; }

    bool canPlace(const TileMap& map, const glm::ivec2& tile, uint8_t rotation) const;
    bool canPlacePreview(const TileMap& map, const glm::ivec2& tile) const;
    bool placeSelected(const TileMap& map, const glm::ivec2& tile);
    bool removeAt(const glm::ivec2& tile, std::string* removedDefId = nullptr);
    bool beginMoveAt(const glm::ivec2& tile);
    bool confirmMove(const TileMap& map, const glm::ivec2& tile);
    void cancelMove();
    bool isMovingFurniture() const { return movingInstanceId != 0; }

    int getComfort() const;
    int getNostalgiaScore() const;
    int getLightScore() const;
    int getChildlikeRestoreBonus() const;
    float getNightLightBonus(float hour) const;
    const std::vector<FurnitureInstance>& getInstances() const { return instances; }
    void loadInstances(const std::vector<FurnitureInstance>& savedInstances);
    void setPhysicsEnabled(bool enabled);
    void rebuildPhysics();
    void clearInstances();

    glm::ivec2 getPreviewTile(const TileMap& map, const glm::vec2& worldPos) const;
    uint8_t getPreviewRotation() const { return previewRotation; }

private:
    std::string buildableRegionId;
    bool buildMode = false;
    std::vector<FurnitureDef> definitions;
    std::vector<FurnitureInstance> instances;
    BuildingPhysicsSync physicsSync;
    int nextInstanceId = 1;
    size_t selectedIndex = 0;
    uint8_t previewRotation = 0;
    int movingInstanceId = 0;
    FurnitureInstance movingBackup;

    const FurnitureDef* findDef(const std::string& defId) const;
};
