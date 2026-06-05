#version 330 core

layout(location = 0) in vec2 aPos;  // 覆盖角色包围盒的四边形顶点
uniform mat4 uViewProj;
out vec2 vWorldPos;

void main() {
    gl_Position = uViewProj * vec4(aPos, 0.0, 1.0);
    vWorldPos = aPos;
}
