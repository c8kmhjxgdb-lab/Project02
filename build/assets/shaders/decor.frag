#version 330 core
in vec2 vWorldPos;
in float vTime;
flat in int vType;
flat in int vVariant;
out vec4 FragColor;

// SDF primitives
float sdCircle(vec2 p, float r) {
    return length(p) - r;
}

float sdCapsule(vec2 p, float len, float rad) {
    p.y -= clamp(p.y, 0.0, len);
    return length(p) - rad;
}

// Tree: trunk + crown with wind animation
vec4 renderTree(vec2 uv, int variant, float time) {
    // Trunk
    float trunk = sdCapsule(uv - vec2(0.0, -0.2), 0.3, 0.08);
    vec3 trunkColor = vec3(0.4, 0.25, 0.1);

    // Crown (varies by variant)
    float crown1 = sdCircle(uv - vec2(0.0, 0.3), 0.35);
    float crown2 = sdCircle(uv - vec2(-0.15, 0.15), 0.25);
    float crown3 = sdCircle(uv - vec2(0.15, 0.15), 0.25);
    float crown = min(crown1, min(crown2, crown3));
    vec3 leafColor = vec3(0.2, 0.6, 0.15);

    // Wind effect
    leafColor += sin(time * 2.0 + float(vVariant)) * 0.03;

    float tree = min(trunk, crown);
    float alpha = 1.0 - smoothstep(0.0, 0.02, tree);
    vec3 color = trunk < crown ? trunkColor : leafColor;
    return vec4(color, alpha);
}

// Bush: overlapping circles
vec4 renderBush(vec2 uv, int variant) {
    float b1 = sdCircle(uv - vec2(-0.1, 0.0), 0.25);
    float b2 = sdCircle(uv - vec2(0.1, 0.05), 0.2);
    float bush = min(b1, b2);
    float alpha = 1.0 - smoothstep(0.0, 0.02, bush);
    vec3 color = vec3(0.15, 0.5, 0.1);
    color += float(variant) * 0.05;
    return vec4(color, alpha);
}

// Flower: small colored circle with stem
vec4 renderFlower(vec2 uv) {
    // Stem
    float stem = sdCapsule(uv - vec2(0.0, -0.15), 0.2, 0.02);
    vec3 stemColor = vec3(0.2, 0.5, 0.1);

    // Petal
    float petal = sdCircle(uv - vec2(0.0, 0.1), 0.08);
    vec3 petalColor = vec3(0.9, 0.3, 0.5);

    float flower = min(stem, petal);
    float alpha = 1.0 - smoothstep(0.0, 0.02, flower);
    vec3 color = stem < petal ? stemColor : petalColor;
    return vec4(color, alpha);
}

// Tall grass: thin blades
vec4 renderTallGrass(vec2 uv, float time) {
    float sway = sin(time * 3.0) * 0.05;
    float blade1 = sdCapsule(uv - vec2(sway, 0.0), 0.35, 0.03);
    float blade2 = sdCapsule(uv - vec2(-0.05 + sway, -0.05), 0.25, 0.025);
    float grass = min(blade1, blade2);
    float alpha = 1.0 - smoothstep(0.0, 0.02, grass);
    vec3 color = vec3(0.25, 0.65, 0.15);
    return vec4(color, alpha);
}

// Rock: irregular shape
vec4 renderRock(vec2 uv) {
    float rock = sdCircle(uv, 0.2);
    rock = max(rock, -sdCircle(uv + vec2(0.1, 0.05), 0.15));
    float alpha = 1.0 - smoothstep(0.0, 0.02, rock);
    vec3 color = vec3(0.5, 0.48, 0.45);
    return vec4(color, alpha);
}

// Stump: short trunk
vec4 renderStump(vec2 uv) {
    float stump = sdCapsule(uv - vec2(0.0, -0.05), 0.15, 0.12);
    float alpha = 1.0 - smoothstep(0.0, 0.02, stump);
    vec3 color = vec3(0.45, 0.3, 0.15);
    // Top surface
    if (uv.y > 0.05 && uv.y < 0.15) {
        color = vec3(0.55, 0.4, 0.25);
    }
    return vec4(color, alpha);
}

void main() {
    vec4 result = vec4(0.0, 0.0, 0.0, 0.0);

    if (vType == 1) {
        result = renderTree(vWorldPos, vVariant, vTime);
    } else if (vType == 2) {
        result = renderBush(vWorldPos, vVariant);
    } else if (vType == 3) {
        result = renderFlower(vWorldPos);
    } else if (vType == 4) {
        result = renderTallGrass(vWorldPos, vTime);
    } else if (vType == 5) {
        result = renderRock(vWorldPos);
    } else if (vType == 6) {
        result = renderStump(vWorldPos);
    } else {
        discard;
    }

    FragColor = result;
}
