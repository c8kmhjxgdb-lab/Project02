#include "Game/Presentation/GameRenderer.h"

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/DialogueUI.h"
#include "Engine/Renderer/MiniMap.h"
#include "Engine/Renderer/PostProcess.h"
#include "Game/Emotion/EmotionSystem.h"
#include "Game/Presentation/AbilityView.h"
#include "Game/Presentation/AimReticleView.h"
#include "Game/Presentation/BuildingView.h"
#include "Game/Presentation/EntityView.h"
#include "Game/Presentation/GameMenuView.h"
#include "Game/Presentation/HudView.h"
#include "Game/Presentation/MainMenuView.h"
#include "Game/Presentation/ParticleView.h"
#include "Game/Presentation/WorldRenderer.h"
#include "Game/Presentation/WorldSignView.h"
#include "Game/Services/WorldQuery.h"
#include "Game/Social/Princess.h"
#include "Game/Toy/ToySystem.h"
#include "Game/World/TimeSystem.h"

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

void renderSortedEntities(GameRenderer::WorldRenderContext& context, const glm::mat4& viewProj) {
    std::vector<RenderEntry> renderQueue;

    const auto& drops = context.dropManager.getActive();
    for (size_t i = 0; i < drops.size(); ++i) {
        if (!drops[i].active || !b2Body_IsValid(drops[i].bodyId)) continue;
        b2Vec2 pos = b2Body_GetPosition(drops[i].bodyId);
        renderQueue.push_back({0, pos.y, i});
    }

    const auto& enemies = context.enemyManager.getActive();
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i].active || !b2Body_IsValid(enemies[i].bodyId)) continue;
        b2Vec2 pos = b2Body_GetPosition(enemies[i].bodyId);
        renderQueue.push_back({1, pos.y, i});
    }

    if (context.princess) {
        glm::vec2 pPos = context.princess->getPosition();
        renderQueue.push_back({2, pPos.y, 0});
    }

    if (!context.isDead) {
        b2Vec2 pPos = b2Body_GetPosition(context.playerBodyId);
        renderQueue.push_back({3, pPos.y, 0});
    }

    std::sort(renderQueue.begin(), renderQueue.end());

    for (const auto& entry : renderQueue) {
        switch (entry.type) {
            case 0:
                EntityView::renderDrop(drops[entry.index], viewProj);
                break;
            case 1:
                EntityView::renderEnemy(
                    context.enemyResources,
                    enemies[entry.index],
                    viewProj,
                    context.charTime);
                break;
            case 2:
                if (context.princess) context.princess->render(viewProj);
                break;
            case 3:
                EntityView::renderCharacter(
                    context.characterResources,
                    context.characterModel,
                    viewProj);
                break;
        }
    }

    auto aliveEnemies = context.enemyManager.getAlive();
    EntityView::renderEnemyHealthBars(aliveEnemies, viewProj);
}

void renderWorldScene(GameRenderer::WorldRenderContext& context,
                      const TileColors& tileColors,
                      const glm::mat4& viewProj,
                      float dt) {
    if (MapRegion* region = context.regionManager.getCurrentRegion()) {
        float left, right, bottom, top;
        context.camera.getViewportBounds(static_cast<float>(context.screenWidth),
                                         static_cast<float>(context.screenHeight),
                                         left, right, bottom, top);
        WorldRenderer::renderTileMap(
            region->getTileMap(),
            tileColors,
            viewProj,
            WorldRenderer::ViewBounds{left, right, bottom, top},
            context.charTime);
    }

    if (MapRegion* region = context.regionManager.getCurrentRegion()) {
        WorldRenderer::renderLowDecorations(context.decorRenderer,
                                            region->getTileMap(),
                                            region->getDecorations(),
                                            viewProj,
                                            dt);
    }

    if (MapRegion* region = context.regionManager.getCurrentRegion()) {
        WorldSignView::renderBoards(region->getTileMap(), region->getPOIs(), viewProj);
    }

    if (MapRegion* buildRegion = context.regionManager.getCurrentRegion()) {
        if (context.buildingSystem.isBuildableHere(buildRegion->getId())) {
            BuildingView::render(context.buildingSystem, buildRegion->getTileMap(), viewProj);
            BuildingView::renderPreview(
                context.buildingSystem,
                buildRegion->getTileMap(),
                viewProj,
                context.mouseWorldPoint);
            context.toySystem.renderMiniCar(viewProj);
        }
    }

    renderSortedEntities(context, viewProj);

    Draw2D::beginFrame(viewProj);
    AbilityView::renderWorldEffects(context.abilityEffectsModel);
    Draw2D::endFrame();

    for (const auto& proj : context.projectileManager.getActive()) {
        EntityView::renderProjectile(
            context.projectileResources,
            proj,
            viewProj,
            context.charTime);
    }

    if (MapRegion* region = context.regionManager.getCurrentRegion()) {
        WorldRenderer::renderHighDecorations(context.decorRenderer,
                                             region->getTileMap(),
                                             region->getDecorations(),
                                             viewProj,
                                             dt);
    }

    ParticleView::render(
        context.particleSystem.getActive(),
        context.ventTearsModel,
        viewProj);
    AimReticleView::render(context.aimReticleModel, viewProj);
}

void renderScreenEffectsAndUi(GameRenderer::WorldRenderContext& context) {
    if (context.postProcessShader && context.postProcess.isValid()) {
        float postIntensity = context.emotionSystem.getPostProcessIntensity();
        if (WorldQuery::isCurrentRegion(context.regionManager, "home_base")) {
            postIntensity = std::max(0.0f,
                postIntensity -
                context.buildingSystem.getNightLightBonus(context.timeSystem.getHour()));
        }
        context.postProcess.setVignetteIntensity(postIntensity);
        context.postProcess.draw(context.postProcessShader);
    }

    const float designW = 800.0f;
    const float designH = 600.0f;
    float uiW = static_cast<float>(context.screenWidth);
    float uiH = static_cast<float>(context.screenHeight);
    if (uiW < designW) uiW = designW;
    if (uiH < designH) uiH = designH;
    glm::mat4 uiProj = glm::ortho(0.0f, uiW, 0.0f, uiH);

    HudView::render(
        context.hudModel,
        static_cast<int>(uiW),
        static_cast<int>(uiH));

    context.miniMap.render(uiProj, static_cast<int>(uiW), static_cast<int>(uiH));

    glm::mat4 screenTextProj = glm::ortho(
        0.0f, static_cast<float>(context.screenWidth),
        0.0f, static_cast<float>(context.screenHeight));
    if (MapRegion* region = context.regionManager.getCurrentRegion()) {
        WorldSignView::renderText(region->getTileMap(),
                                  region->getPOIs(),
                                  context.camera,
                                  screenTextProj,
                                  context.screenWidth,
                                  context.screenHeight);
    }

    if (context.dialogueUI.isVisible()) {
        context.dialogueUI.render(uiProj, static_cast<int>(uiW), static_cast<int>(uiH));
    }

    GameMenuView::render(
        context.gameMenuModel,
        static_cast<int>(uiW),
        static_cast<int>(uiH));
}

}  // namespace

namespace GameRenderer {

void renderMainMenu(const MainMenuRenderContext& context) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, context.screenWidth, context.screenHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    float uiW = static_cast<float>(std::max(context.screenWidth, 800));
    float uiH = static_cast<float>(std::max(context.screenHeight, 600));
    glm::mat4 menuProj = glm::ortho(0.0f, uiW, 0.0f, uiH);
    MainMenuView::render(
        context.model,
        menuProj,
        static_cast<int>(uiW),
        static_cast<int>(uiH));
}

void renderWorld(WorldRenderContext& context, const TileColors& tileColors, float dt) {
    glm::mat4 viewProj = context.camera.getViewProjMatrix(
        static_cast<float>(context.screenWidth),
        static_cast<float>(context.screenHeight));

    context.postProcess.beginRender();
    glClear(GL_COLOR_BUFFER_BIT);
    renderWorldScene(context, tileColors, viewProj, dt);
    context.postProcess.endRender();

    renderScreenEffectsAndUi(context);
}

}  // namespace GameRenderer
