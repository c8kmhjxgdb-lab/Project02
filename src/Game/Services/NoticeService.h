#pragma once

#include <string>

struct GameState;

namespace NoticeService {

struct Context {
    std::string& notice;
    float& timer;
};

Context makeContext(GameState& gs);

void showNotice(Context& context, const std::string& notice, float durationSeconds = 4.0f);

}  // namespace NoticeService
