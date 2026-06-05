/**
 * 星愿之子 (Starchild) - 阶段6：世界完整化
 *
 * 新增功能：
 * 1. 多区域系统（RegionManager, MapRegion）
 * 2. A*寻路系统（PathfindingSystem）
 * 3. 存档/读档系统（SaveSystem）
 * 4. 日夜循环系统（TimeSystem）
 * 5. 天气系统（WeatherSystem）
 *
 * 保留的功能：
 * - 阶段1-5的所有系统（战斗、对话、情感、地图等）
 */

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Renderer/Draw2D.h"
#include "Engine/Renderer/ParticleSystem.h"
#include "Engine/Renderer/PostProcess.h"
#include "Engine/Renderer/DialogueUI.h"
#include "Engine/Camera/Camera2D.h"
#include "Engine/Physics/PhysicsWorld.h"
#include "Engine/Scripting/LuaVM.h"
#include "Utils/ShaderUtils.h"
#include "Game/World/TileMap.h"
#include "Game/World/MapTileManager.h"
#include "Game/World/TerrainGenerator.h"
#include "Game/World/Decoration.h"
#include "Game/World/DecorationGen.h"
#include "Game/World/RegionManager.h"
#include "Game/World/TimeSystem.h"
#include "Game/World/WeatherSystem.h"
#include "Engine/Renderer/DecorRenderer.h"
#include "Engine/Renderer/RenderLayer.h"
#include "Engine/Renderer/MiniMap.h"
#include "Game/Health.h"
#include "Game/Drop.h"
#include "Game/Ability/Projectile.h"
#include "Game/Ability/SuperStrength.h"
#include "Game/Ability/Shield.h"
#include "Game/Ability/Lightning.h"
#include "Game/Ability/BondTechnique.h"
#include "Game/AI/Enemy.h"
#include "Game/AI/PathfindingSystem.h"
#include "Game/Social/NPC.h"
#include "Game/Social/Princess.h"
#include "Game/Social/DialogueTree.h"
#include "Game/Emotion/EmotionSystem.h"
#include "Game/Emotion/VentAnimation.h"
#include "Game/SaveSystem.h"
#include "Utils/Math.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>

// ========== 游戏状态 ==========

struct GameState {
    // Box2D
    b2WorldId worldId;
    b2BodyId groundBodyId;
    b2BodyId playerBodyId;

    // Input
    bool keys[SDL_NUM_SCANCODES];
    glm::vec2 mousePos;
    bool mouseLeft;

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
    int coinsCollected = 0;

    // Enemy spawn timer
    float enemySpawnTimer = 0.0f;
    float enemySpawnInterval = 5.0f;
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

    // Ambient light uniform cache (for shader updates)
    GLint ambientLightUniform = -1;
};

// ========== Box2D Init ==========

static bool initBox2D(GameState& gs) {
    gs.physicsWorld.create();
    gs.worldId = gs.physicsWorld.getWorldId();

    // Dynamic player
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

    // Physics collision callback — currently unused.
    // Collision detection is handled manually in handleCollisions() via distance checks.
    // The Box2D contact callback is reserved for future use when we switch to
    // physics-driven collision detection (would require tracking which b2BodyId
    // maps to which game entity, via shape user data or a body-to-entity map).
    gs.physicsWorld.setCollisionCallback([](const CollisionInfo&) {
        // Intentionally empty — see comment above.
    });

    return true;
}

// ========== TileMap Init ==========

static bool initTileMap(GameState& gs) {
    // Stage 6: Use RegionManager for multi-region support
    gs.regionManager.init();
    gs.regionManager.setWorldId(gs.worldId);

    // Build physics for the initial region
    MapRegion* currentRegion = gs.regionManager.getCurrentRegion();
    if (currentRegion) {
        currentRegion->buildPhysics(gs.worldId);

        // Initialize MiniMap for the current region
        gs.miniMap.init(150);
        gs.miniMap.setMapDimensions(currentRegion->getWidth(), currentRegion->getHeight(),
                                   currentRegion->getTileMap().tileSize);
        gs.miniMap.setTileGetter([&gs](int x, int y) -> uint8_t {
            MapRegion* region = gs.regionManager.getCurrentRegion();
            if (!region) return 0;
            return static_cast<uint8_t>(region->getTileMap().getTile(x, y));
        });
        gs.miniMap.forceUpdate(gs.spawnPoint);
    }

    // Initialize DecorRenderer
    gs.decorRenderer.init();

    return true;
}

// ========== Character Shader Init ==========

static bool initCharacterShader(GameState& gs) {
    gs.characterShader = createShaderProgram("assets/shaders/character.vert", "assets/shaders/character.frag");
    if (!gs.characterShader) {
        fprintf(stderr, "Failed to create character shader\n");
        return false;
    }

    gs.charUniformViewProj = glGetUniformLocation(gs.characterShader, "uViewProj");
    gs.charUniformPosition = glGetUniformLocation(gs.characterShader, "uPosition");
    gs.charUniformTime = glGetUniformLocation(gs.characterShader, "uTime");
    gs.charUniformBodyColor = glGetUniformLocation(gs.characterShader, "uBodyColor");
    gs.charUniformExpression = glGetUniformLocation(gs.characterShader, "uExpression");
    gs.charUniformArmAngle = glGetUniformLocation(gs.characterShader, "uArmAngle");

    // Create VAO/VBO for character quad
    glGenVertexArrays(1, &gs.charVAO);
    glGenBuffers(1, &gs.charVBO);

    glBindVertexArray(gs.charVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gs.charVBO);

    // Character bounding box: 2.0 wide x 3.0 tall (human-shaped, not square)
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

    // Create reusable quad VAO/VBO for projectiles and enemies
    // This avoids per-entity VAO/VBO allocation in the render loop
    glGenVertexArrays(1, &gs.reusableQuadVAO);
    glGenBuffers(1, &gs.reusableQuadVBO);
    glBindVertexArray(gs.reusableQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gs.reusableQuadVBO);
    // Allocate buffer with max expected size (4 vertices * 2 floats)
    GLfloat quadVerts[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return true;
}

// ========== Stage 3 Shader Init ==========

static bool initStage3Shaders(GameState& gs) {
    // Projectile shader
    gs.projectileShader = createShaderProgram("assets/shaders/projectile.vert", "assets/shaders/projectile.frag");
    if (gs.projectileShader) {
        gs.projUniformViewProj = glGetUniformLocation(gs.projectileShader, "uViewProj");
        gs.projUniformPosition = glGetUniformLocation(gs.projectileShader, "uPosition");
        gs.projUniformTime = glGetUniformLocation(gs.projectileShader, "uTime");
        gs.projUniformColor = glGetUniformLocation(gs.projectileShader, "uColor");
        gs.projUniformRadius = glGetUniformLocation(gs.projectileShader, "uRadius");
        gs.projUniformType = glGetUniformLocation(gs.projectileShader, "uType");
    }

    // Enemy shader
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

    // Enemy VAO/VBO (simple quad for each enemy)
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

// ========== Render Character ==========

static void renderCharacter(GameState& gs, const glm::mat4& viewProj) {
    if (!gs.characterShader || gs.isDead) return;

    b2Vec2 pos = b2Body_GetPosition(gs.playerBodyId);
    float px = pos.x;
    float py = pos.y;

    // Flight height offset (character appears higher when flying)
    float flightYOffset = -gs.flightHeight * 0.3f;  // In 2D top-down, "up" is negative Y

    // Character quad in world space (centered at player position)
    float s = 1.0f;
    float sh = 1.5f;
    GLfloat verts[] = {
        px - s, py + flightYOffset - sh,
        px + s, py + flightYOffset - sh,
        px + s, py + flightYOffset + sh,
        px - s, py + flightYOffset + sh,
    };

    glUseProgram(gs.characterShader);
    glUniformMatrix4fv(gs.charUniformViewProj, 1, GL_FALSE, &viewProj[0][0]);
    glUniform2f(gs.charUniformPosition, px, py + flightYOffset);
    glUniform1f(gs.charUniformTime, gs.charTime);
    glUniform3f(gs.charUniformBodyColor, 0.4f, 0.7f, 0.95f);
    glUniform1i(gs.charUniformExpression, gs.charExpression);
    glUniform1f(gs.charUniformArmAngle, gs.armAngle);

    glBindVertexArray(gs.charVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gs.charVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);

    // Render character shadow (scales with flight height)
    if (gs.flightHeight > 0.1f) {
        float shadowAlpha = 0.3f * gs.shadowScale;
        float shadowS = s * gs.shadowScale;
        float shadowSh = sh * gs.shadowScale;
        float shadowY = py - gs.flightHeight * 0.1f;  // Shadow stays near ground

        Draw2D::drawRectFilled(
            px - shadowS, shadowY - shadowSh,
            shadowS * 2.0f, shadowSh * 2.0f,
            glm::vec3(shadowAlpha));  // Use grayscale with alpha as intensity
    }
}

// ========== Render Shield ==========

static void renderShield(GameState& gs) {
    if (!gs.shield.isActive()) return;

    float intensity = gs.shield.getIntensity();
    if (intensity <= 0.01f) return;

    b2Vec2 pos = b2Body_GetPosition(gs.playerBodyId);
    glm::vec2 playerPos(pos.x, pos.y);
    float radius = gs.shield.getRadius();
    float rotation = gs.shield.getRotationAngle();

    // Draw rotating dashed circle using Draw2D
    const int DASH_COUNT = 8;
    float spacingAngle = glm::two_pi<float>() / DASH_COUNT;  // angle between dash starts
    float dashAngle = spacingAngle * 0.5f;  // each dash is half the spacing (equal dash and gap)

    glm::vec3 shieldColor(0.3f, 0.7f, 1.0f);
    float lineWidth = 0.04f;

    for (int d = 0; d < DASH_COUNT; ++d) {
        float startAngle = rotation + d * spacingAngle;
        float endAngle = startAngle + dashAngle;

        // Draw arc segment
        const int arcSegments = 6;
        for (int i = 0; i < arcSegments; ++i) {
            float a0 = startAngle + (endAngle - startAngle) * (i / static_cast<float>(arcSegments));
            float a1 = startAngle + (endAngle - startAngle) * ((i + 1) / static_cast<float>(arcSegments));

            glm::vec2 p0 = playerPos + glm::vec2(cos(a0), sin(a0)) * radius;
            glm::vec2 p1 = playerPos + glm::vec2(cos(a1), sin(a1)) * radius;

            Draw2D::drawLine(p0.x, p0.y, p1.x, p1.y, shieldColor, lineWidth);
        }
    }

    // Inner glow (subtle filled circle with low alpha)
    Draw2D::drawCircleFilled(playerPos.x, playerPos.y, radius * 0.8f,
        shieldColor, 0.03f * intensity);
}

// ========== Render Lightning ==========

static void renderLightning(GameState& gs) {
    if (!gs.lightning.isActive()) return;

    const LightningChain& chain = gs.lightning.getCurrentChain();
    if (chain.points.size() < 2) return;

    // Flash intensity based on remaining time
    float flashIntensity = chain.remainingTime / chain.lifetime;

    // Draw lightning chain as zigzag lines
    glm::vec3 lightningColor(0.5f, 0.8f, 1.0f);
    float lineWidth = 0.06f * flashIntensity;

    for (size_t i = 0; i < chain.points.size() - 1; ++i) {
        glm::vec2 start = chain.points[i];
        glm::vec2 end = chain.points[i + 1];

        // Create zigzag effect with offset points
        glm::vec2 direction = glm::normalize(end - start);
        glm::vec2 perpendicular(-direction.y, direction.x);

        // Number of segments in this chain link
        int segments = 4;
        float segLen = glm::distance(start, end) / segments;

        glm::vec2 prevPoint = start;
        for (int s = 0; s < segments; ++s) {
            float t = (s + 1) / static_cast<float>(segments);
            glm::vec2 targetPoint = start + (end - start) * t;

            // Add zigzag offset
            float offset = (s % 2 == 0 ? 1.0f : -1.0f) * 0.15f * flashIntensity;
            glm::vec2 zigzagPoint = (prevPoint + targetPoint) * 0.5f + perpendicular * offset;

            Draw2D::drawLine(prevPoint.x, prevPoint.y,
                           zigzagPoint.x, zigzagPoint.y,
                           lightningColor, lineWidth);
            Draw2D::drawLine(zigzagPoint.x, zigzagPoint.y,
                           targetPoint.x, targetPoint.y,
                           lightningColor, lineWidth);

            prevPoint = targetPoint;
        }
    }

    // Draw bright glow at each hit point
    for (const auto& point : chain.points) {
        Draw2D::drawCircleFilled(point.x, point.y, 0.15f * flashIntensity,
            glm::vec3(0.8f, 0.9f, 1.0f), flashIntensity * 0.5f);
    }
}

// ========== Render Projectile ==========

static void renderProjectile(GameState& gs, const Projectile& proj, const glm::mat4& viewProj) {
    if (!gs.projectileShader || !proj.active) return;

    b2Vec2 pos = b2Body_GetPosition(proj.bodyId);

    glUseProgram(gs.projectileShader);
    glUniformMatrix4fv(gs.projUniformViewProj, 1, GL_FALSE, &viewProj[0][0]);
    glUniform2f(gs.projUniformPosition, pos.x, pos.y);
    glUniform1f(gs.projUniformTime, gs.charTime);
    glUniform3f(gs.projUniformColor, proj.color.r, proj.color.g, proj.color.b);
    glUniform1f(gs.projUniformRadius, proj.radius);
    glUniform1i(gs.projUniformType, static_cast<int>(proj.type));

    // Draw a quad at the projectile position using reusable VAO
    float s = proj.radius * 2.0f;
    GLfloat verts[] = {
        pos.x - s, pos.y - s,
        pos.x + s, pos.y - s,
        pos.x + s, pos.y + s,
        pos.x - s, pos.y + s,
    };

    glBindVertexArray(gs.reusableQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gs.reusableQuadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);
}

// ========== Render Enemy ==========

static void renderEnemy(GameState& gs, const Enemy& enemy, const glm::mat4& viewProj) {
    if (!gs.enemyShader || !enemy.active) return;

    b2Vec2 pos = b2Body_GetPosition(enemy.bodyId);

    glUseProgram(gs.enemyShader);
    glUniformMatrix4fv(gs.enemyUniformViewProj, 1, GL_FALSE, &viewProj[0][0]);
    glUniform2f(gs.enemyUniformPosition, pos.x, pos.y);
    glUniform1f(gs.enemyUniformTime, gs.charTime);
    glUniform3f(gs.enemyUniformColor, enemy.color.r, enemy.color.g, enemy.color.b);
    glUniform1f(gs.enemyUniformRadius, enemy.radius);
    glUniform1i(gs.enemyUniformType, static_cast<int>(enemy.type));
    glUniform1f(gs.enemyUniformHealthPercent, enemy.health / enemy.maxHealth);

    float s = enemy.radius;
    GLfloat verts[] = {
        pos.x - s, pos.y - s,
        pos.x + s, pos.y - s,
        pos.x + s, pos.y + s,
        pos.x - s, pos.y + s,
    };

    glBindVertexArray(gs.reusableQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gs.reusableQuadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Health bar
    glUseProgram(0);
    float hpPercent = enemy.health / enemy.maxHealth;
    float barW = enemy.radius * 2.0f;
    float barH = 0.08f;
    float barX = pos.x - barW / 2.0f;
    float barY = pos.y + enemy.radius + 0.1f;

    Draw2D::beginFrame(viewProj);
    // Background
    Draw2D::drawRectFilled(barX, barY, barW, barH, glm::vec3(0.3f));
    // Health
    Draw2D::drawRectFilled(barX, barY, barW * hpPercent, barH,
        hpPercent > 0.5f ? glm::vec3(0.2f, 0.8f, 0.2f) :
        hpPercent > 0.25f ? glm::vec3(0.8f, 0.8f, 0.2f) : glm::vec3(0.8f, 0.2f, 0.2f));
    Draw2D::endFrame();

    glBindVertexArray(0);
}

// ========== Render Drop ==========

static void renderDrop(GameState& /*gs*/, const Drop& drop, const glm::mat4& viewProj) {
    if (!drop.active) return;

    b2Vec2 pos = b2Body_GetPosition(drop.bodyId);

    // Bob animation
    float bobY = std::sin(drop.bobTimer) * 0.1f;

    Draw2D::beginFrame(viewProj);

    if (drop.collecting) {
        // Collect animation: fly up and fade
        float t = drop.collectTimer;
        Draw2D::drawCircleFilled(pos.x, pos.y + bobY + t * 2.0f,
            0.15f * (1.0f - t), drop.color * (1.0f - t));
    } else {
        // Normal drop
        Draw2D::drawCircleFilled(pos.x, pos.y + bobY, 0.15f, drop.color);
        // Glow
        Draw2D::drawCircle(pos.x, pos.y + bobY, 0.25f, drop.color * 0.5f, 0.02f);
    }

    Draw2D::endFrame();
}

// ========== Render TileMap ==========

static void renderTileMap(GameState& gs, const TileColors& colors, const glm::mat4& viewProj) {
    int tx0, ty0, tx1, ty1;
    float left, right, bottom, top;
    gs.camera.getViewportBounds(800, 600, left, right, bottom, top);

    // Expand by 1 tile for safety
    glm::ivec2 tl = gs.tileMap.worldToTile(left - 1, bottom - 1);
    glm::ivec2 br = gs.tileMap.worldToTile(right + 1, top + 1);

    tx0 = std::max(0, tl.x);
    ty0 = std::max(0, tl.y);
    tx1 = std::min(gs.tileMap.width - 1, br.x);
    ty1 = std::min(gs.tileMap.height - 1, br.y);

    float ts = gs.tileMap.tileSize;

    Draw2D::beginFrame(viewProj);

    for (int y = ty0; y <= ty1; ++y) {
        for (int x = tx0; x <= tx1; ++x) {
            TileType type = gs.tileMap.getTile(x, y);
            glm::vec3 color = colors.get(type);
            float wx = x * ts;
            float wy = y * ts;

            if (type == TileType::Wall) {
                Draw2D::drawRectFilled(wx, wy, ts, ts, color * 0.8f);
                Draw2D::drawRect(wx, wy, ts, ts, color, 0.02f);
            } else if (type == TileType::Water) {
                Draw2D::drawRectFilled(wx, wy, ts, ts, color);
            } else {
                Draw2D::drawRectFilled(wx, wy, ts, ts, color);
                Draw2D::drawRect(wx, wy, ts, ts, color * 0.85f, 0.005f);
            }
        }
    }

    Draw2D::endFrame();
}

// ========== Render UI ==========

static void renderUI(GameState& gs) {
    // Player health bar (top-left, screen space)
    float hpPercent = gs.playerHealth.getHealthPercent();
    float barW = 200.0f;
    float barH = 20.0f;
    float barX = 20.0f;
    float barY = 600.0f - 40.0f;  // Top of screen (600 is window height)

    // Use orthographic projection for UI
    glm::mat4 uiProj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    Draw2D::beginFrame(uiProj);

    // Health bar background
    Draw2D::drawRectFilled(barX, barY, barW, barH, glm::vec3(0.2f));

    // Health bar fill
    glm::vec3 hpColor;
    if (hpPercent > 0.5f) hpColor = glm::vec3(0.2f, 0.8f, 0.2f);
    else if (hpPercent > 0.25f) hpColor = glm::vec3(0.8f, 0.8f, 0.2f);
    else hpColor = glm::vec3(0.8f, 0.2f, 0.2f);

    Draw2D::drawRectFilled(barX, barY, barW * hpPercent, barH, hpColor);

    // Health bar border
    Draw2D::drawRect(barX, barY, barW, barH, glm::vec3(0.5f), 0.02f);

    // Mana bar (below health bar)
    float manaPercent = gs.playerMana / gs.playerMaxMana;
    float manaBarY = barY - barH - 4.0f;

    Draw2D::drawRectFilled(barX, manaBarY, barW, barH * 0.8f, glm::vec3(0.15f));
    Draw2D::drawRectFilled(barX, manaBarY, barW * manaPercent, barH * 0.8f,
        glm::vec3(0.2f, 0.4f, 0.9f));
    Draw2D::drawRect(barX, manaBarY, barW, barH * 0.8f, glm::vec3(0.4f), 0.02f);

    // Shield cooldown indicator
    if (gs.shieldCooldown > 0) {
        float shieldCdPercent = 1.0f - (gs.shieldCooldown / gs.shieldCooldownMax);
        float shieldBarY = manaBarY - barH * 0.8f - 4.0f;
        Draw2D::drawRectFilled(barX, shieldBarY, barW * shieldCdPercent, barH * 0.6f,
            glm::vec3(0.3f, 0.7f, 1.0f));
    }

    // Flight height indicator
    if (gs.isFlying || gs.flightHeight > 0.1f) {
        float flightPercent = gs.flightHeight / gs.flightMaxHeight;
        float flightBarY = manaBarY - barH * 1.6f - 8.0f;
        Draw2D::drawRectFilled(barX, flightBarY, barW * flightPercent, barH * 0.5f,
            glm::vec3(0.6f, 0.8f, 1.0f));
    }

    // Score
    Draw2D::endFrame();

    // Draw score text (using simple rectangles for now - a real game would use a font)
    // For simplicity, we'll just show it in the window title
}

// ========== Spawn Enemy ==========

static void spawnEnemy(GameState& gs, const glm::vec2& pos) {
    // Pick random type based on distance from spawn
    b2Vec2 playerPos = b2Body_GetPosition(gs.playerBodyId);
    float dist = glm::distance(glm::vec2(playerPos.x, playerPos.y), pos);

    EnemyType type;
    float r = Math::hashRandom(static_cast<unsigned int>(gs.charTime * 1000.0f + gs.enemiesKilled));
    if (dist > 15.0f && r < 0.3f) {
        type = EnemyType::Shooter;
    } else if (r < 0.6f) {
        type = EnemyType::Exploder;
    } else {
        type = EnemyType::Chaser;
    }

    gs.enemyManager.spawn(gs.worldId, pos, type);
}

// ========== Handle Collisions ==========

static void handleCollisions(GameState& gs) {
    // Check projectile vs enemy collisions
    // Collect hits first, apply after to avoid iterator invalidation
    struct HitInfo {
        ProjectileId projId;
        EnemyId enemyId;
        b2BodyId enemyBody;
        glm::vec2 hitPos;
        float damage;
        ProjectileType type;
    };
    std::vector<HitInfo> hits;

    const auto& projectiles = gs.projectileManager.getActive();
    auto aliveEnemies = gs.enemyManager.getAlive();

    for (const auto& proj : projectiles) {
        if (!proj.active) continue;

        b2Vec2 projPos = b2Body_GetPosition(proj.bodyId);
        glm::vec2 pPos(projPos.x, projPos.y);

        for (const Enemy* enemy : aliveEnemies) {
            if (!enemy->active) continue;

            b2Vec2 enemyPos = b2Body_GetPosition(enemy->bodyId);
            glm::vec2 ePos(enemyPos.x, enemyPos.y);

            float dist = glm::distance(pPos, ePos);
            if (dist < (proj.radius + enemy->radius)) {
                // Record hit, apply later
                hits.push_back({proj.id, enemy->id, enemy->bodyId, pPos, proj.damage, proj.type});
                break;
            }
        }
    }

    // Apply hits
    for (const auto& hit : hits) {
        gs.projectileManager.onHit(hit.projId, hit.enemyBody);
        gs.enemyManager.damage(hit.enemyId, hit.damage);

        // Find the enemy for additional effects
        const Enemy* enemy = gs.enemyManager.find(hit.enemyId);
        if (enemy) {
            if (hit.type == ProjectileType::IceSpike) {
                const_cast<Enemy*>(enemy)->applySlow(3.0f, 0.4f);
            }
            gs.particleSystem.emitBurst(hit.hitPos, 8,
                hit.type == ProjectileType::IceSpike ? glm::vec3(0.5f, 0.8f, 1.0f) :
                hit.type == ProjectileType::Thunder ? glm::vec3(0.8f, 0.9f, 1.0f) :
                glm::vec3(1.0f, 0.5f, 0.0f), 3.0f, 0.3f, 0.1f);
        }
    }

    // Check enemy vs player collisions
    b2Vec2 playerPos = b2Body_GetPosition(gs.playerBodyId);
    glm::vec2 pPos2(playerPos.x, playerPos.y);

    if (!gs.playerHealth.isInvincible() && !gs.isDead && !gs.isFlying) {
        for (const Enemy* enemy : aliveEnemies) {
            if (!enemy->active) continue;

            b2Vec2 enemyPos = b2Body_GetPosition(enemy->bodyId);
            glm::vec2 ePos2(enemyPos.x, enemyPos.y);

            float dist = glm::distance(pPos2, ePos2);
            if (dist < (0.3f + enemy->radius)) {
                // Player takes damage
                DamageInfo info;
                info.amount = enemy->damage;
                info.sourcePosition = ePos2;
                info.sourceBody = enemy->bodyId;
                info.type = DamageType::Normal;

                gs.playerHealth.takeDamage(info);
                gs.playerHealth.setInvincible(0.5f);

                // Knockback
                glm::vec2 kbDir = Math::normalize(pPos2 - ePos2);
                b2Vec2 kbForce = { kbDir.x * 10.0f, kbDir.y * 10.0f };
                b2Body_ApplyLinearImpulseToCenter(gs.playerBodyId, kbForce, true);

                // Hurt particles
                gs.particleSystem.emitBurst(pPos2, 5, glm::vec3(1, 0, 0), 2.0f, 0.2f, 0.08f);

                if (!gs.playerHealth.isAlive()) {
                    gs.isDead = true;
                    gs.deathTimer = 3.0f;
                }
                break;
            }
        }
    }

    // Check exploder enemies.
    // Use getActive() (not getAlive()) so Dead-state exploders are still iterated —
    // getAlive() filters out Dead, which previously made the explosion dead code.
    // getActive() returns a const reference to the manager's vector<Enemy>, so
    // we iterate by const reference and use const_cast for the deathTimer write.
    const auto& allEnemies = gs.enemyManager.getActive();
    for (const Enemy& enemy : allEnemies) {
        if (!enemy.active || enemy.type != EnemyType::Exploder) continue;
        if (enemy.state != Enemy::State::Dead) continue;

        // Trigger explosion when death timer is still in the explosion window
        // deathTimer starts at 0.5f and counts down; explode when timer > 0.0f
        if (enemy.deathTimer > 0.0f) {
            b2Vec2 enemyPos = b2Body_GetPosition(enemy.bodyId);
            glm::vec2 ePos3(enemyPos.x, enemyPos.y);

            // Explosion particles
            gs.particleSystem.emitBurst(ePos3, 30, glm::vec3(1, 0.5f, 0.1f), 5.0f, 0.5f, 0.2f);
            gs.particleSystem.emitRing(ePos3, 20, glm::vec3(1, 0.3f, 0.0),
                enemy.explosionRange, 0.3f, 0.15f);

            // Damage player if in range.
            // Note: drops & score were already awarded by onEnemyDeath in
            // EnemyManager::damage() (when killed by a projectile) or in the
            // Exploder attack branch in EnemyManager::update() (when self-destructing).
            // We do NOT spawn drops here to avoid doubling.
            float distToPlayer = glm::distance(pPos2, ePos3);
            if (distToPlayer < enemy.explosionRange && !gs.playerHealth.isInvincible() && !gs.isDead && !gs.isFlying) {
                DamageInfo info;
                info.amount = enemy.explosionDamage;
                info.sourcePosition = ePos3;
                info.type = DamageType::Explosion;

                gs.playerHealth.takeDamage(info);
                gs.playerHealth.setInvincible(0.5f);

                // Knockback from explosion
                glm::vec2 kbDir = Math::normalize(pPos2 - ePos3);
                b2Vec2 kbForce = { kbDir.x * 15.0f, kbDir.y * 15.0f };
                b2Body_ApplyLinearImpulseToCenter(gs.playerBodyId, kbForce, true);

                if (!gs.playerHealth.isAlive()) {
                    gs.isDead = true;
                    gs.deathTimer = 3.0f;
                }
            }

            // Mark explosion as triggered by setting deathTimer to 0
            // The enemy will be cleaned up by EnemyManager::update() when timer expires
            const_cast<Enemy&>(enemy).deathTimer = 0.0f;
        }
    }
}

// ========== Main ==========

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    // -- SDL2 init --
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_StopTextInput();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow(
        "Starchild 2D - Stage 4: Emotion & Princess System",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    if (!glCtx) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // -- GLEW init --
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "glewInit failed\n");
        SDL_GL_DeleteContext(glCtx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    glViewport(0, 0, 800, 600);
    glClearColor(0.96f, 0.94f, 0.85f, 1.0f);

    // -- Init Draw2D --
    if (!Draw2D::init()) {
        fprintf(stderr, "Draw2D::init failed\n");
        SDL_GL_DeleteContext(glCtx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // -- Init GameState --
    GameState gs = {};
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) gs.keys[i] = false;

    if (!initBox2D(gs)) {
        fprintf(stderr, "Failed to init Box2D\n");
        SDL_GL_DeleteContext(glCtx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!initTileMap(gs)) {
        fprintf(stderr, "Failed to init TileMap\n");
        SDL_GL_DeleteContext(glCtx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!initCharacterShader(gs)) {
        fprintf(stderr, "Failed to init Character Shader\n");
        SDL_GL_DeleteContext(glCtx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!initStage3Shaders(gs)) {
        fprintf(stderr, "Warning: Failed to init some Stage 3 shaders\n");
    }

    // Init Stage 3 systems
    gs.projectileManager.init();
    gs.enemyManager.init();
    gs.enemyManager.setDeathCallback([&gs](EnemyId id, const glm::vec2& pos, int minC, int maxC) {
        (void)id;  // id is the dying enemy's id; we don't need it here
        int coins = minC + (std::rand() % (maxC - minC + 1));
        gs.dropManager.spawn(gs.worldId, pos, DropType::Coin, coins);
        gs.score += 50;
    });

    // Shooter AI: actually fire a projectile at the player.
    gs.enemyManager.setShootCallback([&gs](const glm::vec2& origin, const glm::vec2& direction, b2BodyId ownerBody) {
        gs.projectileManager.fire(gs.worldId, origin, direction,
            ProjectileType::Thunder, 12.0f, 300.0f, ownerBody);
    });
    gs.dropManager.init();
    gs.particleSystem.init();

    // ===== Stage 4 System Init =====

    // Lua VM
    gs.luaVM.init();
    gs.luaVM.loadFile("assets/scripts/abilities.lua");
    gs.luaVM.loadFile("assets/scripts/emotion_config.lua");

    // Post process
    gs.screenWidth = 800;
    gs.screenHeight = 600;
    gs.postProcess.init(gs.screenWidth, gs.screenHeight);
    gs.postProcessShader = createShaderProgram("assets/shaders/postprocess.vert", "assets/shaders/postprocess.frag");

    // Emotion system
    gs.isVenting = false;

    // Home area
    gs.homePosition = glm::vec2(3.0f, 2.0f);
    gs.homeRadius = 3.0f;

    // Game time
    gs.gameTime = 10.0f;  // Start at 10:00 AM

    // Create Princess (小夏)
    gs.princess = std::make_unique<Princess>("小夏");
    gs.princess->setColor(glm::vec3(0.9f, 0.6f, 0.8f));
    gs.princess->createBody(gs.worldId, glm::vec2(5.0f, 3.0f));
    gs.princess->setDialogueId("first_meeting");

    // Set princess schedule
    gs.princess->setSchedule({
        {6.0f, 8.0f,  glm::vec2(10, 5),  "idle",  ""},
        {8.0f, 12.0f, glm::vec2(12, 8),  "walk",  "daily_greetings"},
        {12.0f, 14.0f, glm::vec2(10, 5), "idle",  ""},
        {14.0f, 18.0f, glm::vec2(15, 10), "walk", ""},
        {18.0f, 22.0f, glm::vec2(3, 2),   "idle",  ""},
        {22.0f, 24.0f, glm::vec2(3, 2),   "idle",  ""},
        {0.0f, 6.0f,  glm::vec2(3, 2),    "idle",  ""},
    });

    // Load dialogue tree (uses the shared LuaVM so dialogue scripts can
    // call into the same global helpers and loaded modules as the rest of the game).
    gs.dialogueTree.setLuaVM(&gs.luaVM);
    gs.dialogueTree.loadFromLua("first_meeting");
    gs.dialogueTree.setOnNodeEnter([&gs](const DialogueNode& node) {
        gs.dialogueUI.begin(node);
    });
    gs.dialogueTree.setOnDialogueEnd([&gs](const DialogueNode&) {
        gs.dialogueUI.hide();
    });

    // Set hurt callback to add grievance
    gs.playerHealth.setHurtCallback([&gs](const DamageInfo&) {
        gs.emotionSystem.addGrievance(10.0f);
    });

    // Camera setup
    gs.camera.zoom = 40.0f;
    b2Vec2 initPos = b2Body_GetPosition(gs.playerBodyId);
    gs.camera.position = glm::vec2(initPos.x, initPos.y);
    gs.spawnPoint = gs.camera.position;

    TileColors tileColors;

    // ===== Stage 6 System Init =====

    // Time system
    gs.timeSystem.init(10.0f);  // Start at 10:00 AM
    gs.timeSystem.setDaySpeed(2.0f);  // 2x speed: 30 real seconds = 1 game hour

    // Weather system
    gs.weatherSystem.init(&gs.particleSystem, &gs.camera);
    gs.weatherSystem.setRandomWeather(300.0f);  // Change weather every 5 minutes

    // Spawn initial enemies
    for (int i = 0; i < 5; ++i) {
        float angle = (static_cast<float>(i) / 5.0f) * 6.28318f;
        glm::vec2 spawnPos = gs.spawnPoint + glm::vec2(std::cos(angle), std::sin(angle)) * 10.0f;
        spawnEnemy(gs, spawnPos);
    }

    // -- Game loop --
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    int frameCount = 0;
    const float dt = 1.0f / 60.0f;

    while (running) {
        // Event handling
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Scancode scancode = e.key.keysym.scancode;
                if (scancode < SDL_NUM_SCANCODES && scancode >= 0) {
                    gs.keys[scancode] = true;
                }
                if (scancode == SDL_SCANCODE_ESCAPE)
                    running = false;
                if (scancode == SDL_SCANCODE_1) gs.charExpression = 0;
                if (scancode == SDL_SCANCODE_2) gs.charExpression = 1;
                if (scancode == SDL_SCANCODE_3) gs.charExpression = 2;
                if (scancode == SDL_SCANCODE_EQUALS || scancode == SDL_SCANCODE_KP_PLUS)
                    gs.camera.setZoom(gs.camera.zoom / 1.15f);
                if (scancode == SDL_SCANCODE_MINUS || scancode == SDL_SCANCODE_KP_MINUS)
                    gs.camera.setZoom(gs.camera.zoom * 1.15f);

                // Stage 3: Fireball
                if (scancode == SDL_SCANCODE_J && !gs.isDead) {
                    if (gs.fireballCooldown <= 0) {
                        // Use the player's actual body position, not the camera
                        // (the camera is interpolated and lags behind the body).
                        b2Vec2 pBody = b2Body_GetPosition(gs.playerBodyId);
                        glm::vec2 playerPos(pBody.x, pBody.y);
                        gs.projectileManager.fire(gs.worldId,
                            playerPos, gs.facingDir,
                            ProjectileType::Fireball, 25.0f, 400.0f,
                            gs.playerBodyId);
                        gs.fireballCooldown = gs.fireballCooldownMax;

                        // Muzzle flash particles
                        glm::vec2 muzzlePos = playerPos + gs.facingDir * 0.5f;
                        gs.particleSystem.emitBurst(muzzlePos, 5,
                            glm::vec3(1, 0.5, 0), 2.0f, 0.15f, 0.08f);
                    }
                }

                // Ice Spike (slow projectile)
                if (scancode == SDL_SCANCODE_L && !gs.isDead) {
                    if (gs.fireballCooldown <= 0) {
                        b2Vec2 pBody = b2Body_GetPosition(gs.playerBodyId);
                        glm::vec2 playerPos(pBody.x, pBody.y);
                        gs.projectileManager.fire(gs.worldId,
                            playerPos, gs.facingDir,
                            ProjectileType::IceSpike, 20.0f, 500.0f,
                            gs.playerBodyId);
                        gs.fireballCooldown = gs.fireballCooldownMax;

                        // Ice spike particles
                        glm::vec2 muzzlePos = playerPos + gs.facingDir * 0.5f;
                        gs.particleSystem.emitBurst(muzzlePos, 5,
                            glm::vec3(0.4, 0.7, 1.0), 2.0f, 0.15f, 0.08f);
                    }
                }

                // Stage 3: Super strength grab/throw
                if (scancode == SDL_SCANCODE_K && !gs.isDead) {
                    if (gs.superStrength.isGrabbing()) {
                        gs.superStrength.throwObject(gs.facingDir, 20.0f);
                    } else {
                        gs.superStrength.tryGrab(gs.worldId, gs.playerBodyId, gs.facingDir);
                    }
                }

                // Stage 5: Flight (Space key - hold to fly)
                if (scancode == SDL_SCANCODE_SPACE && !gs.isDead
                    && gs.flightCooldown <= 0
                    && gs.playerMana >= 5.0f
                    && gs.flightHeight <= 0.1f) {
                    if (!gs.isFlying) {
                        gs.isFlying = true;
                        gs.flightHeightTarget = gs.flightMaxHeight;
                    }
                }

                // Stage 5: Shield ability (F key)
                if (scancode == SDL_SCANCODE_F && !gs.isDead && !gs.shield.isActive()
                    && gs.shieldCooldown <= 0
                    && gs.playerMana >= 15.0f) {
                    gs.shield.activate(15.0f);
                    gs.playerMana -= 15.0f;
                    gs.shieldCooldown = gs.shieldCooldownMax;

                    // Shield activation particles
                    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                    gs.particleSystem.emitRing(glm::vec2(pPos.x, pPos.y),
                        16, glm::vec3(0.3f, 0.7f, 1.0f),
                        gs.shield.getRadius(), 0.5f, 0.1f);
                }

                // Stage 5: Lightning ability (Q key)
                if (scancode == SDL_SCANCODE_Q && !gs.isDead
                    && gs.lightning.canCast()
                    && gs.playerMana >= gs.lightning.getManaCost()) {
                    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                    glm::vec2 playerPos(pPos.x, pPos.y);

                    // Begin lightning chain
                    gs.lightning.begin(playerPos);
                    gs.playerMana -= gs.lightning.getManaCost();
                    gs.lightning.setCooldown(3.0f);

                    // Find and chain to enemies
                    float currentDamage = gs.lightning.getBaseDamage();
                    glm::vec2 currentPos = playerPos;
                    float range = gs.lightning.getChainRange();

                    // Cache alive enemies once — avoid repeated getAlive() calls
                    auto aliveEnemies = gs.enemyManager.getAlive();

                    for (int chain = 0; chain < gs.lightning.getMaxChains(); ++chain) {
                        // Find nearest enemy to current position
                        const Enemy* nearestEnemy = nullptr;
                        float nearestDist = range;

                        for (const Enemy* enemy : aliveEnemies) {
                            if (!enemy || !b2Body_IsValid(enemy->bodyId)) continue;

                            b2Vec2 ePos = b2Body_GetPosition(enemy->bodyId);
                            glm::vec2 enemyPos(ePos.x, ePos.y);
                            float dist = glm::distance(currentPos, enemyPos);

                            // Check if this enemy is already in the chain (using body ID)
                            if (gs.lightning.getCurrentChain().hasHit(enemy->bodyId)) continue;

                            if (dist < nearestDist) {
                                nearestDist = dist;
                                nearestEnemy = enemy;
                            }
                        }

                        if (!nearestEnemy) break;

                        b2Vec2 ePos = b2Body_GetPosition(nearestEnemy->bodyId);
                        glm::vec2 enemyPos(ePos.x, ePos.y);

                        // Add to chain and deal damage
                        gs.lightning.addHit(enemyPos, currentDamage, nearestEnemy->bodyId);
                        gs.enemyManager.damage(nearestEnemy->id, currentDamage);

                        // Lightning hit particles
                        gs.particleSystem.emitBurst(enemyPos, 6,
                            glm::vec3(0.5f, 0.8f, 1.0f), 4.0f, 0.3f, 0.08f);

                        currentDamage *= gs.lightning.getCurrentChain().damageDecay;
                        currentPos = enemyPos;
                    }

                    gs.lightning.end();
                }

                // Stage 5: Bond Technique (G key) - Princess combo attack
                if (scancode == SDL_SCANCODE_G && !gs.isDead
                    && gs.princess && gs.princess->isFollowing()
                    && gs.princess->isUltimateReady()
                    && gs.bondTechnique.canActivate()) {
                    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                    glm::vec2 centerPos(pPos.x, pPos.y);

                    // Activate bond technique
                    gs.bondTechnique.activate(centerPos);
                    gs.princess->ultimateCharge = 0.0f;  // Reset princess ultimate
                    gs.bondTechnique.setCooldown(30.0f);  // 30 second cooldown

                    // Big explosion particles
                    gs.particleSystem.emitBurst(centerPos, 40,
                        glm::vec3(1.0f, 0.9f, 0.5f), 8.0f, 0.8f, 0.15f);
                    gs.particleSystem.emitRing(centerPos, 24,
                        glm::vec3(1.0f, 0.7f, 0.3f),
                        gs.bondTechnique.getMaxRadius(), 1.0f, 0.2f);
                }

                // Stage 4: Interact / Dialogue
                if (scancode == SDL_SCANCODE_E && !gs.isDead) {
                    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                    glm::vec2 playerPos(pPos.x, pPos.y);

                    if (gs.dialogueTree.isActive()) {
                        // In dialogue: advance
                        if (gs.dialogueUI.isVisible()) {
                            gs.dialogueUI.confirm();
                            const DialogueNode* currentNode = gs.dialogueTree.getCurrentNode();
                            if (!gs.dialogueUI.getSelectedChoice() || (currentNode && currentNode->choices.empty())) {
                                gs.dialogueTree.next();
                            } else {
                                gs.dialogueTree.choose(gs.dialogueUI.getSelectedChoice());
                            }
                        } else {
                            gs.dialogueTree.next();
                        }
                    } else if (gs.princess && gs.princess->canInteract(playerPos, 2.0f)) {
                        // Near princess: start dialogue
                        gs.dialogueTree.start("start");
                    } else if (gs.homePosition != glm::vec2(0,0) &&
                               glm::distance(playerPos, gs.homePosition) <= gs.homeRadius &&
                               gs.emotionSystem.getState().grievance > 30.0f) {
                        // At home with high grievance: vent
                        gs.isVenting = true;
                        gs.ventAnimation.start(playerPos);
                        gs.emotionSystem.vent();
                    }
                }

                // Stage 4: Dialogue navigation
                if (gs.dialogueUI.isVisible()) {
                    if (scancode == SDL_SCANCODE_W || scancode == SDL_SCANCODE_UP) {
                        gs.dialogueUI.navigateUp();
                    } else if (scancode == SDL_SCANCODE_S || scancode == SDL_SCANCODE_DOWN) {
                        gs.dialogueUI.navigateDown();
                    } else if (scancode == SDL_SCANCODE_J || scancode == SDL_SCANCODE_SPACE) {
                        gs.dialogueUI.confirm();
                        if (gs.dialogueTree.getCurrentNode() && !gs.dialogueTree.getCurrentNode()->choices.empty()) {
                            gs.dialogueTree.choose(gs.dialogueUI.getSelectedChoice());
                        } else {
                            gs.dialogueTree.next();
                        }
                    }
                }

#ifdef DEBUG
                // Debug: spawn enemy (B = "bad guy", kept distinct from the gameplay E key
                // to avoid double-firing the E-handler above in Debug builds).
                if (scancode == SDL_SCANCODE_B) {
                    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                    glm::vec2 spawnPos(glm::vec2(pPos.x, pPos.y) + gs.facingDir * 3.0f);
                    spawnEnemy(gs, spawnPos);
                }

                // Debug: heal
                if (scancode == SDL_SCANCODE_H) {
                    gs.playerHealth.heal(30.0f);
                }
#endif

                // Stage 6: Save game (F5)
                if (scancode == SDL_SCANCODE_F5) {
                    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                    PlayerProgress progress;
                    progress.discoveredRegions = gs.regionManager.getDiscoveredRegions();
                    progress.totalPlayTime = gs.timeSystem.getHour();
                    progress.maxHealth = 100;
                    progress.maxMana = 0;

                    bool saved = gs.saveSystem.saveGame(
                        "autosave",
                        glm::vec2(pPos.x, pPos.y),
                        gs.playerHealth.getCurrentHealth(),
                        gs.playerHealth.getMaxHealth(),
                        0.0f,  // mana
                        0.0f,  // maxMana
                        gs.coinsCollected,
                        progress,
                        gs.regionManager,
                        gs.emotionSystem.getState().grievance
                    );
                    // Note: save message would be shown via UI (not implemented yet)
                }

                // Stage 6: Load game (F9)
                if (scancode == SDL_SCANCODE_F9) {
                    SaveData saveData;
                    if (gs.saveSystem.loadGame("autosave", saveData)) {
                        // Teleport player to saved position
                        b2Body_SetTransform(gs.playerBodyId,
                            b2Vec2{saveData.player.position.x, saveData.player.position.y},
                            b2Rot{0});
                        b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);
                        gs.playerHealth.heal(saveData.player.health - gs.playerHealth.getCurrentHealth());
                        gs.coinsCollected = saveData.player.coins;
                        gs.emotionSystem.setGrievance(saveData.grievance);
                    }
                }
            } else if (e.type == SDL_KEYUP) {
                SDL_Scancode scancode = e.key.keysym.scancode;
                if (scancode < SDL_NUM_SCANCODES && scancode >= 0) {
                    gs.keys[scancode] = false;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0)
                    gs.camera.setZoom(gs.camera.zoom * 1.15f);
                else
                    gs.camera.setZoom(gs.camera.zoom / 1.15f);
            } else if (e.type == SDL_MOUSEMOTION) {
                gs.mousePos = glm::vec2(static_cast<float>(e.motion.x),
                                        static_cast<float>(e.motion.y));
            }
        }

        // Skip update if dead (but still render)
        if (!gs.isDead) {
            // Update cooldowns
            if (gs.flightCooldown > 0) gs.flightCooldown -= dt;
            if (gs.shieldCooldown > 0) gs.shieldCooldown -= dt;

            // Mana regeneration
            if (gs.playerMana < gs.playerMaxMana) {
                gs.playerMana = std::min(gs.playerMaxMana, gs.playerMana + gs.manaRegen * dt);
            }

            // Update shield
            gs.shield.update(dt);
            gs.lightning.update(dt);
            gs.lightning.updateCooldown(dt);
            gs.bondTechnique.update(dt);
            gs.bondTechnique.updateCooldown(dt);

            // Cache alive enemies once for shield and bond technique checks
            auto aliveEnemies = gs.enemyManager.getAlive();

            // Apply bond technique damage when the wave expands
            if (gs.bondTechnique.isActive()) {
                BondTechnique& tech = gs.bondTechnique.getCurrentTechnique();
                if (!tech.hasDealtDamage() && tech.radius > tech.maxRadius * 0.3f) {
                    // Damage all enemies in range
                    for (const Enemy* enemy : aliveEnemies) {
                        if (!enemy || !b2Body_IsValid(enemy->bodyId)) continue;
                        b2Vec2 ePos = b2Body_GetPosition(enemy->bodyId);
                        glm::vec2 enemyPos(ePos.x, ePos.y);
                        if (glm::distance(tech.waveFronts[0], enemyPos) < tech.radius) {
                            gs.enemyManager.damage(enemy->id, tech.damage);
                        }
                    }
                    tech.markDamaged();
                }
            }

            // Shield hit detection: check enemies near player
            if (gs.shield.isActive()) {
                b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                glm::vec2 playerPos(pPos.x, pPos.y);

                // Check all active enemies (reuse aliveEnemies from above)
                for (const Enemy* enemy : aliveEnemies) {
                    if (enemy && b2Body_IsValid(enemy->bodyId)) {
                        gs.shield.checkAndRepelEnemy(enemy->bodyId, playerPos, 15.0f);
                    }
                }
            }

            // Flight logic
            if (gs.isFlying) {
                // Increase height toward target
                if (gs.flightHeight < gs.flightHeightTarget) {
                    gs.flightHeight += gs.flightSpeed * dt;
                    if (gs.flightHeight >= gs.flightHeightTarget) {
                        gs.flightHeight = gs.flightHeightTarget;
                    }
                }

                // Drain mana while flying
                gs.playerMana -= gs.flightManaDrain * dt;
                if (gs.playerMana <= 0) {
                    gs.playerMana = 0;
                    gs.isFlying = false;
                    gs.flightHeightTarget = 0.0f;
                    gs.flightCooldown = gs.flightCooldownMax;
                }

                // Flight particles (dust rising from ground)
                if (gs.flightHeight > 1.0f) {
                    b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                    // Emit particles below player
                    gs.particleSystem.emit(
                        glm::vec2(pPos.x, pPos.y - gs.flightHeight * 0.3f),
                        glm::vec2(0.0f, 0.5f),
                        glm::vec3(0.8f, 0.8f, 0.7f),
                        0.5f + gs.flightHeight * 0.1f,
                        0.05f + gs.flightHeight * 0.02f,
                        ParticleType::Circle
                    );
                }
            } else {
                // Descend when not flying
                if (gs.flightHeight > 0.0f) {
                    gs.flightHeight -= gs.flightSpeed * gs.flightDescentSpeedMult * dt;
                    if (gs.flightHeight <= 0.0f) {
                        gs.flightHeight = 0.0f;
                    }
                }
            }

            // Update shadow scale based on height
            gs.shadowScale = 1.0f - (gs.flightHeight / gs.flightMaxHeight) * 0.6f;
            if (gs.shadowScale < 0.4f) gs.shadowScale = 0.4f;

            // Check if space is still held for flight
            if (!gs.keys[SDL_SCANCODE_SPACE] && gs.isFlying) {
                gs.isFlying = false;
                gs.flightHeightTarget = 0.0f;
                gs.flightCooldown = gs.flightCooldownMax;
            }

            // Physics step
            b2Vec2 force;
            force.x = 0.0f;
            force.y = 0.0f;
            if (gs.keys[SDL_SCANCODE_W] || gs.keys[SDL_SCANCODE_UP])    force.y += gs.playerForce;
            if (gs.keys[SDL_SCANCODE_S] || gs.keys[SDL_SCANCODE_DOWN])  force.y -= gs.playerForce;
            if (gs.keys[SDL_SCANCODE_A] || gs.keys[SDL_SCANCODE_LEFT])  force.x -= gs.playerForce;
            if (gs.keys[SDL_SCANCODE_D] || gs.keys[SDL_SCANCODE_RIGHT]) force.x += gs.playerForce;

            // Update facing direction
            if (force.x != 0 || force.y != 0) {
                gs.facingDir = Math::normalize(glm::vec2(force.x, force.y));
            }

            // Apply emotion speed modifier
            float speedMult = gs.emotionSystem.getSpeedMultiplier();
            force.x *= speedMult;
            force.y *= speedMult;

            // Get player position and terrain info (needed for multiple systems)
            b2Vec2 playerPosVec = b2Body_GetPosition(gs.playerBodyId);
            glm::vec2 playerPos(playerPosVec.x, playerPosVec.y);
            glm::ivec2 playerTile;

            // Stage 6: Get terrain info from current region
            MapRegion* currentRegion = gs.regionManager.getCurrentRegion();
            if (currentRegion) {
                playerTile = currentRegion->getTileMap().worldToTile(playerPos.x, playerPos.y);
            } else {
                playerTile = gs.tileMap.worldToTile(playerPos.x, playerPos.y);
            }

            // Flight mode: ignore terrain cost, increased mobility
            if (!gs.isFlying) {
                // Apply terrain movement cost
                float terrainCost = currentRegion ?
                    currentRegion->getTileMap().getMovementCost(playerTile.x, playerTile.y) :
                    gs.tileMap.getMovementCost(playerTile.x, playerTile.y);

                if (terrainCost > 0.0f) {
                    force.x /= terrainCost;
                    force.y /= terrainCost;
                }
            } else {
                // Flying: 1.5x movement speed
                force.x *= 1.5f;
                force.y *= 1.5f;
            }

            // Stage 6: Apply weather movement modifier
            float weatherMult = gs.weatherSystem.getMovementMultiplier();
            force.x *= weatherMult;
            force.y *= weatherMult;

            b2Body_ApplyForceToCenter(gs.playerBodyId, force, true);
            gs.physicsWorld.step(dt, 8, 3);

            // Apply terrain damage (lava, etc.)
            float dps = currentRegion ?
                currentRegion->getTileMap().getDamagePerSecond(playerTile.x, playerTile.y) :
                gs.tileMap.getDamagePerSecond(playerTile.x, playerTile.y);
            if (dps > 0.0f && !gs.playerHealth.isInvincible() && !gs.isDead) {
                DamageInfo info{};
                info.amount = dps * dt;
                gs.playerHealth.takeDamage(info);
            }

            // Clamp player velocity
            gs.physicsWorld.clampVelocity(gs.playerBodyId, 8.0f);

            // Update character time
            gs.charTime += dt;

            // Wave animation when moving
            if (force.x != 0.0f || force.y != 0.0f) {
                gs.armAngle = sinf(gs.charTime * 8.0f) * 0.3f;
            } else {
                gs.armAngle = 0.0f;
            }

            // Camera follow (reuse playerPos from above — already updated by physics step)
            playerPosVec = b2Body_GetPosition(gs.playerBodyId);
            gs.camera.smoothFollow(glm::vec2(playerPosVec.x, playerPosVec.y), dt, 5.0f);

            // Update Stage 3 systems.
            // particleSystem.update is intentionally skipped here so the
            // shared always-run block below can update it uniformly (it ticks
            // both alive and dead frames exactly once).
            gs.projectileManager.update(dt, gs.worldId);

            // Emit trail particles from active projectiles
            for (const auto& proj : gs.projectileManager.getActive()) {
                if (!proj.active) continue;

                // Check if it's time to emit a particle
                if (proj.particleEmitTimer >= proj.particleEmitRate) {
                    // Reset timer
                    gs.projectileManager.resetParticleTimer(proj.id);

                    b2Vec2 pPos = b2Body_GetPosition(proj.bodyId);
                    glm::vec2 pos(pPos.x, pPos.y);

                    // Emit trail particle based on projectile type
                    switch (proj.type) {
                        case ProjectileType::Fireball:
                            // Fire trail - spark type with upward velocity
                            gs.particleSystem.emit(
                                pos,
                                glm::vec2(
                                    (Math::hashRandom(static_cast<unsigned>(gs.charTime * 1000))) * 0.5f - 0.25f,
                                    0.3f
                                ),
                                glm::vec3(1.0f, 0.5f, 0.0f),
                                0.3f,
                                0.08f + (Math::hashRandom(static_cast<unsigned>(gs.charTime * 2000))) * 0.05f,
                                ParticleType::Spark
                            );
                            break;
                        case ProjectileType::IceSpike:
                            // Ice trail - circle type, blue-white
                            gs.particleSystem.emit(
                                pos,
                                glm::vec2(0.0f, 0.0f),
                                glm::vec3(0.5f, 0.8f, 1.0f),
                                0.4f,
                                0.06f,
                                ParticleType::Circle
                            );
                            break;
                        case ProjectileType::Thunder:
                            // Thunder trail - small bright sparks
                            gs.particleSystem.emit(
                                pos,
                                glm::vec2(
                                    (Math::hashRandom(static_cast<unsigned>(gs.charTime * 3000))) * 0.8f - 0.4f,
                                    (Math::hashRandom(static_cast<unsigned>(gs.charTime * 4000))) * 0.8f - 0.4f
                                ),
                                glm::vec3(0.8f, 0.9f, 1.0f),
                                0.2f,
                                0.04f,
                                ParticleType::Spark
                            );
                            break;
                    }
                }
            }

            gs.enemyManager.update(dt, gs.worldId, gs.camera.position);
            gs.dropManager.update(dt, gs.camera.position);

            // Update cooldowns
            if (gs.fireballCooldown > 0) gs.fireballCooldown -= dt;

            // Handle collisions
            handleCollisions(gs);

            // Cleanup dead enemies (after collisions to avoid dangling pointers)
            gs.enemyManager.cleanup();

            // Update health
            gs.playerHealth.update(dt);

            // ===== Stage 4 Updates =====

            // Stage 6: Update time system (replaces manual gameTime update)
            gs.timeSystem.update(dt);
            // Keep gameTime in sync for backward compatibility with Stage 4 code
            gs.gameTime = gs.timeSystem.getHour();

            // Stage 6: Update weather system
            gs.weatherSystem.update(dt, gs.camera);

            // Stage 6: Check for region transitions
            if (!gs.regionManager.isTransitioning() && currentRegion) {
                MapConnection* conn = currentRegion->getConnectionAt(
                    glm::vec2(playerPos.x, playerPos.y), 1.5f);
                if (conn) {
                    // Perform region transition
                    gs.regionManager.transitionTo(*conn, gs.worldId);

                    // Teleport player to entry point in new region
                    MapRegion* newRegion = gs.regionManager.getCurrentRegion();
                    if (newRegion) {
                        glm::ivec2 targetTile = conn->targetTile;
                        glm::vec2 targetWorld = newRegion->getTileMap().tileToWorld(
                            targetTile.x, targetTile.y);
                        b2Body_SetTransform(gs.playerBodyId,
                            b2Vec2{targetWorld.x, targetWorld.y}, b2Rot{0});
                        b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);

                        // Update minimap for new region
                        gs.miniMap.setMapDimensions(newRegion->getWidth(), newRegion->getHeight(),
                                                   newRegion->getTileMap().tileSize);
                        gs.miniMap.forceUpdate(glm::vec2(targetWorld.x, targetWorld.y));
                    }
                }
            }

            // Stage 6: Update region manager (transition animations)
            gs.regionManager.update(dt);

            // Check if at home
            glm::vec2 pPos2(playerPos.x, playerPos.y);
            bool atHome = gs.homePosition != glm::vec2(0,0) &&
                          glm::distance(pPos2, gs.homePosition) <= gs.homeRadius;
            gs.emotionSystem.setAtHome(atHome);

            // Update emotion system
            gs.emotionSystem.update(dt);

            // Update vent animation
            if (gs.isVenting) {
                gs.ventAnimation.update(dt);
                if (!gs.ventAnimation.isActive()) {
                    gs.isVenting = false;
                }
            }

            // Update dialogue UI
            if (gs.dialogueUI.isVisible()) {
                gs.dialogueUI.update(dt);
            }

            // Update princess
            if (gs.princess) {
                b2Vec2 pPosPrincess = b2Body_GetPosition(gs.playerBodyId);
                gs.princess->setLastPlayerPos(glm::vec2(pPosPrincess.x, pPosPrincess.y));
                gs.princess->update(dt, gs.gameTime);
            }

            // Enemy spawn timer
            gs.enemySpawnTimer += dt;
            if (gs.enemySpawnTimer >= gs.enemySpawnInterval) {
                gs.enemySpawnTimer = 0;
                // Spawn enemy at random position away from player.
                // Use the player's actual body position, not the camera, so spawns
                // don't drift to where the camera is interpolated to.
                b2Vec2 pPosSpawn = b2Body_GetPosition(gs.playerBodyId);
                glm::vec2 playerPosSpawn(pPosSpawn.x, pPosSpawn.y);
                float angle = Math::hashRandom(static_cast<unsigned int>(gs.charTime * 777)) * 6.28318f;
                float dist = 8.0f + Math::hashRandom(static_cast<unsigned int>(gs.charTime * 333)) * 5.0f;
                glm::vec2 spawnPos = playerPosSpawn + glm::vec2(std::cos(angle), std::sin(angle)) * dist;
                spawnEnemy(gs, spawnPos);
            }
        } else {
            // Death timer
            gs.deathTimer -= dt;
            if (gs.deathTimer <= 0) {
                // Respawn
                gs.isDead = false;
                b2Body_SetTransform(gs.playerBodyId, { gs.spawnPoint.x, gs.spawnPoint.y }, b2Rot_identity);
                b2Body_SetLinearVelocity(gs.playerBodyId, b2Vec2_zero);
                gs.playerHealth.respawn(0.5f);
                gs.camera.position = gs.spawnPoint;
            }
        }

        // Systems that should keep running while the player is dead so the
        // world doesn't freeze (in-flight particles, vent/dialogue animations,
        // minimap camera follow). The !isDead branch above already updated
        // these for the alive case, so we run them only when dead to avoid
        // double-ticking.
        if (gs.isDead) {
            gs.particleSystem.update(dt);
            if (gs.isVenting) {
                gs.ventAnimation.update(dt);
                if (!gs.ventAnimation.isActive()) {
                    gs.isVenting = false;
                }
            }
            if (gs.dialogueUI.isVisible()) {
                gs.dialogueUI.update(dt);
            }
            if (gs.princess) {
                b2Vec2 pPosDead = b2Body_GetPosition(gs.playerBodyId);
                gs.princess->setLastPlayerPos(glm::vec2(pPosDead.x, pPosDead.y));
                gs.princess->update(dt, gs.gameTime);
            }
            b2Vec2 pPosMM = b2Body_GetPosition(gs.playerBodyId);
            gs.miniMap.update(dt, glm::vec2(pPosMM.x, pPosMM.y));
        }

        // Build view-projection matrix
        glm::mat4 viewProj = gs.camera.getViewProjMatrix(800, 600);

        // -- Render --

        // ===== Stage 4: Post-process pipeline =====
        // 1. Render scene to FBO
        gs.postProcess.beginRender();
        glClear(GL_COLOR_BUFFER_BIT);

        // 1. Render tile map
        renderTileMap(gs, tileColors, viewProj);

        // 2. Render low decorations (flowers, grass, rocks)
        if (gs.decorRenderer.isInitialized()) {
            gs.decorRenderer.beginFrame(viewProj);
            for (const auto& decor : gs.decorations) {
                if (decor.type == DecorType::None) continue;
                glm::vec2 worldPos = gs.tileMap.tileToWorld(decor.tileX, decor.tileY);
                // Low decorations: flower, tall grass, rock
                if (decor.type == DecorType::Flower || decor.type == DecorType::TallGrass
                    || decor.type == DecorType::Rock) {
                    gs.decorRenderer.addDecor(worldPos, decor.type, decor.variant,
                        decor.getRotationRadians(), decor.getScaleFactor());
                }
            }
            gs.decorRenderer.endFrame();
        }

        // 3. Y-sort objects layer (drops, enemies, princess, character)
        {
            // Render entry: type + y position + index
            struct RenderEntry {
                int type;   // 0=drop, 1=enemy, 2=princess, 3=player
                float y;
                size_t index;
                bool operator<(const RenderEntry& other) const {
                    return y < other.y;
                }
            };
            std::vector<RenderEntry> renderQueue;

            // Collect drops
            const auto& drops = gs.dropManager.getActive();
            for (size_t i = 0; i < drops.size(); ++i) {
                b2Vec2 pos = b2Body_GetPosition(drops[i].bodyId);
                renderQueue.push_back({0, pos.y, i});
            }

            // Collect enemies
            const auto& enemies = gs.enemyManager.getActive();
            for (size_t i = 0; i < enemies.size(); ++i) {
                b2Vec2 pos = b2Body_GetPosition(enemies[i].bodyId);
                renderQueue.push_back({1, pos.y, i});
            }

            // Collect princess
            if (gs.princess) {
                glm::vec2 pPos = gs.princess->getPosition();
                renderQueue.push_back({2, pPos.y, 0});
            }

            // Collect player
            if (!gs.isDead) {
                b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
                renderQueue.push_back({3, pPos.y, 0});
            }

            // Sort by y coordinate
            std::sort(renderQueue.begin(), renderQueue.end());

            // Render in sorted order
            for (const auto& entry : renderQueue) {
                switch (entry.type) {
                    case 0: renderDrop(gs, drops[entry.index], viewProj); break;
                    case 1: renderEnemy(gs, enemies[entry.index], viewProj); break;
                    case 2:
                        if (gs.princess) gs.princess->render(viewProj);
                        break;
                    case 3:
                        if (!gs.isDead) renderCharacter(gs, viewProj);
                        break;
                }
            }
        }

        // 3.5. Render shield (after character, before projectiles)
        renderShield(gs);

        // 3.5b. Render flight fog effect (when flying high)
        if (gs.isFlying && gs.flightHeight > 2.0f) {
            b2Vec2 pPos = b2Body_GetPosition(gs.playerBodyId);
            float fogAlpha = (gs.flightHeight / gs.flightMaxHeight) * 0.15f;
            float fogRadius = 3.0f + gs.flightHeight;

            // Height fog circle around player
            Draw2D::drawCircleFilled(pPos.x, pPos.y, fogRadius,
                glm::vec3(0.8f, 0.85f, 0.9f), fogAlpha);
        }

        // 3.6. Render lightning (after shield, before projectiles)
        renderLightning(gs);

        // 3.7. Render bond technique (fullscreen shockwave)
        if (gs.bondTechnique.isActive()) {
            const BondTechnique& tech = gs.bondTechnique.getCurrentTechnique();
            float progress = 1.0f - (tech.remainingTime / tech.lifetime);
            float alpha = 0.3f * (1.0f - progress);

            // Expanding ring
            Draw2D::drawCircle(gs.bondTechnique.getCurrentTechnique().waveFronts[0].x,
                gs.bondTechnique.getCurrentTechnique().waveFronts[0].y,
                tech.radius,
                glm::vec3(1.0f, 0.9f, 0.5f), 0.05f * (1.0f - progress));

            // Inner glow
            if (progress < 0.5f) {
                Draw2D::drawCircleFilled(gs.bondTechnique.getCurrentTechnique().waveFronts[0].x,
                    gs.bondTechnique.getCurrentTechnique().waveFronts[0].y,
                    tech.radius * 0.5f,
                    glm::vec3(1.0f, 0.8f, 0.3f), alpha * 0.3f);
            }
        }

        // 4. Render projectiles (always on top of objects)
        for (const auto& proj : gs.projectileManager.getActive()) {
            renderProjectile(gs, proj, viewProj);
        }

        // 5. Render high decorations (trees, stumps - occluded by character)
        if (gs.decorRenderer.isInitialized()) {
            gs.decorRenderer.beginFrame(viewProj);
            for (const auto& decor : gs.decorations) {
                if (decor.type == DecorType::None) continue;
                if (isTallDecor(decor.type)) {
                    glm::vec2 worldPos = gs.tileMap.tileToWorld(decor.tileX, decor.tileY);
                    gs.decorRenderer.addDecor(worldPos, decor.type, decor.variant,
                        decor.getRotationRadians(), decor.getScaleFactor());
                }
            }
            gs.decorRenderer.endFrame();
        }

        // 7. Render particles
        Draw2D::beginFrame(viewProj);
        for (const auto& p : gs.particleSystem.getActive()) {
            if (p.active) {
                float alpha = p.lifetime / p.maxLifetime;
                Draw2D::drawCircleFilled(p.position.x, p.position.y,
                    p.size * alpha, p.color * alpha);
            }
        }

        // Vent animation tear particle effect
        if (gs.isVenting) {
            float shakeX = gs.ventAnimation.getShakeAmount();
            glm::vec2 ventPos = gs.ventAnimation.getPosition();
            // Simple tear drops
            for (int i = 0; i < 3; ++i) {
                float tearY = ventPos.y - 0.5f - i * 0.3f - gs.charTime * 2.0f;
                if (tearY > ventPos.y - 2.0f) {
                    Draw2D::drawCircleFilled(ventPos.x + shakeX + (i - 1) * 0.15f,
                        tearY, 0.03f, glm::vec3(0.3f, 0.5f, 0.9f));
                }
            }
        }

        Draw2D::endFrame();

        gs.postProcess.endRender();

        // 2. Render post-process effect to screen
        if (gs.postProcessShader && gs.postProcess.isValid()) {
            gs.postProcess.setVignetteIntensity(gs.emotionSystem.getPostProcessIntensity());
            gs.postProcess.draw(gs.postProcessShader);
        }

        // 3. Render UI (direct to screen, not through FBO)
        glm::mat4 uiProj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

        // Player health bar
        renderUI(gs);

        // MiniMap
        gs.miniMap.render(uiProj, 800, 600);

        // Dialogue UI
        if (gs.dialogueUI.isVisible()) {
            gs.dialogueUI.render(uiProj, 800, 600);
        }

        // Emotion status display (simple text via window title)

        SDL_GL_SwapWindow(window);

        // FPS counter
        frameCount++;
        Uint32 now = SDL_GetTicks();
        if (now - lastTime >= 1000) {
            char title[256];
            const EmotionState& emo = gs.emotionSystem.getState();
            snprintf(title, sizeof(title),
                "Starchild 2D | Stage 4 Emotion | FPS: %d | HP: %d/%d | Grief: %d | Aff: %s | Score: %d | E:Interact",
                frameCount,
                static_cast<int>(gs.playerHealth.getCurrentHealth()),
                static_cast<int>(gs.playerHealth.getMaxHealth()),
                static_cast<int>(emo.grievance),
                gs.princess ? gs.princess->getAffectionLevelName() : "-",
                gs.score);
            SDL_SetWindowTitle(window, title);
            frameCount = 0;
            lastTime = now;
        }
    }

    // Cleanup
    Draw2D::shutdown();
    gs.tileManager.shutdown();
    gs.decorRenderer.shutdown();
    gs.miniMap.shutdown();
    gs.physicsWorld.destroy();
    gs.projectileManager.clear();
    gs.enemyManager.clear();
    gs.dropManager.clear();

    glDeleteVertexArrays(1, &gs.charVAO);
    glDeleteBuffers(1, &gs.charVBO);
    glDeleteVertexArrays(1, &gs.reusableQuadVAO);
    glDeleteBuffers(1, &gs.reusableQuadVBO);
    glDeleteProgram(gs.characterShader);
    glDeleteProgram(gs.projectileShader);
    glDeleteProgram(gs.enemyShader);
    if (gs.postProcessShader) glDeleteProgram(gs.postProcessShader);

    SDL_GL_DeleteContext(glCtx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Starchild 2D: Stage 3 prototype closed cleanly. Score: %d, Kills: %d\n",
           gs.score, gs.enemiesKilled);
    return 0;
}
