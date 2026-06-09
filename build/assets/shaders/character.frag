#version 330 core

in vec2 vWorldPos;

uniform vec2 uPosition;
uniform float uTime;
uniform vec3 uBodyColor;
uniform int uExpression;
uniform float uArmAngle;

out vec4 FragColor;

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

float sdCapsule(vec2 p, vec2 a, vec2 b, float r) {
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

float sdTriangle(vec2 p, float r) {
    p.x = abs(p.x);
    return max(p.x * 0.866025 + p.y * 0.5, -p.y) - r * 0.5;
}

float sdStar(vec2 p, float r1, float r2, int n) {
    float a = atan(p.y, p.x);
    float r = length(p);
    float k = 3.14159265 / float(n);
    float m = cos(floor(0.5 + a / k) * k - a);
    float target = mix(r1, r2, step(0.5, mod(floor(0.5 + a / k), 2.0)));
    return r * m - target;
}

vec2 rotate2D(vec2 p, float a) {
    float s = sin(a);
    float c = cos(a);
    return vec2(p.x * c - p.y * s, p.x * s + p.y * c);
}

vec3 shadeColor(vec3 color, float amount) {
    return color * (1.0 - amount);
}

vec3 tintColor(vec3 color, float amount) {
    return color + (vec3(1.0) - color) * amount;
}

void paint(inout vec3 color, inout float alpha, float d, vec3 layerColor) {
    float aa = max(fwidth(d) * 1.6, 0.006);
    float coverage = 1.0 - smoothstep(-aa, aa, d);
    color = mix(color, layerColor, coverage);
    alpha = max(alpha, coverage);
}

void main() {
    vec2 uv = vWorldPos - uPosition;
    float bob = sin(uTime * 7.0) * 0.025;
    vec2 p = uv - vec2(0.0, bob);

    vec3 outline = vec3(0.055, 0.060, 0.075);
    vec3 skin = vec3(0.96, 0.78, 0.64);
    vec3 blush = vec3(0.92, 0.34, 0.34);
    vec3 hair = vec3(0.20, 0.12, 0.09);
    vec3 hairLight = vec3(0.42, 0.25, 0.16);
    vec3 tunic = vec3(0.17, 0.48, 0.88);
    vec3 tunicLight = vec3(0.35, 0.66, 0.98);
    vec3 cape = vec3(0.24, 0.20, 0.54);
    vec3 capeDeep = vec3(0.12, 0.10, 0.32);
    vec3 scarf = vec3(0.96, 0.48, 0.28);
    vec3 belt = vec3(0.26, 0.16, 0.09);
    vec3 gold = vec3(1.00, 0.78, 0.22);
    vec3 boot = vec3(0.21, 0.14, 0.10);
    vec3 eye = vec3(0.055, 0.070, 0.085);

    vec3 color = vec3(0.0);
    float alpha = 0.0;

    float armSwing = sin(uArmAngle + uTime * 5.0) * 0.10;
    vec2 leftHand = vec2(-0.58, -0.34) + vec2(armSwing * 0.35, 0.0);
    vec2 rightHand = vec2(0.58, -0.34) - vec2(armSwing * 0.35, 0.0);

    float capeShape = min(
        sdRoundedBox(p - vec2(0.0, -0.34), vec2(0.55, 0.72), 0.16),
        sdTriangle(p - vec2(0.0, -0.98), 0.96)
    );
    float body = sdRoundedBox(p - vec2(0.0, -0.30), vec2(0.42, 0.48), 0.12);
    float head = sdRoundedBox(p - vec2(0.0, 0.50), vec2(0.36, 0.34), 0.11);
    float neck = sdRoundedBox(p - vec2(0.0, 0.12), vec2(0.12, 0.11), 0.04);
    float leftLeg = sdCapsule(p, vec2(-0.18, -0.72), vec2(-0.22, -1.15), 0.11);
    float rightLeg = sdCapsule(p, vec2(0.18, -0.72), vec2(0.22, -1.15), 0.11);
    float leftBoot = sdRoundedBox(p - vec2(-0.25, -1.28), vec2(0.20, 0.09), 0.04);
    float rightBoot = sdRoundedBox(p - vec2(0.25, -1.28), vec2(0.20, 0.09), 0.04);
    float leftArm = sdCapsule(p, vec2(-0.36, -0.04), leftHand, 0.075);
    float rightArm = sdCapsule(p, vec2(0.36, -0.04), rightHand, 0.075);
    float leftPalm = sdCircle(p - leftHand, 0.105);
    float rightPalm = sdCircle(p - rightHand, 0.105);

    float shadow = sdCircle((uv - vec2(0.0, -1.34)) * vec2(1.0, 2.8), 0.44);

    float silhouette = capeShape;
    silhouette = min(silhouette, body);
    silhouette = min(silhouette, head);
    silhouette = min(silhouette, neck);
    silhouette = min(silhouette, min(leftLeg, rightLeg));
    silhouette = min(silhouette, min(leftBoot, rightBoot));
    silhouette = min(silhouette, min(leftArm, rightArm));
    silhouette = min(silhouette, min(leftPalm, rightPalm));
    silhouette = min(silhouette, shadow);

    float aa = max(fwidth(silhouette) * 1.6, 0.006);
    float outlineAlpha = 1.0 - smoothstep(0.035, 0.035 + aa, silhouette);
    if (outlineAlpha < 0.01) discard;
    color = outline;
    alpha = outlineAlpha;

    paint(color, alpha, shadow, vec3(0.018, 0.022, 0.028));

    vec3 capeShade = mix(capeDeep, cape, smoothstep(-0.95, 0.28, p.y));
    paint(color, alpha, capeShape, capeShade);
    paint(color, alpha, sdCapsule(p, vec2(-0.34, -0.80), vec2(-0.10, -0.10), 0.025), vec3(0.47, 0.42, 0.78));
    paint(color, alpha, sdCapsule(p, vec2(0.34, -0.80), vec2(0.10, -0.10), 0.025), vec3(0.47, 0.42, 0.78));

    paint(color, alpha, leftLeg, shadeColor(tunic, 0.18));
    paint(color, alpha, rightLeg, shadeColor(tunic, 0.18));
    paint(color, alpha, leftBoot, boot);
    paint(color, alpha, rightBoot, boot);

    paint(color, alpha, leftArm, skin);
    paint(color, alpha, rightArm, skin);
    paint(color, alpha, leftPalm, skin);
    paint(color, alpha, rightPalm, skin);

    vec3 bodyShade = mix(tunic, tunicLight, smoothstep(-0.74, 0.18, p.y));
    paint(color, alpha, body, bodyShade);
    paint(color, alpha, sdRoundedBox(p - vec2(0.0, -0.38), vec2(0.44, 0.08), 0.03), belt);
    paint(color, alpha, sdRoundedBox(p - vec2(0.0, -0.38), vec2(0.075, 0.055), 0.02), gold);
    paint(color, alpha, sdCapsule(p, vec2(-0.30, -0.02), vec2(0.18, -0.58), 0.030), tintColor(tunicLight, 0.22));
    paint(color, alpha, sdCapsule(p, vec2(0.30, -0.02), vec2(-0.18, -0.58), 0.030), tintColor(tunicLight, 0.22));

    float scarfBand = sdCapsule(p, vec2(-0.29, 0.07), vec2(0.30, 0.07), 0.075);
    float scarfTail = sdCapsule(p, vec2(0.23, 0.04), vec2(0.50, -0.27), 0.060);
    paint(color, alpha, min(scarfBand, scarfTail), scarf);
    paint(color, alpha, neck, skin);

    paint(color, alpha, head, skin);
    paint(color, alpha, sdCircle((p - vec2(-0.23, 0.43)) * vec2(1.0, 1.55), 0.055), blush);
    paint(color, alpha, sdCircle((p - vec2(0.23, 0.43)) * vec2(1.0, 1.55), 0.055), blush);

    float hairMass = min(
        sdRoundedBox(p - vec2(0.0, 0.76), vec2(0.39, 0.18), 0.09),
        sdRoundedBox(p - vec2(-0.16, 0.62), vec2(0.25, 0.15), 0.08)
    );
    hairMass = min(hairMass, sdRoundedBox(p - vec2(0.18, 0.63), vec2(0.22, 0.13), 0.07));
    hairMass = min(hairMass, sdCapsule(p, vec2(-0.31, 0.62), vec2(-0.36, 0.34), 0.065));
    hairMass = min(hairMass, sdCapsule(p, vec2(0.31, 0.62), vec2(0.36, 0.36), 0.060));
    paint(color, alpha, hairMass, hair);
    paint(color, alpha, sdCapsule(p, vec2(-0.26, 0.78), vec2(0.10, 0.83), 0.025), hairLight);
    paint(color, alpha, sdCapsule(p, vec2(-0.18, 0.66), vec2(0.28, 0.70), 0.020), hairLight * 0.82);

    float blink = smoothstep(0.94, 1.0, sin(uTime * 1.7));
    float eyeH = mix(0.045, 0.010, blink);
    paint(color, alpha, sdRoundedBox(p - vec2(-0.13, 0.49), vec2(0.075, eyeH), 0.018), vec3(0.98));
    paint(color, alpha, sdRoundedBox(p - vec2(0.13, 0.49), vec2(0.075, eyeH), 0.018), vec3(0.98));
    paint(color, alpha, sdCircle(p - vec2(-0.13, 0.49), 0.032), eye);
    paint(color, alpha, sdCircle(p - vec2(0.13, 0.49), 0.032), eye);
    paint(color, alpha, sdCircle(p - vec2(-0.115, 0.505), 0.012), vec3(1.0));
    paint(color, alpha, sdCircle(p - vec2(0.145, 0.505), 0.012), vec3(1.0));

    if (uExpression == 1) {
        paint(color, alpha, sdCapsule(p, vec2(-0.09, 0.32), vec2(0.09, 0.32), 0.020), vec3(0.48, 0.08, 0.07));
    } else if (uExpression == 2) {
        paint(color, alpha, sdCapsule(p, vec2(-0.08, 0.31), vec2(0.08, 0.31), 0.014), vec3(0.40, 0.08, 0.09));
        paint(color, alpha, sdCapsule(p, vec2(-0.19, 0.57), vec2(-0.07, 0.53), 0.014), hair);
        paint(color, alpha, sdCapsule(p, vec2(0.07, 0.53), vec2(0.19, 0.57), 0.014), hair);
    } else {
        paint(color, alpha, sdCapsule(p, vec2(-0.075, 0.32), vec2(0.075, 0.32), 0.015), vec3(0.42, 0.08, 0.07));
    }

    float star = sdStar(p - vec2(0.0, -0.12), 0.050, 0.105, 5);
    paint(color, alpha, star, gold);
    paint(color, alpha, sdCircle(p - vec2(0.0, -0.12), 0.020), vec3(1.0, 0.94, 0.62));

    float rim = 1.0 - smoothstep(-0.03, 0.18, silhouette);
    color = mix(color, color * 0.72, rim * 0.22);
    float light = smoothstep(-1.25, 0.95, p.y);
    color += vec3(0.06, 0.07, 0.09) * light * alpha;

    FragColor = vec4(color, alpha);
}
