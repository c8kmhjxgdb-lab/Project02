#pragma once

#include "MapRegion.h"
#include <unordered_map>
#include <memory>
#include <string>

class RegionManager {
public:
    // 初始化
    void init();
    void shutdown();

    // 区域加载/卸载
    bool loadRegion(const std::string& regionId);
    void unloadCurrentRegion();

    // 区域切换（所有权交换）
    bool transitionTo(const std::string& targetRegionId,
                      const glm::ivec2& entryTile,
                      b2WorldId world);
    bool transitionTo(const MapConnection& connection, b2WorldId world);

    // 设置 Box2D 世界引用（在初始化时调用）
    void setWorldId(b2WorldId world) { worldId = world; }

    // 获取当前区域
    MapRegion* getCurrentRegion();
    const MapRegion* getCurrentRegion() const;

    // 获取过渡透明度（0=完全可见，1=全黑）
    float getTransitionAlpha() const { return transitionAlpha; }

    // 区域查询（优先返回当前区域，再查加载列表）
    bool hasRegion(const std::string& regionId) const;
    MapRegion* getRegion(const std::string& regionId);
    const MapRegion* getRegion(const std::string& regionId) const;

    // 获取已加载的区域列表
    const std::vector<std::string>& getDiscoveredRegions() const { return discoveredRegions; }

    // 区域过渡效果控制
    void setTransitionEffectEnabled(bool enabled) { useTransitionEffect = enabled; }
    void setTransitionDuration(float duration) { transitionDuration = duration; }

    // 更新（处理区域过渡动画）
    void update(float dt);
    bool isTransitioning() const { return isTransitioningFlag; }
    float getTransitionProgress() const { return transitionProgress; }

private:
    std::string currentRegionId;
    std::unique_ptr<MapRegion> currentRegion;
    std::unordered_map<std::string, std::unique_ptr<MapRegion>> loadedRegions;
    std::vector<std::string> discoveredRegions;
    b2WorldId worldId;  // Box2D 世界引用（用于物理同步）

    // 过渡效果
    bool useTransitionEffect = true;
    float transitionDuration = 0.5f;
    bool isTransitioningFlag = false;
    float transitionProgress = 0.0f;
    float transitionAlpha = 0.0f;  // 过渡透明度（0=完全可见，1=全黑）
    bool transitionFadeOut = true;

    // 过渡期间的临时数据
    std::string transitionTargetRegionId;
    glm::ivec2 transitionEntryTile;

    // 固定种子表（确保跨运行一致性）
    static int getRegionSeed(const std::string& regionId);

    // 过渡效果
    void beginTransition();
    void updateTransition(float dt);
    void completeTransition();
};
