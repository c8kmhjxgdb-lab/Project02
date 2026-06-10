#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class ChapterState : uint8_t {
    Locked,
    Unlocked,
    InProgress,
    Completed
};

struct ChapterProgressEntry {
    std::string chapterId;
    ChapterState state = ChapterState::Locked;
};

struct StoryFlagEntry {
    std::string flagId;
    bool value = false;
};

struct StoryProgressSnapshot {
    std::vector<ChapterProgressEntry> chapters;
    std::vector<std::string> unlockedPartners;
    std::vector<StoryFlagEntry> flags;
};

bool isValidChapterState(ChapterState state);
bool tryParseChapterState(long long value, ChapterState& outState);

class StoryProgress {
public:
    ChapterState getChapterState(const std::string& chapterId) const;
    void unlockChapter(const std::string& chapterId);
    void startChapter(const std::string& chapterId);
    void completeChapter(const std::string& chapterId);

    bool isPartnerUnlocked(const std::string& partnerId) const;
    void unlockPartner(const std::string& partnerId);

    bool getFlag(const std::string& flagId) const;
    void setFlag(const std::string& flagId, bool value);

    StoryProgressSnapshot getSnapshot() const;
    void loadSnapshot(const StoryProgressSnapshot& snapshot);

private:
    std::unordered_map<std::string, ChapterState> chapters;
    std::unordered_set<std::string> partners;
    std::unordered_map<std::string, bool> flags;
};
