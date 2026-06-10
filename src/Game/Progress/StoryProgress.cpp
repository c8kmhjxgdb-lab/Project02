#include "Game/Progress/StoryProgress.h"

#include <algorithm>

namespace {

bool isKnownStateAdvance(ChapterState current, ChapterState next) {
    return static_cast<int>(next) > static_cast<int>(current);
}

}  // namespace

bool isValidChapterState(ChapterState state) {
    switch (state) {
        case ChapterState::Locked:
        case ChapterState::Unlocked:
        case ChapterState::InProgress:
        case ChapterState::Completed:
            return true;
        default:
            return false;
    }
}

bool tryParseChapterState(long long value, ChapterState& outState) {
    if (value < static_cast<int>(ChapterState::Locked) ||
        value > static_cast<int>(ChapterState::Completed)) {
        return false;
    }
    outState = static_cast<ChapterState>(value);
    return true;
}

ChapterState StoryProgress::getChapterState(const std::string& chapterId) const {
    auto it = chapters.find(chapterId);
    return it == chapters.end() ? ChapterState::Locked : it->second;
}

void StoryProgress::unlockChapter(const std::string& chapterId) {
    if (chapterId.empty()) return;
    ChapterState current = getChapterState(chapterId);
    if (isKnownStateAdvance(current, ChapterState::Unlocked)) {
        chapters[chapterId] = ChapterState::Unlocked;
    }
}

void StoryProgress::startChapter(const std::string& chapterId) {
    if (chapterId.empty()) return;
    ChapterState current = getChapterState(chapterId);
    if (isKnownStateAdvance(current, ChapterState::InProgress)) {
        chapters[chapterId] = ChapterState::InProgress;
    }
}

void StoryProgress::completeChapter(const std::string& chapterId) {
    if (chapterId.empty()) return;
    chapters[chapterId] = ChapterState::Completed;
}

bool StoryProgress::isPartnerUnlocked(const std::string& partnerId) const {
    return partners.find(partnerId) != partners.end();
}

void StoryProgress::unlockPartner(const std::string& partnerId) {
    if (!partnerId.empty()) {
        partners.insert(partnerId);
    }
}

bool StoryProgress::getFlag(const std::string& flagId) const {
    auto it = flags.find(flagId);
    return it != flags.end() && it->second;
}

void StoryProgress::setFlag(const std::string& flagId, bool value) {
    if (!flagId.empty()) {
        flags[flagId] = value;
    }
}

StoryProgressSnapshot StoryProgress::getSnapshot() const {
    StoryProgressSnapshot snapshot;
    for (const auto& pair : chapters) {
        snapshot.chapters.push_back({pair.first, pair.second});
    }
    std::sort(snapshot.chapters.begin(), snapshot.chapters.end(),
        [](const ChapterProgressEntry& a, const ChapterProgressEntry& b) {
            return a.chapterId < b.chapterId;
        });

    snapshot.unlockedPartners.assign(partners.begin(), partners.end());
    std::sort(snapshot.unlockedPartners.begin(), snapshot.unlockedPartners.end());

    for (const auto& pair : flags) {
        snapshot.flags.push_back({pair.first, pair.second});
    }
    std::sort(snapshot.flags.begin(), snapshot.flags.end(),
        [](const StoryFlagEntry& a, const StoryFlagEntry& b) {
            return a.flagId < b.flagId;
        });
    return snapshot;
}

void StoryProgress::loadSnapshot(const StoryProgressSnapshot& snapshot) {
    chapters.clear();
    partners.clear();
    flags.clear();

    for (const ChapterProgressEntry& chapter : snapshot.chapters) {
        if (!chapter.chapterId.empty() && isValidChapterState(chapter.state)) {
            chapters[chapter.chapterId] = chapter.state;
        }
    }
    for (const std::string& partner : snapshot.unlockedPartners) {
        unlockPartner(partner);
    }
    for (const StoryFlagEntry& flag : snapshot.flags) {
        setFlag(flag.flagId, flag.value);
    }
}
