#pragma once

struct EmotionConfig {
    float childlikeHeartMin = 0.0f;
    float childlikeHeartMax = 1000.0f;
    float childlikeHeartLowThreshold = 200.0f;
    float childlikeHeartLowSpeedMultiplier = 0.7f;
    float initialChildlikeHeart = 950.0f;
    bool hasChildlikeHeartConfig = false;

    float grievanceDepressedThreshold = 70.0f;
    float grievanceExtremeThreshold = 90.0f;
    float homeGrievanceRecoveryPerMinute = 1.0f;
    float ventCooldownDuration = 5.0f;
};
