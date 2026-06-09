#include "RegionManager.h"
#include "Game/World/RegionFactory.h"

#include <algorithm>

void RegionManager::init() {
    // 创建初始区域（新手村）
    auto starterRegion = RegionFactory::createRegion("starter_village");
    currentRegionId = "starter_village";
    currentRegion = std::move(starterRegion);
    discoveredRegions.push_back("starter_village");
    // 注意：buildPhysics 在 main.cpp 中单独调用，因为此时 worldId 可能还未初始化
}

void RegionManager::shutdown() {
    currentRegion.reset();
    loadedRegions.clear();
    discoveredRegions.clear();
    currentRegionId.clear();
    isTransitioningFlag = false;
    transitionProgress = 0.0f;
    transitionAlpha = 0.0f;
    transitionFadeOut = true;
    transitionTargetRegionId.clear();
    transitionEntryTile = glm::ivec2(0, 0);
}

void RegionManager::resetLoadedRegions() {
    shutdown();
}

bool RegionManager::loadRegion(const std::string& regionId) {
    if (regionId == currentRegionId && currentRegion) {
        return true;  // 已经是当前区域
    }

    if (loadedRegions.find(regionId) != loadedRegions.end()) {
        return true;  // 已经加载
    }

    auto region = RegionFactory::createRegion(regionId);

    if (worldId.index1 != 0) {
        region->buildPhysics(worldId);
    }

    loadedRegions[regionId] = std::move(region);
    if (std::find(discoveredRegions.begin(), discoveredRegions.end(), regionId) == discoveredRegions.end()) {
        discoveredRegions.push_back(regionId);
    }

    return true;
}

void RegionManager::unloadCurrentRegion() {
    if (!currentRegion) return;

    // 将当前区域存入加载列表
    archiveCurrentRegion();
    currentRegion.reset();
    currentRegionId.clear();

    // 限制已加载区域数量，防止物理体累积过多
    const size_t maxLoadedRegions = 5;
    while (loadedRegions.size() > maxLoadedRegions) {
        auto it = loadedRegions.begin();
        if (it->second) {
            it->second->clearPhysics();  // 销毁 Box2D 物理体
        }
        loadedRegions.erase(it);
    }
}

bool RegionManager::transitionTo(const std::string& targetRegionId,
                                  const glm::ivec2& entryTile,
                                  b2WorldId world) {
    if (targetRegionId == currentRegionId) return false;

    worldId = world;  // 保存世界引用

    // 如果目标区域已在加载列表中，仅交换所有权（物理体已存在，无需重建）
    auto it = loadedRegions.find(targetRegionId);
    if (it != loadedRegions.end()) {
        // Archive current region, swap in target from cache.
        archiveCurrentRegion();
        currentRegion = std::move(it->second);
        loadedRegions.erase(it);
        currentRegionId = targetRegionId;

        enforceLoadedCap();
        // Teleport the player to the entry tile in the now-current region.
        teleportPlayerToEntry(targetRegionId, entryTile);
        return true;
    }

    // 否则开始过渡流程（创建新区域）
    if (useTransitionEffect) {
        // Defer the actual region swap AND the player teleport to completeTransition().
        // Returning false here tells the caller "not done yet, don't teleport on your
        // behalf against the still-old current region".
        transitionTargetRegionId = targetRegionId;
        transitionEntryTile = entryTile;
        isTransitioningFlag = true;
        transitionProgress = 0.0f;
        transitionAlpha = 0.0f;
        transitionFadeOut = true;
        beginTransition();
        return false;
    } else {
        // 直接切换（无过渡效果）：直接 swap 并传送玩家
        performImmediateSwap(targetRegionId);
        teleportPlayerToEntry(targetRegionId, entryTile);
        return true;
    }
}

bool RegionManager::transitionTo(const MapConnection& connection, b2WorldId world) {
    return transitionTo(connection.targetRegionId, connection.targetTile, world);
}

void RegionManager::beginTransition() {
    // 过渡开始，淡出
}

void RegionManager::performImmediateSwap(const std::string& targetRegionId) {
    // Archive current region to the loaded cache, then create+load the target
    // if it isn't already in the cache. Used by both the no-fade path and
    // the deferred-fade path's completeTransition().
    archiveCurrentRegion();

    currentRegionId = targetRegionId;

    auto it = loadedRegions.find(targetRegionId);
    if (it != loadedRegions.end()) {
        // Already cached — just swap ownership.
        currentRegion = std::move(it->second);
        loadedRegions.erase(it);
    } else {
        currentRegion = RegionFactory::createRegion(targetRegionId);
        if (b2World_IsValid(worldId)) {
            currentRegion->buildPhysics(worldId);
        }
    }

    if (std::find(discoveredRegions.begin(), discoveredRegions.end(),
                  currentRegionId) == discoveredRegions.end()) {
        discoveredRegions.push_back(currentRegionId);
    }

    enforceLoadedCap();
}

void RegionManager::archiveCurrentRegion() {
    if (!currentRegion || currentRegionId.empty()) return;
    loadedRegions[currentRegionId] = std::move(currentRegion);
}

void RegionManager::teleportPlayerToEntry(const std::string& /*regionId*/,
                                          const glm::ivec2& entryTile) {
    if (!b2Body_IsValid(playerBody) || !currentRegion) return;
    if (!currentRegion->getTileMap().isInBounds(entryTile.x, entryTile.y)) return;
    glm::vec2 worldPos = currentRegion->getTileMap().tileToWorld(entryTile.x, entryTile.y);
    b2Body_SetTransform(playerBody, b2Vec2{worldPos.x, worldPos.y}, b2Rot{0});
    b2Body_SetLinearVelocity(playerBody, b2Vec2_zero);
}

void RegionManager::enforceLoadedCap() {
    const size_t maxLoadedRegions = 5;
    while (loadedRegions.size() > maxLoadedRegions) {
        auto it = loadedRegions.begin();
        if (it->second) {
            it->second->clearPhysics();
        }
        loadedRegions.erase(it);
    }
}

void RegionManager::updateTransition(float dt) {
    transitionProgress += dt / transitionDuration;

    // 限制进度在 [0, 1]
    if (transitionProgress > 1.0f) transitionProgress = 1.0f;

    // 更新透明度（淡出：0→1，淡入：1→0）
    if (transitionFadeOut) {
        transitionAlpha = transitionProgress;
    } else {
        transitionAlpha = 1.0f - transitionProgress;
    }

    if (transitionFadeOut && transitionProgress >= 1.0f) {
        // 淡出完成，切换区域
        transitionProgress = 0.0f;
        transitionFadeOut = false;
        completeTransition();
    } else if (!transitionFadeOut && transitionProgress >= 1.0f) {
        // 淡入完成，过渡结束
        isTransitioningFlag = false;
        transitionProgress = 0.0f;
        transitionAlpha = 0.0f;
    }
}

void RegionManager::completeTransition() {
    // Defer-was-pending: do the actual swap now (deferred from transitionTo)
    performImmediateSwap(transitionTargetRegionId);
    // Teleport the player to the entry tile in the freshly-loaded region.
    // This used to be the caller's responsibility, which broke when the caller
    // teleported against the still-old region during the fade.
    teleportPlayerToEntry(transitionTargetRegionId, transitionEntryTile);
    transitionEntryTile = glm::ivec2(0, 0);
}

void RegionManager::update(float dt) {
    if (isTransitioningFlag) {
        updateTransition(dt);
    }
}

MapRegion* RegionManager::getCurrentRegion() {
    return currentRegion.get();
}

const MapRegion* RegionManager::getCurrentRegion() const {
    return currentRegion.get();
}

bool RegionManager::hasRegion(const std::string& regionId) const {
    if (regionId == currentRegionId) return true;
    return loadedRegions.find(regionId) != loadedRegions.end();
}

MapRegion* RegionManager::getRegion(const std::string& regionId) {
    if (regionId == currentRegionId) {
        return currentRegion.get();
    }
    auto it = loadedRegions.find(regionId);
    return it != loadedRegions.end() ? it->second.get() : nullptr;
}

const MapRegion* RegionManager::getRegion(const std::string& regionId) const {
    if (regionId == currentRegionId) {
        return currentRegion.get();
    }
    auto it = loadedRegions.find(regionId);
    return it != loadedRegions.end() ? it->second.get() : nullptr;
}
