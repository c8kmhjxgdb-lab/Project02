#include "Engine/Audio/AudioSystem.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>

namespace Engine {
namespace Audio {

namespace {

constexpr int kSampleRate = 48000;
constexpr int kChannels = 2;
constexpr float kTwoPi = 6.28318530718f;

float frequencyForBgm(const std::string& id) {
    if (id.find("home_base") != std::string::npos) return 196.0f;
    if (id.find("starter_village") != std::string::npos) return 246.94f;
    return 220.0f;
}

float clampVolume(float volume) {
    return std::clamp(volume, 0.0f, 1.0f);
}

}  // namespace

bool AudioSystem::init() {
    if (initialized) return true;

    SDL_AudioSpec desired{};
    desired.freq = kSampleRate;
    desired.format = AUDIO_F32SYS;
    desired.channels = kChannels;
    desired.samples = 1024;
    desired.callback = &AudioSystem::audioCallback;
    desired.userdata = this;

    deviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &deviceSpec, 0);
    if (deviceId == 0) {
        initialized = false;
        return false;
    }

    initialized = true;
    SDL_PauseAudioDevice(deviceId, 0);
    return true;
}

void AudioSystem::shutdown() {
    if (deviceId != 0) {
        SDL_CloseAudioDevice(deviceId);
        deviceId = 0;
    }
    sfxVoices.clear();
    currentBgmId.clear();
    bgmPlaying = false;
    initialized = false;
}

bool AudioSystem::loadManifest(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        return false;
    }

    nlohmann::json json;
    try {
        input >> json;
    } catch (...) {
        return false;
    }

    std::unordered_map<std::string, AudioAsset> loadedBgm;
    std::unordered_map<std::string, AudioAsset> loadedSfx;

    auto loadSection = [](const nlohmann::json& section,
                          std::unordered_map<std::string, AudioAsset>& target,
                          bool defaultLoop) {
        if (!section.is_object()) return;

        for (const auto& entry : section.items()) {
            if (!entry.value().is_object()) continue;

            AudioAsset asset;
            asset.id = entry.key();
            asset.filePath = entry.value().value("file", std::string{});
            asset.fallbackFrequency = entry.value().value("fallbackFrequency", frequencyForBgm(asset.id));
            asset.durationSeconds = entry.value().value("durationSeconds", defaultLoop ? 0.0f : 0.18f);
            asset.volume = clampVolume(entry.value().value("volume", defaultLoop ? 0.08f : 0.18f));
            asset.loop = entry.value().value("loop", defaultLoop);
            target[asset.id] = std::move(asset);
        }
    };

    loadSection(json.value("bgm", nlohmann::json::object()), loadedBgm, true);
    loadSection(json.value("sfx", nlohmann::json::object()), loadedSfx, false);

    auto ensureBgmFallback = [&loadedBgm](const std::string& id) {
        if (loadedBgm.find(id) == loadedBgm.end()) {
            loadedBgm[id] = makeFallbackBgmAsset(id);
        }
    };
    auto ensureSfxFallback = [&loadedSfx](const std::string& id) {
        if (loadedSfx.find(id) == loadedSfx.end()) {
            loadedSfx[id] = makeFallbackSfxAsset(id);
        }
    };

    ensureBgmFallback("bgm/default");
    ensureBgmFallback("bgm/starter_village");
    ensureBgmFallback("bgm/home_base");
    ensureSfxFallback("sfx/skill/fireball");
    ensureSfxFallback("sfx/skill/ice_spike");
    ensureSfxFallback("sfx/skill/lightning");
    ensureSfxFallback("sfx/ui/navigate");
    ensureSfxFallback("sfx/ui/confirm");
    ensureSfxFallback("sfx/ui/cancel");

    loadClipAssets(loadedBgm);
    loadClipAssets(loadedSfx);

    lockDevice();
    bgmAssets = std::move(loadedBgm);
    sfxAssets = std::move(loadedSfx);
    if (!currentBgmId.empty()) {
        auto it = bgmAssets.find(currentBgmId);
        currentBgmAsset = it != bgmAssets.end() ? &it->second : nullptr;
    }
    unlockDevice();
    return true;
}

void AudioSystem::setMasterVolume(float volume) {
    lockDevice();
    masterVolume = clampVolume(volume);
    unlockDevice();
}

void AudioSystem::setBgmVolume(float volume) {
    lockDevice();
    bgmVolume = clampVolume(volume);
    unlockDevice();
}

void AudioSystem::setSfxVolume(float volume) {
    lockDevice();
    sfxVolume = clampVolume(volume);
    unlockDevice();
}

void AudioSystem::setBgmFadeSeconds(float seconds) {
    lockDevice();
    bgmFadeSeconds = std::max(0.0f, seconds);
    unlockDevice();
}

bool AudioSystem::hasBgmAsset(const std::string& id) const {
    return bgmAssets.find(id) != bgmAssets.end();
}

bool AudioSystem::hasSfxAsset(const std::string& id) const {
    return sfxAssets.find(id) != sfxAssets.end();
}

void AudioSystem::playBgm(const std::string& id, bool loop) {
    if (!loop) return;

    lockDevice();
    auto it = bgmAssets.find(id);
    if (it == bgmAssets.end()) {
        it = bgmAssets.emplace(id, makeFallbackBgmAsset(id)).first;
    }
    currentBgmId = id;
    currentBgmAsset = &it->second;
    bgmSampleCursor = 0;
    bgmFrequency = currentBgmAsset->fallbackFrequency;
    bgmTrackVolume = currentBgmAsset->volume;
    bgmLooping = loop;
    bgmPlaying = true;
    bgmTargetGain = 1.0f;
    if (bgmFadeSeconds <= 0.0f) {
        bgmFadeGain = 1.0f;
    }
    unlockDevice();
}

void AudioSystem::stopBgm() {
    lockDevice();
    bgmTargetGain = 0.0f;
    if (deviceId == 0 || bgmFadeSeconds <= 0.0f) {
        currentBgmId.clear();
        currentBgmAsset = nullptr;
        bgmPlaying = false;
        bgmFadeGain = 0.0f;
    }
    unlockDevice();
}

void AudioSystem::playSfx(const std::string& id) {
    lockDevice();
    if (sfxVoices.size() < 16) {
        sfxVoices.push_back(voiceForSfx(id));
    }
    unlockDevice();
}

AudioSystem::Voice AudioSystem::voiceForSfx(const std::string& id) const {
    auto it = sfxAssets.find(id);
    AudioAsset fallback;
    const AudioAsset* asset = nullptr;
    if (it != sfxAssets.end()) {
        asset = &it->second;
    } else {
        fallback = makeFallbackSfxAsset(id);
        asset = &fallback;
    }

    Voice voice;
    voice.frequency = asset->fallbackFrequency;
    voice.durationSeconds = asset->durationSeconds > 0.0f ? asset->durationSeconds : 0.18f;
    voice.remainingSeconds = voice.durationSeconds;
    voice.volume = asset->volume;
    voice.samples = asset->samples;
    voice.sampleCursor = 0;
    return voice;
}

AudioSystem::AudioAsset AudioSystem::makeFallbackBgmAsset(const std::string& id) {
    AudioAsset asset;
    asset.id = id;
    asset.fallbackFrequency = frequencyForBgm(id);
    asset.durationSeconds = 0.0f;
    asset.volume = 0.08f;
    asset.loop = true;
    return asset;
}

AudioSystem::AudioAsset AudioSystem::makeFallbackSfxAsset(const std::string& id) {
    AudioAsset asset;
    asset.id = id;
    asset.durationSeconds = 0.18f;
    asset.volume = 0.18f;
    asset.loop = false;
    asset.fallbackFrequency = 523.25f;

    if (id.find("ice") != std::string::npos) {
        asset.fallbackFrequency = 880.0f;
        asset.durationSeconds = 0.14f;
        asset.volume = 0.14f;
    } else if (id.find("lightning") != std::string::npos) {
        asset.fallbackFrequency = 1320.0f;
        asset.durationSeconds = 0.22f;
        asset.volume = 0.16f;
    } else if (id.find("ui") != std::string::npos) {
        asset.fallbackFrequency = 660.0f;
        asset.durationSeconds = 0.08f;
        asset.volume = 0.10f;
    }

    return asset;
}

bool AudioSystem::loadWavClip(AudioAsset& asset) const {
    if (asset.filePath.empty()) return false;

    SDL_AudioSpec wavSpec{};
    Uint8* wavBuffer = nullptr;
    Uint32 wavLength = 0;
    if (!SDL_LoadWAV(asset.filePath.c_str(), &wavSpec, &wavBuffer, &wavLength)) {
        return false;
    }

    SDL_AudioStream* stream = SDL_NewAudioStream(
        wavSpec.format,
        wavSpec.channels,
        wavSpec.freq,
        AUDIO_F32SYS,
        kChannels,
        kSampleRate);
    if (!stream) {
        SDL_FreeWAV(wavBuffer);
        return false;
    }

    bool ok = SDL_AudioStreamPut(stream, wavBuffer, static_cast<int>(wavLength)) == 0 &&
              SDL_AudioStreamFlush(stream) == 0;
    SDL_FreeWAV(wavBuffer);
    if (!ok) {
        SDL_FreeAudioStream(stream);
        return false;
    }

    int available = SDL_AudioStreamAvailable(stream);
    if (available <= 0) {
        SDL_FreeAudioStream(stream);
        return false;
    }

    std::vector<float> samples(static_cast<std::size_t>(available) / sizeof(float));
    int read = SDL_AudioStreamGet(stream, samples.data(), available);
    SDL_FreeAudioStream(stream);
    if (read <= 0) return false;

    samples.resize(static_cast<std::size_t>(read) / sizeof(float));
    asset.samples = std::move(samples);
    if (!asset.loop && asset.durationSeconds <= 0.0f) {
        asset.durationSeconds = static_cast<float>(asset.samples.size()) /
            static_cast<float>(kSampleRate * kChannels);
    }
    return true;
}

void AudioSystem::loadClipAssets(std::unordered_map<std::string, AudioAsset>& assets) const {
    for (auto& pair : assets) {
        loadWavClip(pair.second);
    }
}

void AudioSystem::audioCallback(void* userdata, Uint8* stream, int len) {
    auto* audio = static_cast<AudioSystem*>(userdata);
    float* out = reinterpret_cast<float*>(stream);
    int sampleCount = len / static_cast<int>(sizeof(float));
    std::fill(out, out + sampleCount, 0.0f);
    if (!audio || sampleCount <= 0) return;
    audio->mix(out, sampleCount / kChannels);
}

void AudioSystem::mix(float* out, int frameCount) {
    const float sampleRate = static_cast<float>(deviceSpec.freq > 0 ? deviceSpec.freq : kSampleRate);
    const int channels = std::max<int>(deviceSpec.channels, 1);

    for (int frame = 0; frame < frameCount; ++frame) {
        float mixed = 0.0f;

        mixed += mixBgmFrame(sampleRate);

        for (Voice& voice : sfxVoices) {
            mixed += mixVoiceFrame(voice, sampleRate);
        }

        mixed = std::clamp(mixed * masterVolume, -0.8f, 0.8f);
        for (int channel = 0; channel < channels; ++channel) {
            out[frame * channels + channel] = mixed;
        }
    }

    sfxVoices.erase(
        std::remove_if(
            sfxVoices.begin(),
            sfxVoices.end(),
            [](const Voice& voice) { return voice.remainingSeconds <= 0.0f; }),
        sfxVoices.end());
}

float AudioSystem::mixBgmFrame(float sampleRate) {
    if (!bgmPlaying) return 0.0f;

    if (bgmFadeSeconds <= 0.0f) {
        bgmFadeGain = bgmTargetGain;
    } else {
        float step = 1.0f / (bgmFadeSeconds * sampleRate);
        if (bgmFadeGain < bgmTargetGain) {
            bgmFadeGain = std::min(bgmTargetGain, bgmFadeGain + step);
        } else if (bgmFadeGain > bgmTargetGain) {
            bgmFadeGain = std::max(bgmTargetGain, bgmFadeGain - step);
        }
    }

    if (bgmTargetGain <= 0.0f && bgmFadeGain <= 0.0f) {
        currentBgmId.clear();
        currentBgmAsset = nullptr;
        bgmPlaying = false;
        return 0.0f;
    }

    float sample = 0.0f;
    if (currentBgmAsset && !currentBgmAsset->samples.empty()) {
        sample = currentBgmAsset->samples[bgmSampleCursor];
        bgmSampleCursor += static_cast<std::size_t>(kChannels);
        if (bgmSampleCursor >= currentBgmAsset->samples.size()) {
            if (bgmLooping) {
                bgmSampleCursor = 0;
            } else {
                bgmPlaying = false;
                currentBgmId.clear();
                currentBgmAsset = nullptr;
            }
        }
    } else {
        sample = std::sin(bgmPhase);
        bgmPhase += kTwoPi * bgmFrequency / sampleRate;
        if (bgmPhase > kTwoPi) bgmPhase -= kTwoPi;
    }

    return sample * bgmTrackVolume * bgmVolume * bgmFadeGain;
}

float AudioSystem::mixVoiceFrame(Voice& voice, float sampleRate) {
    if (voice.remainingSeconds <= 0.0f) return 0.0f;

    float sample = 0.0f;
    if (!voice.samples.empty()) {
        sample = voice.samples[voice.sampleCursor];
        voice.sampleCursor += static_cast<std::size_t>(kChannels);
        if (voice.sampleCursor >= voice.samples.size()) {
            voice.remainingSeconds = 0.0f;
        }
    } else {
        sample = std::sin(voice.phase);
        voice.phase += kTwoPi * voice.frequency / sampleRate;
        if (voice.phase > kTwoPi) voice.phase -= kTwoPi;
    }

    float t = voice.durationSeconds > 0.0f
        ? voice.remainingSeconds / voice.durationSeconds
        : 0.0f;
    float envelope = std::clamp(t, 0.0f, 1.0f);
    voice.remainingSeconds -= 1.0f / sampleRate;
    return sample * voice.volume * sfxVolume * envelope;
}

void AudioSystem::lockDevice() {
    if (deviceId != 0) {
        SDL_LockAudioDevice(deviceId);
    }
}

void AudioSystem::unlockDevice() {
    if (deviceId != 0) {
        SDL_UnlockAudioDevice(deviceId);
    }
}

}  // namespace Audio
}  // namespace Engine
