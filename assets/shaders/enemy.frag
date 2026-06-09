#version 330 core
in vec2 vWorldPos;

uniform vec2 uPosition;
uniform float uTime;
uniform vec3 uColor;
uniform float uRadius;
uniform int uType;            // 0=Chaser, 1=Shooter, 2=Exploder
uniform float uHealthPercent;

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

float unite(float a, float b) {
    return min(a, b);
}

void main() {
    float r = uRadius;
    vec2 uv = vWorldPos - uPosition;

    float stepBob = sin(uTime * 7.0 + uPosition.x) * 0.035 * r;
    vec2 p = uv;

    float body = sdRoundedBox(p - vec2(0.0, -0.15 * r), vec2(0.72 * r, 0.58 * r), 0.18 * r);
    float head = sdCircle(p - vec2(0.0, 0.62 * r), 0.58 * r);
    float leftLeg = sdRoundedBox(p - vec2(-0.28 * r, -0.88 * r + stepBob), vec2(0.18 * r, 0.28 * r), 0.08 * r);
    float rightLeg = sdRoundedBox(p - vec2(0.28 * r, -0.88 * r - stepBob), vec2(0.18 * r, 0.28 * r), 0.08 * r);
    float leftArm = sdCapsule(p, vec2(-0.62 * r, 0.08 * r), vec2(-0.92 * r, -0.36 * r), 0.13 * r);
    float rightArm = sdCapsule(p, vec2(0.62 * r, 0.08 * r), vec2(0.92 * r, -0.36 * r), 0.13 * r);

    float creature = unite(unite(unite(body, head), unite(leftLeg, rightLeg)), unite(leftArm, rightArm));

    float typeFeature = 10.0;
    if (uType == 0) {
        float leftEar = sdCapsule(p, vec2(-0.34 * r, 0.98 * r), vec2(-0.62 * r, 1.25 * r), 0.13 * r);
        float rightEar = sdCapsule(p, vec2(0.34 * r, 0.98 * r), vec2(0.62 * r, 1.25 * r), 0.13 * r);
        typeFeature = unite(leftEar, rightEar);
    } else if (uType == 1) {
        float antennaL = sdCapsule(p, vec2(-0.18 * r, 1.02 * r), vec2(-0.34 * r, 1.38 * r), 0.055 * r);
        float antennaR = sdCapsule(p, vec2(0.18 * r, 1.02 * r), vec2(0.34 * r, 1.38 * r), 0.055 * r);
        float beadL = sdCircle(p - vec2(-0.36 * r, 1.42 * r), 0.10 * r);
        float beadR = sdCircle(p - vec2(0.36 * r, 1.42 * r), 0.10 * r);
        typeFeature = unite(unite(antennaL, antennaR), unite(beadL, beadR));
    } else {
        float pulse = 1.0 + sin(uTime * 9.0) * 0.12;
        typeFeature = sdCircle(p - vec2(0.0, -0.14 * r), 0.78 * r * pulse);
    }

    creature = unite(creature, typeFeature);

    float edge = creature;
    float aa = max(fwidth(edge) * 1.5, 0.01);
    float alpha = 1.0 - smoothstep(0.0, aa, edge);
    float outlineAlpha = 1.0 - smoothstep(0.05 * r, 0.05 * r + aa, edge);
    if (outlineAlpha < 0.01) discard;

    vec3 base = uColor;
    vec3 shade = base * 0.58;
    vec3 light = min(base + vec3(0.24), vec3(1.0));
    vec3 color = mix(shade, light, smoothstep(-0.75 * r, 0.9 * r, p.y));

    if (uType == 2) {
        float pulse = sin(uTime * 8.0) * 0.5 + 0.5;
        color = mix(color, vec3(1.0, 0.45, 0.08), 0.25 * pulse);
    }

    if (edge > -0.035 * r) {
        color = vec3(0.08, 0.06, 0.05);
    }

    float leftEye = sdCircle(p - vec2(-0.20 * r, 0.68 * r), 0.12 * r);
    float rightEye = sdCircle(p - vec2(0.20 * r, 0.68 * r), 0.12 * r);
    if (leftEye < 0.0 || rightEye < 0.0) color = vec3(0.98);

    float pupilL = sdCircle(p - vec2(-0.18 * r, 0.66 * r), 0.055 * r);
    float pupilR = sdCircle(p - vec2(0.22 * r, 0.66 * r), 0.055 * r);
    if (pupilL < 0.0 || pupilR < 0.0) color = vec3(0.03, 0.035, 0.04);

    float mouth = sdBox(p - vec2(0.0, 0.38 * r), vec2(0.20 * r, 0.035 * r));
    if (mouth < 0.0) color = vec3(0.12, 0.04, 0.04);

    if (uHealthPercent < 0.3) {
        float flash = sin(uTime * 12.0) * 0.5 + 0.5;
        color = mix(color, vec3(1.0, 0.08, 0.06), flash * 0.45);
    }

    FragColor = vec4(color, max(alpha, outlineAlpha * 0.85));
}
