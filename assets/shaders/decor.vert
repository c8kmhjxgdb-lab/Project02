#version 330 core
layout(location = 0) in vec2 aPos;        // 单位四边形顶点（-0.5~0.5）
layout(location = 1) in vec2 iPosition;   // 实例：世界位置
layout(location = 2) in float iRotation;  // 实例：旋转
layout(location = 3) in float iScale;     // 实例：缩放
layout(location = 4) in int iType;        // 实例：类型
layout(location = 5) in int iVariant;     // 实例：变种

uniform mat4 uViewProj;
uniform float uTime;

out vec2 vWorldPos;
out float vTime;
flat out int vType;
flat out int vVariant;

void main() {
    vec2 pos = aPos * iScale;

    float c = cos(iRotation);
    float s = sin(iRotation);
    pos = vec2(c * pos.x - s * pos.y, s * pos.x + c * pos.y);

    vec2 worldPos = iPosition + pos;
    vWorldPos = worldPos;
    vTime = uTime;
    vType = iType;
    vVariant = iVariant;

    gl_Position = uViewProj * vec4(worldPos, 0.0, 1.0);
}
