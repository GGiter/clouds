#version 460
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D RippleTexture;
uniform float Time;
uniform vec2 SourceSize;
uniform float u_rainIntensity;

#define PI  3.14159265358979323846

// Function to generate a random offset based on time
vec2 RandomOffset(float seed) {
    return vec2(fract(sin(seed) * 43758.5453123), fract(cos(seed) * 15731.5798123));
}

vec3 ComputeRipple(vec2 UV, float CurrentTime, float Weight, out float RippleIntensity) {
    vec4 Ripple = texture2D(RippleTexture, UV);
    Ripple.yz = Ripple.yz * 2 - 1; // Decompress perturbation

    float DropFrac = fract(Ripple.w + CurrentTime); // Apply time shift
    float TimeFrac = DropFrac - 1.0f + Ripple.x;
    float DropFactor = clamp(0.2f + Weight * 0.8f - DropFrac, 0, 1);
    float FinalFactor = DropFactor * Ripple.x * 
                        sin(clamp(TimeFrac * 9.0f, 0.0f, 3.0f) * PI);

    RippleIntensity = FinalFactor * 0.35f; // Store ripple intensity for alpha output

    return vec3(Ripple.yz * FinalFactor * 0.35f, 1.0f);
}

void main() {      
    // Periodically shift the UVs every 60 seconds
    float period = 60.0;
    float modTime = mod(Time, period);
    
    // Generate a random offset using modTime as the seed
    vec2 randomOffset = RandomOffset(modTime);
    vec2 uv = vec2(gl_FragCoord.xy / (SourceSize));
    
    // This are just visually tweak value
    vec4 TimeMul = vec4(1.0f, 0.85f, 0.93f, 1.13f); 
    vec4 TimeAdd = vec4(0.0f, 0.2f, 0.45f, 0.7f);
    vec4 Times = (Time * TimeMul + TimeAdd) * 1.0;
    Times = fract(Times);

    float RainIntensity = 0.5 * u_rainIntensity;
    vec4 Weights = RainIntensity - vec4(0, 0.25, 0.5, 0.75);
    Weights = clamp(Weights * 4, 0, 1);

    float RippleIntensity1, RippleIntensity2, RippleIntensity3, RippleIntensity4;

    vec3 Ripple1 = ComputeRipple(uv + vec2( 0.25,0.0), Times.x, Weights.x, RippleIntensity1);
    vec3 Ripple2 = ComputeRipple(uv + vec2(-0.55,0.3), Times.y, Weights.y, RippleIntensity2);
    vec3 Ripple3 = ComputeRipple(uv + vec2(0.6, 0.85), Times.z, Weights.z, RippleIntensity3);
    vec3 Ripple4 = ComputeRipple(uv + vec2(0.5,-0.75), Times.w, Weights.w, RippleIntensity4);

    vec4 Z = mix(vec4(1), vec4(Ripple1.z, Ripple2.z, Ripple3.z, Ripple4.z), Weights);
    vec3 Normal = vec3(Weights.x * Ripple1.xy +
                       Weights.y * Ripple2.xy + 
                       Weights.z * Ripple3.xy + 
                       Weights.w * Ripple4.xy, 
                       Z.x * Z.y * Z.z * Z.w);

    float CombinedRippleIntensity = max(max(RippleIntensity1, RippleIntensity2), 
                                        max(RippleIntensity3, RippleIntensity4));

    FragColor = vec4(normalize(Normal) * 0.5 + 0.5, CombinedRippleIntensity);
}
