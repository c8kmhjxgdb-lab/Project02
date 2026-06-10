#include "Game/Controllers/BuildingInputController.h"

#include "Game/Building/BuildingSystem.h"
#include "Game/GameState.h"
#include "Game/Inventory/Inventory.h"
#include "Game/Services/NoticeService.h"
#include "Game/Services/PlayerInputQuery.h"
#include "Game/Services/WorldQuery.h"
#include "Game/Toy/ToySystem.h"
#include "Game/World/RegionManager.h"
#include "Game/World/TimeSystem.h"

#include <glm/vec2.hpp>

#include <string>

namespace BuildingInputController {

Context makeContext(GameState& gs) {
    return {
        gs.isDead,
        gs.buildingSystem,
        gs.regionManager,
        gs.toySystem,
        gs.timeSystem,
        gs.inventory
    };
}

Callbacks makeCallbacks(GameState& gs) {
    return {
        [&gs](const Context&) {
            return PlayerInputQuery::getMouseWorldPoint(gs);
        },
        [&gs](Context&, const std::string& notice) {
            NoticeService::Context noticeContext = NoticeService::makeContext(gs);
            NoticeService::showNotice(noticeContext, notice);
        }
    };
}

bool canBuildHere(Context& context) {
    MapRegion* region = context.regionManager.getCurrentRegion();
    return region && context.buildingSystem.isBuildableHere(region->getId());
}

void handleToggleKey(Context& context, SDL_Scancode scancode) {
    if ((scancode != SDL_SCANCODE_TAB && scancode != SDL_SCANCODE_B) || context.isDead) {
        return;
    }

    if (canBuildHere(context) && !context.toySystem.isMiniCarActive()) {
        context.buildingSystem.toggleBuildMode();
    } else {
        context.buildingSystem.setBuildMode(false);
    }
}

void handleKeyDown(Context& context, SDL_Scancode scancode, const Callbacks& callbacks) {
    bool buildable = canBuildHere(context);

    if (scancode == SDL_SCANCODE_T && !context.isDead && buildable) {
        if (context.toySystem.isMiniCarActive()) {
            context.toySystem.stopMiniCar();
        } else if (context.toySystem.canStartMiniCar(
                       WorldQuery::hasPlacedFurniture(context.buildingSystem, "toy_shelf"))) {
            context.buildingSystem.setBuildMode(false);
            context.toySystem.startMiniCar(context.timeSystem.getDay());
        }
    }

    if (!context.buildingSystem.isActive() || !buildable || context.toySystem.isMiniCarActive()) {
        return;
    }

    if (scancode == SDL_SCANCODE_Q) {
        context.buildingSystem.rotatePreview(-1);
    } else if (scancode == SDL_SCANCODE_E) {
        context.buildingSystem.rotatePreview(1);
    } else if (scancode == SDL_SCANCODE_M) {
        MapRegion* buildRegion = context.regionManager.getCurrentRegion();
        if (buildRegion && callbacks.getMouseWorldPoint) {
            glm::vec2 world = callbacks.getMouseWorldPoint(context);
            const TileMap& map = buildRegion->getTileMap();
            if (context.buildingSystem.beginMoveAt(map.worldToTile(world.x, world.y)) &&
                callbacks.showNotice) {
                callbacks.showNotice(context, "移动家具 Move: 左键确认，右键取消");
            }
        }
    } else if (scancode == SDL_SCANCODE_DELETE) {
        MapRegion* buildRegion = context.regionManager.getCurrentRegion();
        if (buildRegion && callbacks.getMouseWorldPoint) {
            glm::vec2 world = callbacks.getMouseWorldPoint(context);
            const TileMap& map = buildRegion->getTileMap();
            std::string removedDefId;
            if (context.buildingSystem.removeAt(map.worldToTile(world.x, world.y), &removedDefId)) {
                context.inventory.addFurniture(removedDefId, 1);
            }
        }
    }
}

void handleMouseButtonDown(Context& context, Uint8 button, const Callbacks& callbacks) {
    MapRegion* buildRegion = context.regionManager.getCurrentRegion();
    if (!buildRegion ||
        !context.buildingSystem.isBuildableHere(buildRegion->getId()) ||
        !callbacks.getMouseWorldPoint) {
        return;
    }

    glm::vec2 world = callbacks.getMouseWorldPoint(context);
    const TileMap& map = buildRegion->getTileMap();
    glm::ivec2 tile = context.buildingSystem.getPreviewTile(map, world);
    if (button == SDL_BUTTON_LEFT) {
        if (context.buildingSystem.isMovingFurniture()) {
            if (context.buildingSystem.confirmMove(map, tile) && callbacks.showNotice) {
                callbacks.showNotice(context, "家具已移动 Furniture moved");
            }
        } else {
            glm::ivec2 clickedTile = map.worldToTile(world.x, world.y);
            if (context.buildingSystem.beginMoveAt(clickedTile)) {
                if (callbacks.showNotice) {
                    callbacks.showNotice(context, "移动家具 Move: 左键确认，右键取消");
                }
            } else {
                const FurnitureDef* selected = context.buildingSystem.getSelectedDef();
                if (selected) {
                    if (context.inventory.getFurnitureCount(selected->id) <= 0) {
                        if (context.inventory.isFurnitureUnlocked(selected->id) &&
                            context.inventory.spendCoins(selected->price)) {
                            context.inventory.addFurniture(selected->id, 1);
                        }
                    }
                    if (context.inventory.getFurnitureCount(selected->id) > 0 &&
                        context.buildingSystem.placeSelected(map, tile)) {
                        context.inventory.consumeFurniture(selected->id, 1);
                    }
                }
            }
        }
    } else if (button == SDL_BUTTON_RIGHT) {
        if (context.buildingSystem.isMovingFurniture()) {
            context.buildingSystem.cancelMove();
            if (callbacks.showNotice) {
                callbacks.showNotice(context, "移动已取消 Move canceled");
            }
        } else {
            std::string removedDefId;
            if (context.buildingSystem.removeAt(
                    map.worldToTile(world.x, world.y),
                    &removedDefId)) {
                context.inventory.addFurniture(removedDefId, 1);
            }
        }
    }
}

bool handleMouseWheel(Context& context, int wheelY) {
    if (!context.buildingSystem.isActive()) {
        return false;
    }

    context.buildingSystem.selectNextDef(wheelY >= 0 ? 1 : -1);
    return true;
}

}  // namespace BuildingInputController
