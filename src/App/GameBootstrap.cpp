#include "App/GameBootstrap.h"

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/TextRenderer.h"
#include "Game/Data/LuaConfigRepository.h"
#include "Game/GameState.h"
#include "Game/Services/AudioService.h"
#include "Game/Services/DropCollectionService.h"
#include "Game/Services/GameSessionService.h"
#include "Game/Services/RegionService.h"
#include "Utils/ShaderUtils.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace {

bool initBox2D(GameState& gs) {
    gs.physicsWorld.create();
    gs.worldId = gs.physicsWorld.getWorldId();

    b2BodyDef playerDef = b2DefaultBodyDef();
    b2Vec2 playerPos;
    playerPos.x = 0.0f;
    playerPos.y = 0.0f;
    playerDef.position = playerPos;
    playerDef.type = b2_dynamicBody;
    gs.playerBodyId = b2CreateBody(gs.worldId, &playerDef);

    b2Polygon playerShape = b2MakeBox(0.3f, 0.3f);
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.density = 1.0f;
    // 玩家碰撞类别
    playerShapeDef.filter.categoryBits = 0x0001;
    playerShapeDef.filter.maskBits = 0x0002 | 0x0004 | 0x0008;  // 投射物、敌人、掉落物
    b2CreatePolygonShape(gs.playerBodyId, &playerShapeDef, &playerShape);

    b2Body_SetLinearDamping(gs.playerBodyId, 3.0f);

    // Physics collision callback is currently unused. Collision detection is
    // handled manually in CombatService::handleCollisions() via distance checks.
    gs.physicsWorld.setCollisionCallback([](const CollisionInfo&) {
        // Reserved for future physics-driven collision detection.
    });

    return true;
}

bool initCharacterShader(GameState& gs) {
    gs.characterShader = createShaderProgram("assets/shaders/character.vert", "assets/shaders/character.frag");
    if (!gs.characterShader) {
        std::fprintf(stderr, "Failed to create character shader\n");
        return false;
    }

    gs.charUniformViewProj = glGetUniformLocation(gs.characterShader, "uViewProj");
    gs.charUniformPosition = glGetUniformLocation(gs.characterShader, "uPosition");
    gs.charUniformTime = glGetUniformLocation(gs.characterShader, "uTime");
    gs.charUniformBodyColor = glGetUniformLocation(gs.characterShader, "uBodyColor");
    gs.charUniformExpression = glGetUniformLocation(gs.characterShader, "uExpression");
    gs.charUniformArmAngle = glGetUniformLocation(gs.characterShader, "uArmAngle");

    glGenVertexArrays(1, &gs.charVAO);
    glGenBuffers(1, &gs.charVBO);

    glBindVertexArray(gs.charVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gs.charVBO);

    GLfloat verts[] = {
        -1.0f, -1.5f,
         1.0f, -1.5f,
         1.0f,  1.5f,
        -1.0f,  1.5f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    glGenVertexArrays(1, &gs.reusableQuadVAO);
    glGenBuffers(1, &gs.reusableQuadVBO);
    glBindVertexArray(gs.reusableQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gs.reusableQuadVBO);
    GLfloat quadVerts[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return true;
}

bool initStage3Shaders(GameState& gs) {
    gs.projectileShader = createShaderProgram("assets/shaders/projectile.vert", "assets/shaders/projectile.frag");
    if (gs.projectileShader) {
        gs.projUniformViewProj = glGetUniformLocation(gs.projectileShader, "uViewProj");
        gs.projUniformPosition = glGetUniformLocation(gs.projectileShader, "uPosition");
        gs.projUniformTime = glGetUniformLocation(gs.projectileShader, "uTime");
        gs.projUniformColor = glGetUniformLocation(gs.projectileShader, "uColor");
        gs.projUniformRadius = glGetUniformLocation(gs.projectileShader, "uRadius");
        gs.projUniformType = glGetUniformLocation(gs.projectileShader, "uType");
        gs.projUniformDirection = glGetUniformLocation(gs.projectileShader, "uDirection");
    }

    gs.enemyShader = createShaderProgram("assets/shaders/enemy.vert", "assets/shaders/enemy.frag");
    if (gs.enemyShader) {
        gs.enemyUniformViewProj = glGetUniformLocation(gs.enemyShader, "uViewProj");
        gs.enemyUniformPosition = glGetUniformLocation(gs.enemyShader, "uPosition");
        gs.enemyUniformTime = glGetUniformLocation(gs.enemyShader, "uTime");
        gs.enemyUniformColor = glGetUniformLocation(gs.enemyShader, "uColor");
        gs.enemyUniformRadius = glGetUniformLocation(gs.enemyShader, "uRadius");
        gs.enemyUniformType = glGetUniformLocation(gs.enemyShader, "uType");
        gs.enemyUniformHealthPercent = glGetUniformLocation(gs.enemyShader, "uHealthPercent");
    }

    glGenVertexArrays(1, &gs.enemyVAO);
    glGenBuffers(1, &gs.enemyVBO);
    glBindVertexArray(gs.enemyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gs.enemyVBO);
    GLfloat enemyVerts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(enemyVerts), enemyVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return true;
}

void initCombatSystems(GameState& gs) {
    gs.projectileManager.init();
    gs.enemyManager.init();
    gs.enemyManager.setDeathCallback([&gs](EnemyId id, const glm::vec2& pos, int minC, int maxC) {
        (void)id;
        int coins = minC + (std::rand() % (maxC - minC + 1));
        gs.dropManager.spawn(gs.worldId, pos, DropType::Coin, coins);
        gs.score += 50;
    });

    gs.enemyManager.setShootCallback([&gs](const glm::vec2& origin, const glm::vec2& direction, b2BodyId ownerBody) {
        gs.projectileManager.fire(gs.worldId, origin, direction,
            ProjectileType::Thunder, 12.0f, 12.0f, ownerBody);
    });

    gs.dropManager.init();
    gs.dropManager.setCollectCallback([&gs](const Drop& drop) {
        DropCollectionService::Context context = DropCollectionService::makeContext(gs);
        DropCollectionService::collect(context, drop);
    });
    gs.particleSystem.init();
}

void initAudio(GameState& gs) {
    if (gs.audioSystem.init()) {
        gs.audioSystem.loadManifest("assets/audio/manifest.json");
        gs.audioSystem.setMasterVolume(0.85f);
        gs.audioSystem.setBgmVolume(0.85f);
        gs.audioSystem.setSfxVolume(0.90f);
        gs.audioSystem.setBgmFadeSeconds(0.35f);
    }
}

void initRuntimeRenderState(GameState& gs, SDL_Window* window) {
    int actualW = 0;
    int actualH = 0;
    SDL_GetWindowSize(window, &actualW, &actualH);
    if (actualW <= 0) actualW = 800;
    if (actualH <= 0) actualH = 600;
    gs.screenWidth = actualW;
    gs.screenHeight = actualH;
    gs.input.mousePos = glm::vec2(actualW * 0.5f, actualH * 0.5f);

    gs.postProcess.init(gs.screenWidth, gs.screenHeight);
    glViewport(0, 0, gs.screenWidth, gs.screenHeight);
    gs.postProcessShader = createShaderProgram("assets/shaders/postprocess.vert", "assets/shaders/postprocess.frag");
}

void initStoryAndPlayerSystems(GameState& gs) {
    gs.isVenting = false;
    gs.homePosition = glm::vec2(3.0f, 2.0f);
    gs.homeRadius = 3.0f;
    gs.gameTime = 10.0f;

    gs.princess = std::make_unique<Princess>("艾莉娅");
    gs.princess->setColor(glm::vec3(0.9f, 0.6f, 0.8f));
    gs.princess->createBody(gs.worldId, glm::vec2(5.0f, 3.0f));
    gs.princess->setDialogueId("first_meeting");
    gs.princess->setSchedule({
        {6.0f, 8.0f,  glm::vec2(10, 5),   "idle", ""},
        {8.0f, 12.0f, glm::vec2(12, 8),   "walk", "daily_greetings"},
        {12.0f, 14.0f, glm::vec2(10, 5),  "idle", ""},
        {14.0f, 18.0f, glm::vec2(15, 10), "walk", ""},
        {18.0f, 22.0f, glm::vec2(3, 2),   "idle", ""},
        {22.0f, 24.0f, glm::vec2(3, 2),   "idle", ""},
        {0.0f, 6.0f,  glm::vec2(3, 2),    "idle", ""},
    });

    gs.dialogueTree.setLuaVM(&gs.luaVM);
    gs.dialogueTree.loadFromLua("first_meeting");
    gs.dialogueTree.setOnNodeEnter([&gs](const DialogueNode& node) {
        gs.dialogueUI.begin(node);
    });
    gs.dialogueTree.setOnDialogueEnd([&gs](const DialogueNode&) {
        gs.dialogueUI.hide();
    });

    gs.playerHealth.setHurtCallback([&gs](const DamageInfo&) {
        gs.emotionSystem.addGrievance(10.0f);
    });
}

void initCamera(GameState& gs) {
    gs.camera.zoom = 40.0f;
    b2Vec2 initPos = b2Body_GetPosition(gs.playerBodyId);
    gs.camera.position = glm::vec2(initPos.x, initPos.y);
    gs.spawnPoint = gs.camera.position;
}

void initProgressionSystems(GameState& gs) {
    gs.timeSystem.init(10.0f);
    gs.timeSystem.loadConfig(gs.luaVM, LuaConfigRepository::timeEventsPath());
    gs.timeSystem.setDaySpeed(2.0f);

    gs.weatherSystem.init(&gs.particleSystem, &gs.camera);
    gs.weatherSystem.loadConfig(gs.luaVM, LuaConfigRepository::weatherConfigPath());
    gs.weatherSystem.setRandomWeather(gs.weatherSystem.getDefaultChangeInterval());
    RegionService::refreshWeatherContext(gs.regionManager, gs.weatherSystem);

    gs.emotionSystem.loadConfig(gs.luaVM, LuaConfigRepository::childhoodConfigPath());
    gs.buildingSystem.init(gs.worldId);
    gs.buildingSystem.loadDefinitions(gs.luaVM, LuaConfigRepository::furniturePath());
    gs.buildingSystem.setBuildableRegion("home_base");
    gs.toySystem.init();
    gs.toySystem.loadDefinitions(gs.luaVM, LuaConfigRepository::toysPath());
    gs.questSystem.init();
    gs.questSystem.loadDefinitions(gs.luaVM, LuaConfigRepository::questsPath());
    gs.inventory.setCoins(60);
    gs.inventory.unlockFurnitureDefaults();
    gs.inventory.addFurniture("simple_bed", 1);
    gs.inventory.addFurniture("writing_desk", 1);
    gs.inventory.addFurniture("star_lamp", 1);
    gs.inventory.addFurniture("soft_rug", 1);
    RegionService::GameplayContext regionGameplay = RegionService::makeGameplayContext(gs);
    RegionService::refreshGameplayContext(regionGameplay);
}

bool initRendererServices() {
    if (!Draw2D::init()) {
        std::fprintf(stderr, "Draw2D::init failed\n");
        return false;
    }

    if (!TextRenderer::init()) {
        std::fprintf(stderr, "TextRenderer::init failed\n");
        Draw2D::shutdown();
        return false;
    }

    return true;
}

void deleteGlResources(GameState& gs) {
    if (gs.charVAO) glDeleteVertexArrays(1, &gs.charVAO);
    if (gs.charVBO) glDeleteBuffers(1, &gs.charVBO);
    if (gs.reusableQuadVAO) glDeleteVertexArrays(1, &gs.reusableQuadVAO);
    if (gs.reusableQuadVBO) glDeleteBuffers(1, &gs.reusableQuadVBO);
    if (gs.enemyVAO) glDeleteVertexArrays(1, &gs.enemyVAO);
    if (gs.enemyVBO) glDeleteBuffers(1, &gs.enemyVBO);
    if (gs.characterShader) glDeleteProgram(gs.characterShader);
    if (gs.projectileShader) glDeleteProgram(gs.projectileShader);
    if (gs.enemyShader) glDeleteProgram(gs.enemyShader);
    if (gs.postProcessShader) glDeleteProgram(gs.postProcessShader);

    gs.charVAO = 0;
    gs.charVBO = 0;
    gs.reusableQuadVAO = 0;
    gs.reusableQuadVBO = 0;
    gs.enemyVAO = 0;
    gs.enemyVBO = 0;
    gs.characterShader = 0;
    gs.projectileShader = 0;
    gs.enemyShader = 0;
    gs.postProcessShader = 0;
}

}  // namespace

namespace GameBootstrap {

bool initialize(GameState& gs, SDL_Window* window) {
    gs.input.clear();

    if (!initRendererServices()) return false;

    if (!initBox2D(gs)) {
        std::fprintf(stderr, "Failed to init Box2D\n");
        return false;
    }

    if (!GameSessionService::initializeWorld(gs)) {
        std::fprintf(stderr, "Failed to init world\n");
        return false;
    }

    if (!initCharacterShader(gs)) {
        std::fprintf(stderr, "Failed to init Character Shader\n");
        return false;
    }

    if (!initStage3Shaders(gs)) {
        std::fprintf(stderr, "Warning: Failed to init some Stage 3 shaders\n");
    }

    initCombatSystems(gs);
    initAudio(gs);

    gs.luaVM.init();
    LuaConfigRepository::loadSharedRuntimeScripts(gs.luaVM);

    initRuntimeRenderState(gs, window);
    initStoryAndPlayerSystems(gs);
    initCamera(gs);
    initProgressionSystems(gs);

    return true;
}

void shutdown(GameState& gs) {
    TextRenderer::shutdown();
    Draw2D::shutdown();
    gs.tileManager.shutdown();
    gs.decorRenderer.shutdown();
    gs.miniMap.shutdown();
    gs.postProcess.shutdown();
    gs.projectileManager.clear();
    gs.enemyManager.clear();
    gs.dropManager.clear();
    gs.buildingSystem.shutdown();
    gs.audioSystem.shutdown();
    gs.physicsWorld.destroy();
    deleteGlResources(gs);
}

}  // namespace GameBootstrap
