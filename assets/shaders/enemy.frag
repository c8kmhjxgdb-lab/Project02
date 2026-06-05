#version 330 core
in vec2 vWorldPos;
uniform vec2 uPosition;      // 敌人世界位置
uniform float uTime;         // 动画时间
uniform vec3 uColor;         // 敌人颜色
uniform float uRadius;       // 半径
uniform int uType;           // 0=Chaser, 1=Shooter, 2=Exploder
uniform float uHealthPercent; // 生命值百分比
out vec4 FragColor;

// 伪噪声函数
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

// SDF圆
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

// SDF等边三角形
float sdTriangle(vec2 p, float r) {
    p.x = abs(p.x);
    vec2 q = vec2(p.x - r * 0.5, p.y + r * 0.288675);
    return length(max(q, 0.0)) - r * 0.288675 + min(max(q.x, q.y), 0.0);
}

// SDF菱形
float sdRhombus(vec2 p, float r) {
    p = abs(p);
    return (p.x * 0.707 + p.y * 0.707 - r * 0.707) / 0.707;
}

void main() {
    vec2 uv = vWorldPos - uPosition;
    float d;

    // 根据类型选择形状
    if (uType == 0) {
        // Chaser: 三角形
        d = sdTriangle(uv, uRadius);
    } else if (uType == 1) {
        // Shooter: 菱形
        d = sdRhombus(uv, uRadius);
    } else {
        // Exploder: 圆形
        d = sdCircle(uv, uRadius);
    }

    // 抗锯齿边缘
    float alpha = 1.0 - smoothstep(0.0, 0.02, d);

    if (alpha < 0.01) discard;

    vec3 color = uColor;

    // 内部发光效果
    float glow = 1.0 - smoothstep(0.0, uRadius * 0.6, length(uv));
    color += vec3(glow * 0.3);

    // Exploder脉冲效果
    if (uType == 2) {
        float pulse = sin(uTime * 6.0) * 0.5 + 0.5;
        color += vec3(1.0, 0.5, 0.0) * pulse * 0.3;

        // 外圈光环
        float ringDist = length(uv) - uRadius;
        float ring = 1.0 - smoothstep(0.0, 0.1, abs(ringDist));
        color += vec3(1.0, 0.6, 0.1) * ring * pulse * 0.4;
    }

    // 受伤闪烁（生命低于30%时）
    if (uHealthPercent < 0.3) {
        float flash = sin(uTime * 10.0) * 0.5 + 0.5;
        color = mix(color, vec3(1.0, 0.0, 0.0), flash * 0.5);
    }

    // 边缘描边
    float edge = smoothstep(0.02, 0.04, abs(d));
    color = mix(color, color * 0.5, edge);

    FragColor = vec4(color, alpha);
}
