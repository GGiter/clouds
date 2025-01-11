
#define FLT_MAX 3.402823466e+38

uniform int u_NumberOfBolts;
const vec3 boltColor = vec3(0.3, 0.6, 1.0);
const int MAX_BOLTS = 16;
vec2 boltPositions[MAX_BOLTS];


float getGlow(float dist, float radius, float intensity) {
    return pow(radius / max(dist, 1e-6), intensity);
}

float hash(float p) {
    vec3 p3 = fract(vec3(p) * 0.1031);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float perlinNoise1D(float pos) {
    float pi = floor(pos);
    float pf = pos - pi;
    float fade = pf * pf * pf * (pf * (pf * 6.0 - 15.0) + 10.0);
    float grad1 = (int(1e4 * hash(pi)) & 1) == 0 ? pf : -pf;
    float grad2 = (int(1e4 * hash(pi + 1.0)) & 1) == 0 ? pf - 1.0 : -(pf - 1.0);
    return mix(grad1, grad2, fade) * 2.0;
}

float fbm(float pos, int octaves) {
    float total = 0.0;
    float frequency = 0.2;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; i++) {
        total += perlinNoise1D(pos * frequency) * amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return total;
}

float sdCappedCylinder(vec3 p, float h, float r) {
    vec2 d = abs(vec2(length(p.xz), p.y)) - vec2(h, r);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float getBoltDistance(vec3 p, vec3 cameraPosition, float sky_b_radius, float g_radius, float u_Time) {
    float dist = FLT_MAX;
    const float strikeFrequency = 0.7;
    const float minDistanceToCamera = 0.01;
    const float speed = 2.5;
    const float descentDuration = 0.5;
    const float range = 200.0;
    const float boltLength = (sky_b_radius - g_radius) * 0.1;
    const int octaves = 4;
    const float scale = 0.5;
    float shapeOffset = 15.2;

    // Adjust position to cloud layer
    p.y -= sky_b_radius - 3490.0;

    // Initialization
    vec3 offset;
    vec2 location;
    float radius = 0.01;
    float t, shift = 0.0, time, progress;

    for (int i = 0; i < u_NumberOfBolts; i++) {
        shapeOffset *= 2.0;
        shift = fract(shift + 0.25);
        time = u_Time * speed + shift;
        t = floor(time) + 1.0;

        boltPositions[i] = vec2(FLT_MAX);

        // Skip if bolt does not strike
        if (hash(float(i) + t * 0.026) > strikeFrequency) {
            continue;
        }

        // Compute bolt location
        location = 2.0 * (vec2(hash(t + float(i) + 0.43), hash(t + float(i) + 0.3)) - 0.5) * range;
        progress = clamp(fract(time) / descentDuration, 0.0, 1.0);

        // Adjust radius based on bolt contact
        radius = (progress > 0.95 && fract(time) - descentDuration < 0.1) ? 0.1 : 0.01;
        radius *= 0.02;
        progress *= boltLength;

        // Calculate bolt offset
        offset = vec3(
            location.x + fbm(shapeOffset + t * 0.2 + scale * p.y, octaves),
            progress,
            location.y + fbm(shapeOffset + t * 0.12 - scale * p.y, octaves)
        );

        // Check distance to camera
        float distanceToCamera = abs(p.z + offset.z - cameraPosition.z);
        if (distanceToCamera >= minDistanceToCamera) {
            boltPositions[i] = location;

            // Calculate minimum distance using cylinder SDF
            dist = min(dist, sdCappedCylinder(p + offset, radius, progress));
        }
    }

    return dist;
}

float raymarchScene(vec3 cameraPos, vec3 rayDir, float start, float end, out vec3 accumulatedGlow, float sky_b_radius, float g_radius, float u_Time) {
    float depth = start;
    float dist;
    accumulatedGlow = vec3(0.0);

    for (int i = 0; i < 32; i++) {
        vec3 p = cameraPos + depth * rayDir;
        dist = getBoltDistance(p, cameraPos, sky_b_radius, g_radius, u_Time);
        accumulatedGlow += getGlow(dist, 0.01, 0.8) * boltColor;

        if (dist < 1e-6) return depth;
        depth += dist;
        if (cameraPos.y + depth >= end) return end;
    }
    return end;
}

void initializeBoltPositions() {
    for (int i = 0; i < u_NumberOfBolts; i++) {
        boltPositions[i] = vec2(1e10);
    }
}
