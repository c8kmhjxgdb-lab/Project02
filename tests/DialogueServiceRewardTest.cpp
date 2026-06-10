#include "Engine/Renderer/DialogueUI.h"
#include "Game/Building/BuildingSystem.h"
#include "Game/Emotion/EmotionSystem.h"
#include "Game/Emotion/VentAnimation.h"
#include "Game/Services/DialogueService.h"
#include "Game/Services/WorldQuery.h"
#include "Game/Social/DialogueTree.h"
#include "Game/Social/Princess.h"
#include "Game/World/RegionManager.h"
#include "Game/World/TimeSystem.h"
#include "Game/World/WeatherSystem.h"
#include "TestSupport.h"

#include <box2d/box2d.h>

#include <memory>
#include <string>
#include <vector>

namespace {

struct NoticeRecorder {
    std::vector<std::string> notices;

    DialogueService::NoticeSink makeSink() {
        return {
            [this](const std::string& notice) {
                notices.push_back(notice);
            }
        };
    }
};

struct Fixture {
    b2WorldId worldId = b2_nullWorldId;
    RegionManager regionManager;
    BuildingSystem buildingSystem;
    DialogueTree dialogueTree;
    DialogueUI dialogueUI;
    TimeSystem timeSystem;
    EmotionSystem emotionSystem;
    WeatherSystem weatherSystem;
    VentAnimation ventAnimation;
    std::unique_ptr<Princess> princess;
    glm::vec2 homePosition{0.0f, 0.0f};
    float homeRadius = 3.0f;
    bool isVenting = false;
    bool talkedWithPrincessAtBaseThisFrame = false;

    Fixture() {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, 0.0f};
        worldId = b2CreateWorld(&worldDef);

        regionManager.setWorldId(worldId);
        regionManager.setTransitionEffectEnabled(false);
        regionManager.init();
        regionManager.transitionTo("home_base", glm::ivec2(9, 11), worldId);

        buildingSystem.init(worldId);
        timeSystem.init(10.0f);

        princess = std::make_unique<Princess>("Princess");
        princess->createBody(worldId, glm::vec2(8.5f, 6.5f));
    }

    ~Fixture() {
        princess.reset();
        regionManager.shutdown();
        buildingSystem.shutdown();
        if (b2World_IsValid(worldId)) {
            b2DestroyWorld(worldId);
        }
    }

    DialogueService::Context makeContext() {
        return {
            buildingSystem,
            dialogueTree,
            dialogueUI,
            regionManager,
            timeSystem,
            emotionSystem,
            weatherSystem,
            ventAnimation,
            princess,
            homePosition,
            homeRadius,
            isVenting,
            talkedWithPrincessAtBaseThisFrame
        };
    }
};

void placeSimpleBed(BuildingSystem& buildingSystem) {
    FurnitureInstance bed;
    bed.instanceId = 1;
    bed.defId = "simple_bed";
    bed.tile = glm::ivec2(7, 6);
    bed.rotation = 0;
    buildingSystem.loadInstances({bed});
}

bool containsNotice(const NoticeRecorder& recorder, const std::string& text) {
    for (const std::string& notice : recorder.notices) {
        if (notice.find(text) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void restAtBaseBedAppliesRewardsAndNotice() {
    Fixture fixture;
    placeSimpleBed(fixture.buildingSystem);
    fixture.timeSystem.setHour(22.0f);
    fixture.emotionSystem.setChildlikeHeart(900.0f);
    fixture.emotionSystem.setGrievance(40.0f);
    fixture.princess->affection = 0.0f;

    NoticeRecorder notices;
    DialogueService::Context context = fixture.makeContext();

    bool rested = DialogueService::tryRestAtBaseBed(
        context,
        glm::vec2(7.5f, 6.5f),
        notices.makeSink());

    TestSupport::require(rested, "base bed rest succeeds near simple bed");
    TestSupport::require(fixture.timeSystem.getHour() == fixture.timeSystem.getRestUntilHour(), "rest advances to configured hour");
    TestSupport::require(fixture.timeSystem.getDay() == 2, "rest across night advances day");
    TestSupport::require(fixture.emotionSystem.getState().childlikeHeart > 900.0f, "rest increases childlike heart");
    TestSupport::require(fixture.emotionSystem.getState().grievance < 40.0f, "rest reduces grievance");
    TestSupport::require(fixture.princess->affection == 2.0f, "rest increases princess affection");
    TestSupport::require(containsNotice(notices, "床铺休息"), "rest emits notice");
}

void rainyNightPrincessTalkAppliesRewards() {
    Fixture fixture;
    fixture.timeSystem.setHour(21.0f);
    fixture.weatherSystem.setWeatherImmediate(WeatherType::Rain, 1.0f);
    fixture.emotionSystem.setChildlikeHeart(500.0f);
    fixture.emotionSystem.setGrievance(30.0f);
    fixture.princess->affection = 0.0f;

    NoticeRecorder notices;
    DialogueService::Context context = fixture.makeContext();

    bool started = DialogueService::tryStartPrincessDialogue(
        context,
        glm::vec2(8.0f, 6.0f),
        notices.makeSink());

    TestSupport::require(started, "rainy night princess dialogue starts");
    TestSupport::require(fixture.talkedWithPrincessAtBaseThisFrame, "rainy night marks princess talk for quests");
    TestSupport::require(fixture.emotionSystem.getState().childlikeHeart == 510.0f, "rainy night increases childlike heart");
    TestSupport::require(fixture.emotionSystem.getState().grievance == 18.0f, "rainy night reduces grievance");
    TestSupport::require(fixture.princess->affection == 3.0f, "rainy night increases affection");
    TestSupport::require(containsNotice(notices, "雨夜谈心"), "rainy night emits notice");
}

void lowChildlikePrincessTalkAppliesComfortRewards() {
    Fixture fixture;
    fixture.timeSystem.setHour(12.0f);
    fixture.weatherSystem.setWeatherImmediate(WeatherType::Clear, 0.0f);
    fixture.emotionSystem.setChildlikeHeart(100.0f);
    fixture.emotionSystem.setGrievance(20.0f);
    fixture.princess->affection = 0.0f;

    NoticeRecorder notices;
    DialogueService::Context context = fixture.makeContext();

    bool started = DialogueService::tryStartPrincessDialogue(
        context,
        glm::vec2(8.0f, 6.0f),
        notices.makeSink());

    TestSupport::require(started, "low childlike princess dialogue starts");
    TestSupport::require(fixture.talkedWithPrincessAtBaseThisFrame, "low childlike marks princess talk for quests");
    TestSupport::require(fixture.emotionSystem.getState().childlikeHeart == 116.0f, "low childlike increases childlike heart");
    TestSupport::require(fixture.emotionSystem.getState().grievance == 12.0f, "low childlike reduces grievance");
    TestSupport::require(fixture.princess->affection == 2.0f, "low childlike increases affection");
    TestSupport::require(containsNotice(notices, "低童心安抚"), "low childlike emits notice");
}

void ventClearsGrievanceAndStartsAnimation() {
    Fixture fixture;
    fixture.emotionSystem.setGrievance(60.0f);

    NoticeRecorder notices;
    DialogueService::Context context = fixture.makeContext();

    bool vented = DialogueService::tryVent(
        context,
        glm::vec2(8.0f, 6.0f),
        notices.makeSink());

    TestSupport::require(vented, "vent succeeds in home base when grievance is high");
    TestSupport::require(fixture.isVenting, "vent sets venting flag");
    TestSupport::require(fixture.ventAnimation.isActive(), "vent starts animation");
    TestSupport::require(fixture.emotionSystem.getState().grievance == 0.0f, "vent clears grievance");
    TestSupport::require(containsNotice(notices, "宣泄"), "vent emits notice");
}

}  // namespace

int main() {
    restAtBaseBedAppliesRewardsAndNotice();
    rainyNightPrincessTalkAppliesRewards();
    lowChildlikePrincessTalkAppliesComfortRewards();
    ventClearsGrievanceAndStartsAnimation();
    return 0;
}
