#include "Game/Services/AudioService.h"

#include "Engine/Audio/AudioSystem.h"
#include "Game/World/MapRegion.h"

namespace {

std::string bgmForRegion(const MapRegion* region) {
    if (!region) return "bgm/default";
    if (region->getId() == "home_base") return "bgm/home_base";
    return "bgm/" + region->getId();
}

}  // namespace

namespace AudioService {

void playRegionBgm(Engine::Audio::AudioSystem& audio, const MapRegion* region) {
    audio.playBgm(bgmForRegion(region), true);
}

void playSkillSfx(Engine::Audio::AudioSystem& audio, const std::string& skillId) {
    audio.playSfx("sfx/skill/" + skillId);
}

void playUiSfx(Engine::Audio::AudioSystem& audio, const std::string& uiEventId) {
    audio.playSfx("sfx/ui/" + uiEventId);
}

}  // namespace AudioService
