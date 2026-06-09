#include "QuestSystem.h"

#include "Game/Data/QuestDefinitionLoader.h"

#include <algorithm>
#include <utility>

static std::vector<QuestDef> createDefaultQuestDefs() {
    return {
        {"organize_home_base", "整理秘密基地",
            {"simple_bed", "writing_desk", "star_lamp"}, false, false, false, false, false,
            {"organize_home_base", 20, 20.0f, 5.0f, 5.0f, "toy_shelf", true}},
        {"flower_for_princess", "给公主的花",
            {"flower_pot"}, false, false, false, false, false,
            {"flower_for_princess", 8, 10.0f, 8.0f, 8.0f, "", true}},
        {"starchild_toy_shelf", "星愿玩具架",
            {"toy_shelf"}, true, false, false, false, false,
            {"starchild_toy_shelf", 10, 18.0f, 0.0f, 5.0f, "childhood_poster", true}},
        {"rainy_night_talk", "雨夜谈心",
            {}, false, true, true, false, true,
            {"rainy_night_talk", 0, 30.0f, 25.0f, 15.0f, "", true}},
        {"childlike_heart_alarm", "童心警报",
            {"simple_bed"}, false, false, false, true, false,
            {"childlike_heart_alarm", 0, 80.0f, 15.0f, 0.0f, "", true}},
    };
}

void QuestSystem::init() {
    completedQuests.clear();
    definitions = createDefaultQuestDefs();
    questEntries.clear();
    for (const QuestDef& quest : definitions) {
        ensureEntry(quest);
    }
}

bool QuestSystem::loadDefinitions(LuaVM& lua, const char* path) {
    std::vector<QuestDef> loaded;
    if (!QuestDefinitionLoader::load(lua, path, loaded)) {
        return false;
    }

    definitions = std::move(loaded);
    questEntries.clear();
    for (const QuestDef& quest : definitions) {
        ensureEntry(quest);
    }
    return true;
}

bool QuestSystem::isCompleted(const std::string& questId) const {
    return std::find(completedQuests.begin(), completedQuests.end(), questId) != completedQuests.end();
}

void QuestSystem::loadCompletedQuests(const std::vector<std::string>& quests) {
    completedQuests.clear();
    questEntries.clear();

    for (const QuestDef& quest : definitions) {
        QuestSaveEntry& entry = ensureEntry(quest);
        if (std::find(quests.begin(), quests.end(), quest.id) != quests.end()) {
            completedQuests.push_back(quest.id);
            entry.state = QuestState::Rewarded;
            entry.rewardClaimed = true;
        }
    }
}

std::vector<QuestSaveEntry> QuestSystem::getSaveData() const {
    return questEntries;
}

void QuestSystem::loadSaveData(const std::vector<QuestSaveEntry>& quests) {
    questEntries.clear();
    completedQuests.clear();

    for (const QuestDef& quest : definitions) {
        QuestSaveEntry entry;
        entry.id = quest.id;
        entry.state = QuestState::Available;
        entry.objectives = buildObjectives(quest, QuestSnapshot{});
        entry.rewardClaimed = false;

        auto saved = std::find_if(quests.begin(), quests.end(),
            [&quest](const QuestSaveEntry& candidate) { return candidate.id == quest.id; });
        if (saved != quests.end()) {
            entry = *saved;
            if (entry.id.empty()) entry.id = quest.id;
        }

        if (isRewardedState(entry.state) || entry.rewardClaimed) {
            entry.state = QuestState::Rewarded;
            entry.rewardClaimed = true;
            completedQuests.push_back(quest.id);
        }
        questEntries.push_back(entry);
    }
}

QuestState QuestSystem::getQuestState(const std::string& questId) const {
    const QuestSaveEntry* entry = findEntry(questId);
    return entry ? entry->state : QuestState::Hidden;
}

int QuestSystem::getRewardedCount() const {
    int count = 0;
    for (const QuestSaveEntry& entry : questEntries) {
        if (isRewardedState(entry.state) || entry.rewardClaimed) {
            ++count;
        }
    }
    return count;
}

QuestSaveEntry& QuestSystem::ensureEntry(const QuestDef& quest) {
    auto it = std::find_if(questEntries.begin(), questEntries.end(),
        [&quest](const QuestSaveEntry& entry) { return entry.id == quest.id; });
    if (it != questEntries.end()) {
        return *it;
    }

    QuestSaveEntry entry;
    entry.id = quest.id;
    entry.state = QuestState::Available;
    entry.objectives = buildObjectives(quest, QuestSnapshot{});
    entry.rewardClaimed = false;
    questEntries.push_back(entry);
    return questEntries.back();
}

const QuestSaveEntry* QuestSystem::findEntry(const std::string& questId) const {
    auto it = std::find_if(questEntries.begin(), questEntries.end(),
        [&questId](const QuestSaveEntry& entry) { return entry.id == questId; });
    return it == questEntries.end() ? nullptr : &*it;
}

bool QuestSystem::rewardOnce(const std::string& questId) {
    if (isCompleted(questId)) return false;
    completedQuests.push_back(questId);
    return true;
}

std::vector<QuestReward> QuestSystem::update(const QuestSnapshot& snapshot) {
    std::vector<QuestReward> rewards;
    if (!snapshot.inHomeBase) return rewards;

    for (const QuestDef& quest : definitions) {
        QuestSaveEntry& entry = ensureEntry(quest);
        if (entry.rewardClaimed) continue;

        entry.objectives = buildObjectives(quest, snapshot);
        entry.state = QuestState::Active;

        if (isSatisfied(quest, snapshot) && rewardOnce(quest.id)) {
            entry.state = QuestState::Rewarded;
            entry.rewardClaimed = true;
            rewards.push_back(quest.reward);
        }
    }

    return rewards;
}

bool QuestSystem::isSatisfied(const QuestDef& quest, const QuestSnapshot& snapshot) const {
    for (const std::string& required : quest.requiredFurniture) {
        int count = 0;
        if (required == "simple_bed") count = snapshot.placedBedCount;
        else if (required == "writing_desk") count = snapshot.placedDeskCount;
        else if (required == "star_lamp") count = snapshot.placedLampCount;
        else if (required == "flower_pot") count = snapshot.placedFlowerPotCount;
        else if (required == "toy_shelf") count = snapshot.placedToyShelfCount;
        if (count <= 0) return false;
    }

    if (quest.requiresMiniCar && !snapshot.miniCarCollected) return false;
    if (quest.requiresRainy && !snapshot.isRainy) return false;
    if (quest.requiresNight && !snapshot.isNight) return false;
    if (quest.requiresLowChildlikeHeart &&
        snapshot.childlikeHeart >= snapshot.lowChildlikeHeartThreshold) return false;
    if (quest.requiresTalk && !snapshot.talkedWithPrincessAtBase) return false;
    return true;
}

std::vector<QuestObjectiveProgress> QuestSystem::buildObjectives(const QuestDef& quest,
                                                                 const QuestSnapshot& snapshot) const {
    std::vector<QuestObjectiveProgress> objectives;

    for (const std::string& required : quest.requiredFurniture) {
        QuestObjectiveProgress objective;
        objective.type = "place_furniture";
        objective.targetId = required;
        objective.required = 1;
        if (required == "simple_bed") objective.current = snapshot.placedBedCount;
        else if (required == "writing_desk") objective.current = snapshot.placedDeskCount;
        else if (required == "star_lamp") objective.current = snapshot.placedLampCount;
        else if (required == "flower_pot") objective.current = snapshot.placedFlowerPotCount;
        else if (required == "toy_shelf") objective.current = snapshot.placedToyShelfCount;
        else objective.current = 0;
        objectives.push_back(objective);
    }

    if (quest.requiresMiniCar) {
        objectives.push_back({"collect_toy", "mini_car", 1, snapshot.miniCarCollected ? 1 : 0});
    }
    if (quest.requiresRainy || quest.requiresNight) {
        objectives.push_back({"weather_time", "Rain:Night", 1,
            (!quest.requiresRainy || snapshot.isRainy) &&
            (!quest.requiresNight || snapshot.isNight) ? 1 : 0});
    }
    if (quest.requiresLowChildlikeHeart) {
        objectives.push_back({"childlike_low", "childlikeHeart", 1,
            snapshot.childlikeHeart < snapshot.lowChildlikeHeartThreshold ? 1 : 0});
    }
    if (quest.requiresTalk) {
        objectives.push_back({"talk", "active_princess", 1,
            snapshot.talkedWithPrincessAtBase ? 1 : 0});
    }

    return objectives;
}

bool QuestSystem::isRewardedState(QuestState state) {
    return state == QuestState::Rewarded;
}
