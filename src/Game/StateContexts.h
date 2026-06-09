#pragma once

#include "Game/Scenes/AppMode.h"

#include <glm/vec2.hpp>

class BuildingSystem;
struct Camera2D;
class DialogueTree;
class DialogueUI;
class DropManager;
class EnemyManager;
class EmotionSystem;
class Inventory;
class LuaVM;
class MapTileManager;
class ParticleSystem;
class PhysicsWorld;
class Princess;
class ProjectileManager;
class QuestSystem;
class RegionManager;
class TimeSystem;
class ToySystem;
class WeatherSystem;
struct TileMap;

struct GameState;
struct InputState;

struct WorldState {
    RegionManager& regionManager;
    TileMap& tileMap;
    MapTileManager& tileManager;
    TimeSystem& timeSystem;
    WeatherSystem& weatherSystem;
    float& gameTime;
};

struct PlayerState {
    InputState& input;
    Camera2D& camera;
    glm::vec2& spawnPoint;
    glm::vec2& facingDir;
    float& playerMana;
    float& playerMaxMana;
    bool& isDead;
    bool& isFlying;
};

struct CombatState {
    EnemyManager& enemyManager;
    ProjectileManager& projectileManager;
    DropManager& dropManager;
    ParticleSystem& particleSystem;
    int& score;
    int& enemiesKilled;
};

struct UIState {
    AppMode& appMode;
    int& menuSelection;
    DialogueTree& dialogueTree;
    DialogueUI& dialogueUI;
};

struct RuntimeServices {
    PhysicsWorld& physicsWorld;
    LuaVM& luaVM;
};

WorldState makeWorldState(GameState& gs);
PlayerState makePlayerState(GameState& gs);
CombatState makeCombatState(GameState& gs);
UIState makeUIState(GameState& gs);
RuntimeServices makeRuntimeServices(GameState& gs);
