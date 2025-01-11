#version 460

#include "thundersUtil.frag"

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform float u_Time;
uniform float u_aspect;
uniform mat4 u_MVPM;
uniform float u_thunderstorm;
uniform vec2 u_resolution;
uniform float u_downscale;

const float g_radius = 197500.0;
const float sky_b_radius = 201000.0;

void main() {
    initializeBoltPositions();

    vec2 uv = (gl_FragCoord.xy * u_downscale) / u_resolution - 0.5;
    uv *= vec2(2.0 * u_aspect, 2.0);
    vec4 worldPos = inverse(u_MVPM) * vec4(uv, 1.0, 1.0);
    vec3 rayDir = normalize(worldPos.xyz / worldPos.w);

    if (rayDir.y <= 0.0) return;

    vec3 cameraPos = vec3(0.0, g_radius, 0.0);
    vec3 glow;
    float dist = raymarchScene(cameraPos, rayDir, 0.1, sky_b_radius, glow, sky_b_radius, g_radius, u_Time);

    float maxGlow = max(max(glow.r, glow.g), glow.b);
    color = (maxGlow > 0.3 && u_thunderstorm > 0.0) ? vec4(glow, maxGlow) : vec4(0.0);
}
