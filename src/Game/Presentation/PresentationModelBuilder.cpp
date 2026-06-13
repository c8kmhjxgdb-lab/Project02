#include "Game/Presentation/PresentationModelBuilder.h"

#include "Game/Ability/ChildlikeSkillProfile.h"
#include "Game/GameState.h"
#include "Game/Inventory/ItemCatalog.h"
#include "Game/Services/PlayerInputQuery.h"
#include "Game/Services/SaveGameService.h"
#include "Game/Services/WorldQuery.h"

#include <algorithm>
#include <string>

namespace {

std::string questStateName(QuestState state) {
    switch (state) {
        case QuestState::Hidden: return "隐藏";
        case QuestState::Available: return "可接";
        case QuestState::Active: return "进行中";
        case QuestState::Completed: return "完成";
        case QuestState::Rewarded: return "已领奖";
    }
    return "?";
}

std::string progressText(const QuestObjectiveProgress& objective) {
    return objective.type + " " + objective.targetId + " " +
        std::to_string(objective.current) + "/" +
        std::to_string(objective.required);
}

}  // namespace

namespace PresentationModelBuilder {

MainMenuView::Model buildMainMenuModel(const GameState& gs) {
    SaveGameService::SaveMeta meta = SaveGameService::getSaveMeta("autosave");
    MainMenuView::Model model;
    model.selectedIndex = gs.ui.menuSelection;
    model.hasSave = SaveGameService::hasSave("autosave");
    model.animationTime = gs.charTime;
    model.message = gs.ui.menuMessage;
    model.messageTimer = gs.ui.menuMessageTimer;
    model.saveTimestamp = meta.timestamp;
    model.saveRegionName = meta.regionName;
    model.childlikeHeart = meta.childlikeHeart;
    model.rescuedPartners = meta.rescuedPartners;
    return model;
}

EntityView::CharacterRenderResources buildCharacterRenderResources(const GameState& gs) {
    EntityView::CharacterRenderResources resources;
    resources.shader = gs.characterShader;
    resources.uniformViewProj = gs.charUniformViewProj;
    resources.uniformPosition = gs.charUniformPosition;
    resources.uniformTime = gs.charUniformTime;
    resources.uniformBodyColor = gs.charUniformBodyColor;
    resources.uniformExpression = gs.charUniformExpression;
    resources.uniformArmAngle = gs.charUniformArmAngle;
    resources.vao = gs.charVAO;
    resources.vbo = gs.charVBO;
    return resources;
}

EntityView::CharacterModel buildCharacterModel(const GameState& gs) {
    EntityView::CharacterModel model;
    model.visible = !gs.isDead;
    if (!model.visible) return model;

    model.position = PlayerInputQuery::getPlayerPosition(gs);
    model.flightHeight = gs.flightHeight;
    model.shadowScale = gs.shadowScale;
    model.animationTime = gs.charTime;
    model.expression = gs.charExpression;
    model.armAngle = gs.armAngle;
    return model;
}

EntityView::ProjectileRenderResources buildProjectileRenderResources(const GameState& gs) {
    EntityView::ProjectileRenderResources resources;
    resources.shader = gs.projectileShader;
    resources.uniformViewProj = gs.projUniformViewProj;
    resources.uniformPosition = gs.projUniformPosition;
    resources.uniformTime = gs.projUniformTime;
    resources.uniformColor = gs.projUniformColor;
    resources.uniformRadius = gs.projUniformRadius;
    resources.uniformType = gs.projUniformType;
    resources.uniformDirection = gs.projUniformDirection;
    resources.quadVao = gs.reusableQuadVAO;
    resources.quadVbo = gs.reusableQuadVBO;
    return resources;
}

EntityView::EnemyRenderResources buildEnemyRenderResources(const GameState& gs) {
    EntityView::EnemyRenderResources resources;
    resources.shader = gs.enemyShader;
    resources.uniformViewProj = gs.enemyUniformViewProj;
    resources.uniformPosition = gs.enemyUniformPosition;
    resources.uniformTime = gs.enemyUniformTime;
    resources.uniformColor = gs.enemyUniformColor;
    resources.uniformRadius = gs.enemyUniformRadius;
    resources.uniformType = gs.enemyUniformType;
    resources.uniformHealthPercent = gs.enemyUniformHealthPercent;
    resources.quadVao = gs.reusableQuadVAO;
    resources.quadVbo = gs.reusableQuadVBO;
    return resources;
}

HudView::Model buildHudModel(const GameState& gs) {
    HudView::Model model;
    model.healthPercent = gs.playerHealth.getHealthPercent();
    model.manaPercent = gs.playerMaxMana > 0.0f ? gs.playerMana / gs.playerMaxMana : 0.0f;
    model.flightPercent = gs.flightMaxHeight > 0.0f ? gs.flightHeight / gs.flightMaxHeight : 0.0f;
    model.ultimatePercent = gs.princess ? gs.princess->ultimateCharge / 100.0f : 0.0f;
    model.shieldActivePercent = (gs.shield.isActive() && gs.shield.getDuration() > 0.0f)
        ? gs.shield.getRemainingTime() / gs.shield.getDuration()
        : 0.0f;
    model.coins = gs.inventory.getCoins();

    model.fireReady = gs.fireballCooldownMax > 0.0f
        ? 1.0f - gs.fireballCooldown / gs.fireballCooldownMax
        : 1.0f;
    model.shieldReady = gs.shieldCooldownMax > 0.0f
        ? 1.0f - gs.shieldCooldown / gs.shieldCooldownMax
        : 1.0f;
    model.lightningReady = 1.0f - gs.lightning.getCooldown() / 3.0f;
    model.bondReady = 1.0f - gs.bondTechnique.getCooldown() / 30.0f;
    model.flightReady = gs.flightCooldownMax > 0.0f
        ? 1.0f - gs.flightCooldown / gs.flightCooldownMax
        : 1.0f;

    model.canUseFireball = gs.playerMana >= 0.0f;
    model.canUseIceSpike = gs.playerMana >= 0.0f;
    model.canUseShield = gs.playerMana >= 15.0f;
    model.canUseLightning = gs.playerMana >= gs.lightning.getManaCost();
    model.canUseBond = gs.princess && gs.princess->isFollowing() && gs.princess->isUltimateReady();
    model.canUseFlight = gs.playerMana >= 5.0f;

    const EmotionState& emotion = gs.emotionSystem.getState();
    const SkillTierProfile profile = ChildlikeSkillProfile::forTier(gs.emotionSystem.getChildlikeHeartTier());
    model.childlikeTierName = gs.emotionSystem.getChildlikeHeartTierName();
    model.fireSkillName = profile.fireName;
    model.iceSkillName = profile.iceName;
    model.lightningSkillName = profile.lightningName;
    model.shieldSkillName = profile.shieldName;
    model.movementSkillName = profile.movementName;
    model.trackedQuestText = gs.questSystem.getTrackedQuestText();
    if (model.trackedQuestText.empty()) {
        if (!gs.storyProgress.getFlag("star_candy_collected")) {
            model.trackedQuestText = "主线: 调查小卖部门口的星星糖";
        } else if (!gs.storyProgress.getFlag("alya_following")) {
            model.trackedQuestText = "主线: 和艾莉娅对话";
        } else if (!WorldQuery::isCurrentRegion(gs.regionManager, "home_base") &&
                   gs.storyProgress.getChapterState("chapter_1_popup_arcade") == ChapterState::Unlocked) {
            model.trackedQuestText = "主线: 从南侧裂缝回秘密基地";
        } else if (WorldQuery::isCurrentRegion(gs.regionManager, "home_base") &&
                   gs.storyProgress.getChapterState("chapter_1_popup_arcade") == ChapterState::Unlocked) {
            model.trackedQuestText = "主线: 从右侧入口进入弹窗游乐厅";
        }
    }

    TimeSnapshot timeSnapshot = gs.timeSystem.getSnapshot();
    const MapRegion* region = gs.regionManager.getCurrentRegion();
    std::string status =
        "童心 Heart: " + std::to_string(static_cast<int>(emotion.childlikeHeart)) + "/1000\n" +
        "委屈 Grief: " + std::to_string(static_cast<int>(emotion.grievance)) +
        "  Day " + std::to_string(timeSnapshot.day) + " " + gs.timeSystem.getTimeString() +
        " " + TimeSystem::getPeriodName(timeSnapshot.period) + "\n" +
        "天气 Weather: " + WeatherSystem::getWeatherName(gs.weatherSystem.getCurrentWeather()) +
        (gs.weatherSystem.isIndoorContext() ? " (室内)" : "") + "\n" +
        "区域 Region: " + (region ? region->getName() : std::string("-"));
    if (gs.buildingSystem.isActive()) {
        const FurnitureDef* selected = gs.buildingSystem.getSelectedDef();
        status += "\n建造 Build: ";
        status += gs.buildingSystem.isMovingFurniture()
            ? "Moving"
            : (selected ? selected->name : "-");
        status += "  Comfort " + std::to_string(gs.buildingSystem.getComfort());
        if (selected && !gs.buildingSystem.isMovingFurniture()) {
            status += "\n库存 Stock: " + std::to_string(gs.inventory.getFurnitureCount(selected->id)) +
                "  Price " + std::to_string(selected->price);
            if (!gs.inventory.isFurnitureUnlocked(selected->id)) {
                status += "  Locked";
            }
        } else if (gs.buildingSystem.isMovingFurniture()) {
            status += "\n左键确认 / 右键取消";
        }
    }
    if (region && region->getId() == "home_base") {
        bool hasToyShelf = WorldQuery::hasPlacedFurniture(gs.buildingSystem, "toy_shelf");
        status += "\n基地 Base: C" + std::to_string(gs.buildingSystem.getComfort()) +
            " N" + std::to_string(gs.buildingSystem.getNostalgiaScore()) +
            " L" + std::to_string(gs.buildingSystem.getLightScore());
        status += "\n玩具 Toy: ";
        status += gs.toySystem.isMiniCarActive()
            ? "模型车进行中"
            : (hasToyShelf ? "T 模型车" : "需要玩具架");
        status += gs.toySystem.hasClaimedMiniCarRewardToday(gs.timeSystem.getDay())
            ? " 今日已领奖"
            : " 今日可领奖";
        if (gs.toySystem.getMiniCarBestTime() > 0.0f) {
            status += " Best " + std::to_string(static_cast<int>(gs.toySystem.getMiniCarBestTime())) + "s";
        }
        status += "\n任务 Quests: " + std::to_string(gs.questSystem.getRewardedCount()) +
            "/" + std::to_string(gs.questSystem.getQuestCount());
    }
    model.statusText = status;
    model.noticeText = gs.ui.stage7Notice;
    model.noticeTimer = gs.ui.stage7NoticeTimer;
    return model;
}

GameMenuView::Model buildGameMenuModel(const GameState& gs) {
    GameMenuView::Model model;
    model.open = gs.ui.gameMenuOpen;
    model.page = gs.ui.gameMenuPage;

    for (const QuestSaveEntry& entry : gs.questSystem.getSaveData()) {
        model.questLines.push_back(entry.id + " [" + questStateName(entry.state) + "]");
        for (const QuestObjectiveProgress& objective : entry.objectives) {
            model.questLines.push_back("  - " + progressText(objective));
        }
    }
    if (model.questLines.empty()) {
        model.questLines.push_back("暂无任务");
    }

    const EmotionState& emotion = gs.emotionSystem.getState();
    const SkillTierProfile profile = ChildlikeSkillProfile::forTier(gs.emotionSystem.getChildlikeHeartTier());
    model.characterLines.push_back("生命: " + std::to_string(static_cast<int>(gs.playerHealth.getCurrentHealth())) +
        "/" + std::to_string(static_cast<int>(gs.playerHealth.getMaxHealth())));
    model.characterLines.push_back("魔力: " + std::to_string(static_cast<int>(gs.playerMana)) +
        "/" + std::to_string(static_cast<int>(gs.playerMaxMana)));
    model.characterLines.push_back("童心: " + std::to_string(static_cast<int>(emotion.childlikeHeart)) +
        " / 档位 " + gs.emotionSystem.getChildlikeHeartTierName());
    model.characterLines.push_back("委屈: " + std::to_string(static_cast<int>(emotion.grievance)));
    model.characterLines.push_back("移动技能: " + profile.movementName +
        " 距离 " + std::to_string(static_cast<int>(profile.movementDistance)));

    std::vector<ItemStack> stacks = gs.inventory.getItemStacks();
    for (const ItemStack& stack : stacks) {
        const ItemDef* def = ItemCatalog::find(stack.itemId);
        std::string name = def ? def->name : stack.itemId;
        model.inventoryLines.push_back(name + " x" + std::to_string(stack.count));
    }
    if (model.inventoryLines.empty()) {
        model.inventoryLines.push_back("背包空空");
    }

    model.partnerLines.push_back(gs.princess && gs.princess->isFollowing()
        ? "艾莉娅: 跟随中 / 星形水晶已点亮"
        : "艾莉娅: 等待序章对话");
    model.partnerLines.push_back(gs.storyProgress.isPartnerUnlocked("tieyi")
        ? "铁翼: 已入队 / 火箭破盾支援"
        : "铁翼: 未救出");

    model.systemLines.push_back("F5 保存");
    model.systemLines.push_back("F9 读取");
    model.systemLines.push_back("Esc / Tab 返回");
    model.systemLines.push_back("金币: " + std::to_string(gs.inventory.getCoins()));
    if (gs.storyProgress.getFlag("alya_following") &&
        gs.storyProgress.getChapterState("chapter_1_popup_arcade") != ChapterState::Completed) {
        model.systemLines.push_back("下一步: 回基地地图桌，进入弹窗游乐厅");
    }

    return model;
}

AimReticleView::Model buildAimReticleModel(const GameState& gs) {
    AimReticleView::Model model;
    model.visible = !gs.isDead;
    if (!model.visible) return model;
    glm::vec2 playerPos = PlayerInputQuery::getPlayerPosition(gs);
    model.playerPosition = playerPos;
    model.targetPosition = PlayerInputQuery::getMouseWorldPoint(gs);
    model.aimDirection = PlayerInputQuery::getAimDirection(gs, playerPos);
    return model;
}

ParticleView::VentTearsModel buildVentTearsModel(const GameState& gs) {
    ParticleView::VentTearsModel model;
    model.visible = gs.isVenting;
    if (!model.visible) return model;
    model.position = gs.ventAnimation.getPosition();
    model.shakeX = gs.ventAnimation.getShakeAmount();
    model.animationTime = gs.charTime;
    return model;
}

AbilityView::WorldEffectsModel buildAbilityEffectsModel(const GameState& gs) {
    AbilityView::WorldEffectsModel model;
    model.playerPosition = PlayerInputQuery::getPlayerPosition(gs);
    model.shield = &gs.shield;
    model.lightning = &gs.lightning;
    model.bondTechnique = &gs.bondTechnique;
    model.isFlying = gs.isFlying;
    model.flightHeight = gs.flightHeight;
    model.flightMaxHeight = gs.flightMaxHeight;
    model.animationTime = gs.charTime;
    return model;
}

}  // namespace PresentationModelBuilder
