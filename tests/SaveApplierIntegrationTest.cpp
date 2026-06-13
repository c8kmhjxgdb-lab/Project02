#include "Engine/Renderer/ParticleSystem.h"
#include "Game/AI/Enemy.h"
#include "Game/Ability/Projectile.h"
#include "Game/Data/SaveData.h"
#include "Game/GameState.h"
#include "Game/Controllers/InteractionInputController.h"
#include "Game/Services/DropCollectionService.h"
#include "Game/Services/GameSessionService.h"
#include "Game/Services/ProgressionUpdateService.h"
#include "Game/Services/SaveGameService.h"
#include "Game/Services/WorldCombatUpdateService.h"
#include "Game/Services/WorldUpdateService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/Social/Princess.h"
#include "Game/World/WeatherTypes.h"
#include "TestSupport.h"

#include <SDL2/SDL_scancode.h>
#include <box2d/box2d.h>
#include <glm/geometric.hpp>

#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace {

bool near(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

bool nearVec(const glm::vec2& a, const glm::vec2& b, float epsilon = 0.001f) {
    return glm::length(a - b) < epsilon;
}

bool contains(const std::vector<std::string>& values, const std::string& expected) {
    for (const std::string& value : values) {
        if (value == expected) return true;
    }
    return false;
}

bool containsText(const std::string& value, const std::string& expected) {
    return value.find(expected) != std::string::npos;
}

glm::vec2 poiWorldPosition(const RegionManager& regionManager, const char* poiId) {
    const MapRegion* region = regionManager.getCurrentRegion();
    TestSupport::require(region != nullptr, "current region exists");
    const PointOfInterest* poi = WorldQuery::findPOI(region, poiId);
    TestSupport::require(poi != nullptr, "poi exists");
    return region->getTileMap().tileToWorld(poi->tilePos.x, poi->tilePos.y);
}

glm::vec2 playerWorldPosition(b2BodyId playerBodyId) {
    b2Vec2 pos = b2Body_GetPosition(playerBodyId);
    return glm::vec2(pos.x, pos.y);
}

void movePlayerTo(GameState& gs, const glm::vec2& position) {
    b2Body_SetTransform(gs.playerBodyId, b2Vec2{position.x, position.y}, b2Rot{0});
    b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);
}

void interact(GameState& gs) {
    InteractionInputController::Context context = InteractionInputController::makeContext(gs);
    InteractionInputController::Callbacks callbacks = InteractionInputController::makeCallbacks(gs);
    InteractionInputController::handleInteract(context, callbacks);
}

int countItemDrops(const DropManager& dropManager, const std::string& itemId) {
    int count = 0;
    for (const Drop& drop : dropManager.getActive()) {
        if (drop.active && drop.type == DropType::Item && drop.itemId == itemId) {
            ++count;
        }
    }
    return count;
}

bool hasCollectionFlag(const DropManager& dropManager, const std::string& collectionFlag) {
    for (const Drop& drop : dropManager.getActive()) {
        if (drop.active && drop.collectionFlag == collectionFlag) {
            return true;
        }
    }
    return false;
}

struct Fixture {
    GameState gs{};

    Fixture() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, 0.0f};
        gs.worldId = b2CreateWorld(&worldDef);

        b2BodyDef groundDef = b2DefaultBodyDef();
        gs.groundBodyId = b2CreateBody(gs.worldId, &groundDef);

        b2BodyDef playerDef = b2DefaultBodyDef();
        playerDef.type = b2_dynamicBody;
        playerDef.position = b2Vec2{1.0f, 2.0f};
        gs.playerBodyId = b2CreateBody(gs.worldId, &playerDef);

        gs.regionManager.setWorldId(gs.worldId);
        gs.regionManager.setPlayerBody(gs.playerBodyId);
        gs.regionManager.setTransitionEffectEnabled(false);
        gs.regionManager.init();
        if (MapRegion* region = gs.regionManager.getCurrentRegion()) {
            region->buildPhysics(gs.worldId);
        }

        gs.buildingSystem.init(gs.worldId);
        gs.projectileManager.init();
        gs.enemyManager.init();
        gs.dropManager.init();
        gs.particleSystem.init();
        gs.timeSystem.init(9.0f);
        gs.weatherSystem.init(&gs.particleSystem, &gs.camera);
        gs.toySystem.init();
        gs.questSystem.init();
        gs.inventory.unlockFurnitureDefaults();

        gs.princess = std::make_unique<Princess>("Princess");
        gs.princess->createBody(gs.worldId, glm::vec2(4.0f, 5.0f));

        gs.homePosition = glm::vec2(0.0f, 0.0f);
        gs.homeRadius = 3.0f;
        gs.screenWidth = 1280;
        gs.screenHeight = 720;
        gs.isVenting = false;
        gs.gameTime = gs.timeSystem.getHour();
    }

    ~Fixture() {
        gs.princess.reset();
        gs.regionManager.shutdown();
        gs.buildingSystem.shutdown();
        gs.projectileManager.clear();
        gs.enemyManager.clear();
        gs.dropManager.clear();
        gs.particleSystem.clear();
        if (b2World_IsValid(gs.worldId)) {
            b2DestroyWorld(gs.worldId);
        }
    }
};

void makeTransientState(GameState& gs) {
    gs.input.setKey(SDL_SCANCODE_W, true);
    gs.input.setKey(SDL_SCANCODE_SPACE, true);
    gs.input.mouseLeft = true;
    gs.input.mousePos = glm::vec2(300.0f, 200.0f);

    gs.regionManager.transitionTo("home_base", glm::ivec2(9, 11), gs.worldId);
    gs.buildingSystem.setBuildMode(true);
    gs.toySystem.startMiniCar(gs.timeSystem.getDay());

    DialogueNode node;
    node.speaker = "Before";
    node.text = "Visible before load";
    gs.dialogueUI.begin(node);

    gs.isVenting = true;
    gs.ui.talkedWithPrincessAtBaseThisFrame = true;
    gs.ui.stage7Notice = "stale";
    gs.ui.stage7NoticeTimer = 99.0f;

    gs.isDead = true;
    gs.deathTimer = 4.0f;
    gs.isFlying = true;
    gs.flightHeight = 3.0f;
    gs.flightHeightTarget = 5.0f;
    gs.flightCooldown = 1.5f;
    gs.shieldCooldown = 2.0f;
    gs.fireballCooldown = 0.25f;
    gs.enemySpawnTimer = 4.0f;
    gs.score = 50;
    gs.enemiesKilled = 6;

    gs.projectileManager.fire(
        gs.worldId,
        glm::vec2(2.0f, 2.0f),
        glm::vec2(1.0f, 0.0f),
        ProjectileType::Fireball,
        25.0f,
        12.0f,
        gs.playerBodyId);
    gs.enemyManager.spawn(gs.worldId, glm::vec2(4.0f, 4.0f), EnemyType::Chaser);
    gs.dropManager.spawn(gs.worldId, glm::vec2(3.0f, 3.0f), DropType::Coin, 3);
    gs.particleSystem.emit(
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec3(1.0f, 0.2f, 0.1f),
        1.0f,
        0.1f);
}

SaveData makeSaveData() {
    SaveData data;
    data.player.regionId = "starter_village";
    data.player.position = glm::vec2(12.25f, 8.75f);
    data.player.health = 63.0f;
    data.player.maxHealth = 140.0f;
    data.player.mana = 37.0f;
    data.player.maxMana = 80.0f;
    data.player.coins = 123;
    data.player.progress.discoveredRegions = {"starter_village", "home_base"};
    data.player.progress.completedQuests = {"organize_home_base"};
    data.player.progress.totalPlayTime = 456.5f;

    data.environment.day = 7;
    data.environment.hour = 18.25f;
    data.environment.weather = "Rain";
    data.environment.weatherIntensity = 0.65f;

    data.childlikeHeart = 777.0f;
    data.grievance = 12.0f;
    data.joy = 66.0f;
    data.stress = 23.0f;

    FurnitureInstance bed;
    bed.instanceId = 10;
    bed.defId = "simple_bed";
    bed.tile = glm::ivec2(6, 6);
    bed.rotation = 0;
    data.homeFurniture.push_back(bed);

    data.furnitureStock.push_back({"simple_bed", 2});
    data.itemStacks.push_back({"trial_token", 3});
    data.unlockedFurniture.push_back("simple_bed");
    data.storyProgress.unlockedPartners.push_back("tieyi");
    data.toyData.collectedToys = {"mini_car"};
    data.toyData.miniCarLastRewardDay = 4;
    data.toyData.miniCarBestTime = 9.5f;

    data.princess.position = glm::vec2(8.5f, 6.25f);
    data.princess.affection = 42.0f;
    data.princess.following = true;
    data.princess.ultimateCharge = 70.0f;

    return data;
}

QuestDef makeChapterFactQuest() {
    QuestDef quest;
    quest.id = "chapter_fact_test";
    quest.name = "章节事实测试";
    quest.objectives.push_back({"collect", "trial_token", 3});
    quest.objectives.push_back({"interact", "tieyi_cage", 1});
    quest.objectives.push_back({"clear_boss", "popup_crown", 1});
    quest.reward.questId = quest.id;
    quest.reward.completed = true;
    quest.reward.storyFlag = "chapter_fact_rewarded";
    quest.reward.maxChildlikeHeart = 10.0f;
    return quest;
}

void itemDropsCarryItemMetadata() {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{0.0f, 0.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    DropManager dropManager;
    dropManager.init();

    DropId dropId = dropManager.spawnItem(
        worldId,
        glm::vec2(1.0f, 2.0f),
        "trial_token",
        3,
        "collected_trial_token_1");

    const auto& drops = dropManager.getActive();
    TestSupport::require(dropId != DROP_NULL, "item drop id is valid");
    TestSupport::require(drops.size() == 1, "one item drop is created");
    TestSupport::require(drops.front().type == DropType::Item, "item drop uses item type");
    TestSupport::require(drops.front().itemId == "trial_token", "item drop stores item id");
    TestSupport::require(drops.front().collectionFlag == "collected_trial_token_1", "item drop stores collection flag");
    TestSupport::require(drops.front().value == 3, "item drop stores count");

    dropManager.clear();
    b2DestroyWorld(worldId);
}

void popupArcadeSpawnsChapterPickupsWithoutDuplicates() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.regionManager.transitionTo("popup_arcade", glm::ivec2(30, 53), gs.worldId);
    gs.emotionSystem.setChildlikeHeart(650.0f);

    ProgressionUpdateService::State state;
    ProgressionUpdateService::Context context = ProgressionUpdateService::makeContext(gs);
    ProgressionUpdateService::update(context, 0.1f, glm::vec2(30.0f, 56.0f), state);

    TestSupport::require(countItemDrops(gs.dropManager, "trial_token") == 3, "three trial tokens spawn");
    TestSupport::require(countItemDrops(gs.dropManager, "recovery_candy") == 4, "four recovery candies spawn");
    TestSupport::require(countItemDrops(gs.dropManager, "color_battery") == 2, "two color batteries spawn");
    TestSupport::require(countItemDrops(gs.dropManager, "half_melody_arcade") == 1, "hidden melody spawns for vivid heart");
    TestSupport::require(hasCollectionFlag(gs.dropManager, "collected_trial_token_1"), "trial token 1 has collection flag");

    ProgressionUpdateService::update(context, 0.1f, glm::vec2(30.0f, 56.0f), state);
    TestSupport::require(countItemDrops(gs.dropManager, "trial_token") == 3, "trial tokens do not duplicate");

    gs.storyProgress.setFlag("collected_trial_token_1", true);
    gs.dropManager.clear();
    ProgressionUpdateService::update(context, 0.1f, glm::vec2(30.0f, 56.0f), state);

    TestSupport::require(countItemDrops(gs.dropManager, "trial_token") == 2, "collected token does not respawn");
    TestSupport::require(!hasCollectionFlag(gs.dropManager, "collected_trial_token_1"), "collected token flag stays absent");
}

void progressionFactsDriveChapterQuestReward() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.regionManager.transitionTo("popup_arcade", glm::ivec2(30, 53), gs.worldId);
    gs.inventory.addItem("trial_token", 3);
    gs.storyProgress.unlockPartner("tieyi");
    gs.storyProgress.setFlag("popup_crown_defeated", true);
    gs.questSystem.initWithDefinitions({makeChapterFactQuest()});

    ProgressionUpdateService::State state;
    ProgressionUpdateService::Context context = ProgressionUpdateService::makeContext(gs);
    ProgressionUpdateService::update(context, 0.1f, glm::vec2(30.0f, 56.0f), state);

    TestSupport::require(gs.questSystem.isCompleted("chapter_fact_test"), "chapter quest completes from facts");
    TestSupport::require(gs.storyProgress.getFlag("chapter_fact_rewarded"), "quest reward story flag applies");
    TestSupport::require(
        gs.storyProgress.getFlag("max_childlike_bonus_chapter_fact_test"),
        "max childlike reward marker applies");
}

void popupCrownBossDefeatAppliesRewardsOnce() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.regionManager.transitionTo("popup_arcade", glm::ivec2(30, 50), gs.worldId);
    gs.inventory.setCoins(10);
    gs.emotionSystem.setChildlikeHeart(650.0f);
    gs.storyProgress.startChapter("chapter_1_popup_arcade");
    gs.popupCrownBoss.start();
    gs.popupCrownBoss.applyDamage(1000.0f);

    ProgressionUpdateService::State state;
    ProgressionUpdateService::Context context = ProgressionUpdateService::makeContext(gs);
    ProgressionUpdateService::update(context, 0.1f, glm::vec2(30.0f, 50.0f), state);

    TestSupport::require(gs.inventory.getCoins() == 70, "boss reward adds coins once");
    TestSupport::require(near(gs.emotionSystem.getState().childlikeHeart, 770.0f), "boss reward adds childlike heart");
    TestSupport::require(gs.inventory.getItemCount("pixel_controller") == 1, "boss reward grants pixel controller");
    TestSupport::require(gs.inventory.getItemCount("pixel_screw") == 4, "boss reward grants pixel screws");
    TestSupport::require(gs.inventory.getItemCount("old_button") == 2, "boss reward grants old buttons");
    TestSupport::require(gs.inventory.getItemCount("no_pay_victory_sticker") == 1, "boss reward grants hidden sticker");
    TestSupport::require(gs.storyProgress.getFlag("popup_crown_defeated"), "boss defeat story flag applies");
    TestSupport::require(gs.storyProgress.getFlag("popup_crown_rewarded"), "boss reward flag applies");
    TestSupport::require(gs.storyProgress.isPartnerUnlocked("tieyi"), "boss reward unlocks Tieyi");
    TestSupport::require(
        gs.storyProgress.getChapterState("chapter_1_popup_arcade") == ChapterState::Completed,
        "boss reward completes chapter");

    ProgressionUpdateService::update(context, 0.1f, glm::vec2(30.0f, 50.0f), state);

    TestSupport::require(gs.inventory.getCoins() == 70, "boss reward does not duplicate coins");
    TestSupport::require(gs.inventory.getItemCount("pixel_controller") == 1, "boss reward does not duplicate relic");
    TestSupport::require(gs.inventory.getItemCount("no_pay_victory_sticker") == 1, "boss reward does not duplicate sticker");
    TestSupport::require(near(gs.emotionSystem.getState().childlikeHeart, 770.0f), "boss reward does not duplicate childlike heart");
}

void prologueStarCandyInteractionStartsChapterAndAwardsCoin() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.regionManager.transitionTo("real_street_prologue", glm::ivec2(8, 12), gs.worldId);
    movePlayerTo(gs, poiWorldPosition(gs.regionManager, "star_candy"));

    interact(gs);

    TestSupport::require(gs.storyProgress.getFlag("star_candy_collected"), "star candy sets collected flag");
    TestSupport::require(gs.inventory.getItemCount("old_game_coin") == 1, "star candy grants old game coin");
    TestSupport::require(
        gs.storyProgress.getChapterState("prologue_star_candy") == ChapterState::InProgress,
        "star candy starts prologue chapter");

    interact(gs);

    TestSupport::require(gs.inventory.getItemCount("old_game_coin") == 1, "star candy reward does not duplicate");
}

void prologueStarCandyQuestCompletesWithoutDuplicatingCoinReward() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.luaVM.init();
    TestSupport::require(
        gs.questSystem.loadDefinitions(gs.luaVM, "assets/scripts/quests.lua"),
        "quest definitions load for star candy regression");

    gs.regionManager.transitionTo("real_street_prologue", glm::ivec2(8, 12), gs.worldId);
    movePlayerTo(gs, poiWorldPosition(gs.regionManager, "star_candy"));

    interact(gs);

    ProgressionUpdateService::State state;
    ProgressionUpdateService::Context context = ProgressionUpdateService::makeContext(gs);
    ProgressionUpdateService::update(context, 0.1f, playerWorldPosition(gs.playerBodyId), state);

    TestSupport::require(gs.questSystem.isCompleted("prologue_star_candy"), "star candy quest completes from progression facts");
    TestSupport::require(gs.inventory.getItemCount("old_game_coin") == 1, "star candy quest does not duplicate old game coin");
}

void tieyiRescueRequiresEliteDefeatAndRewardsBattery() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.regionManager.transitionTo("popup_arcade", glm::ivec2(30, 53), gs.worldId);
    movePlayerTo(gs, poiWorldPosition(gs.regionManager, "tieyi_cage"));

    interact(gs);

    TestSupport::require(!gs.storyProgress.isPartnerUnlocked("tieyi"), "Tieyi stays locked before elite defeat");
    TestSupport::require(gs.inventory.getItemCount("color_battery") == 0, "rescue does not award battery before elite defeat");

    gs.storyProgress.setFlag("scrap_elite_defeated", true);
    interact(gs);

    TestSupport::require(gs.storyProgress.isPartnerUnlocked("tieyi"), "Tieyi unlocks after rescue");
    TestSupport::require(gs.storyProgress.getFlag("tieyi_rescued"), "rescue sets Tieyi flag");
    TestSupport::require(gs.inventory.getItemCount("color_battery") == 1, "rescue grants color battery");
    TestSupport::require(containsText(gs.ui.stage7Notice, "铁翼恢复了火箭核心"), "rescue shows Tieyi notice");

    interact(gs);

    TestSupport::require(gs.inventory.getItemCount("color_battery") == 1, "rescue reward does not duplicate");
}

void scrapEliteDeathMarksTieyiRescuePrerequisite() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    EnemyId eliteId = gs.enemyManager.spawnByDefinition(
        gs.worldId,
        glm::vec2(8.0f, 8.0f),
        "scrap_soldier_elite");
    gs.enemyManager.damage(eliteId, 1000.0f);

    WorldCombatUpdateService::Context context = WorldCombatUpdateService::makeContext(gs);
    WorldCombatUpdateService::updateAlive(context, 0.1f, playerWorldPosition(gs.playerBodyId));

    TestSupport::require(gs.storyProgress.getFlag("scrap_elite_defeated"), "scrap elite death sets rescue prerequisite flag");
}

void arcadeBossDoorRequiresTieyiAndTokensThenStartsBoss() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.regionManager.transitionTo("popup_arcade", glm::ivec2(30, 53), gs.worldId);
    movePlayerTo(gs, poiWorldPosition(gs.regionManager, "arcade_boss_door"));

    interact(gs);
    TestSupport::require(!gs.popupCrownBoss.isActive(), "boss door stays closed without requirements");
    TestSupport::require(
        gs.storyProgress.getChapterState("chapter_1_popup_arcade") != ChapterState::InProgress,
        "boss door does not start chapter without requirements");

    gs.inventory.addItem("trial_token", 3);
    interact(gs);
    TestSupport::require(!gs.popupCrownBoss.isActive(), "boss door stays closed before Tieyi rescue");

    gs.storyProgress.setFlag("tieyi_rescued", true);
    interact(gs);

    TestSupport::require(gs.popupCrownBoss.isActive(), "boss door starts popup crown boss");
    TestSupport::require(
        gs.popupCrownBoss.getPhase() == PopupCrownPhase::WelcomePopup,
        "boss starts in welcome popup phase");
    TestSupport::require(
        gs.storyProgress.getChapterState("chapter_1_popup_arcade") == ChapterState::InProgress,
        "boss door starts chapter progress");
    TestSupport::require(
        nearVec(playerWorldPosition(gs.playerBodyId), poiWorldPosition(gs.regionManager, "popup_crown_arena")),
        "boss door teleports player to arena");
}

void worldUpdateAutomaticArcadePortalEntersPopupArcade() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.regionManager.transitionTo("home_base", glm::ivec2(9, 11), gs.worldId);
    gs.regionManager.update(1.0f);
    gs.regionManager.setTransitionEffectEnabled(true);
    gs.regionManager.setTransitionDuration(0.2f);

    MapRegion* baseRegion = gs.regionManager.getCurrentRegion();
    TestSupport::require(baseRegion != nullptr, "home base exists before world update portal");
    movePlayerTo(gs, baseRegion->getTileMap().tileToWorld(20, 9));

    WorldUpdateService::State worldState;
    for (int i = 0; i < 12; ++i) {
        WorldUpdateService::update(gs, 0.1f, worldState);
    }

    TestSupport::require(
        WorldQuery::isCurrentRegion(gs.regionManager, "popup_arcade"),
        "world update automatic arcade portal enters popup arcade");
    TestSupport::require(
        !gs.regionManager.isTransitioning(),
        "world update automatic arcade portal transition completes");
}

void pixelControllerSpotConsumesRelicAndRewardsBaseSupplies() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.regionManager.transitionTo("home_base", glm::ivec2(12, 15), gs.worldId);
    movePlayerTo(gs, poiWorldPosition(gs.regionManager, "pixel_controller_spot"));
    gs.inventory.addItem("pixel_controller", 1);

    interact(gs);

    TestSupport::require(gs.inventory.getItemCount("pixel_controller") == 0, "controller spot consumes relic");
    TestSupport::require(gs.storyProgress.getFlag("pixel_controller_placed"), "controller spot sets placed flag");
    TestSupport::require(gs.inventory.getItemCount("recovery_candy") == 2, "controller spot grants recovery candy");
    TestSupport::require(containsText(gs.ui.stage7Notice, "试玩币机关已点亮"), "controller spot shows activation notice");

    interact(gs);

    TestSupport::require(gs.inventory.getItemCount("recovery_candy") == 2, "controller spot reward does not duplicate");
}

void firstPrincessTalkAfterStarCandyUnlocksFollowingAndCompletesPrologue() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.luaVM.init();
    gs.dialogueTree.setLuaVM(&gs.luaVM);
    gs.dialogueTree.loadFromLua("first_meeting");
    gs.dialogueTree.setOnNodeEnter([&gs](const DialogueNode& node) {
        gs.dialogueUI.begin(node);
    });
    gs.dialogueTree.setOnDialogueEnd([&gs](const DialogueNode&) {
        gs.dialogueUI.hide();
    });

    gs.regionManager.transitionTo("real_street_prologue", glm::ivec2(8, 12), gs.worldId);
    gs.storyProgress.setFlag("star_candy_collected", true);
    gs.storyProgress.startChapter("prologue_star_candy");
    movePlayerTo(gs, gs.princess->getPosition());

    interact(gs);

    TestSupport::require(gs.princess->isFollowing(), "first prologue princess talk enables following");
    TestSupport::require(gs.storyProgress.getFlag("alya_following"), "first prologue princess talk sets Alya following flag");
    TestSupport::require(
        gs.storyProgress.getChapterState("prologue_star_candy") == ChapterState::Completed,
        "first prologue princess talk completes prologue chapter");
    TestSupport::require(
        containsText(gs.ui.stage7Notice, "艾莉娅加入") &&
            containsText(gs.ui.stage7Notice, "秘密基地"),
        "first prologue princess talk shows the next-step notice");
}

void startNewGameBeginsInPrologueWithInitialStoryState() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    SaveData defaults;
    TestSupport::require(defaults.player.regionId == "real_street_prologue", "default save region is prologue");

    gs.luaVM.init();
    GameSessionService::startNewGame(gs);

    const MapRegion* region = gs.regionManager.getCurrentRegion();
    TestSupport::require(region != nullptr, "new game region exists");
    TestSupport::require(region->getId() == "real_street_prologue", "new game starts in prologue");
    TestSupport::require(
        gs.storyProgress.getChapterState("prologue_star_candy") == ChapterState::Unlocked,
        "prologue chapter starts unlocked");
    TestSupport::require(
        gs.storyProgress.getChapterState("chapter_1_popup_arcade") == ChapterState::Unlocked,
        "chapter one starts unlocked");
}

void startNewGameCanEnterPopupArcadeThroughBasePortal() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    gs.luaVM.init();
    GameSessionService::startNewGame(gs);
    WorldUpdateService::State worldState;
    constexpr float dt = 1.0f / 60.0f;

    MapRegion* prologue = gs.regionManager.getCurrentRegion();
    TestSupport::require(prologue != nullptr, "prologue exists before full arcade route");
    movePlayerTo(gs, prologue->getTileMap().tileToWorld(18, 26));
    for (int frame = 0; frame < 120; ++frame) {
        WorldUpdateService::update(gs, dt, worldState);
    }

    TestSupport::require(
        WorldQuery::isCurrentRegion(gs.regionManager, "home_base"),
        "full route enters home base from prologue crack");
    TestSupport::require(!gs.regionManager.isTransitioning(),
                         "full route home base transition completes");

    MapRegion* base = gs.regionManager.getCurrentRegion();
    TestSupport::require(base != nullptr, "home base exists before full arcade route portal");
    movePlayerTo(gs, base->getTileMap().tileToWorld(20, 9));
    for (int frame = 0; frame < 120; ++frame) {
        WorldUpdateService::update(gs, dt, worldState);
    }

    TestSupport::require(
        WorldQuery::isCurrentRegion(gs.regionManager, "popup_arcade"),
        "full route enters popup arcade from home base portal");
    TestSupport::require(!gs.regionManager.isTransitioning(),
                         "full route popup arcade transition completes");
}

void collectedItemDropUpdatesInventoryAndStoryFlags() {
    Fixture fixture;
    GameState& gs = fixture.gs;

    Drop drop;
    drop.type = DropType::Item;
    drop.value = 2;
    drop.itemId = "color_battery";
    drop.collectionFlag = "collected_color_battery_1";

    DropCollectionService::Context context = DropCollectionService::makeContext(gs);
    DropCollectionService::collect(context, drop);

    TestSupport::require(gs.inventory.getItemCount("color_battery") == 2, "item drop adds inventory item");
    TestSupport::require(
        gs.storyProgress.getFlag("collected_color_battery"),
        "item drop sets generic collected item flag");
    TestSupport::require(
        gs.storyProgress.getFlag("collected_color_battery_1"),
        "item drop sets per-pickup collection flag");
}

void applySaveDataRestoresPersistentStateAndClearsTransientState() {
    Fixture fixture;
    GameState& gs = fixture.gs;
    makeTransientState(gs);

    TestSupport::require(gs.projectileManager.getActive().size() == 1, "test starts with a projectile");
    TestSupport::require(gs.enemyManager.getActive().size() == 1, "test starts with an enemy");
    TestSupport::require(gs.dropManager.getActive().size() == 1, "test starts with a drop");
    TestSupport::require(gs.particleSystem.getActiveCount() == 1, "test starts with a particle");
    TestSupport::require(gs.buildingSystem.isActive(), "test starts with build mode enabled");
    TestSupport::require(gs.toySystem.isMiniCarActive(), "test starts with mini car active");
    TestSupport::require(gs.dialogueUI.isVisible(), "test starts with dialogue UI visible");

    SaveData data = makeSaveData();
    bool applied = SaveGameService::applySaveData(gs, data);

    TestSupport::require(applied, "applySaveData succeeds");

    b2Vec2 bodyPos = b2Body_GetPosition(gs.playerBodyId);
    glm::vec2 restoredPos(bodyPos.x, bodyPos.y);
    TestSupport::require(nearVec(restoredPos, data.player.position), "player body moves to saved position");
    TestSupport::require(nearVec(gs.camera.position, data.player.position), "camera moves to saved position");
    TestSupport::require(nearVec(gs.spawnPoint, data.player.position), "spawn point moves to saved position");
    TestSupport::require(near(b2Body_GetLinearVelocity(gs.playerBodyId).x, 0.0f), "player x velocity clears");
    TestSupport::require(near(b2Body_GetLinearVelocity(gs.playerBodyId).y, 0.0f), "player y velocity clears");

    TestSupport::require(near(gs.playerHealth.getCurrentHealth(), data.player.health), "health restores");
    TestSupport::require(near(gs.playerHealth.getMaxHealth(), data.player.maxHealth), "max health restores");
    TestSupport::require(near(gs.playerMana, data.player.mana), "mana restores");
    TestSupport::require(near(gs.playerMaxMana, data.player.maxMana), "max mana restores");
    TestSupport::require(gs.inventory.getCoins() == data.player.coins, "coins restore");
    TestSupport::require(gs.inventory.getFurnitureCount("simple_bed") == 2, "furniture stock restores");
    TestSupport::require(gs.inventory.getItemCount("trial_token") == 3, "trial tokens restore");
    TestSupport::require(gs.inventory.isFurnitureUnlocked("simple_bed"), "unlocked furniture restores");
    TestSupport::require(gs.storyProgress.isPartnerUnlocked("tieyi"), "Tieyi restore");

    TestSupport::require(gs.timeSystem.getDay() == data.environment.day, "day restores");
    TestSupport::require(near(gs.timeSystem.getHour(), data.environment.hour), "hour restores");
    TestSupport::require(near(gs.gameTime, data.environment.hour), "gameTime mirrors restored hour");
    TestSupport::require(gs.weatherSystem.getCurrentWeather() == WeatherType::Rain, "weather restores");
    TestSupport::require(near(gs.weatherSystem.getIntensity(), data.environment.weatherIntensity), "weather intensity restores");

    const EmotionState& emotion = gs.emotionSystem.getState();
    TestSupport::require(near(emotion.childlikeHeart, data.childlikeHeart), "childlike heart restores");
    TestSupport::require(near(emotion.grievance, data.grievance), "grievance restores");
    TestSupport::require(near(emotion.joy, data.joy), "joy restores");
    TestSupport::require(near(emotion.stress, data.stress), "stress restores");
    TestSupport::require(near(gs.totalPlayTimeSeconds, data.player.progress.totalPlayTime), "total play time restores");

    TestSupport::require(gs.regionManager.getCurrentRegion() != nullptr, "current region exists after load");
    TestSupport::require(gs.regionManager.getCurrentRegion()->getId() == data.player.regionId, "current region restores");
    TestSupport::require(gs.regionManager.hasRegion("home_base"), "discovered home base is loaded");
    TestSupport::require(contains(gs.regionManager.getDiscoveredRegions(), "starter_village"), "starter village is discovered");
    TestSupport::require(contains(gs.regionManager.getDiscoveredRegions(), "home_base"), "home base is discovered");

    TestSupport::require(gs.buildingSystem.getInstances().size() == 1, "home furniture restores");
    TestSupport::require(gs.buildingSystem.getInstances().front().defId == "simple_bed", "home furniture id restores");
    TestSupport::require(near(gs.toySystem.getMiniCarBestTime(), data.toyData.miniCarBestTime), "toy save data restores");
    TestSupport::require(gs.questSystem.isCompleted("organize_home_base"), "completed quest restores");

    TestSupport::require(gs.princess != nullptr, "princess exists");
    TestSupport::require(near(gs.princess->affection, data.princess.affection), "princess affection restores");
    TestSupport::require(gs.princess->isFollowing(), "princess following restores");
    TestSupport::require(near(gs.princess->ultimateCharge, data.princess.ultimateCharge), "princess ultimate restores");
    TestSupport::require(gs.princess->hasBody(), "princess body exists");
    b2Vec2 princessPos = b2Body_GetPosition(gs.princess->getBodyId());
    TestSupport::require(
        nearVec(glm::vec2(princessPos.x, princessPos.y), data.princess.position),
        "princess body moves to saved position");

    TestSupport::require(gs.projectileManager.getActive().empty(), "projectiles clear");
    TestSupport::require(gs.enemyManager.getActive().empty(), "enemies clear");
    TestSupport::require(gs.dropManager.getActive().empty(), "drops clear");
    TestSupport::require(gs.particleSystem.getActiveCount() == 0, "particles clear");

    TestSupport::require(!gs.input.isDown(SDL_SCANCODE_W), "input W clears");
    TestSupport::require(!gs.input.isDown(SDL_SCANCODE_SPACE), "input Space clears");
    TestSupport::require(!gs.input.mouseLeft, "input mouse clears");
    TestSupport::require(!gs.isDead, "death flag resets");
    TestSupport::require(near(gs.deathTimer, 0.0f), "death timer resets");
    TestSupport::require(!gs.isFlying, "flight flag resets");
    TestSupport::require(near(gs.flightHeight, 0.0f), "flight height resets");
    TestSupport::require(near(gs.flightHeightTarget, 0.0f), "flight target resets");
    TestSupport::require(near(gs.flightCooldown, 0.0f), "flight cooldown resets");
    TestSupport::require(!gs.isVenting, "venting flag resets");
    TestSupport::require(near(gs.fireballCooldown, 0.0f), "fireball cooldown resets");
    TestSupport::require(near(gs.shieldCooldown, 0.0f), "shield cooldown resets");
    TestSupport::require(near(gs.enemySpawnTimer, 0.0f), "enemy spawn timer resets");
    TestSupport::require(gs.score == 0, "score resets");
    TestSupport::require(gs.enemiesKilled == 0, "enemy kill count resets");
    TestSupport::require(!gs.buildingSystem.isActive(), "build mode resets outside home base");
    TestSupport::require(!gs.toySystem.isMiniCarActive(), "mini car resets");
    TestSupport::require(!gs.dialogueUI.isVisible(), "dialogue UI hides");
    TestSupport::require(!gs.dialogueTree.isActive(), "dialogue tree is inactive after load");
    TestSupport::require(!gs.ui.talkedWithPrincessAtBaseThisFrame, "talked-with-princess frame flag resets");
    TestSupport::require(gs.ui.stage7NoticeTimer > 0.0f, "load notice timer starts");
    TestSupport::require(
        containsText(gs.ui.stage7Notice, "Loaded") || containsText(gs.ui.stage7Notice, "读档完成"),
        "load notice text is shown");
}

}  // namespace

int main() {
    itemDropsCarryItemMetadata();
    popupArcadeSpawnsChapterPickupsWithoutDuplicates();
    progressionFactsDriveChapterQuestReward();
    popupCrownBossDefeatAppliesRewardsOnce();
    prologueStarCandyInteractionStartsChapterAndAwardsCoin();
    prologueStarCandyQuestCompletesWithoutDuplicatingCoinReward();
    tieyiRescueRequiresEliteDefeatAndRewardsBattery();
    scrapEliteDeathMarksTieyiRescuePrerequisite();
    arcadeBossDoorRequiresTieyiAndTokensThenStartsBoss();
    worldUpdateAutomaticArcadePortalEntersPopupArcade();
    pixelControllerSpotConsumesRelicAndRewardsBaseSupplies();
    firstPrincessTalkAfterStarCandyUnlocksFollowingAndCompletesPrologue();
    startNewGameBeginsInPrologueWithInitialStoryState();
    startNewGameCanEnterPopupArcadeThroughBasePortal();
    collectedItemDropUpdatesInventoryAndStoryFlags();
    applySaveDataRestoresPersistentStateAndClearsTransientState();
    return 0;
}
