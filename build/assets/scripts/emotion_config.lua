-- 情感系统配置
emotionConfig = {
    grievance = {
        hurtAmount = 10,        -- 受伤增加
        deathAmount = 20,       -- 死亡增加
        rejectedAmount = 5,     -- 对话被拒增加
        depressedThreshold = 70, -- 抑郁阈值
        speedMultiplier = 0.7,  -- 抑郁时速度修正
        naturalRecoveryRate = 1.0, -- 在家每分钟恢复
        ventCooldown = 5.0,     -- 宣泄冷却时间
    },
    affection = {
        interactBonus = 2,      -- 交互增加
        levels = {
            {name = "陌生人", min = 0, max = 100},
            {name = "熟人", min = 100, max = 300},
            {name = "朋友", min = 300, max = 500},
            {name = "亲密朋友", min = 500, max = 700},
            {name = "心上人", min = 700, max = 1000},
        }
    },
    home = {
        positionX = 3.0,
        positionY = 2.0,
        radius = 3.0
    }
}
