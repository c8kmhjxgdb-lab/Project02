#pragma once

#include <box2d/box2d.h>

class DialogueUI;
class Princess;
class VentAnimation;
struct GameState;

namespace WorldNpcUiUpdateService {

struct Context {
    bool& isVenting;
    VentAnimation& ventAnimation;
    DialogueUI& dialogueUI;
    Princess* princess;
    b2BodyId playerBodyId;
    float& gameTime;
};

Context makeContext(GameState& gs);

void update(Context& context, float dt);

}  // namespace WorldNpcUiUpdateService
