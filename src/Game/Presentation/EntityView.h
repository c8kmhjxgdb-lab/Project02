#pragma once

#include "Game/AI/Enemy.h"
#include "Game/Ability/Projectile.h"
#include "Game/Drop.h"
#include "Game/Presentation/PixelActorView.h"

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <vector>

namespace EntityView {

struct CharacterRenderResources {
    GLuint shader = 0;
    GLint uniformViewProj = -1;
    GLint uniformPosition = -1;
    GLint uniformTime = -1;
    GLint uniformBodyColor = -1;
    GLint uniformExpression = -1;
    GLint uniformArmAngle = -1;
    GLuint vao = 0;
    GLuint vbo = 0;
};

struct CharacterModel {
    bool visible = false;
    glm::vec2 position{0.0f, 0.0f};
    float flightHeight = 0.0f;
    float shadowScale = 1.0f;
    float animationTime = 0.0f;
    int expression = 0;
    float armAngle = 0.0f;
};

struct ProjectileRenderResources {
    GLuint shader = 0;
    GLint uniformViewProj = -1;
    GLint uniformPosition = -1;
    GLint uniformTime = -1;
    GLint uniformColor = -1;
    GLint uniformRadius = -1;
    GLint uniformType = -1;
    GLint uniformDirection = -1;
    GLuint quadVao = 0;
    GLuint quadVbo = 0;
};

struct EnemyRenderResources {
    GLuint shader = 0;
    GLint uniformViewProj = -1;
    GLint uniformPosition = -1;
    GLint uniformTime = -1;
    GLint uniformColor = -1;
    GLint uniformRadius = -1;
    GLint uniformType = -1;
    GLint uniformHealthPercent = -1;
    GLuint quadVao = 0;
    GLuint quadVbo = 0;
};

void renderCharacter(const CharacterRenderResources& resources,
                     const CharacterModel& model,
                     const glm::mat4& viewProj);

void renderProjectile(const ProjectileRenderResources& resources,
                      const Projectile& projectile,
                      const glm::mat4& viewProj,
                      float animationTime);

void renderEnemy(const EnemyRenderResources& resources,
                 const Enemy& enemy,
                 const glm::mat4& viewProj,
                 float animationTime);

void renderDrop(const Drop& drop, const glm::mat4& viewProj);

void renderEnemyHealthBars(const std::vector<const Enemy*>& aliveEnemies,
                           const glm::mat4& viewProj);

bool tryGetPixelActorKind(const Enemy& enemy, PixelActorView::ActorKind& outKind);

}  // namespace EntityView
