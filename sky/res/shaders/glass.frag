#version 460
#extension GL_NV_shadow_samplers_cube : enable
out vec4 FragColor;

in vec2 TexCoords;
in vec3 position;
in vec3 normal;

uniform vec3 u_lightPosition;
uniform sampler2D texture_diffuse1;
uniform sampler2D RippleTexture;
uniform sampler2D glassNoiseTexture;
uniform samplerCube cubemap;
uniform float u_useCubemap;
uniform float u_rainIntensity;
uniform float u_Time;
uniform mat4 view;
uniform float snowAccumulation;
uniform float u_shadowIntensity;

#define S(x, y, z) smoothstep(x, y, z)

// Random function for adding noise
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 N31(float p) {
    //  3 out, 1 in... DAVE HOSKINS
    vec3 p3 = fract(vec3(p) * vec3(0.1031, 0.11369, 0.13787));
    p3 += dot(p3, p3.yzx + 19.19);
    return fract(vec3((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y, (p3.y + p3.z) * p3.x));
}

float SawTooth(float t) {
    return cos(t + cos(t)) + sin(2.0 * t) * 0.2 + sin(4.0 * t) * 0.02;
}

float DeltaSawTooth(float t) {
    return 0.4 * cos(2.0 * t) + 0.08 * cos(4.0 * t) - (1.0 - sin(t)) * sin(t + cos(t));
}

vec2 GetDrops(vec2 uv, float seed, float m) {
    float t = u_Time + m * 30.0;
    vec2 o = vec2(0.0);

    #ifndef DROP_DEBUG
    uv.y += t * 0.05;
    #endif

    // Scale UV coordinates for drop identification
    uv *= vec2(20.0, 5.0);
    vec2 id = floor(uv);
    vec3 n = N31(id.x + (id.y + seed) * 546.3524);
    vec2 bd = fract(uv) - 0.5;

    // Adjust the Y coordinate for a pronounced drop effect
    bd.y *= 4.0;
    bd.x += (n.x - 0.5) * 0.6;

    t += n.z * 6.28;
    float slide = SawTooth(t);
    
    // Calculate trail position based on the drop's movement
    float ts = 1.5;
    vec2 trailPos = vec2(bd.x * ts, (fract(bd.y * ts * 2.0 - t * 2.0) - 0.5) * 0.5);

    // Make drops slide down
    bd.y += slide * 2.0;

    #ifdef HIGH_QUALITY
    float dropShape = bd.x * bd.x * DeltaSawTooth(t);
    bd.y += dropShape; // Modify the shape of the drop while falling
    #endif

    // Calculate distances for the main drop and trail
    float d = length(bd);
    float trailMask = S(-0.2, 0.2, bd.y) * bd.y; // Mask out drops below the main drop
    float td = length(trailPos * max(0.5, trailMask)); // Distance to trail drops

    // Generate main drop and trail effects
    float mainDrop = S(0.2, 0.1, d);
    float dropTrail = S(0.1, 0.02, td) * trailMask;
    
    // Mix main drop and drop trail
    o = mix(bd * mainDrop, trailPos, dropTrail);

    #ifdef DROP_DEBUG
    if (uv.x < 0.02 || uv.y < 0.01) o = vec2(1.0);
    #endif

    return o;
}

float stepfun(float x) {
    return (sign(x) + 1.0) / 2.0;
}

float square(vec2 pos) {
    return (stepfun(pos.x + 1.0) * stepfun(1.0 - pos.x)) *
           (stepfun(pos.y + 1.0) * stepfun(1.0 - pos.y));
}

vec3 FrostDist(vec3 pos) {
    vec2 pos2D = pos.xy;
    return vec3(vec2(pos2D + square(pos2D) * 
           texture(glassNoiseTexture, pos2D).xy * 0.2), pos.z);
}

void main() {
    float sunIntensity = 2.0;
    vec3 L = normalize(u_lightPosition - position);

    // Get the drop offsets based on UV coordinates
    vec2 dropUv = TexCoords;
    vec2 offs = GetDrops(dropUv * 5.0, 1.0, 0.0);
    
    // Adjust offsets based on randomness and time
    if (offs.x != 0.0 && offs.y != 0.0) {
        offs += vec2(rand(dropUv + vec2(0.0, u_Time)), rand(dropUv + vec2(u_Time, 0.0))) * 0.02;
    }


    // Discard drops near the edges of the object
    float edgeThreshold = 0.38; // Fine-tune edge exclusion
    if (dropUv.x < edgeThreshold || dropUv.x > 1.0 - edgeThreshold) {
        offs = vec2(0.0);
    }

    // Disable drops if rain intensity is zero
    if (u_rainIntensity <= 0.0) {
        offs.x = 0;
        offs.y = 0;
    }

    // Calculate normal and reflection
    offs.y = 0;
    vec3 N = normal;
    N.x = offs.x + normal.x;
    vec3 I = normalize(position);
    vec3 Reflect = reflect(I, normalize(N)); 

    // Apply frost effect if there is snow accumulation
    if (snowAccumulation > 0.0) {
        Reflect = FrostDist(Reflect);
    }

    // Sample reflection from the cubemap
    vec3 reflection = texture(cubemap, Reflect).rgb;

    // Define water color and mix with reflection
    vec3 waterColor = vec3(0.0);
    vec3 color = mix(reflection, waterColor, 0.0);

    FragColor = vec4(color, 1.0);
}
