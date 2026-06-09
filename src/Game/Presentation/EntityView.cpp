#include "Game/Presentation/EntityView.h"

#include "Engine/Renderer/Draw2D.h"

#include <GL/glew.h>
#include <box2d/box2d.h>
#include <algorithm>
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace {

float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

}  // namespace

namespace EntityView {

void renderCharacter(const CharacterRenderResources& resources,
                     const CharacterModel& model,
                     const glm::mat4& viewProj) {
    if (!resources.shader || !model.visible) return;

    float px = model.position.x;
    float py = model.position.y;
    float flightYOffset = -model.flightHeight * 0.3f;

    float s = 1.0f;
    float sh = 1.5f;
    GLfloat verts[] = {
        px - s, py + flightYOffset - sh,
        px + s, py + flightYOffset - sh,
        px + s, py + flightYOffset + sh,
        px - s, py + flightYOffset + sh,
    };

    glUseProgram(resources.shader);
    glUniformMatrix4fv(resources.uniformViewProj, 1, GL_FALSE, &viewProj[0][0]);
    glUniform2f(resources.uniformPosition, px, py + flightYOffset);
    glUniform1f(resources.uniformTime, model.animationTime);
    glUniform3f(resources.uniformBodyColor, 0.4f, 0.7f, 0.95f);
    glUniform1i(resources.uniformExpression, model.expression);
    glUniform1f(resources.uniformArmAngle, model.armAngle);

    glBindVertexArray(resources.vao);
    glBindBuffer(GL_ARRAY_BUFFER, resources.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);

    if (model.flightHeight > 0.1f) {
        float shadowAlpha = 0.3f * model.shadowScale;
        float shadowS = s * model.shadowScale;
        float shadowSh = sh * model.shadowScale;
        float shadowY = py - model.flightHeight * 0.1f;

        Draw2D::drawRectFilled(
            px - shadowS, shadowY - shadowSh,
            shadowS * 2.0f, shadowSh * 2.0f,
            glm::vec3(shadowAlpha));
    }
}

void renderProjectile(const ProjectileRenderResources& resources,
                      const Projectile& projectile,
                      const glm::mat4& viewProj,
                      float animationTime) {
    if (!resources.shader || !projectile.active) return;
    if (!b2Body_IsValid(projectile.bodyId)) return;

    b2Vec2 pos = b2Body_GetPosition(projectile.bodyId);

    glUseProgram(resources.shader);
    glUniformMatrix4fv(resources.uniformViewProj, 1, GL_FALSE, &viewProj[0][0]);
    glUniform2f(resources.uniformPosition, pos.x, pos.y);
    glUniform1f(resources.uniformTime, animationTime);
    glUniform3f(resources.uniformColor, projectile.color.r, projectile.color.g, projectile.color.b);
    glUniform1f(resources.uniformRadius, projectile.radius);
    glUniform1i(resources.uniformType, static_cast<int>(projectile.type));

    glm::vec2 dir = projectile.velocity;
    float dirLen = glm::length(dir);
    if (dirLen > 0.001f) {
        dir /= dirLen;
    } else {
        dir = glm::vec2(1.0f, 0.0f);
    }
    if (resources.uniformDirection >= 0) {
        glUniform2f(resources.uniformDirection, dir.x, dir.y);
    }

    float s = projectile.radius * 2.55f;
    GLfloat verts[] = {
        pos.x - s, pos.y - s,
        pos.x + s, pos.y - s,
        pos.x + s, pos.y + s,
        pos.x - s, pos.y + s,
    };

    glBindVertexArray(resources.quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, resources.quadVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);
}

void renderEnemy(const EnemyRenderResources& resources,
                 const Enemy& enemy,
                 const glm::mat4& viewProj,
                 float animationTime) {
    if (!resources.shader || !enemy.active) return;
    if (!b2Body_IsValid(enemy.bodyId)) return;

    b2Vec2 pos = b2Body_GetPosition(enemy.bodyId);

    glUseProgram(resources.shader);
    glUniformMatrix4fv(resources.uniformViewProj, 1, GL_FALSE, &viewProj[0][0]);
    glUniform2f(resources.uniformPosition, pos.x, pos.y);
    glUniform1f(resources.uniformTime, animationTime);
    glUniform3f(resources.uniformColor, enemy.color.r, enemy.color.g, enemy.color.b);
    glUniform1f(resources.uniformRadius, enemy.radius);
    glUniform1i(resources.uniformType, static_cast<int>(enemy.type));
    float hpPercent = enemy.maxHealth > 0.0f ? clamp01(enemy.health / enemy.maxHealth) : 0.0f;
    glUniform1f(resources.uniformHealthPercent, hpPercent);

    float s = enemy.radius * 2.1f;
    GLfloat verts[] = {
        pos.x - s, pos.y - s,
        pos.x + s, pos.y - s,
        pos.x + s, pos.y + s,
        pos.x - s, pos.y + s,
    };

    glBindVertexArray(resources.quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, resources.quadVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}

void renderDrop(const Drop& drop, const glm::mat4& viewProj) {
    if (!drop.active) return;
    if (!b2Body_IsValid(drop.bodyId)) return;

    b2Vec2 pos = b2Body_GetPosition(drop.bodyId);
    float bobY = std::sin(drop.bobTimer) * 0.1f;

    Draw2D::beginFrame(viewProj);

    if (drop.collecting) {
        float t = drop.collectTimer;
        Draw2D::drawCircleFilled(pos.x, pos.y + bobY + t * 2.0f,
            0.15f * (1.0f - t), drop.color * (1.0f - t));
    } else {
        Draw2D::drawCircleFilled(pos.x, pos.y + bobY, 0.15f, drop.color);
        Draw2D::drawCircle(pos.x, pos.y + bobY, 0.25f, drop.color * 0.5f, 0.02f);
    }

    Draw2D::endFrame();
}

void renderEnemyHealthBars(const std::vector<const Enemy*>& aliveEnemies,
                           const glm::mat4& viewProj) {
    if (aliveEnemies.empty()) return;

    Draw2D::beginFrame(viewProj);
    for (const Enemy* enemy : aliveEnemies) {
        if (!enemy->active) continue;
        if (!b2Body_IsValid(enemy->bodyId)) continue;

        b2Vec2 pos = b2Body_GetPosition(enemy->bodyId);
        float hpPercent = enemy->maxHealth > 0.0f ? clamp01(enemy->health / enemy->maxHealth) : 0.0f;
        float barW = enemy->radius * 2.0f;
        float barH = 0.08f;
        float barX = pos.x - barW / 2.0f;
        float barY = pos.y + enemy->radius + 0.1f;

        Draw2D::drawRectFilled(barX, barY, barW, barH, glm::vec3(0.3f));
        Draw2D::drawRectFilled(barX, barY, barW * hpPercent, barH,
            hpPercent > 0.5f ? glm::vec3(0.2f, 0.8f, 0.2f) :
            hpPercent > 0.25f ? glm::vec3(0.8f, 0.8f, 0.2f) : glm::vec3(0.8f, 0.2f, 0.2f));
    }
    Draw2D::endFrame();
}

}  // namespace EntityView
