#pragma once

#include "Game/Presentation/AbilityView.h"
#include "Game/Presentation/AimReticleView.h"
#include "Game/Presentation/EntityView.h"
#include "Game/Presentation/GameMenuView.h"
#include "Game/Presentation/HudView.h"
#include "Game/Presentation/MainMenuView.h"
#include "Game/Presentation/ParticleView.h"
#include "Game/World/TileMap.h"

#include <GL/glew.h>
#include <box2d/box2d.h>
#include <glm/vec2.hpp>

class BuildingSystem;
class DecorRenderer;
class DialogueUI;
class DropManager;
class EmotionSystem;
class EnemyManager;
class MiniMap;
class ParticleSystem;
class PostProcess;
class PopupCrownBoss;
class Princess;
class ProjectileManager;
class RegionManager;
class TimeSystem;
class ToySystem;
struct Camera2D;

namespace GameRenderer {

struct MainMenuRenderContext {
    MainMenuView::Model model;
    int screenWidth = 0;
    int screenHeight = 0;
};

struct WorldRenderContext {
    Camera2D& camera;
    RegionManager& regionManager;
    DecorRenderer& decorRenderer;
    BuildingSystem& buildingSystem;
    ToySystem& toySystem;
    DropManager& dropManager;
    EnemyManager& enemyManager;
    ProjectileManager& projectileManager;
    ParticleSystem& particleSystem;
    PostProcess& postProcess;
    MiniMap& miniMap;
    DialogueUI& dialogueUI;
    EmotionSystem& emotionSystem;
    TimeSystem& timeSystem;
    PopupCrownBoss& popupCrownBoss;
    Princess* princess = nullptr;
    b2BodyId playerBodyId;
    bool isDead = false;
    bool usePixelActors = true;
    int screenWidth = 0;
    int screenHeight = 0;
    float charTime = 0.0f;
    GLuint postProcessShader = 0;
    glm::vec2 mouseWorldPoint{0.0f, 0.0f};
    EntityView::EnemyRenderResources enemyResources;
    EntityView::CharacterRenderResources characterResources;
    EntityView::CharacterModel characterModel;
    EntityView::ProjectileRenderResources projectileResources;
    AbilityView::WorldEffectsModel abilityEffectsModel;
    ParticleView::VentTearsModel ventTearsModel;
    AimReticleView::Model aimReticleModel;
    HudView::Model hudModel;
    GameMenuView::Model gameMenuModel;
};

void renderMainMenu(const MainMenuRenderContext& context);

void renderWorld(WorldRenderContext& context, const TileColors& tileColors, float dt);

}  // namespace GameRenderer
