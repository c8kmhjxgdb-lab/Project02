#include "Game/Emotion/EmotionSystem.h"
#include "TestSupport.h"

#include <string>

namespace {

void requireTier(float childlikeHeart, ChildlikeHeartTier expectedTier,
                 const std::string& expectedName, bool expectedHiddenPickups) {
    EmotionSystem emotions;
    emotions.setChildlikeHeart(childlikeHeart);

    TestSupport::require(emotions.getChildlikeHeartTier() == expectedTier, "childlike heart tier matches threshold");
    TestSupport::require(emotions.getChildlikeHeartTierName() == expectedName, "childlike heart tier name matches");
    TestSupport::require(emotions.canSeeHiddenPickups() == expectedHiddenPickups,
        "hidden pickup visibility matches tier");
}

void tiersFollowBoundaryThresholds() {
    requireTier(199.99f, ChildlikeHeartTier::Faded, "失色", false);
    requireTier(200.0f, ChildlikeHeartTier::Normal, "寻常", false);
    requireTier(499.99f, ChildlikeHeartTier::Normal, "寻常", false);
    requireTier(500.0f, ChildlikeHeartTier::Vivid, "鲜活", true);
    requireTier(799.99f, ChildlikeHeartTier::Vivid, "鲜活", true);
    requireTier(800.0f, ChildlikeHeartTier::Radiant, "绚烂", true);
}

}  // namespace

int main() {
    tiersFollowBoundaryThresholds();
    return 0;
}
