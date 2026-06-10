#include "Game/Quest/QuestSystem.h"
#include "TestSupport.h"

#include <vector>

namespace {

QuestDef makeObjectiveQuest() {
    QuestDef quest;
    quest.id = "arcade_trial_tokens";
    quest.name = "找回试玩币";
    quest.objectives.push_back({"collect", "trial_token", 3});
    quest.objectives.push_back({"defeat", "popup_bubble", 5});
    quest.reward.questId = quest.id;
    quest.reward.coins = 20;
    quest.reward.itemRewards.push_back({"old_button", 2});
    quest.reward.completed = true;
    return quest;
}

const QuestSaveEntry* findEntry(const std::vector<QuestSaveEntry>& entries, const std::string& id) {
    for (const QuestSaveEntry& entry : entries) {
        if (entry.id == id) return &entry;
    }
    return nullptr;
}

const QuestObjectiveProgress* findObjective(const QuestSaveEntry& entry,
                                            const std::string& type,
                                            const std::string& targetId) {
    for (const QuestObjectiveProgress& objective : entry.objectives) {
        if (objective.type == type && objective.targetId == targetId) return &objective;
    }
    return nullptr;
}

void objectiveQuestTracksProgressAndRewardsOnce() {
    QuestSystem quests;
    quests.initWithDefinitions({makeObjectiveQuest()});

    QuestSnapshot partialSnapshot;
    partialSnapshot.inHomeBase = false;
    partialSnapshot.facts.push_back({"collect", "trial_token", 2});
    partialSnapshot.facts.push_back({"collect", "trial_token", 1});
    partialSnapshot.facts.push_back({"defeat", "popup_bubble", 4});

    std::vector<QuestReward> partialRewards = quests.update(partialSnapshot);
    std::vector<QuestSaveEntry> partialSaveData = quests.getSaveData();
    const QuestSaveEntry* partialEntry = findEntry(partialSaveData, "arcade_trial_tokens");

    TestSupport::require(partialRewards.empty(), "partial objective quest grants no reward");
    TestSupport::require(partialEntry != nullptr, "partial objective quest save entry exists");
    TestSupport::require(partialEntry->state == QuestState::Active, "partial objective quest becomes active outside base");

    const QuestObjectiveProgress* collectProgress = findObjective(*partialEntry, "collect", "trial_token");
    const QuestObjectiveProgress* defeatProgress = findObjective(*partialEntry, "defeat", "popup_bubble");
    TestSupport::require(collectProgress != nullptr, "collect objective progress exists");
    TestSupport::require(defeatProgress != nullptr, "defeat objective progress exists");
    TestSupport::require(collectProgress->current == 2, "collect objective tracks max matching fact count");
    TestSupport::require(defeatProgress->current == 4, "defeat objective tracks matching fact count");

    QuestSnapshot completeSnapshot;
    completeSnapshot.inHomeBase = false;
    completeSnapshot.facts.push_back({"collect", "trial_token", 3});
    completeSnapshot.facts.push_back({"defeat", "popup_bubble", 5});

    std::vector<QuestReward> firstRewards = quests.update(completeSnapshot);
    std::vector<QuestReward> secondRewards = quests.update(completeSnapshot);

    TestSupport::require(firstRewards.size() == 1, "complete objective quest grants one reward");
    TestSupport::require(firstRewards.front().questId == "arcade_trial_tokens", "objective quest reward id");
    TestSupport::require(firstRewards.front().coins == 20, "objective quest grants coin reward");
    TestSupport::require(firstRewards.front().itemRewards.size() == 1, "objective quest grants item reward");
    TestSupport::require(firstRewards.front().itemRewards.front().itemId == "old_button", "objective quest reward item id");
    TestSupport::require(firstRewards.front().itemRewards.front().count == 2, "objective quest reward item count");
    TestSupport::require(secondRewards.empty(), "objective quest does not reward twice");
    TestSupport::require(
        quests.getQuestState("arcade_trial_tokens") == QuestState::Rewarded,
        "objective quest enters rewarded state");
    TestSupport::require(quests.isCompleted("arcade_trial_tokens"), "objective quest is completed");
}

void trackedQuestTextShowsFirstIncompleteObjective() {
    QuestSystem quests;
    quests.initWithDefinitions({makeObjectiveQuest()});

    QuestSnapshot snapshot;
    snapshot.facts.push_back({"collect", "trial_token", 3});
    snapshot.facts.push_back({"defeat", "popup_bubble", 2});
    quests.update(snapshot);

    TestSupport::require(
        quests.getTrackedQuestText() == "找回试玩币: popup_bubble 2/5",
        "tracked quest shows first incomplete objective");
}

}  // namespace

int main() {
    objectiveQuestTracksProgressAndRewardsOnce();
    trackedQuestTextShowsFirstIncompleteObjective();
    return 0;
}
