#pragma once

#include "Game/Presentation/AbilityView.h"
#include "Game/Presentation/AimReticleView.h"
#include "Game/Presentation/EntityView.h"
#include "Game/Presentation/GameMenuView.h"
#include "Game/Presentation/HudView.h"
#include "Game/Presentation/MainMenuView.h"
#include "Game/Presentation/ParticleView.h"

struct GameState;

namespace PresentationModelBuilder {

MainMenuView::Model buildMainMenuModel(const GameState& gs);

EntityView::CharacterRenderResources buildCharacterRenderResources(const GameState& gs);

EntityView::CharacterModel buildCharacterModel(const GameState& gs);

EntityView::ProjectileRenderResources buildProjectileRenderResources(const GameState& gs);

EntityView::EnemyRenderResources buildEnemyRenderResources(const GameState& gs);

HudView::Model buildHudModel(const GameState& gs);

GameMenuView::Model buildGameMenuModel(const GameState& gs);

AimReticleView::Model buildAimReticleModel(const GameState& gs);

ParticleView::VentTearsModel buildVentTearsModel(const GameState& gs);

AbilityView::WorldEffectsModel buildAbilityEffectsModel(const GameState& gs);

}  // namespace PresentationModelBuilder
