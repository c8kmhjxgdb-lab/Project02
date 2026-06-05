#version 330 core
in vec2 vTexCoords;
uniform sampler2D uScreenTexture;
uniform float uVignetteIntensity;  // 0-1，由委屈值决定（clamp）
out vec4 FragColor;

void main() {
    vec4 color = texture(uScreenTexture, vTexCoords);

    if (uVignetteIntensity > 0.0) {
        vec2 center = vec2(0.5, 0.5);
        float dist = distance(vTexCoords, center);
        // 暗角：边缘变暗，强度由委屈值控制
        float vignette = smoothstep(0.8, 0.3, dist * (1.0 + uVignetteIntensity * 0.5));
        color.rgb *= vignette;

        // 委屈时降低饱和度
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        float sat = 1.0 - uVignetteIntensity * 0.3;
        color.rgb = mix(vec3(gray), color.rgb, sat);
    }

    FragColor = color;
}
