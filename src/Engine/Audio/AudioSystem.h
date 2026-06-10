#pragma once

#include <SDL2/SDL.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {
namespace Audio {

class AudioSystem {
public:
    bool init();
    void shutdown();

    void playBgm(const std::string& id, bool loop = true);
    void stopBgm();
    void playSfx(const std::string& id);

    bool loadManifest(const std::string& path);

    void setMasterVolume(float volume);
    void setBgmVolume(float volume);
    void setSfxVolume(float volume);
    float getMasterVolume() const { return masterVolume; }
    float getBgmVolume() const { return bgmVolume; }
    float getSfxVolume() const { return sfxVolume; }

    void setBgmFadeSeconds(float seconds);
    float getBgmFadeSeconds() const { return bgmFadeSeconds; }

    const std::string& currentBgm() const { return currentBgmId; }
    bool isInitialized() const { return initialized; }
    bool hasBgmAsset(const std::string& id) const;
    bool hasSfxAsset(const std::string& id) const;

private:
    struct AudioAsset {
        std::string id;
        std::string filePath;
        float fallbackFrequency = 440.0f;
        float durationSeconds = 0.18f;
        float volume = 1.0f;
        bool loop = true;
        std::vector<float> samples;
    };

    struct Voice {
        float frequency = 440.0f;
        float remainingSeconds = 0.0f;
        float durationSeconds = 0.0f;
        float phase = 0.0f;
        float volume = 0.2f;
        std::vector<float> samples;
        std::size_t sampleCursor = 0;
    };

    SDL_AudioDeviceID deviceId = 0;
    SDL_AudioSpec deviceSpec{};
    bool initialized = false;
    std::string currentBgmId;
    const AudioAsset* currentBgmAsset = nullptr;
    std::size_t bgmSampleCursor = 0;
    float bgmFrequency = 220.0f;
    float bgmPhase = 0.0f;
    float bgmTrackVolume = 0.08f;
    bool bgmPlaying = false;
    bool bgmLooping = true;
    float masterVolume = 1.0f;
    float bgmVolume = 1.0f;
    float sfxVolume = 1.0f;
    float bgmFadeSeconds = 0.35f;
    float bgmFadeGain = 0.0f;
    float bgmTargetGain = 0.0f;
    std::vector<Voice> sfxVoices;
    std::unordered_map<std::string, AudioAsset> bgmAssets;
    std::unordered_map<std::string, AudioAsset> sfxAssets;

    static void audioCallback(void* userdata, Uint8* stream, int len);
    static AudioAsset makeFallbackBgmAsset(const std::string& id);
    static AudioAsset makeFallbackSfxAsset(const std::string& id);
    Voice voiceForSfx(const std::string& id) const;
    bool loadWavClip(AudioAsset& asset) const;
    void loadClipAssets(std::unordered_map<std::string, AudioAsset>& assets) const;
    void mix(float* out, int frameCount);
    float mixBgmFrame(float sampleRate);
    float mixVoiceFrame(Voice& voice, float sampleRate);
    void lockDevice();
    void unlockDevice();
};

}  // namespace Audio
}  // namespace Engine
