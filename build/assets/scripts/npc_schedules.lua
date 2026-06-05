-- NPC日程配置
npcSchedules = {
    xiaoxia = {
        name = "小夏",
        color = {r = 0.9, g = 0.6, b = 0.8},
        schedule = {
            {startTime = 6.0,  endTime = 8.0,  posX = 10, posY = 5,  action = "idle",  dialogue = ""},
            {startTime = 8.0,  endTime = 12.0, posX = 12, posY = 8,  action = "walk",  dialogue = "daily_greetings"},
            {startTime = 12.0, endTime = 14.0, posX = 10, posY = 5,  action = "idle",  dialogue = ""},
            {startTime = 14.0, endTime = 18.0, posX = 15, posY = 10, action = "walk",  dialogue = ""},
            {startTime = 18.0, endTime = 22.0, posX = 3,  posY = 2,  action = "idle",  dialogue = ""},
            {startTime = 22.0, endTime = 24.0, posX = 3,  posY = 2,  action = "idle",  dialogue = ""},
            {startTime = 0.0,  endTime = 6.0,  posX = 3,  posY = 2,  action = "idle",  dialogue = ""},
        }
    }
}
