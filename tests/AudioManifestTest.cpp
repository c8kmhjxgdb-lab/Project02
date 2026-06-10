#include "Engine/Audio/AudioSystem.h"
#include "TestSupport.h"

int main() {
    Engine::Audio::AudioSystem audio;

    TestSupport::require(
        audio.loadManifest("assets/audio/manifest.json"),
        "audio manifest loads");
    TestSupport::require(audio.hasBgmAsset("bgm/starter_village"), "starter village bgm is registered");
    TestSupport::require(audio.hasBgmAsset("bgm/home_base"), "home base bgm is registered");
    TestSupport::require(audio.hasSfxAsset("sfx/skill/fireball"), "fireball sfx is registered");
    TestSupport::require(audio.hasSfxAsset("sfx/ui/navigate"), "ui navigate sfx is registered");
    TestSupport::require(audio.hasSfxAsset("sfx/ui/confirm"), "ui confirm sfx is registered");

    audio.setMasterVolume(2.0f);
    audio.setBgmVolume(-1.0f);
    audio.setSfxVolume(0.5f);
    audio.setBgmFadeSeconds(-3.0f);

    TestSupport::require(audio.getMasterVolume() == 1.0f, "master volume clamps high values");
    TestSupport::require(audio.getBgmVolume() == 0.0f, "bgm volume clamps low values");
    TestSupport::require(audio.getSfxVolume() == 0.5f, "sfx volume stores valid values");
    TestSupport::require(audio.getBgmFadeSeconds() == 0.0f, "bgm fade clamps low values");

    audio.playBgm("bgm/starter_village");
    TestSupport::require(audio.currentBgm() == "bgm/starter_village", "playBgm tracks current bgm without device");
    audio.stopBgm();
    TestSupport::require(audio.currentBgm().empty(), "stopBgm clears current bgm without device");

    audio.playSfx("sfx/skill/fireball");
    return 0;
}
