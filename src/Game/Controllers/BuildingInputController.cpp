#include "Game/Controllers/BuildingInputController.h"

#include "Game/GameState.h"
#include "Game/Services/PlayerInputQuery.h"
#include "Game/Services/SessionService.h"
#include "Game/Services/WorldQuery.h"

#include <glm/vec2.hpp>

#include <string>

namespace BuildingInputController {

bool canBuildHere(GameState& gs) {
    MapRegion* region = gs.regionManager.getCurrentRegion();
    return region && gs.buildingSystem.isBuildableHere(region->getId());
}

void handleToggleKey(GameState& gs, SDL_Scancode scancode) {
    if ((scancode != SDL_SCANCODE_TAB && scancode != SDL_SCANCODE_B) || gs.isDead) {
        return;
    }

    if (canBuildHere(gs) && !gs.toySystem.isMiniCarActive()) {
        gs.buildingSystem.toggleBuildMode();
    } else {
        gs.buildingSystem.setBuildMode(false);
    }
}

void handleKeyDown(GameState& gs, SDL_Scancode scancode) {
    bool buildable = canBuildHere(gs);

    if (scancode == SDL_SCANCODE_T && !gs.isDead && buildable) {
        if (gs.toySystem.isMiniCarActive()) {
            gs.toySystem.stopMiniCar();
        } else if (gs.toySystem.canStartMiniCar(
                       WorldQuery::hasPlacedFurniture(gs.buildingSystem, "toy_shelf"))) {
            gs.buildingSystem.setBuildMode(false);
            gs.toySystem.startMiniCar(gs.timeSystem.getDay());
        }
    }

    if (!gs.buildingSystem.isActive() || !buildable || gs.toySystem.isMiniCarActive()) {
        return;
    }

    if (scancode == SDL_SCANCODE_Q) {
        gs.buildingSystem.rotatePreview(-1);
    } else if (scancode == SDL_SCANCODE_E) {
        gs.buildingSystem.rotatePreview(1);
    } else if (scancode == SDL_SCANCODE_M) {
        MapRegion* buildRegion = gs.regionManager.getCurrentRegion();
        if (buildRegion) {
            glm::vec2 world = PlayerInputQuery::getMouseWorldPoint(gs);
            const TileMap& map = buildRegion->getTileMap();
            if (gs.buildingSystem.beginMoveAt(map.worldToTile(world.x, world.y))) {
                SessionService::showNotice(gs, "移动家具 Move: 左键确认，右键取消");
            }
        }
    } else if (scancode == SDL_SCANCODE_DELETE) {
        MapRegion* buildRegion = gs.regionManager.getCurrentRegion();
        if (buildRegion) {
            glm::vec2 world = PlayerInputQuery::getMouseWorldPoint(gs);
            const TileMap& map = buildRegion->getTileMap();
            std::string removedDefId;
            if (gs.buildingSystem.removeAt(map.worldToTile(world.x, world.y), &removedDefId)) {
                gs.inventory.addFurniture(removedDefId, 1);
            }
        }
    }
}

void handleMouseButtonDown(GameState& gs, Uint8 button) {
    MapRegion* buildRegion = gs.regionManager.getCurrentRegion();
    if (!buildRegion || !gs.buildingSystem.isBuildableHere(buildRegion->getId())) {
        return;
    }

    glm::vec2 world = PlayerInputQuery::getMouseWorldPoint(gs);
    const TileMap& map = buildRegion->getTileMap();
    glm::ivec2 tile = gs.buildingSystem.getPreviewTile(map, world);
    if (button == SDL_BUTTON_LEFT) {
        if (gs.buildingSystem.isMovingFurniture()) {
            if (gs.buildingSystem.confirmMove(map, tile)) {
                SessionService::showNotice(gs, "家具已移动 Furniture moved");
            }
        } else {
            glm::ivec2 clickedTile = map.worldToTile(world.x, world.y);
            if (gs.buildingSystem.beginMoveAt(clickedTile)) {
                SessionService::showNotice(gs, "移动家具 Move: 左键确认，右键取消");
            } else {
                const FurnitureDef* selected = gs.buildingSystem.getSelectedDef();
                if (selected) {
                    if (gs.inventory.getFurnitureCount(selected->id) <= 0) {
                        if (gs.inventory.isFurnitureUnlocked(selected->id) &&
                            gs.inventory.spendCoins(selected->price)) {
                            gs.inventory.addFurniture(selected->id, 1);
                        }
                    }
                    if (gs.inventory.getFurnitureCount(selected->id) > 0 &&
                        gs.buildingSystem.placeSelected(map, tile)) {
                        gs.inventory.consumeFurniture(selected->id, 1);
                    }
                }
            }
        }
    } else if (button == SDL_BUTTON_RIGHT) {
        if (gs.buildingSystem.isMovingFurniture()) {
            gs.buildingSystem.cancelMove();
            SessionService::showNotice(gs, "移动已取消 Move canceled");
        } else {
            std::string removedDefId;
            if (gs.buildingSystem.removeAt(map.worldToTile(world.x, world.y), &removedDefId)) {
                gs.inventory.addFurniture(removedDefId, 1);
            }
        }
    }
}

bool handleMouseWheel(GameState& gs, int wheelY) {
    if (!gs.buildingSystem.isActive()) {
        return false;
    }

    gs.buildingSystem.selectNextDef(wheelY >= 0 ? 1 : -1);
    return true;
}

}  // namespace BuildingInputController
