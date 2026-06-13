-- 与艾莉娅初次见面
return {
    start = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "我才不需要帮助……只是今天还没吃东西而已。等等，你脖子上那枚旧游戏币，是星星糖变的？",
        textEn = "I do not need help... I just have not eaten today. Wait, that old arcade coin around your neck came from the Star Candy?",
        textColor = {r = 1.0, g = 0.82, b = 0.45},
        next = "self_intro"
    },

    self_intro = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "这里是童心层，重叠在现实街道下面。灰钟局正在把颜色收走。你能看见这里，说明你还没把童心交出去。",
        textEn = "This is the childhood layer beneath the real street. The Gray Clock Bureau is draining its color. If you can see this place, you have not given up your childhood heart.",
        next = "player_choice"
    },

    player_choice = {
        speaker = "",
        text = "星愿要怎么回应？",
        textEn = "How do you respond?",
        choices = {
            {
                text = "「那就一起把颜色抢回来。」（并肩）",
                textEn = "\"Then let's take the colors back together.\" (Together)",
                next = "friendly_response",
                affectionChange = 10
            },
            {
                text = "「我只是捡了颗糖。」（嘴硬）",
                textEn = "\"I only picked up a piece of candy.\" (Guarded)",
                next = "cold_response",
                affectionChange = 2
            },
            {
                text = "「所以我是还没完全变成大人？」（调侃）",
                textEn = "\"So I am not fully grown-up yet?\" (Teasing)",
                next = "tease_response",
                affectionChange = 5
            }
        }
    },

    friendly_response = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "说得像你已经想好计划了一样。先回秘密基地吧，那里能让星星糖的光稳定下来。",
        textEn = "You make it sound like you already have a plan. Let's get to the secret base first; it can stabilize the Star Candy's light.",
        next = "dialogue_end"
    },

    cold_response = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "那是星星糖。它选了你。别急着否认，先跟我去秘密基地，你会看到更多证据。",
        textEn = "That was the Star Candy. It chose you. Do not deny it yet. Follow me to the secret base and you will see more proof.",
        next = "dialogue_end"
    },

    tease_response = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "哼，至少你还会开玩笑。能开玩笑的人，颜色通常还没被收干净。走吧，星愿。",
        textEn = "Hmph. At least you can still joke. People who can joke usually still have some color left. Come on, Xingyuan.",
        next = "dialogue_end"
    },

    base_rain_night = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "外面下雨了呢。今晚就把秘密基地当成小小宇宙，难过的事先放在门外吧。",
        textEn = "It's raining outside. Tonight, let this base be our tiny universe and leave the hard things at the door.",
        textColor = {r = 0.8, g = 0.72, b = 1.0},
        next = "dialogue_end"
    },

    base_low_heart = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "你的星星有点暗了。坐一会儿吧，我陪你把童心一点点找回来。",
        textEn = "Your star looks a little dim. Sit here for a while, and I'll help you find your childhood heart again.",
        textColor = {r = 1.0, g = 0.78, b = 0.60},
        next = "dialogue_end"
    },

    dialogue_end = {
        speaker = "",
        text = "",
        textEn = "",
        next = ""  -- 空next表示对话结束
    }
}
