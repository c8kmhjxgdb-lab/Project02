#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <glm/vec2.hpp>

#include "Engine/Camera/Camera2D.h"
#include "Engine/Physics/PhysicsWorld.h"
#include "Engine/Renderer/DecorRenderer.h"
#include "Engine/Renderer/DialogueUI.h"
#include "Engine/Renderer/MiniMap.h"
#include "Engine/Renderer/ParticleSystem.h"
#include "Engine/Renderer/PostProcess.h"
#include "Engine/Scripting/LuaVM.h"
#include "Game/AI/Enemy.h"
#include "Game/AI/PathfindingSystem.h"
#include "Game/Ability/BondTechnique.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/Projectile.h"
#include "Game/Ability/Shield.h"
#include "Game/Ability/SuperStrength.h"
#include "Game/Building/BuildingSystem.h"
#include "Game/Controllers/InputState.h"
#include "Game/Drop.h"
#include "Game/Emotion/EmotionSystem.h"
#include "Game/Emotion/VentAnimation.h"
#include "Game/Health.h"
#include "Game/Inventory/Inventory.h"
#include "Game/Quest/QuestSystem.h"
#include "Game/SaveSystem.h"
#include "Game/Scenes/AppMode.h"
#include "Game/Social/DialogueTree.h"
#include "Game/Social/Princess.h"
#include "Game/Toy/ToySystem.h"
#include "Game/World/Decoration.h"
#include "Game/World/MapTileManager.h"
#include "Game/World/RegionManager.h"
#include "Game/World/TileMap.h"
#include "Game/World/TimeSystem.h"
#include "Game/World/WeatherSystem.h"

#include <memory>
#include <string>
#include <vector>

struct GameState {
    // Box2D
    b2WorldId worldId;
    b2BodyId groundBodyId;
    b2BodyId playerBodyId;

    // Input
    InputState input;

    // Player physics
    float playerForce = 15.0f;

    // Camera
    Camera2D camera;

    // TileMap
    TileMap tileMap;

    // MapTileManager (Stage 5: tile modification with physics sync)
    MapTileManager tileManager;

    // Decorations (Stage 5)
    std::vector<Decoration> decorations;
    DecorRenderer decorRenderer;

    // MiniMap (Stage 5)
    MiniMap miniMap;

    // Character SDF shader
    GLuint characterShader = 0;
    GLint charUniformViewProj = -1;
    GLint charUniformPosition = -1;
    GLint charUniformTime = -1;
    GLint charUniformBodyColor = -1;
    GLint charUniformExpression = -1;
    GLint charUniformArmAngle = -1;

    // Character animation
    float charTime = 0.0f;
    int charExpression = 0;  // 0=normal, 1=happy, 2=sad
    float armAngle = 0.0f;

    // VAO/VBO for character quad
    GLuint charVAO = 0;
    GLuint charVBO = 0;

    // Reusable quad VAO/VBO for projectiles and enemies (avoid per-frame allocation)
    GLuint reusableQuadVAO = 0;
    GLuint reusableQuadVBO = 0;

    // ===== Stage 3 Systems =====

    // Physics
    PhysicsWorld physicsWorld;

    // Health
    HealthComponent playerHealth;

    // Projectile system
    ProjectileManager projectileManager;
    GLuint projectileShader = 0;
    GLint projUniformViewProj = -1;
    GLint projUniformPosition = -1;
    GLint projUniformTime = -1;
    GLint projUniformColor = -1;
    GLint projUniformRadius = -1;
    GLint projUniformType = -1;
    GLint projUniformDirection = -1;

    // Enemy system
    EnemyManager enemyManager;
    GLuint enemyShader = 0;
    GLint enemyUniformViewProj = -1;
    GLint enemyUniformPosition = -1;
    GLint enemyUniformTime = -1;
    GLint enemyUniformColor = -1;
    GLint enemyUniformRadius = -1;
    GLint enemyUniformType = -1;
    GLint enemyUniformHealthPercent = -1;

    // Enemy VAO/VBO
    GLuint enemyVAO = 0;
    GLuint enemyVBO = 0;

    // Drop system
    DropManager dropManager;

    // Particle system
    ParticleSystem particleSystem;

    // Super strength
    SuperStrength superStrength;

    // Shield (Stage 5: new ability)
    Shield shield;
    float shieldCooldown = 0.0f;
    float shieldCooldownMax = 5.0f;

    // Lightning (Stage 5: new ability)
    Lightning lightning;

    // Bond Technique (Stage 5: Princess combo)
    BondTechniqueSystem bondTechnique;

    float playerMana = 100.0f;
    float playerMaxMana = 100.0f;
    float manaRegen = 5.0f;  // per second

    // Skill cooldowns
    float fireballCooldown = 0.0f;
    float fireballCooldownMax = 0.3f;

    // Player facing direction
    glm::vec2 facingDir = glm::vec2(1, 0);

    // Spawn point
    glm::vec2 spawnPoint = glm::vec2(0, 0);

    // Death/respawn
    float deathTimer = 0.0f;
    bool isDead = false;

    // Score
    int score = 0;

    // Flight (Stage 5: replaces dash)
    bool isFlying = false;
    float flightHeight = 0.0f;         // 0 = ground level, max ~5
    float flightHeightTarget = 0.0f;
    float flightMaxHeight = 5.0f;
    float flightSpeed = 2.0f;          // height change speed
    float flightDescentSpeedMult = 1.5f;  // descent is 1.5x ascent speed
    float flightManaDrain = 15.0f;     // per second
    float flightCooldown = 0.0f;
    float flightCooldownMax = 2.0f;
    float shadowScale = 1.0f;          // shadow scale based on height
    // Enemy spawn timer
    float enemySpawnTimer = 0.0f;
    float enemySpawnInterval = 5.0f;
    int maxEnemies = 12;               // Cap total active enemies to prevent performance degradation
    int enemiesKilled = 0;

    // Collision callback for projectiles hitting enemies
    struct CollisionState {
        GameState* gs;
    } collisionState;

    // ===== Stage 4 Systems =====

    // Lua VM
    LuaVM luaVM;

    // Emotion system
    EmotionSystem emotionSystem;
    VentAnimation ventAnimation;
    bool isVenting;

    // Post process
    PostProcess postProcess;
    GLuint postProcessShader = 0;

    // Dialogue system
    DialogueTree dialogueTree;
    DialogueUI dialogueUI;

    // Princess NPC
    std::unique_ptr<Princess> princess;

    // Home area
    glm::vec2 homePosition;
    float homeRadius;

    // Game time (0-24 hours)
    float gameTime;

    // Window dimensions
    int screenWidth;
    int screenHeight;

    // ===== Stage 6 Systems =====

    // Region manager (multi-region world)
    RegionManager regionManager;

    // Pathfinding system (A* pathfinding)
    PathfindingSystem pathfinding;

    // Save/Load system
    SaveSystem saveSystem;

    // Time system (day/night cycle)
    TimeSystem timeSystem;

    // Weather system
    WeatherSystem weatherSystem;

    // Stage 7: home base building
    BuildingSystem buildingSystem;

    // Stage 7: minimal economy/inventory
    Inventory inventory;

    // Stage 7: toy collection and mini-games
    ToySystem toySystem;

    // Stage 7: base quest loop
    QuestSystem questSystem;
    std::string stage7Notice;
    float stage7NoticeTimer = 0.0f;
    bool talkedWithPrincessAtBaseThisFrame = false;

    // Main menu / persistent runtime state
    AppMode appMode = AppMode::MainMenu;
    int menuSelection = 0;
    std::string menuMessage;
    float menuMessageTimer = 0.0f;
    float totalPlayTimeSeconds = 0.0f;
    bool miniMapInitialized = false;
    bool decorRendererInitialized = false;

    // Ambient light uniform cache (for shader updates)
    GLint ambientLightUniform = -1;
};
