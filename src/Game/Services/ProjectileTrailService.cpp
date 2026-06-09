#include "Game/Services/ProjectileTrailService.h"

#include "Game/GameState.h"
#include "Utils/Math.h"

#include <box2d/box2d.h>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace ProjectileTrailService {

Context makeContext(GameState& gs) {
    return {
        gs.projectileManager,
        gs.particleSystem,
        gs.charTime
    };
}

void emitTrails(Context& context) {
    for (const auto& proj : context.projectileManager.getActive()) {
        if (!proj.active) continue;
        if (!b2Body_IsValid(proj.bodyId)) continue;

        if (proj.particleEmitTimer < proj.particleEmitRate) {
            continue;
        }

        context.projectileManager.resetParticleTimer(proj.id);

        b2Vec2 pPos = b2Body_GetPosition(proj.bodyId);
        glm::vec2 pos(pPos.x, pPos.y);
        glm::vec2 dir = proj.velocity;
        float dirLen = glm::length(dir);
        if (dirLen > 0.001f) {
            dir /= dirLen;
        } else {
            dir = glm::vec2(1.0f, 0.0f);
        }
        glm::vec2 side(-dir.y, dir.x);
        float r1 = Math::hashRandom(static_cast<unsigned>(context.charTime * 13000.0f + proj.id.id * 17));
        float r2 = Math::hashRandom(static_cast<unsigned>(context.charTime * 17000.0f + proj.id.id * 31));

        switch (proj.type) {
            case ProjectileType::Fireball:
                context.particleSystem.emit(
                    pos - dir * 0.18f + side * ((r1 - 0.5f) * 0.16f),
                    -dir * (0.9f + r2 * 0.8f) + side * ((r1 - 0.5f) * 0.6f),
                    glm::vec3(1.0f, 0.46f + r2 * 0.22f, 0.08f),
                    0.24f + r2 * 0.16f,
                    0.055f + r1 * 0.055f,
                    ParticleType::Spark
                );
                if (r2 > 0.48f) {
                    context.particleSystem.emit(
                        pos - dir * 0.28f,
                        -dir * 0.45f + side * ((r2 - 0.5f) * 0.35f),
                        glm::vec3(1.0f, 0.22f, 0.04f),
                        0.18f,
                        0.040f,
                        ParticleType::Circle
                    );
                }
                break;
            case ProjectileType::IceSpike:
                context.particleSystem.emit(
                    pos - dir * 0.10f + side * ((r1 - 0.5f) * 0.20f),
                    -dir * (0.25f + r1 * 0.35f) + side * ((r2 - 0.5f) * 0.50f),
                    glm::vec3(0.62f, 0.88f, 1.0f),
                    0.34f + r2 * 0.18f,
                    0.035f + r1 * 0.035f,
                    ParticleType::Circle
                );
                break;
            case ProjectileType::Thunder:
                context.particleSystem.emit(
                    pos + side * ((r1 - 0.5f) * 0.22f),
                    side * ((r2 - 0.5f) * 1.6f) - dir * (0.25f + r1 * 0.45f),
                    glm::vec3(0.82f, 0.94f, 1.0f),
                    0.12f + r2 * 0.10f,
                    0.025f + r1 * 0.025f,
                    ParticleType::Spark
                );
                break;
        }
    }
}

void emitTrails(GameState& gs) {
    Context context = makeContext(gs);
    emitTrails(context);
}

}  // namespace ProjectileTrailService
