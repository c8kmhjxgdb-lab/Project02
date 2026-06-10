#include "Game/Quest/QuestSystem.h"
#include "TestSupport.h"

#include <string>
#include <vector>

namespace {

QuestSnapshot makeHomeSnapshot() {
    QuestSnapshot snapshot;
    snapshot.inHomeBase = true;
    return snapshot;
}

void outsideHomeBaseDoesNotProgressQuests() {
    QuestSystem quests;
    quests.init();

    QuestSnapshot snapshot = makeHomeSnapshot();
    snapshot.inHomeBase = false;
    snapshot.placedBedCount = 1;
    snapshot.placedDeskCount = 1;
    snapshot.placedLampCount = 1;

    std::vector<QuestReward> rewards = quests.update(snapshot);

    TestSupport::require(rewards.empty(), "quests do not reward outside home base");
    TestSupport::require(
        quests.getQuestState("organize_home_base") == QuestState::Available,
        "outside home base leaves quest available");
}

void satisfiedQuestRewardsOnce() {
    QuestSystem quests;
    quests.init();

    QuestSnapshot snapshot = makeHomeSnapshot();
    snapshot.placedBedCount = 1;
    snapshot.placedDeskCount = 1;
    snapshot.placedLampCount = 1;

    std::vector<QuestReward> firstRewards = quests.update(snapshot);
    std::vector<QuestReward> secondRewards = quests.update(snapshot);

    TestSupport::require(firstRewards.size() == 1, "organize quest grants one reward");
    TestSupport::require(firstRewards.front().questId == "organize_home_base", "organize quest reward id");
    TestSupport::require(firstRewards.front().unlockFurniture == "toy_shelf", "organize quest unlocks toy shelf");
    TestSupport::require(secondRewards.empty(), "organize quest does not reward twice");
    TestSupport::require(
        quests.getQuestState("organize_home_base") == QuestState::Rewarded,
        "organize quest enters rewarded state");
    TestSupport::require(quests.isCompleted("organize_home_base"), "organize quest is completed");
}

void saveDataRestoresRewardedQuests() {
    QuestSystem quests;
    quests.init();

    QuestSnapshot snapshot = makeHomeSnapshot();
    snapshot.placedBedCount = 1;
    snapshot.placedDeskCount = 1;
    snapshot.placedLampCount = 1;
    quests.update(snapshot);

    std::vector<QuestSaveEntry> saveData = quests.getSaveData();

    QuestSystem restored;
    restored.init();
    restored.loadSaveData(saveData);

    TestSupport::require(
        restored.getQuestState("organize_home_base") == QuestState::Rewarded,
        "rewarded quest state restores from save data");
    TestSupport::require(restored.isCompleted("organize_home_base"), "completed quest restores from save data");
    TestSupport::require(restored.getRewardedCount() == 1, "rewarded count restores from save data");
    TestSupport::require(restored.update(snapshot).empty(), "restored rewarded quest does not reward again");
}

void activeQuestTracksObjectiveProgress() {
    QuestSystem quests;
    quests.init();

    QuestSnapshot snapshot = makeHomeSnapshot();
    snapshot.placedBedCount = 1;

    std::vector<QuestReward> rewards = quests.update(snapshot);
    std::vector<QuestSaveEntry> saveData = quests.getSaveData();

    const QuestSaveEntry* organizeEntry = nullptr;
    for (const QuestSaveEntry& entry : saveData) {
        if (entry.id == "organize_home_base") {
            organizeEntry = &entry;
            break;
        }
    }

    TestSupport::require(rewards.empty(), "partial organize quest grants no reward");
    TestSupport::require(organizeEntry != nullptr, "organize quest save entry exists");
    TestSupport::require(organizeEntry->state == QuestState::Active, "partial organize quest becomes active");

    bool bedComplete = false;
    bool deskIncomplete = false;
    bool lampIncomplete = false;
    for (const QuestObjectiveProgress& objective : organizeEntry->objectives) {
        if (objective.targetId == "simple_bed") bedComplete = objective.current == objective.required;
        if (objective.targetId == "writing_desk") deskIncomplete = objective.current == 0;
        if (objective.targetId == "star_lamp") lampIncomplete = objective.current == 0;
    }

    TestSupport::require(bedComplete, "partial organize quest tracks completed bed objective");
    TestSupport::require(deskIncomplete, "partial organize quest tracks missing desk objective");
    TestSupport::require(lampIncomplete, "partial organize quest tracks missing lamp objective");
}

}  // namespace

int main() {
    outsideHomeBaseDoesNotProgressQuests();
    satisfiedQuestRewardsOnce();
    saveDataRestoresRewardedQuests();
    activeQuestTracksObjectiveProgress();
    return 0;
}
