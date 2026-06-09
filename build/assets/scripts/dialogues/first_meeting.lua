-- 与小夏初次见面
return {
    start = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "咦？你就是新来的转学生吗？我是小夏，很高兴认识你！✨",
        textEn = "Oh? Are you the new transfer student? I'm Xia. Nice to meet you!",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "self_intro"
    },

    self_intro = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "我在这个学校已经一年了，对这里很熟悉的。有什么不懂的可以随时问我哦~",
        textEn = "I've been at this school for a year, so I know it well. Ask me anytime if you need help.",
        next = "player_choice"
    },

    player_choice = {
        speaker = "",
        text = "你要怎么回应？",
        textEn = "How do you respond?",
        choices = {
            {
                text = "「你好，请多关照！」（友好）",
                textEn = "\"Hi, nice to meet you!\" (Friendly)",
                next = "friendly_response",
                affectionChange = 10
            },
            {
                text = "「嗯。」（冷淡）",
                textEn = "\"Yeah.\" (Distant)",
                next = "cold_response",
                affectionChange = -5
            },
            {
                text = "「你就是传说中的校花？」（调侃）",
                textEn = "\"So you're the famous school beauty?\" (Teasing)",
                next = "tease_response",
                affectionChange = 5
            }
        }
    },

    friendly_response = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "嘿嘿，你人真好！以后我们就是朋友啦~",
        textEn = "Hehe, you're really kind. We're friends from now on!",
        next = "dialogue_end"
    },

    cold_response = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "唔...你好像不太爱说话呢。没关系，慢慢来~",
        textEn = "Hmm... you don't seem to talk much. That's okay, we can take it slowly.",
        next = "dialogue_end"
    },

    tease_response = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "哈？谁、谁说的啦！（脸红）别听别人乱说...",
        textEn = "Huh? W-who said that? Don't listen to random rumors...",
        next = "dialogue_end"
    },

    base_rain_night = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "外面下雨了呢。今晚就把秘密基地当成小小宇宙，难过的事先放在门外吧。",
        textEn = "It's raining outside. Tonight, let this base be our tiny universe and leave the hard things at the door.",
        textColor = {r = 0.8, g = 0.72, b = 1.0},
        next = "dialogue_end"
    },

    base_low_heart = {
        speaker = "小夏",
        speakerEn = "Xia",
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
