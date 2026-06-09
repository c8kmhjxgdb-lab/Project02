return {
    defaultInterval = 300,
    regions = {
        default = {
            indoor = false,
            allowParticles = true,
            weights = {
                Clear = 40,
                Cloudy = 20,
                Rain = 15,
                Fog = 10,
                HeavyRain = 10,
                Snow = 5
            }
        },
        home_base = {
            indoor = true,
            allowParticles = false,
            weights = {
                Clear = 55,
                Cloudy = 20,
                Rain = 15,
                Fog = 5,
                Snow = 5
            }
        },
        nursery_rhyme_village = {
            indoor = false,
            allowParticles = true,
            specialTag = "NoiseStorm",
            weights = {
                Clear = 30,
                Cloudy = 20,
                Rain = 20,
                HeavyRain = 20,
                Fog = 10
            }
        }
    },
    mood = {
        Rain = { grievanceAtBase = -1, stressOutside = 1 },
        Fog = { stressOutside = 2 },
        Snow = { joy = 1 },
        HeavyRain = { stressOutside = 3 }
    }
}
