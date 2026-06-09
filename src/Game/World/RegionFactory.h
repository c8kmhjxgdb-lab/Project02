#pragma once

#include "Game/World/MapRegion.h"

#include <memory>
#include <string>

namespace RegionFactory {

std::unique_ptr<MapRegion> createRegion(const std::string& regionId);

}  // namespace RegionFactory
