#pragma once

#include <glm/vec2.hpp>

#include <functional>
#include <memory>
#include <string>

class BuildingSystem;
class DialogueTree;
class DialogueUI;
class EmotionSystem;
class Princess;
class RegionManager;
class TimeSystem;
class VentAnimation;
class WeatherSystem;

#include <SDL2/SDL_scancode.h>

namespace DialogueService {

struct Context {
    BuildingSystem& buildingSystem;
    DialogueTree& dialogueTree;
    DialogueUI& dialogueUI;
    RegionManager& regionManager;
    TimeSystem& timeSystem;
    EmotionSystem& emotionSystem;
    WeatherSystem& weatherSystem;
    VentAnimation& ventAnimation;
    std::unique_ptr<Princess>& princess;
    glm::vec2& homePosition;
    float& homeRadius;
    bool& isVenting;
    bool& talkedWithPrincessAtBaseThisFrame;
};

struct NoticeSink {
    std::function<void(const std::string&)> show;
};

void showNotice(const NoticeSink& noticeSink, const std::string& notice);

void handleNavigation(Context& context, SDL_Scancode scancode);
bool advanceActiveDialogue(Context& context);
bool tryRestAtBaseBed(Context& context,
                      const glm::vec2& playerPos,
                      const NoticeSink& noticeSink);
bool tryStartPrincessDialogue(Context& context,
                              const glm::vec2& playerPos,
                              const NoticeSink& noticeSink);
bool tryVent(Context& context,
             const glm::vec2& playerPos,
             const NoticeSink& noticeSink);

}  // namespace DialogueService
