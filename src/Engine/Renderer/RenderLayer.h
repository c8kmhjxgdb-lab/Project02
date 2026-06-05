#pragma once

/**
 * 渲染图层（按绘制顺序）
 */
enum class RenderLayer : int {
    Ground = 0,      // 地面瓦片层
    Water = 1,       // 水面层（半透明）
    DecorLow = 2,    // 低矮装饰（花草）
    Objects = 3,     // 可交互对象 + 角色（按 y 排序）
    DecorHigh = 4,   // 高大装饰（树木）
    Effects = 5,     // 粒子特效
    UI = 6,          // UI层
    Count
};

/**
 * 可渲染实体排序键
 */
struct Renderable {
    float y;              // 世界 y 坐标（排序键）
    RenderLayer layer;

    bool operator<(const Renderable& other) const {
        if (layer != other.layer)
            return static_cast<int>(layer) < static_cast<int>(other.layer);
        return y < other.y;
    }
};
