#pragma once

#include "Game/Quest/QuestTypes.h"

#include <string>
#include <vector>

class LuaVM;

class QuestSystem {
public:
    void init();
    bool loadDefinitions(LuaVM& lua, const char* path);
    std::vector<QuestReward> update(const QuestSnapshot& snapshot);

    bool isCompleted(const std::string& questId) const;
    const std::vector<std::string>& getCompletedQuests() const { return completedQuests; }
    void loadCompletedQuests(const std::vector<std::string>& quests);
    std::vector<QuestSaveEntry> getSaveData() const;
    void loadSaveData(const std::vector<QuestSaveEntry>& quests);
    QuestState getQuestState(const std::string& questId) const;
    int getRewardedCount() const;
    int getQuestCount() const { return static_cast<int>(definitions.size()); }

private:
    std::vector<std::string> completedQuests;
    std::vector<QuestDef> definitions;
    std::vector<QuestSaveEntry> questEntries;

    QuestSaveEntry& ensureEntry(const QuestDef& quest);
    const QuestSaveEntry* findEntry(const std::string& questId) const;
    bool rewardOnce(const std::string& questId);
    bool isSatisfied(const QuestDef& quest, const QuestSnapshot& snapshot) const;
    std::vector<QuestObjectiveProgress> buildObjectives(const QuestDef& quest, const QuestSnapshot& snapshot) const;
    static bool isRewardedState(QuestState state);
};
