#version 330 core

in vec2 vWorldPos;

uniform vec2 uPosition;
uniform vec2 uDirection;
uniform float uTime;
uniform vec3 uColor;
uniform float uRadius;
uniform int uType;

out vec4 FragColor;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
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

float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

float sdBox(vec2 p, vec2 b) {
    vec2 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
}

float sdCapsule(vec2 p, vec2 a, vec2 b, float r) {
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

float sdDiamond(vec2 p, float w, float h) {
    p = abs(p);
    return (p.x / w + p.y / h - 1.0) * min(w, h);
}

vec3 heatRamp(float t) {
    vec3 c1 = vec3(1.00, 0.96, 0.62);
    vec3 c2 = vec3(1.00, 0.50, 0.10);
    vec3 c3 = vec3(0.86, 0.12, 0.05);
    return mix(mix(c1, c2, smoothstep(0.05, 0.55, t)), c3, smoothstep(0.48, 1.0, t));
}

void main() {
    vec2 dir = normalize(abs(uDirection.x) + abs(uDirection.y) > 0.001 ? uDirection : vec2(1.0, 0.0));
    vec2 side = vec2(-dir.y, dir.x);
    vec2 uv = vWorldPos - uPosition;
    vec2 p = vec2(dot(uv, dir), dot(uv, side)) / max(uRadius, 0.001);
    float pulse = sin(uTime * 18.0) * 0.5 + 0.5;

    vec3 color = uColor;
    float alpha = 0.0;

    if (uType == 0) {
        float core = sdCircle(p - vec2(0.18, 0.0), 0.88);
        float tail = sdCapsule(p, vec2(-2.10, 0.0), vec2(0.32, 0.0), 0.42);
        float ember = noise(p * 4.2 + vec2(-uTime * 9.0, uTime * 0.8));
        tail += (ember - 0.5) * 0.18;
        float d = min(core, tail);
        float aa = max(fwidth(d) * 1.7, 0.018);
        alpha = 1.0 - smoothstep(0.0, aa, d);

        float radial = clamp(length(p - vec2(0.2, 0.0)) / 1.15, 0.0, 1.0);
        float tailFade = smoothstep(-2.15, 0.35, p.x);
        color = heatRamp(radial);
        color += vec3(0.65, 0.14, 0.02) * (1.0 - tailFade) * alpha;
        color += vec3(0.45, 0.20, 0.02) * smoothstep(0.55, 0.95, ember) * alpha;
        alpha *= mix(0.55, 1.0, tailFade);

        float glow = 1.0 - smoothstep(0.75, 2.15, length(p - vec2(0.08, 0.0)));
        color += vec3(1.0, 0.28, 0.05) * glow * (0.35 + pulse * 0.20);
        alpha = max(alpha, glow * 0.30);
    } else if (uType == 1) {
        float shaft = sdDiamond(p - vec2(0.16, 0.0), 1.35, 0.42);
        float tip = sdDiamond(p - vec2(0.82, 0.0), 0.70, 0.30);
        float rear = sdCapsule(p, vec2(-0.92, 0.0), vec2(-0.28, 0.0), 0.20);
        float fin1 = sdDiamond(p - vec2(-0.35, 0.34), 0.38, 0.18);
        float fin2 = sdDiamond(p - vec2(-0.35, -0.34), 0.38, 0.18);
        float d = min(min(shaft, tip), min(rear, min(fin1, fin2)));
        float aa = max(fwidth(d) * 1.6, 0.014);
        alpha = 1.0 - smoothstep(0.0, aa, d);

        float facet = noise(p * 8.0 - vec2(uTime * 1.5, 0.0));
        float sideLight = smoothstep(0.52, 0.0, abs(p.y));
        color = mix(vec3(0.20, 0.52, 0.95), vec3(0.86, 0.97, 1.0), sideLight);
        color += vec3(0.35, 0.82, 1.0) * smoothstep(0.50, 0.78, facet) * 0.35;
        color += vec3(0.90, 1.0, 1.0) * smoothstep(0.92, 1.30, p.x) * 0.45;

        float edge = 1.0 - smoothstep(0.02, 0.20, abs(d));
        color = mix(color, vec3(0.05, 0.32, 0.72), edge * 0.35);
    } else {
        float orb = sdCircle(p, 0.74);
        float bolt1 = sdCapsule(p, vec2(-1.35, 0.16), vec2(1.25, -0.12), 0.13);
        float bolt2 = sdCapsule(p, vec2(-0.72, -0.55), vec2(0.72, 0.55), 0.08);
        float spark = min(orb, min(bolt1, bolt2));
        float jitter = noise(p * 16.0 + uTime * 13.0) - 0.5;
        spark += jitter * 0.09;
        float aa = max(fwidth(spark) * 1.8, 0.018);
        alpha = 1.0 - smoothstep(0.0, aa, spark);

        float radial = clamp(length(p) / 0.95, 0.0, 1.0);
        color = mix(vec3(1.0), vec3(0.34, 0.58, 1.0), radial);
        color *= 0.82 + pulse * 0.35;
        color += vec3(0.40, 0.80, 1.0) * smoothstep(0.35, 0.95, noise(p * 22.0 + uTime * 18.0)) * alpha;

        float halo = 1.0 - smoothstep(0.70, 1.75, length(p));
        alpha = max(alpha, halo * (0.22 + pulse * 0.18));
    }

    if (alpha < 0.01) discard;
    FragColor = vec4(color, alpha);
}
