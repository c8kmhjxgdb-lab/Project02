-- 日常对话
return {
    greeting_morning = {
        speaker = "小夏",
        text = "早上好！今天也要加油哦~ ☀️",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "morning_choice"
    },

    morning_choice = {
        speaker = "",
        text = "如何回应？",
        choices = {
            {
                text = "「早上好！」",
                next = "morning_friendly",
                affectionChange = 5
            },
            {
                text = "「早。」",
                next = "morning_cold",
                affectionChange = 0
            }
        }
    },

    morning_friendly = {
        speaker = "小夏",
        text = "看到你这么有精神，我也更有干劲了！",
        next = "dialogue_end"
    },

    morning_cold = {
        speaker = "小夏",
        text = "还没睡醒吗？要不要一起去买杯咖啡？",
        next = "dialogue_end"
    },

    greeting_afternoon = {
        speaker = "小夏",
        text = "下午好！今天的天气真好呢~",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "dialogue_end"
    },

    greeting_evening = {
        speaker = "小夏",
        text = "晚上好！忙了一天，该好好休息啦~",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "dialogue_end"
    },

    dialogue_end = {
        speaker = "",
        text = "",
        next = ""
    }
}
