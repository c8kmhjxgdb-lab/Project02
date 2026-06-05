#include "RegionManager.h"
#include <algorithm>
#include <functional>

void RegionManager::init() {
    // 创建初始区域（新手村）
    auto starterRegion = std::make_unique<MapRegion>();
    starterRegion->generate("starter_village", "新手村",
                           RegionType::Overworld, 42, 60, 60);

    // 添加家的POI
    PointOfInterest home;
    home.type = PointOfInterest::Type::Home;
    home.id = "player_home";
    home.displayName = "玩家之家";
    home.tilePos = {5, 5};
    home.metadata = 0;
    starterRegion->addPOI(home);

    // 添加通往黑暗森林的连接
    MapConnection forestConn;
    forestConn.direction = MapConnection::Direction::East;
    forestConn.targetRegionId = "dark_forest";
    forestConn.sourceTile = {59, 30};  // 东边界
    forestConn.targetTile = {0, 30};    // 西边界入口
    starterRegion->addConnection(forestConn);

    currentRegionId = "starter_village";
    currentRegion = std::move(starterRegion);
    discoveredRegions.push_back("starter_village");
    // 注意：buildPhysics 在 main.cpp 中单独调用，因为此时 worldId 可能还未初始化
}

void RegionManager::shutdown() {
    currentRegion.reset();
    loadedRegions.clear();
    discoveredRegions.clear();
}

bool RegionManager::loadRegion(const std::string& regionId) {
    if (regionId == currentRegionId && currentRegion) {
        return true;  // 已经是当前区域
    }

    if (loadedRegions.find(regionId) != loadedRegions.end()) {
        return true;  // 已经加载
    }

    // 创建新区域
    auto region = std::make_unique<MapRegion>();
    int seed = getRegionSeed(regionId);
    region->generate(regionId, regionId, RegionType::Overworld, seed, 60, 60);

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
    loadedRegions[currentRegionId] = std::move(currentRegion);
    currentRegion.reset();
    currentRegionId.clear();
}

bool RegionManager::transitionTo(const std::string& targetRegionId,
                                  const glm::ivec2& entryTile,
                                  b2WorldId world) {
    if (targetRegionId == currentRegionId) return false;

    worldId = world;  // 保存世界引用

    // 如果目标区域已在加载列表中，仅交换所有权（物理体已存在，无需重建）
    auto it = loadedRegions.find(targetRegionId);
    if (it != loadedRegions.end()) {
        // 将当前区域存入加载列表
        loadedRegions[currentRegionId] = std::move(currentRegion);
        // 从加载列表取出目标区域
        currentRegion = std::move(it->second);
        loadedRegions.erase(it);
        currentRegionId = targetRegionId;
        return true;
    }

    // 否则开始过渡流程（创建新区域）
    if (useTransitionEffect) {
        transitionTargetRegionId = targetRegionId;
        transitionEntryTile = entryTile;
        isTransitioningFlag = true;
        transitionProgress = 0.0f;
        transitionAlpha = 0.0f;
        transitionFadeOut = true;
        beginTransition();
        return true;
    } else {
        // 直接切换（无过渡效果）
        // 将当前区域存入加载列表
        loadedRegions[currentRegionId] = std::move(currentRegion);

        currentRegionId = targetRegionId;
        currentRegion = std::make_unique<MapRegion>();
        // 使用固定种子表确保跨运行一致性
        int seed = getRegionSeed(targetRegionId);
        currentRegion->generate(targetRegionId, targetRegionId,
                               RegionType::Overworld, seed, 60, 60);
        currentRegion->buildPhysics(worldId);
        discoveredRegions.push_back(targetRegionId);
        return true;
    }
}

bool RegionManager::transitionTo(const MapConnection& connection, b2WorldId world) {
    return transitionTo(connection.targetRegionId, connection.targetTile, world);
}

void RegionManager::beginTransition() {
    // 过渡开始，淡出
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
    // 将当前区域存入加载列表（所有权转移，不清除物理体）
    loadedRegions[currentRegionId] = std::move(currentRegion);

    // 创建新区域
    currentRegionId = transitionTargetRegionId;
    currentRegion = std::make_unique<MapRegion>();
    // 使用固定种子表确保跨运行一致性
    int seed = getRegionSeed(currentRegionId);
    currentRegion->generate(currentRegionId, currentRegionId,
                           RegionType::Overworld, seed, 60, 60);
    // 为新区域创建物理刚体
    currentRegion->buildPhysics(worldId);

    if (std::find(discoveredRegions.begin(), discoveredRegions.end(),
                  currentRegionId) == discoveredRegions.end()) {
        discoveredRegions.push_back(currentRegionId);
    }
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

// 固定种子表：确保跨运行一致性
int RegionManager::getRegionSeed(const std::string& regionId) {
    static const std::unordered_map<std::string, int> REGION_SEEDS = {
        {"starter_village", 42},
        {"dark_forest", 12345},
        {"mountain_pass", 67890},
        {"coastal_town", 11111},
        // 默认种子：使用稳定的哈希函数
    };

    auto it = REGION_SEEDS.find(regionId);
    if (it != REGION_SEEDS.end()) {
        return it->second;
    }

    // 回退：使用稳定的哈希（FNV-1a）
    unsigned int hash = 2166136261u;
    for (char c : regionId) {
        hash ^= static_cast<unsigned int>(c);
        hash *= 16777619u;
    }
    return static_cast<int>(hash);
}
