#pragma once

#include "Game/World/MapRegion.h"

#include <memory>
#include <string>

namespace RegionFactory {

std::unique_ptr<MapRegion> createRegion(const std::string& regionId);

// 对已存在的 MapRegion 重新配置其 specials(POI 和 connections)。
// 在 loadRegion 预加载后发现 connections 有旧坐标时调用。
void configureRegion(MapRegion& region);

}  // namespace RegionFactory
