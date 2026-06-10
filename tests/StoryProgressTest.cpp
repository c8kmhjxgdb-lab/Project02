#include "Game/Progress/StoryProgress.h"
#include "TestSupport.h"

namespace {

void chapterStatesAdvanceMonotonically() {
    StoryProgress progress;
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::Locked,
        "chapter 1 starts locked before explicit unlock");

    progress.unlockChapter("chapter_1_popup_arcade");
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::Unlocked,
        "chapter unlocks");

    progress.startChapter("chapter_1_popup_arcade");
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::InProgress,
        "chapter starts");

    progress.completeChapter("chapter_1_popup_arcade");
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::Completed,
        "chapter completes");

    progress.startChapter("chapter_1_popup_arcade");
    TestSupport::require(progress.getChapterState("chapter_1_popup_arcade") == ChapterState::Completed,
        "completed chapter does not downgrade");
}

void partnerAndFlagsPersistInSnapshot() {
    StoryProgress progress;
    progress.unlockPartner("tieyi");
    progress.setFlag("arcade_boss_defeated", true);
    progress.setFlag("bad_ending_risk", false);

    StoryProgressSnapshot snapshot = progress.getSnapshot();

    StoryProgress restored;
    restored.loadSnapshot(snapshot);

    TestSupport::require(restored.isPartnerUnlocked("tieyi"), "partner unlock restores");
    TestSupport::require(restored.getFlag("arcade_boss_defeated"), "true flag restores");
    TestSupport::require(!restored.getFlag("bad_ending_risk"), "false flag restores");
}

}  // namespace

int main() {
    chapterStatesAdvanceMonotonically();
    partnerAndFlagsPersistInSnapshot();
    return 0;
}
