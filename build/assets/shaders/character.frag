#version 330 core

in vec2 vWorldPos;

uniform vec2 uPosition;      // 角色世界位置
uniform float uTime;         // 动画时间（秒）
uniform vec3 uBodyColor;     // 备用
uniform int uExpression;     // 0=普通, 1=开心, 2=委屈
uniform float uArmAngle;     // 手臂角度

out vec4 FragColor;

// ========== SDF 形状函数 ==========

float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

float sdBox(vec2 p, vec2 b) {
    vec2 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
}

float sdRoundedBox(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return length(max(q, 0.0)) - r + min(max(q.x, q.y), 0.0);
}

float sdCapsule(vec2 p, float len, float rad) {
    p.y = abs(p.y) - len;
    return length(max(vec2(p.x, p.y), 0.0)) + min(max(p.x, p.y), 0.0) - rad;
}

vec2 rotate2D(vec2 p, float a) {
    float s = sin(a);
    float c = cos(a);
    return vec2(p.x * c - p.y * s, p.x * s + p.y * c);
}

void main() {
    vec2 uv = vWorldPos - uPosition;

    // ===== 超级玛丽风格角色 =====
    // 坐标系统：原点(0,0)在角色中心，y轴向上
    // 角色总高度约3.0单位

    // 颜色定义
    vec3 hatRed = vec3(0.9, 0.15, 0.15);
    vec3 skinColor = vec3(0.95, 0.78, 0.65);
    vec3 shirtRed = vec3(0.85, 0.2, 0.2);
    vec3 overallBlue = vec3(0.2, 0.4, 0.85);
    vec3 mustacheBrown = vec3(0.35, 0.22, 0.12);
    vec3 bootBrown = vec3(0.35, 0.2, 0.1);
    vec3 buttonYellow = vec3(0.9, 0.85, 0.2);
    vec3 eyeWhite = vec3(1.0, 1.0, 1.0);
    vec3 pupilBlack = vec3(0.05, 0.05, 0.05);
    vec3 mouthColor = vec3(0.6, 0.1, 0.1);
    vec3 hairBrown = vec3(0.28, 0.18, 0.12);

    vec3 finalColor = vec3(0.0);
    float alpha = 0.0;

    // ---- 渲染顺序：从后到前，从下到上 ----

    // 1. 靴子
    float leftBoot = sdRoundedBox(uv - vec2(-0.22, -1.25), vec2(0.13, 0.1), 0.025);
    float rightBoot = sdRoundedBox(uv - vec2(0.22, -1.25), vec2(0.13, 0.1), 0.025);
    if (leftBoot < 0.0) { finalColor = bootBrown; alpha = 1.0; }
    if (rightBoot < 0.0) { finalColor = bootBrown; alpha = 1.0; }

    // 2. 腿（蓝色裤子）
    float leftLeg = sdRoundedBox(uv - vec2(-0.22, -0.95), vec2(0.12, 0.25), 0.04);
    float rightLeg = sdRoundedBox(uv - vec2(0.22, -0.95), vec2(0.12, 0.25), 0.04);
    if (leftLeg < 0.0 && alpha == 0.0) { finalColor = overallBlue; alpha = 1.0; }
    if (rightLeg < 0.0 && alpha == 0.0) { finalColor = overallBlue; alpha = 1.0; }

    // 3. 背带裤（身体下半部分）
    float overalls = sdRoundedBox(uv - vec2(0.0, -0.55), vec2(0.38, 0.28), 0.07);
    if (overalls < 0.0 && alpha == 0.0) { finalColor = overallBlue; alpha = 1.0; }

    // 4. 背带
    float strapLeft = sdBox(uv - vec2(-0.22, -0.35), vec2(0.06, 0.18));
    float strapRight = sdBox(uv - vec2(0.22, -0.35), vec2(0.06, 0.18));
    if (strapLeft < 0.0 && alpha == 0.0) { finalColor = overallBlue; alpha = 1.0; }
    if (strapRight < 0.0 && alpha == 0.0) { finalColor = overallBlue; alpha = 1.0; }

    // 5. 扣子
    float buttonLeft = sdCircle(uv - vec2(-0.22, -0.3), 0.035);
    float buttonRight = sdCircle(uv - vec2(0.22, -0.3), 0.035);
    if (buttonLeft < 0.0 && alpha == 0.0) { finalColor = buttonYellow; alpha = 1.0; }
    if (buttonRight < 0.0 && alpha == 0.0) { finalColor = buttonYellow; alpha = 1.0; }

    // 6. 身体（红色上衣）
    float body = sdRoundedBox(uv - vec2(0.0, -0.15), vec2(0.4, 0.3), 0.08);
    if (body < 0.0 && alpha == 0.0) { finalColor = shirtRed; alpha = 1.0; }

    // 7. 手臂（从肩膀两侧自然垂下）
    // 左臂 - 从肩膀位置向下
    vec2 shoulderLeft = vec2(-0.38, -0.05);
    vec2 armLeftOffset = uv - shoulderLeft;
    float armSwingL = sin(uArmAngle + uTime * 2.0) * 0.1;
    vec2 armLeftRot = rotate2D(armLeftOffset, armSwingL);
    float leftArm = sdCapsule(vec2(armLeftRot.x, armLeftRot.y + 0.12), 0.12, 0.035);

    // 右臂 - 从肩膀位置向下
    vec2 shoulderRight = vec2(0.38, -0.05);
    vec2 armRightOffset = uv - shoulderRight;
    float armSwingR = sin(uArmAngle + uTime * 2.0 + 3.14) * 0.1;
    vec2 armRightRot = rotate2D(armRightOffset, armSwingR);
    float rightArm = sdCapsule(vec2(armRightRot.x, armRightRot.y + 0.12), 0.12, 0.035);

    if (leftArm < 0.0 && alpha == 0.0) { finalColor = skinColor; alpha = 1.0; }
    if (rightArm < 0.0 && alpha == 0.0) { finalColor = skinColor; alpha = 1.0; }

    // 9. 头部（方形，勇者风格）
    float head = sdRoundedBox(uv - vec2(0.0, 0.38), vec2(0.38, 0.35), 0.08);
    if (head < 0.0 && alpha == 0.0) { finalColor = skinColor; alpha = 1.0; }

    // ---- 脸部特征（叠加在头部上） ----
    if (head < 0.0 && alpha > 0.5) {
        // 46分立体背头（像帽子一样高，蓬松有型）
        // 头发顶部 - 高高隆起，像帽子
        float hairTop = sdRoundedBox(uv - vec2(-0.02, 0.75), vec2(0.35, 0.18), 0.07);
        // 46分分界线 - 偏左，创造立体感
        float hairPartLine = sdBox(uv - vec2(-0.06, 0.7), vec2(0.015, 0.2));
        // 左侧头发（6分，较多）- 向左后方梳
        float hairLeft = sdRoundedBox(uv - vec2(-0.15, 0.68), vec2(0.22, 0.15), 0.05);
        // 右侧头发（4分，较少）- 向右后方梳
        float hairRight = sdRoundedBox(uv - vec2(0.12, 0.68), vec2(0.18, 0.12), 0.05);
        // 后脑勺头发
        float hairBack = sdRoundedBox(uv - vec2(0.0, 0.55), vec2(0.35, 0.12), 0.06);
        // 鬓角 - 两侧
        float sideburnLeft = sdBox(uv - vec2(-0.33, 0.42), vec2(0.06, 0.15));
        float sideburnRight = sdBox(uv - vec2(0.33, 0.42), vec2(0.06, 0.15));
        // 额前刘海 - 46分，略微翘起
        float fringeLeft = sdRoundedBox(uv - vec2(-0.12, 0.58), vec2(0.15, 0.06), 0.03);
        float fringeRight = sdRoundedBox(uv - vec2(0.08, 0.58), vec2(0.12, 0.05), 0.03);

        float hair = min(min(min(min(hairTop, hairPartLine), min(hairLeft, hairRight)), min(hairBack, min(sideburnLeft, sideburnRight))), min(fringeLeft, fringeRight));
        if (hair < 0.0 && alpha == 1.0) { finalColor = hairBrown; alpha = 1.0; }

        // 眉毛（浓密，勇者风格）
        float browLeft = sdBox(uv - vec2(-0.14, 0.48), vec2(0.1, 0.03));
        float browRight = sdBox(uv - vec2(0.14, 0.48), vec2(0.1, 0.03));
        // 眉毛稍微倾斜，显得英气
        vec2 browL = rotate2D(uv - vec2(-0.14, 0.48), -0.1);
        vec2 browR = rotate2D(uv - vec2(0.14, 0.48), 0.1);
        float browL2 = sdBox(browL, vec2(0.1, 0.025));
        float browR2 = sdBox(browR, vec2(0.1, 0.025));
        if (browL2 < 0.0) { finalColor = hairBrown; alpha = 1.0; }
        if (browR2 < 0.0) { finalColor = hairBrown; alpha = 1.0; }

        // 眼睛（勇者风格 - 更有神，蓝灰色虹膜）
        float blink = smoothstep(0.93, 1.0, sin(uTime * 1.5));
        float eyeScaleY = mix(1.0, 0.15, blink);

        // 眼白
        float leftEye = sdCircle((uv - vec2(-0.14, 0.4)) * vec2(1.0, eyeScaleY), 0.1);
        float rightEye = sdCircle((uv - vec2(0.14, 0.4)) * vec2(1.0, eyeScaleY), 0.1);
        if (leftEye < 0.0) { finalColor = eyeWhite; alpha = 1.0; }
        if (rightEye < 0.0) { finalColor = eyeWhite; alpha = 1.0; }

        // 虹膜（蓝灰色，勇者风格）
        vec3 irisColor = vec3(0.2, 0.4, 0.7);
        float pupilMove = sin(uTime * 0.8) * 0.02;
        float leftIris = sdCircle(uv - vec2(-0.14 + pupilMove, 0.4), 0.06);
        float rightIris = sdCircle(uv - vec2(0.14 + pupilMove, 0.4), 0.06);
        if (leftIris < 0.0) { finalColor = irisColor; alpha = 1.0; }
        if (rightIris < 0.0) { finalColor = irisColor; alpha = 1.0; }

        // 瞳孔（黑色，居中）
        float leftPupil = sdCircle(uv - vec2(-0.14 + pupilMove, 0.4), 0.03);
        float rightPupil = sdCircle(uv - vec2(0.14 + pupilMove, 0.4), 0.03);
        if (leftPupil < 0.0) { finalColor = pupilBlack; alpha = 1.0; }
        if (rightPupil < 0.0) { finalColor = pupilBlack; alpha = 1.0; }

        // 眼神高光（让眼睛更有神）
        float leftHighlight = sdCircle(uv - vec2(-0.12, 0.42), 0.015);
        float rightHighlight = sdCircle(uv - vec2(0.16, 0.42), 0.015);
        if (leftHighlight < 0.0) { finalColor = vec3(1.0); alpha = 1.0; }
        if (rightHighlight < 0.0) { finalColor = vec3(1.0); alpha = 1.0; }
    }

    // ===== 抗锯齿边缘 =====
    float minDist = head;
    if (body < minDist) minDist = body;
    if (overalls < minDist) minDist = overalls;
    if (leftLeg < minDist) minDist = leftLeg;
    if (rightLeg < minDist) minDist = rightLeg;
    if (leftBoot < minDist) minDist = leftBoot;
    if (rightBoot < minDist) minDist = rightBoot;
    if (leftArm < minDist) minDist = leftArm;
    if (rightArm < minDist) minDist = rightArm;

    // 如果所有形状都很远，就完全透明
    if (minDist > 0.03) {
        alpha = 0.0;
    } else if (minDist > -0.005) {
        float pixelSize = length(dFdx(vWorldPos)) + length(dFdy(vWorldPos));
        float edgeWidth = max(pixelSize * 2.0, 0.03);
        alpha *= smoothstep(edgeWidth, -0.005, minDist);
    }

    FragColor = vec4(finalColor, alpha);
}
