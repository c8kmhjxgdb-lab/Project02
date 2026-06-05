#version 330 core
in vec3 vColor;
in float vAlpha;
in float vSize;
out vec4 FragColor;

void main() {
    // 圆形软粒子
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);

    if (dist > 0.5) discard;

    // 软边缘
    float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
    alpha *= vAlpha;

    // 中心更亮
    vec3 color = vColor * (1.0 + (1.0 - dist * 2.0) * 0.5);

    FragColor = vec4(color, alpha);
}
