#pragma once

#include "MiniCarGame.h"
#include "Game/Toy/ToyTypes.h"

#include <glm/mat4x4.hpp>
#include <vector>

class LuaVM;

class ToySystem {
public:
    void init();
    bool loadDefinitions(LuaVM& lua, const char* path);
    void collectToy(const std::string& toyId);
    bool hasToy(const std::string& toyId) const;
    const std::vector<std::string>& getCollectedToys() const { return collectedToys; }
    const ToyDef* findToy(const std::string& toyId) const;

    bool isMiniCarActive() const { return miniCarActive; }
    float getMiniCarBestTime() const { return miniCarBestTime; }
    bool hasClaimedMiniCarRewardToday(int currentDay) const;
    bool canStartMiniCar(bool hasToyShelf) const;
    void startMiniCar(int currentDay);
    void stopMiniCar();
    ToyReward updateMiniCar(float dt,
                            bool up,
                            bool down,
                            bool left,
                            bool right,
                            int currentDay);
    void renderMiniCar(const glm::mat4& viewProj) const;

    ToySaveData getSaveData() const;
    void loadSaveData(const ToySaveData& data);

private:
    std::vector<std::string> collectedToys;
    std::vector<ToyDef> definitions;
    bool miniCarActive = false;
    MiniCarGame miniCarGame;
    int miniCarLastRewardDay = 0;
    float miniCarBestTime = 0.0f;
};
