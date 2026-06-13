#pragma once

#include <glm/mat4x4.hpp>
#include <string>

namespace MainMenuView {

constexpr int kMenuItemCount = 3;

struct Model {
    int selectedIndex = 0;
    bool hasSave = false;
    float animationTime = 0.0f;
    std::string message;
    float messageTimer = 0.0f;
    std::string saveTimestamp;
    std::string saveRegionName;
    float childlikeHeart = 0.0f;
    int rescuedPartners = 0;
};

void render(const Model& model, const glm::mat4& uiProj, int screenW, int screenH);

// Returns -1 when the pointer is outside every menu item.
int hitTest(float screenX, float screenYFromTop, int screenW, int screenH);

}  // namespace MainMenuView
