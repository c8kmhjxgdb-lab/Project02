#pragma once

#include <string>

namespace HudView {

struct Model {
    float healthPercent = 1.0f;
    float manaPercent = 1.0f;
    float flightPercent = 0.0f;
    float ultimatePercent = 0.0f;
    float shieldActivePercent = 0.0f;

    int coins = 0;

    float fireReady = 1.0f;
    float shieldReady = 1.0f;
    float lightningReady = 1.0f;
    float bondReady = 1.0f;
    float flightReady = 1.0f;

    bool canUseFireball = true;
    bool canUseIceSpike = true;
    bool canUseShield = false;
    bool canUseLightning = false;
    bool canUseBond = false;
    bool canUseFlight = false;

    std::string statusText;
    std::string noticeText;
    float noticeTimer = 0.0f;
};

void render(const Model& model, int screenW, int screenH);

}  // namespace HudView
