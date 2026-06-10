#include "Game/Presentation/WorldPresentationBuilder.h"

#include "Game/GameState.h"
#include "Game/Presentation/PresentationModelBuilder.h"
#include "Game/Services/PlayerInputQuery.h"

namespace WorldPresentationBuilder {

GameRenderer::WorldRenderContext buildRenderContext(GameState& gs) {
    return {
        gs.camera,
        gs.regionManager,
        gs.decorRenderer,
        gs.buildingSystem,
        gs.toySystem,
        gs.dropManager,
        gs.enemyManager,
        gs.projectileManager,
        gs.particleSystem,
        gs.postProcess,
        gs.miniMap,
        gs.dialogueUI,
        gs.emotionSystem,
        gs.timeSystem,
        gs.princess.get(),
        gs.playerBodyId,
        gs.isDead,
        gs.screenWidth,
        gs.screenHeight,
        gs.charTime,
        gs.postProcessShader,
        PlayerInputQuery::getMouseWorldPoint(gs),
        PresentationModelBuilder::buildEnemyRenderResources(gs),
        PresentationModelBuilder::buildCharacterRenderResources(gs),
        PresentationModelBuilder::buildCharacterModel(gs),
        PresentationModelBuilder::buildProjectileRenderResources(gs),
        PresentationModelBuilder::buildAbilityEffectsModel(gs),
        PresentationModelBuilder::buildVentTearsModel(gs),
        PresentationModelBuilder::buildAimReticleModel(gs),
        PresentationModelBuilder::buildHudModel(gs)
    };
}

}  // namespace WorldPresentationBuilder
