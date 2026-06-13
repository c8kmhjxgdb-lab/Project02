-- 日常对话
return {
    greeting_morning = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "早呀。旧街今天的风很轻，正适合出去捡点还能修的老零件。",
        textEn = "Morning. The wind on the old street is light today. Good weather for salvaging parts we can still repair.",
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
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "有你这句话，今天连基地角落那台旧机器都像是能修好了。",
        textEn = "With that kind of energy, even the broken machine in the corner of the base feels repairable today.",
        next = "dialogue_end"
    },

    morning_cold = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "还没醒透吗？那先在基地门口站一会儿，晨风会把困意吹走。",
        textEn = "Still half asleep? Stand by the base entrance for a moment. The morning wind will clear your head.",
        next = "dialogue_end"
    },

    greeting_afternoon = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "下午好。阳光正好照进基地门口，整理今天捡回来的东西最合适。",
        textEn = "Good afternoon. The sunlight is hitting the base entrance just right. It's a good time to sort what we brought back today.",
        textColor = {r = 0.9, g = 0.6, b = 0.8},
        next = "dialogue_end"
    },

    greeting_evening = {
        speaker = "艾莉娅",
        speakerEn = "Alya",
        text = "晚上好。街机厅那边的霓虹又亮了，忙完这一轮就回基地歇一会儿吧。",
        textEn = "Good evening. The neon over the arcade is glowing again. Once we're done here, let's head back to the base and rest.",
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
