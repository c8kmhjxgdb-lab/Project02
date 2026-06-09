return {
    organize_home_base = {
        name = "整理秘密基地",
        requires = {"simple_bed", "writing_desk", "star_lamp"},
        reward = { coins = 20, childlikeHeart = 20, unlockFurniture = "toy_shelf" }
    },
    flower_for_princess = {
        name = "给公主的花",
        requires = {"flower_pot"},
        reward = { coins = 8, childlikeHeart = 10, reduceGrievance = 8 }
    },
    starchild_toy_shelf = {
        name = "星愿玩具架",
        requires = {"toy_shelf", "mini_car"},
        reward = { coins = 10, childlikeHeart = 18, unlockFurniture = "childhood_poster" }
    },
    rainy_night_talk = {
        name = "雨夜谈心",
        requiresWeather = "rain",
        requiresNight = true,
        requiresTalk = true,
        reward = { childlikeHeart = 30, reduceGrievance = 25 }
    },
    childlike_heart_alarm = {
        name = "童心警报",
        requiresLowChildlikeHeart = true,
        reward = { childlikeHeart = 80, reduceGrievance = 15 }
    }
}
