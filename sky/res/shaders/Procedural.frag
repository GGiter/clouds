#version 460

#include "nightSky.frag"
#include "daySky.frag"
#include "rainbow.frag"
#include "thundersUtil.frag"

#define FLT_MAX 3.402823466e+38

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec2 v_SkyTexCoord;

in vec3 vAmbient;
in vec3 vSunColor;

uniform sampler3D u_Worley;
uniform sampler3D u_PerlinWorley;
uniform sampler2D u_Weather;
uniform vec3 u_Translation;
uniform float u_Jitter;
uniform float u_Time;
uniform float u_CloudType;
uniform int u_UseMipMaps;
uniform int u_UseRainbow;
uniform sampler2D u_depthTexture;

uniform mat4 u_MVPM; 
uniform int u_check;
uniform float u_thunderstorm;
uniform float u_thunderstormTimePassed;
uniform float u_aspect;
uniform vec2 u_resolution;
uniform float u_downscale;

uniform float u_WeatherScale;
uniform float u_WorleyScale;
uniform float u_PerlinWorleyScale;

uniform vec2 u_Moonpos = vec2(0.3, .8); //hard-coded

/* Configuration properties */
#define SHADOW_STEPS 6
#define SHADOW_LENGTH 80

const float g_radius = 200000.0 - 2500; //ground radius
const float sky_b_radius = 201000.0;//bottom of cloud layer
const float sky_t_radius = 202300.0;//top of cloud layer
const float c_radius = 6008400.0; //2d noise layer
const float cwhiteScale = 1.1575370919881305;//precomputed 1/U2Tone(40)

//offset lighting calculations a little
const vec3 RANDOM_VECTORS[6] = vec3[6]
(
	vec3( 0.38051305f,  0.92453449f, -0.02111345f),
	vec3(-0.50625799f, -0.03590792f, -0.86163418f),
	vec3(-0.32509218f, -0.94557439f,  0.01428793f),
	vec3( 0.09026238f, -0.27376545f,  0.95755165f),
	vec3( 0.28128598f,  0.42443639f, -0.86065785f),
	vec3(-0.16852403f,  0.14748697f,  0.97460106f)
	);

vec3 getWorldPosition(vec2 fragCoord, float depth) {
    // Normalize fragment coordinates (NDC space)
    vec4 clipSpacePos;
    clipSpacePos.xy = (fragCoord / u_resolution) * 2.0 - 1.0;  // Convert to NDC (-1 to 1 range)
    clipSpacePos.z = depth * 2.0 - 1.0;                        // Depth in clip space (NDC)
    clipSpacePos.w = 1.0;

    // Transform from clip space back to world space using inverse MVP matrix
    vec4 worldPos = inverse(u_MVPM) * clipSpacePos;
    
    // Perspective divide (homogeneous coordinates)
    worldPos /= worldPos.w;

    // Return world position
    return worldPos.xyz;
}

/* lighting functions */
float beer(float d){
	return exp(-d);
}

float powder(float d){
	return (1. - exp(-2.0*d));
}

/* Helper function used to remap values from one range to another */
float remap(const float originalValue, const float originalMin, const float originalMax, const float newMin, const float newMax) {
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

/* get how high is current point from 0 to 1 */
float GetHeightFractionForPoint(float inPosition)
{ 
	float height_fraction = (inPosition -  sky_b_radius) / (sky_t_radius  - sky_b_radius); 
	return clamp(height_fraction, 0.0, 1.0);
}

/* get gradient for current cloudType */
vec4 mixGradients(const float cloudType){
	const vec4 STRATUS_GRADIENT = vec4(0.0, 0.1, 0.2, 0.3);
	const vec4 STRATOCUMULUS_GRADIENT = vec4(0.02, 0.2, 0.48, 0.625);
	const vec4 CUMULUS_GRADIENT = vec4(0.0, 0.1625, 0.88, 0.98);
	float stratus = 1.0f - clamp(cloudType * 2.0f, 0.0, 1.0);
	float stratocumulus = 1.0f - abs(cloudType - 0.5f) * 2.0f;
	float cumulus = clamp(cloudType - 0.5f, 0.0, 1.0) * 2.0f;
	return STRATUS_GRADIENT * stratus + STRATOCUMULUS_GRADIENT * stratocumulus + CUMULUS_GRADIENT * cumulus;
}

/* calculate gradient for current cloudType and height fraction (0 to 1) */
float densityHeightGradient(const float heightFrac, const float cloudType) {
	vec4 cloudGradient = mixGradients(cloudType);
	return smoothstep(cloudGradient.x, cloudGradient.y, heightFrac) - smoothstep(cloudGradient.z, cloudGradient.w, heightFrac);
}

float getCloudCoverage(vec3 p, float stepSize)
{
    p.x += u_Time * 20;
    vec2 p_xz = p.xz * u_WeatherScale;

    // Define directions for sampling: forward, backward, right, left
    vec2 directions[4] = vec2[](
        vec2(1.0, 0.0),  // forward
        vec2(-1.0, 0.0), // backward
        vec2(0.0, 1.0),  // right
        vec2(0.0, -1.0)  // left
    );

    float minCoverage = FLT_MAX;
    const int numDirections = 4;

    for (int i = 0; i < numDirections; ++i)
    {
        vec2 direction = directions[i];
        vec2 samplePos = p_xz + direction * stepSize;
        vec3 weather = texture(u_Weather, samplePos).xyz;
        float cloudCoverage = smoothstep(0.3, 1.3, weather.x);
        minCoverage = min(cloudCoverage, minCoverage);
    }

    return minCoverage;
}


/* function, which tries to generate volumetic data using procedurally generated noise textures */
float map(vec3 p, float LOD) {
    /* get weather map anc calculate cloud coverage */
	vec3 weather = texture(u_Weather, p.xz*u_WeatherScale).xyz;
    float cloudCoverage = smoothstep(0.3, 1.3, weather.x);

    /* make clouds move a little each frame */
    p.x += u_Time*20;

    // calculate position on the sky
	float heightFraction = GetHeightFractionForPoint(length(p));

    // sample low frequency worley-perlin noise (cloud shape noise  
	vec4 lowFrequencyNoise = textureLod(u_PerlinWorley, p*u_PerlinWorleyScale, LOD);

    // build low frequency worley-perlin noise fbm
	float lowFrequencyFBM = lowFrequencyNoise.g*0.625+lowFrequencyNoise.b*0.25+lowFrequencyNoise.a*0.125; 

    // calculate base cloud shape
	float baseCloud = remap(lowFrequencyNoise.r, -(1.0-lowFrequencyFBM), 1.0, 0.0, 1.0);

    // check if cloud should be in the sky based on cloud coverage and cloud type gradients
	baseCloud = remap(baseCloud * densityHeightGradient(heightFraction, u_CloudType), 1.0-cloudCoverage, 1.0, 0.0, 1.0); 
	baseCloud *= cloudCoverage;
     
	// sample high-frequency worley noise (detail noise)
	vec3 highFrequencyNoises = textureLod(u_Worley, p * u_WorleyScale, LOD - 2.0).rgb;

	// build high frequency worley noise fbm
	float highFrequencyFBM = highFrequencyNoises.r * 0.625 + highFrequencyNoises.g * 0.25 + highFrequencyNoises.b * 0.125;

	// transition from wispy shapes to billowy shapes over height
    float highFrequencyHeightTransitionMultiplier = 10;

	float highFrequencyNoiseModifier = mix(highFrequencyFBM, 1.0 - highFrequencyFBM, clamp(heightFraction * highFrequencyHeightTransitionMultiplier, 0.0, 1.0));

    float highFrequencyNoiseErodeMuliplier = 0.190;

	// erode the base cloud shape with the distorted high-frequency noise (for fluffy or detail-rich clouds). Think of carving a detailed cloud out of a block of clay
	baseCloud = remap(baseCloud, highFrequencyNoiseModifier * highFrequencyNoiseErodeMuliplier, 1.0, 0.0, 1.0);

	return clamp(baseCloud, 0.0, 1.0);
}

float HenyeyGreenstein(float g, float costh){
    return (1.0/(4.0 * 3.1415))  * ((1.0 - g * g) / pow(1.0 + g*g - 2.0*g*costh, 1.5));
}

vec3 hash31(float p){
   vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
   p3 += dot(p3, p3.yzx+33.33);
   return fract((p3.xxy+p3.yzz)*p3.zyx); 
}

/* raymarching function used to calculate lighting for each step from main raymarching function */
void lightRayMarch(in vec3 ray, in vec3 lightVector, in float opacitySample, inout vec3 lightEnergy, inout float transmittance, in vec3 raystep, inout float alpha, inout float T, in float steps, in vec3 cameraPosition, inout float rays)
{
    /* configure variables used to calculate lighting */
    float density = 0.1;
    float light_density = 1.0;
    vec3 lightRay = ray;

    float raystepLength = length(raystep);
    float alphaSample = exp(-0.5*opacitySample*raystepLength);
    lightVector = lightVector * raystepLength;

    const float t_dist = sky_t_radius-sky_b_radius;
	float shadowStepLength = t_dist/SHADOW_LENGTH;

    float dt = exp(-0.5*opacitySample*raystepLength);
    T *= dt;
    
    float lightOpacity = 0.0;
    float lightTransmittance = 0.0;

    float mu = 0.5+0.5*dot(normalize(lightVector), normalize(raystep));;
    /* raymarch to light source and calculate light density */
    for(int s = 0; s < SHADOW_STEPS; ++s)
    {
        lightRay += (lightVector+(RANDOM_VECTORS[s]*float(s+1))*shadowStepLength);
        float lightSample = map(lightRay, u_UseMipMaps > 0 ? float(s) : 0);
        lightOpacity += lightSample; //light_density
        lightTransmittance += mix(1.0, 0.75, mu) * (lightSample * (1.0-(lightOpacity*(1.0/(shadowStepLength*18.0)))));
    }
    lightTransmittance = pow(lightTransmittance, float(0.5));
    /* use calculated density to calculate lighting */

    /* beer and powder */
    float beerPowderTerm = powder(lightTransmittance*shadowStepLength) * max(beer(0.5*lightTransmittance*shadowStepLength), beer(0.5*0.2*lightTransmittance*shadowStepLength)*0.75);
    beerPowderTerm = mix(beerPowderTerm * 2.0 * (1.0 - exp(-0.5*lightTransmittance*shadowStepLength*2.0)), beerPowderTerm, mu);

    /* calculate scattering using henyey-greenstein function */
    float lightDotEye = dot(normalize(lightVector), normalize(raystep));
    float scattering = mix(HenyeyGreenstein(-0.3, mu), HenyeyGreenstein(0.3, mu), 0.7);

    scattering *= mix(5.0, 1.0, u_thunderstormTimePassed);

    /* Modified lightning bolt lighting calculation */
    vec3 internal = vec3(0);
    if(u_thunderstorm > 0)
    {
        const float internalFrequency = 0.5;
        const float internalSpeed = 5.0;
        vec3 source = vec3(0, sky_b_radius + sky_t_radius * 0.5, 0) + 
                         (2.0 * hash31(floor(u_Time * internalSpeed)) - 1.0) * g_radius * 0.25;
        float minProx = length(ray - source) * 0.0005;

        // Vary size for flicker
        float size = sin(45.0 * fract(u_Time)) + 5.0;
        internal = getGlow(minProx, size, 3.2) * boltColor;

        // Calculate proximity to each bolt and determine the closest one
        float prox[MAX_BOLTS];
        for (int i = 0; i < u_NumberOfBolts; ++i) {
            prox[i] = length(ray - vec3(boltPositions[i].x, ray.y, boltPositions[i].y)) * 0.0000000005;
        }
        for (int i = 0; i < u_NumberOfBolts; ++i) {
            minProx = min(minProx, prox[i]);
        }

        // Blend based on proximity and distance falloff
        float falloffDistance = 5.17; // Controls blending range
        float blendFactor = smoothstep(0.0, falloffDistance, minProx);
        blendFactor = pow(blendFactor, 0.5); // Make blending more gradual

        size = 3.0;
        for (int i = 0; i < u_NumberOfBolts; ++i) {
            float currentProx = length(ray - vec3(boltPositions[i].x, ray.y, boltPositions[i].y)) * 0.00005;
            float boltBlendFactor = smoothstep(0.0, falloffDistance, currentProx);
            boltBlendFactor = pow(boltBlendFactor, 0.5);
            internal += getGlow(currentProx, size, 3.2) * boltColor * boltBlendFactor;
        }

        // Fade out internal light if the proximity is above the blending threshold
        if (minProx > falloffDistance) 
            internal *= (1.0 - smoothstep(falloffDistance, falloffDistance + 1.95, minProx));

        internal *= 100;
        // Add an additional smooth transition to reduce the flickering effect
        if (hash(floor(u_Time)) > internalFrequency) {
            internal *= 0.0;
        }
    }

    // We add bolt colour to ambient

    vec3 ambientLight = internal * 100 + 5.0*vAmbient*mix(0.15, 1.0, GetHeightFractionForPoint(length(ray))); // calculate ambient lighting
	vec3 sunColor = pow(vSunColor, vec3(0.75)); // calculate current sun color (in cloud)
    /* calculate light color using beer law, powder and henyey-greenstein function also */
    //Z factor
    lightEnergy += ((ambientLight+vSunColor*beerPowderTerm * scattering)*(opacitySample)*T*raystepLength);	

    /* calculate alpha */
    /* calculate distance from camera to current ray position */
    float distanceToCamera = length(ray - cameraPosition);

    /* remap the distance to affect alpha (adjust max distance as needed) */
    float ZFactor = clamp(remap(distanceToCamera, 0.0, 35000.0, 1.0, 0.0), 0.0, 1.0);
    float alphaToAdd = ((1.0 - alphaSample) * (1.0 - alpha)) * ZFactor;
    alpha = clamp(alpha + alphaToAdd, 0.0, 1.0);
}

float intersectSphere(const vec3 pos, const vec3 dir, const float r) {
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, pos);
    float c = dot(pos, pos) - (r * r);
		float d = sqrt((b*b) - 4.0*a*c);
		float p = -b - d;
		float p2 = -b + d;
    return max(p, p2)/(2.0*a);
}

void main()
{    
    initializeBoltPositions();
    /* configure variables used in raymarching */
    vec3 lightEnergy = vec3(0, 0, 0);
    float transmittance = 1;
    float alpha = 0; //used as alpha to determine how clouds should blend with skyy
    float opacity = 0; // only as optimization
    
    /* calculate ray direction color (perspective camera) */
    vec2 uv = (gl_FragCoord.xy*u_downscale)/(u_resolution);
	uv = uv-vec2(0.5);
	uv *= 2.0;
	uv.x *= u_aspect;
	vec4 uvdir = (vec4(uv.xy, 1.0, 1.0));
	vec4 worldPos = (inverse((u_MVPM))*uvdir);

    vec3 rayDirection = normalize(worldPos.xyz/worldPos.w);
    vec3 cameraPosition = vec3(0.0, g_radius, 0.0);

    //quit if we are below the sky (maybe some ground in the future?)
    if(rayDirection.y <= 0.0)
    {
        vec3 background = preetham(rayDirection, cameraPosition);
        background = max(background, stars(rayDirection));
        color = vec4(mix(vec3(0.0,0.15,0.0), background, 0.5), 1.0);
        return;
    }

    // calculate raystart, step length, number of steps
    vec3 start = cameraPosition+rayDirection*intersectSphere(cameraPosition, rayDirection, sky_b_radius);
	vec3 end = cameraPosition+rayDirection*intersectSphere(cameraPosition, rayDirection, sky_t_radius);
    const float t_dist = sky_t_radius-sky_b_radius;
	float distlength = (length(end-start));
    float dmod = smoothstep(0.0, 1.0, (distlength/t_dist)/14.0);
    float steps = (mix(96.0 * 2.0, 54.0 * 2.0, dot(rayDirection, vec3(0.0, 1.0, 0.0))));
    float step_dist = mix(t_dist, t_dist*4.0, dmod)/(steps);
    vec3 raystep = rayDirection*step_dist;
    vec3 ray = start;
    float T = 1.0;
    float rays = 0.0;
    for(int i = 0; i < steps; ++i)
    {
        /* small optimization no point in increasing opacity after it reaches 1.0 */
        if(opacity >= 1)
            break;

        /* sample opacity from texture data */
        float opacitySample = map(ray, .0);
        if(opacitySample > 0.001)
        {       
            // Calculate lighting for sun
            lightRayMarch(ray, vSunDirection, opacitySample, lightEnergy, transmittance, raystep, alpha, T, steps, cameraPosition, rays);                  
            opacity += opacitySample;        
        }

        ray += raystep;
    }
    vec3 glow = vec3(0);
    float dist = raymarchScene(cameraPosition, rayDirection, 0.1, sky_b_radius, glow, sky_b_radius, g_radius, u_Time);
    if(getCloudCoverage(start, 0) <= 0.1)
     glow *= 0.5;

    /* calculate final color */
    vec4 cloudOutput = vec4(vec3(lightEnergy), alpha);
    cloudOutput.xyz = U2Tone(cloudOutput.xyz)*cwhiteScale;
	cloudOutput.xyz = sqrt(cloudOutput.xyz);

    // mix with background (sky, sun and starts)
    vec3 background = vec3(0.0);
    if(cloudOutput.a < 0.99)
    { 
        background = preetham(rayDirection, cameraPosition);
        background *= mix(1.0, 0.5, u_thunderstormTimePassed);
        background = max(background, moon(rayDirection, background, u_Time));
        background = max(background, stars(rayDirection));
    }
    color = vec4(background*(1.0-cloudOutput.a)+cloudOutput.rgb*cloudOutput.a, 1.0);

    if(u_UseRainbow > 0)
    {
        vec4 rainbowColor = rainbow(gl_FragCoord.xy * u_downscale, 
                                getWorldPosition(gl_FragCoord.xy * u_downscale, 
                                gl_FragCoord.z * u_downscale), start);

        color.rgb = mix(color.rgb, rainbowColor.rgb, rainbowColor.a * 0.5);   
    }
};