#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aVel;
layout(location = 2) in vec3 aColor;
layout(location = 3) in float aLifetime;
layout(location = 4) in float aMaxLifetime;
layout(location = 5) in float aSize;

uniform mat4 uViewProj;
uniform float uTime;

out vec3 vColor;
out float vAlpha;
out float vSize;

void main() {
    // 根据生命周期计算透明度
    float t = aLifetime / aMaxLifetime;

    // 前半段淡入，后半段淡出
    if (t > 0.5) {
        vAlpha = 1.0 - (t - 0.5) * 2.0;
    } else {
        vAlpha = t * 2.0;
    }

    vColor = aColor;
    vSize = aSize;

    gl_Position = uViewProj * vec4(aPos, 0.0, 1.0);
    gl_PointSize = aSize * 50.0 / gl_Position.w;
}
