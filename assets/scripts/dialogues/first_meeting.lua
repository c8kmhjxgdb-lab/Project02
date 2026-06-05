-- 与小夏初次见面
return {
    start = {
        speaker = "小夏",
        text = "咦？你就是新来的转学生吗？我是小夏，很高兴认识你！✨",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "self_intro"
    },

    self_intro = {
        speaker = "小夏",
        text = "我在这个学校已经一年了，对这里很熟悉的。有什么不懂的可以随时问我哦~",
        next = "player_choice"
    },

    player_choice = {
        speaker = "",
        text = "你要怎么回应？",
        choices = {
            {
                text = "「你好，请多关照！」（友好）",
                next = "friendly_response",
                affectionChange = 10
            },
            {
                text = "「嗯。」（冷淡）",
                next = "cold_response",
                affectionChange = -5
            },
            {
                text = "「你就是传说中的校花？」（调侃）",
                next = "tease_response",
                affectionChange = 5
            }
        }
    },

    friendly_response = {
        speaker = "小夏",
        text = "嘿嘿，你人真好！以后我们就是朋友啦~",
        next = "dialogue_end"
    },

    cold_response = {
        speaker = "小夏",
        text = "唔...你好像不太爱说话呢。没关系，慢慢来~",
        next = "dialogue_end"
    },

    tease_response = {
        speaker = "小夏",
        text = "哈？谁、谁说的啦！（脸红）别听别人乱说...",
        next = "dialogue_end"
    },

    dialogue_end = {
        speaker = "",
        text = "",
        next = ""  -- 空next表示对话结束
    }
}
