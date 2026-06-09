-- 日常对话
return {
    greeting_morning = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "早上好！今天也要加油哦~ ☀️",
        textEn = "Good morning! Let's do our best today.",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "morning_choice"
    },

    morning_choice = {
        speaker = "",
        text = "如何回应？",
        textEn = "How do you respond?",
        choices = {
            {
                text = "「早上好！」",
                textEn = "\"Good morning!\"",
                next = "morning_friendly",
                affectionChange = 5
            },
            {
                text = "「早。」",
                textEn = "\"Morning.\"",
                next = "morning_cold",
                affectionChange = 0
            }
        }
    },

    morning_friendly = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "看到你这么有精神，我也更有干劲了！",
        textEn = "Seeing you so energetic makes me feel motivated too!",
        next = "dialogue_end"
    },

    morning_cold = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "还没睡醒吗？要不要一起去买杯咖啡？",
        textEn = "Still sleepy? Want to grab some coffee together?",
        next = "dialogue_end"
    },

    greeting_afternoon = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "下午好！今天的天气真好呢~",
        textEn = "Good afternoon! The weather is lovely today.",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "dialogue_end"
    },

    greeting_evening = {
        speaker = "小夏",
        speakerEn = "Xia",
        text = "晚上好！忙了一天，该好好休息啦~",
        textEn = "Good evening! After a busy day, it's time to rest.",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "dialogue_end"
    },

    dialogue_end = {
        speaker = "",
        text = "",
        textEn = "",
        next = ""
    }
}
