return {
    organize_home_base = {
        name = "整理秘密基地",
        requires = {"simple_bed", "writing_desk", "star_lamp"},
        updateOutsideHomeBase = false,
        reward = { coins = 20, childlikeHeart = 20, unlockFurniture = "toy_shelf" }
    },
    flower_for_princess = {
        name = "给公主的花",
        requires = {"flower_pot"},
        updateOutsideHomeBase = false,
        reward = { coins = 8, childlikeHeart = 10, reduceGrievance = 8 }
    },
    starchild_toy_shelf = {
        name = "星愿玩具架",
        requires = {"toy_shelf", "mini_car"},
        updateOutsideHomeBase = false,
        reward = { coins = 10, childlikeHeart = 18, unlockFurniture = "childhood_poster" }
    },
    rainy_night_talk = {
        name = "雨夜谈心",
        requiresWeather = "rain",
        requiresNight = true,
        requiresTalk = true,
        updateOutsideHomeBase = false,
        reward = { childlikeHeart = 30, reduceGrievance = 25 }
    },
    childlike_heart_alarm = {
        name = "童心警报",
        requiresLowChildlikeHeart = true,
        updateOutsideHomeBase = false,
        reward = { childlikeHeart = 80, reduceGrievance = 15 }
    },
    prologue_star_candy = {
        name = "捡到星星糖",
        objectives = {
            { type = "interact", target = "star_candy", required = 1 },
            { type = "enter_region", target = "real_street_prologue", required = 1 }
        },
        reward = {
            itemRewards = {
                { item = "old_game_coin", count = 1 }
            }
        }
    },
    arcade_trial_tokens = {
        name = "找回试玩币",
        objectives = {
            { type = "collect", target = "trial_token", required = 3 },
            { type = "defeat", target = "popup_bubble", required = 5 }
        },
        reward = {
            coins = 20,
            itemRewards = {
                { item = "old_button", count = 2 }
            }
        }
    }
}
