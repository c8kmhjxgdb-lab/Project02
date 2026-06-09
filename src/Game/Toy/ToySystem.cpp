#include "ToySystem.h"

#include "Game/Data/ToyDefinitionLoader.h"

#include <algorithm>
#include <utility>

void ToySystem::init() {
    definitions = {
        {"mini_car", "模型赛车", "toy_shelf", 8, 12.0f, 5.0f, true},
    };
    collectedToys.clear();
    collectToy("mini_car");
    miniCarActive = false;
    miniCarGame.stop();
    miniCarLastRewardDay = 0;
    miniCarBestTime = 0.0f;
}

bool ToySystem::loadDefinitions(LuaVM& lua, const char* path) {
    std::vector<ToyDef> loaded;
    if (!ToyDefinitionLoader::load(lua, path, loaded)) {
        return false;
    }

    definitions = std::move(loaded);
    if (!hasToy("mini_car")) {
        collectToy("mini_car");
    }
    return true;
}

void ToySystem::collectToy(const std::string& toyId) {
    if (toyId.empty() || hasToy(toyId)) return;
    collectedToys.push_back(toyId);
}

bool ToySystem::hasToy(const std::string& toyId) const {
    return std::find(collectedToys.begin(), collectedToys.end(), toyId) != collectedToys.end();
}

const ToyDef* ToySystem::findToy(const std::string& toyId) const {
    auto it = std::find_if(definitions.begin(), definitions.end(),
        [&toyId](const ToyDef& def) { return def.id == toyId; });
    return it == definitions.end() ? nullptr : &*it;
}

bool ToySystem::hasClaimedMiniCarRewardToday(int currentDay) const {
    return miniCarLastRewardDay == currentDay;
}

bool ToySystem::canStartMiniCar(bool hasToyShelf) const {
    return hasToyShelf && hasToy("mini_car");
}

void ToySystem::startMiniCar(int) {
    if (!hasToy("mini_car")) return;
    miniCarActive = true;
    miniCarGame.start();
}

void ToySystem::stopMiniCar() {
    miniCarActive = false;
    miniCarGame.stop();
}

ToyReward ToySystem::updateMiniCar(float dt,
                                   bool up,
                                   bool down,
                                   bool left,
                                   bool right,
                                   int currentDay) {
    ToyReward reward;
    if (!miniCarActive) return reward;

    MiniCarResult result = miniCarGame.update(dt, up, down, left, right);
    if (result.finished) {
        miniCarActive = false;
        if (miniCarBestTime <= 0.0f || result.finishTime < miniCarBestTime) {
            miniCarBestTime = result.finishTime;
        }
        if (miniCarLastRewardDay != currentDay) {
            const ToyDef* miniCar = findToy("mini_car");
            miniCarLastRewardDay = currentDay;
            reward.coins = miniCar ? miniCar->rewardCoins : 8;
            reward.childlikeHeart = miniCar ? miniCar->rewardChildlikeHeart : 12.0f;
            reward.affection = miniCar ? miniCar->rewardAffection : 5.0f;
            reward.granted = true;
        }
    }

    return reward;
}

void ToySystem::renderMiniCar(const glm::mat4& viewProj) const {
    miniCarGame.render(viewProj);
}

ToySaveData ToySystem::getSaveData() const {
    ToySaveData data;
    data.collectedToys = collectedToys;
    data.miniCarLastRewardDay = miniCarLastRewardDay;
    data.miniCarBestTime = miniCarBestTime;
    return data;
}

void ToySystem::loadSaveData(const ToySaveData& data) {
    collectedToys = data.collectedToys;
    if (!hasToy("mini_car")) {
        collectToy("mini_car");
    }
    miniCarLastRewardDay = data.miniCarLastRewardDay;
    miniCarBestTime = data.miniCarBestTime;
    stopMiniCar();
}
