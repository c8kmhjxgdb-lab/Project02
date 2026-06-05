#version 330 core
in vec2 vWorldPos;
uniform vec2 uPosition;      // 投射物世界位置
uniform float uTime;         // 动画时间
uniform vec3 uColor;         // 基础颜色
uniform float uRadius;       // 半径
uniform int uType;           // 0=火球, 1=冰锥, 2=雷电
out vec4 FragColor;

// 伪噪声函数
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main() {
    vec2 uv = vWorldPos - uPosition;
    float d = length(uv);

    // 基础圆形
    float circle = 1.0 - smoothstep(uRadius * 0.85, uRadius, d);

    vec3 color = uColor;
    float alpha = circle;

    if (uType == 0) {
        // ===== 火球 =====
        // 火焰噪声（随时间变化）
        float flame = noise(uv * 15.0 + uTime * 4.0);
        float fireIntensity = smoothstep(0.3, 0.7, flame) * circle;

        // 颜色：核心白色 -> 橙色 -> 外部红色
        float t = d / uRadius;
        vec3 coreColor = vec3(1.0, 1.0, 0.8);
        vec3 midColor = vec3(1.0, 0.5, 0.0);
        vec3 outerColor = vec3(1.0, 0.2, 0.0);

        color = mix(coreColor, midColor, t);
        color = mix(color, outerColor, t * t);
        color += vec3(fireIntensity * 0.4);

        // 外发光
        float glow = smoothstep(uRadius, uRadius * 1.5, d);
        color += vec3(1.0, 0.3, 0.0) * glow * 0.3;

        alpha = circle * (1.0 - t * 0.3);

    } else if (uType == 1) {
        // ===== 冰锥 =====
        // 冰晶效果
        float crystal = noise(uv * 20.0 - uTime * 2.0);
        float crystalIntensity = smoothstep(0.4, 0.6, crystal) * circle;

        float t = d / uRadius;
        vec3 coreColor = vec3(0.8, 0.9, 1.0);
        vec3 midColor = vec3(0.4, 0.7, 1.0);
        vec3 outerColor = vec3(0.2, 0.4, 0.8);

        color = mix(coreColor, midColor, t);
        color = mix(color, outerColor, t);
        color += vec3(crystalIntensity * 0.3);

        // 边缘锐利
        alpha = 1.0 - smoothstep(uRadius * 0.9, uRadius, d);

    } else {
        // ===== 雷电 =====
        // 闪电脉冲
        float pulse = sin(uTime * 15.0) * 0.5 + 0.5;
        float t = d / uRadius;

        vec3 coreColor = vec3(1.0, 1.0, 1.0);
        vec3 midColor = vec3(0.6, 0.8, 1.0);
        vec3 outerColor = vec3(0.3, 0.5, 1.0);

        color = mix(coreColor, midColor, t);
        color = mix(color, outerColor, t * t);
        color *= (0.7 + pulse * 0.3);

        // 电弧效果
        float arc = noise(uv * 30.0 + uTime * 10.0);
        color += vec3(arc * 0.2 * circle);

        // 外圈光环
        float ring = smoothstep(uRadius * 0.7, uRadius * 1.2, d);
        color += vec3(0.5, 0.7, 1.0) * ring * pulse * 0.5;

        alpha = circle * (0.8 + pulse * 0.2);
    }

    if (alpha < 0.01) discard;

    FragColor = vec4(color, alpha);
}
