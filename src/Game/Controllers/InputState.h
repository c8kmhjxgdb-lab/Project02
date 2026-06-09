#pragma once

#include <SDL2/SDL_scancode.h>
#include <glm/vec2.hpp>

#include <array>
#include <cstddef>

struct InputState {
    std::array<bool, SDL_NUM_SCANCODES> keys{};
    glm::vec2 mousePos{0.0f, 0.0f};
    bool mouseLeft = false;

    void clear() {
        keys.fill(false);
        mouseLeft = false;
    }

    bool isDown(SDL_Scancode scancode) const {
        return scancode >= 0 &&
               scancode < SDL_NUM_SCANCODES &&
               keys[static_cast<std::size_t>(scancode)];
    }

    void setKey(SDL_Scancode scancode, bool down) {
        if (scancode >= 0 && scancode < SDL_NUM_SCANCODES) {
            keys[static_cast<std::size_t>(scancode)] = down;
        }
    }
};
