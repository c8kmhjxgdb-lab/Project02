#pragma once

#include <string>

namespace Engine {
namespace Audio {
class AudioSystem;
}
}

class MapRegion;

namespace AudioService {

void playRegionBgm(Engine::Audio::AudioSystem& audio, const MapRegion* region);
void playSkillSfx(Engine::Audio::AudioSystem& audio, const std::string& skillId);
void playUiSfx(Engine::Audio::AudioSystem& audio, const std::string& uiEventId);

}  // namespace AudioService
