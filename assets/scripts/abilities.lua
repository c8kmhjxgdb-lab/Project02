-- 技能配置表
abilities = {
    fireball = {
        name = "火球术",
        manaCost = 15,
        cooldown = 0.3,
        projectileSpeed = 400,
        damage = 25,
        lifetime = 2.0,
        particleColor = {r = 1.0, g = 0.4, b = 0.0},
        description = "发射一枚火球，对命中的敌人造成伤害"
    },
    grab = {
        name = "超人力量",
        manaCost = 0,
        cooldown = 0.5,
        description = "抓取并投掷物体"
    }
}

-- 技能快捷键绑定
keyBindings = {
    fireball = "J",
    grab = "K",
    interact = "E",
    heal = "H"
}

-- 情感系统配置
emotionConfig = {
    grievance = {
        hurtAmount = 10,        -- 受伤增加
        deathAmount = 20,       -- 死亡增加
        rejectedAmount = 5,     -- 对话被拒增加
        depressedThreshold = 70, -- 抑郁阈值
        speedMultiplier = 0.7,  -- 抑郁时速度修正
        naturalRecoveryRate = 1.0, -- 在家每分钟恢复
    },
    affection = {
        levels = {
            {name = "陌生人", min = 0, max = 100},
            {name = "熟人", min = 100, max = 300},
            {name = "朋友", min = 300, max = 500},
            {name = "亲密朋友", min = 500, max = 700},
            {name = "心上人", min = 700, max = 1000},
        }
    }
}
