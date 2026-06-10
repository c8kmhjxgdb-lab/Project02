#include "Game/Services/NoticeService.h"

#include "Game/GameState.h"

namespace NoticeService {

Context makeContext(GameState& gs) {
    return {
        gs.ui.stage7Notice,
        gs.ui.stage7NoticeTimer
    };
}

void showNotice(Context& context, const std::string& notice, float durationSeconds) {
    context.notice = notice;
    context.timer = durationSeconds;
}

}  // namespace NoticeService
