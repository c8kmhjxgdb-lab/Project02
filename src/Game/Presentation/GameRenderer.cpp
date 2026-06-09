#include "Game/Presentation/GameRenderer.h"

#include "Engine/Renderer/Draw2D.h"
#include "Game/GameState.h"
#include "Game/Presentation/AbilityView.h"
#include "Game/Presentation/AimReticleView.h"
#include "Game/Presentation/BuildingView.h"
#include "Game/Presentation/EntityView.h"
#include "Game/Presentation/HudView.h"
#include "Game/Presentation/MainMenuView.h"
#include "Game/Presentation/ParticleView.h"
#include "Game/Presentation/PresentationModelBuilder.h"
#include "Game/Presentation/WorldRenderer.h"
#include "Game/Presentation/WorldSignView.h"
#include "Game/Services/PlayerInputQuery.h"
#include "Game/Services/WorldQuery.h"

#include <GL/glew.h>
#include <box2d/box2d.h>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <vector>

namespace {

struct RenderEntry {
    int type = 0;  // 0=drop, 1=enemy, 2=princess, 3=player
    float y = 0.0f;
    size_t index = 0;

    bool operator<(const RenderEntry& other) const {
        return y < other.y;
    }
};

void renderSortedEntities(GameState& gs, const glm::mat4& viewProj) {
    std::vector<RenderEntry> renderQueue;
    EntityView::EnemyRenderResources enemyResources =
        PresentationModelBuilder::buildEnemyRenderResources(gs);
    EntityView::CharacterRenderResources characterResources =
        PresentationModelBuilder::buildCharacterRenderResources(gs);
    EntityView::CharacterModel characterModel =
        PresentationModelBuilder::buildCharacterModel(gs);

    const auto& drops = gs.dropManager.getActive();
    for (size_t i = 0; i < drops.size(); ++i) {
        if (!drops[i].active || !b2Body_IsValid(drops[i].bodyId)) continue;
        b2Vec2 pos = b2Body_GetPosition(drops[i].bodyId);
        renderQueue.push_back({0, pos.y, i});
    }

    const auto& enemies = gs.enemyManager.getActive();
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i].active || !b2Body_IsValid(enemies[i].bodyId)) continue;
        b2Vec2 pos = b2Body_GetPosition(enemies[i].bodyId);
        renderQueue.push_back({1, pos.y, i});
    }

    if (gs.princess) {
        glm::vec2 pPos = gs.princess->getPosition();
        renderQueue.push_back({2, pPos.y, 0});
    }

    if (!gs.isDead) {
        b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
        renderQueue.push_back({3, pPos.y, 0});
    }

    std::sort(renderQueue.begin(), renderQueue.end());

    for (const auto& entry : renderQueue) {
        switch (entry.type) {
            case 0:
                EntityView::renderDrop(drops[entry.index], viewProj);
                break;
            case 1:
                EntityView::renderEnemy(enemyResources, enemies[entry.index], viewProj, gs.charTime);
                break;
            case 2:
                if (gs.princess) gs.princess->render(viewProj);
                break;
            case 3:
                EntityView::renderCharacter(characterResources, characterModel, viewProj);
                break;
        }
    }

    auto aliveEnemies = gs.enemyManager.getAlive();
    EntityView::renderEnemyHealthBars(aliveEnemies, viewProj);
}

void renderWorldScene(GameState& gs,
                      const TileColors& tileColors,
                      const glm::mat4& viewProj,
                      float dt) {
    if (MapRegion* region = gs.regionManager.getCurrentRegion()) {
        float left, right, bottom, top;
        gs.camera.getViewportBounds(static_cast<float>(gs.screenWidth),
                                    static_cast<float>(gs.screenHeight),
                                    left, right, bottom, top);
        WorldRenderer::renderTileMap(
            region->getTileMap(),
            tileColors,
            viewProj,
            WorldRenderer::ViewBounds{left, right, bottom, top},
            gs.charTime);
    }

    if (MapRegion* region = gs.regionManager.getCurrentRegion()) {
        WorldRenderer::renderLowDecorations(gs.decorRenderer,
                                            region->getTileMap(),
                                            region->getDecorations(),
                                            viewProj,
                                            dt);
    }

    if (MapRegion* region = gs.regionManager.getCurrentRegion()) {
        WorldSignView::renderBoards(region->getTileMap(), region->getPOIs(), viewProj);
    }

    if (MapRegion* buildRegion = gs.regionManager.getCurrentRegion()) {
        if (gs.buildingSystem.isBuildableHere(buildRegion->getId())) {
            BuildingView::render(gs.buildingSystem, buildRegion->getTileMap(), viewProj);
            BuildingView::renderPreview(gs.buildingSystem, buildRegion->getTileMap(), viewProj,
                PlayerInputQuery::getMouseWorldPoint(gs));
            gs.toySystem.renderMiniCar(viewProj);
        }
    }

    renderSortedEntities(gs, viewProj);

    Draw2D::beginFrame(viewProj);
    AbilityView::renderWorldEffects(PresentationModelBuilder::buildAbilityEffectsModel(gs));
    Draw2D::endFrame();

    EntityView::ProjectileRenderResources projectileResources =
        PresentationModelBuilder::buildProjectileRenderResources(gs);
    for (const auto& proj : gs.projectileManager.getActive()) {
        EntityView::renderProjectile(projectileResources, proj, viewProj, gs.charTime);
    }

    if (MapRegion* region = gs.regionManager.getCurrentRegion()) {
        WorldRenderer::renderHighDecorations(gs.decorRenderer,
                                             region->getTileMap(),
                                             region->getDecorations(),
                                             viewProj,
                                             dt);
    }

    ParticleView::render(
        gs.particleSystem.getActive(),
        PresentationModelBuilder::buildVentTearsModel(gs),
        viewProj);
    AimReticleView::render(PresentationModelBuilder::buildAimReticleModel(gs), viewProj);
}

void renderScreenEffectsAndUi(GameState& gs) {
    if (gs.postProcessShader && gs.postProcess.isValid()) {
        float postIntensity = gs.emotionSystem.getPostProcessIntensity();
        if (WorldQuery::isCurrentRegion(gs.regionManager, "home_base")) {
            postIntensity = std::max(0.0f,
                postIntensity - gs.buildingSystem.getNightLightBonus(gs.timeSystem.getHour()));
        }
        gs.postProcess.setVignetteIntensity(postIntensity);
        gs.postProcess.draw(gs.postProcessShader);
    }

    const float designW = 800.0f;
    const float designH = 600.0f;
    float uiW = static_cast<float>(gs.screenWidth);
    float uiH = static_cast<float>(gs.screenHeight);
    if (uiW < designW) uiW = designW;
    if (uiH < designH) uiH = designH;
    glm::mat4 uiProj = glm::ortho(0.0f, uiW, 0.0f, uiH);

    HudView::render(
        PresentationModelBuilder::buildHudModel(gs),
        static_cast<int>(uiW),
        static_cast<int>(uiH));

    gs.miniMap.render(uiProj, static_cast<int>(uiW), static_cast<int>(uiH));

    glm::mat4 screenTextProj = glm::ortho(
        0.0f, static_cast<float>(gs.screenWidth),
        0.0f, static_cast<float>(gs.screenHeight));
    if (MapRegion* region = gs.regionManager.getCurrentRegion()) {
        WorldSignView::renderText(region->getTileMap(),
                                  region->getPOIs(),
                                  gs.camera,
                                  screenTextProj,
                                  gs.screenWidth,
                                  gs.screenHeight);
    }

    if (gs.dialogueUI.isVisible()) {
        gs.dialogueUI.render(uiProj, static_cast<int>(uiW), static_cast<int>(uiH));
    }
}

}  // namespace

namespace GameRenderer {

void renderMainMenu(GameState& gs) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, gs.screenWidth, gs.screenHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    float uiW = static_cast<float>(std::max(gs.screenWidth, 800));
    float uiH = static_cast<float>(std::max(gs.screenHeight, 600));
    glm::mat4 menuProj = glm::ortho(0.0f, uiW, 0.0f, uiH);
    MainMenuView::render(
        PresentationModelBuilder::buildMainMenuModel(gs),
        menuProj,
        static_cast<int>(uiW),
        static_cast<int>(uiH));
}

void renderWorld(GameState& gs, const TileColors& tileColors, float dt) {
    glm::mat4 viewProj = gs.camera.getViewProjMatrix(static_cast<float>(gs.screenWidth),
                                                     static_cast<float>(gs.screenHeight));

    gs.postProcess.beginRender();
    glClear(GL_COLOR_BUFFER_BIT);
    renderWorldScene(gs, tileColors, viewProj, dt);
    gs.postProcess.endRender();

    renderScreenEffectsAndUi(gs);
}

}  // namespace GameRenderer
