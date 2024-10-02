#version 460

#include "nightSky.frag"
#include "daySky.frag"

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

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
uniform int u_UseRain;
uniform int u_UseSnow;
uniform int u_UseRainbow;
uniform int u_UseSunrays;
uniform int u_NumberOfBolts;
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

/* RAINBOW */
#define RAINBOW_START_Y					0.0
const float RAINBOW_BRIGHTNESS  		= 1.0;
const float RAINBOW_INTENSITY   		= 0.30;
const vec3  RAINBOW_COLOR_RANGE 		= vec3(50.0, 53.0, 56.0);  // The angle for red, green and blue
vec3 RAINBOW_POS						= vec3(10004.5, -600.0, 2.5);
vec3 RAINBOW_DIR 						= vec3(-0.2, -0.1, 0.0);
const vec3 boxMin = vec3(-10000,-sky_t_radius,-1000000);              // Minimum corner of the bounding box
const vec3 boxMax = vec3(10000,sky_t_radius,1000000);              // Maximum corner of the bounding box

// Local variables
vec3 rainbow_pos;
vec3 rainbow_camera_dir;
vec3 rainbow_up;
vec3 rainbow_vertical;
vec3 rainbow_w;

// Smoothstep function
vec3 smoothstepColor(vec3 edge0, vec3 edge1, vec3 x) {
    return smoothstep(edge0, edge1, x);
}

// Rainbow color calculation
vec3 rainbowColor(in vec3 ray_dir) {
    vec3 normalized_dir = normalize(RAINBOW_DIR);
    float theta = degrees(acos(dot(normalized_dir, ray_dir)));
    vec3 nd = clamp(1.0 - abs((RAINBOW_COLOR_RANGE - theta) * 0.2), 0.0, 1.0);
    vec3 color = smoothstepColor(vec3(0.0), vec3(1.0), nd) * RAINBOW_INTENSITY;
    
    return color * max((RAINBOW_BRIGHTNESS - 0.75) * 1.5, 0.0);
}

// Setup rainbow parameters
void rainbowSetup() {
    rainbow_pos = RAINBOW_POS;
    rainbow_w = -normalize(-rainbow_pos);
    rainbow_up = normalize(cross(rainbow_w, vec3(0,1,0)));
    rainbow_vertical = normalize(cross(rainbow_up, rainbow_w));
}

// Function to check if a position is inside a bounding box
bool isInsideBox(vec3 position, vec3 minCorner, vec3 maxCorner) {
    return all(greaterThanEqual(position, minCorner)) && all(lessThanEqual(position, maxCorner));
}

// Main function to compute rainbow color
vec3 rainbow(vec2 fragCoord, vec3 worldPos, vec3 cameraPos) {
    // Initialize the color
    vec3 color = vec3(0.0);

    // Compute the ray direction from the camera to the fragment's world position
    vec3 ray_world = normalize(worldPos - cameraPos);  // Ray from the camera to the world position

    // Setup rainbow parameters (fixed in world space)
    rainbowSetup();

    // Calculate the direction for the rainbow, based on fixed rainbow orientation in world space
    vec3 wdDir = normalize(ray_world.x * rainbow_up + ray_world.y * rainbow_vertical - ray_world.z * rainbow_w);

    // Check if the fragment's world position is within the bounding box and above the rainbow's starting Y
  //  if (isInsideBox(worldPos, boxMin, boxMax) && worldPos.y >= RAINBOW_START_Y) {
        // Calculate the rainbow color based on the ray direction
        color += rainbowColor(wdDir);
  //  }

    // Return the final color, clamped to [0, 1]
    return clamp(color, 0.0, 1.0);
}

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


/* lighting functions */
float beer(float d){
	return exp(-d);
}

float powder(float d){
	return (1. - exp(-2.0*d));
}

float random_rain(in vec2 uv)
{
    return fract(sin(dot(uv.xy, 
                         vec2(12.9898, 78.233))) * 
                 43758.5453123);
}

float noise_rain(in vec2 uv)
{
    vec2 i = floor(uv);
    vec2 f = fract(uv);
    f = f * f * (3. - 2. * f);
    
    float lb = random_rain(i + vec2(0., 0.));
    float rb = random_rain(i + vec2(1., 0.));
    float lt = random_rain(i + vec2(0., 1.));
    float rt = random_rain(i + vec2(1., 1.));
    
    return mix(mix(lb, rb, f.x), 
               mix(lt, rt, f.x), f.y);
}


float rain(vec2 uv)
{
    float travelTime = (u_Time * 0.2) + 0.1;
	
    vec2 tiling = vec2(1., .01);
    vec2 offset = vec2(travelTime * 0.5 + uv.x * 0.2, travelTime * 0.2);
	
    vec2 st = uv * tiling + offset;
    
    float rain = 0.1;  
    float f = noise_rain(st * 200.5) * noise_rain(st * 125.5);  
   	f = clamp(pow(abs(f), 15.0) * 1.5 * (rain * rain * 125.0), 0.0, 0.25);
    return f;
}

float snow(vec2 uv)
{
    float snow = 0.0;
    float gradient = (1.0-float(uv.y / u_resolution.x))*0.3;
    float random = fract(sin(dot(uv.xy,vec2(12.9898,78.233)))* 43758.5453);
    for(int k=0;k<6;k++){
        for(int i=0;i<16;i++){
            float cellSize = 1.0 + (float(i)*1.5);
			float downSpeed = 1.1+(sin(u_Time*0.4+float(k+i*20))+1.0)*0.00018;
            vec2 uv = (uv.xy / u_resolution.x)+vec2(0.01*sin((u_Time+float(k*6185))*0.6+float(i))*(5.0/float(i)),downSpeed*(u_Time+float(k*1352))*(1.0/float(i)));
            vec2 uvStep = (ceil((uv)*cellSize-vec2(0.5,0.5))/cellSize);
            float x = fract(sin(dot(uvStep.xy,vec2(12.9898+float(k)*12.0,78.233+float(k)*315.156)))* 43758.5453+float(k)*12.0)-0.5;
            float y = fract(sin(dot(uvStep.xy,vec2(62.2364+float(k)*23.0,94.674+float(k)*95.0)))* 62159.8432+float(k)*12.0)-0.5;

            float randomMagnitude1 = sin(u_Time*2.5)*0.7/cellSize;
            float randomMagnitude2 = cos(u_Time*2.5)*0.7/cellSize;

            float d = 5.0*distance((uvStep.xy + vec2(x*sin(y),y)*randomMagnitude1 + vec2(y,x)*randomMagnitude2),uv.xy);

            float omiVal = fract(sin(dot(uvStep.xy,vec2(32.4691,94.615)))* 31572.1684);
            if(omiVal<0.08?true:false){
                float newd = (x+1.0)*0.4*clamp(1.9-d*(15.0+(x*6.3))*(cellSize/1.4),0.0,1.0);
                snow += newd;
            }
        }
    }
    return snow;
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

const vec3 boltColour = vec3(0.3, 0.6, 1.0);

float getGlow(float dist, float radius, float intensity){
    dist = max(dist, 1e-6);
    return pow(radius/dist, intensity);	
}

vec3 hash31(float p){
   vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
   p3 += dot(p3, p3.yzx+33.33);
   return fract((p3.xxy+p3.yzz)*p3.zyx); 
}

#define HASHSCALE 0.1031
float hash(float p){
    vec3 p3  = fract(vec3(p) * HASHSCALE);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

const int MAX_NUMBER_OF_BOLTS = 16;
vec2 bolts[MAX_NUMBER_OF_BOLTS];
vec3 boltGlow = vec3(0);

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

    /*Bolt ligthing in cloud*/
    const float internalFrequency = 0.5;
    const float internalSpeed = 5.0;
    vec3 source = vec3(0, sky_b_radius + sky_t_radius * 0.5, 0) + 
                		 (2.0*hash31(floor(u_Time*internalSpeed))-1.0) * g_radius * 0.25;
    float minProx = length(ray - source) * 0.0005;
    //prox = 10.f;
    // Vary size for flicker
    float size = sin(45.0*fract(u_Time))+5.0;
    vec3 internal = getGlow(minProx, size, 3.2) * boltColour;

     // Use the bolts array to calculate proximity
    float prox[MAX_NUMBER_OF_BOLTS];
    for (int i = 0; i < u_NumberOfBolts; ++i) {
        prox[i] = length(ray - vec3(bolts[i].x, ray.y, bolts[i].y)) * 0.0000000005;
    }

    // Determine the closest bolt
    for (int i = 0; i < u_NumberOfBolts; ++i) {
        minProx = min(minProx, prox[i]);
    }


    size = 3.0;
    float h = 0.9*sky_b_radius;

    for (int i = 0; i < u_NumberOfBolts; ++i) {
        float currentProx = length(ray - vec3(bolts[i].x, ray.y, bolts[i].y)) * 0.00005;
        internal += getGlow(currentProx, size, 3.2) * boltColour;
    }

    if(minProx > 7.07106) 
          internal = vec3(0);

    if(hash(floor(u_Time)) > internalFrequency){
          internal = vec3(0);
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

    //TO-DO: probably can be somehow merged into existing code
    /* Additional fog calculations */
    if(u_UseSunrays > 0)
    {
        float den = opacitySample;
        float fogFactor = shadowStepLength * length(raystep);  // Example value, you should replace it with actual logic if needed
   
        fogFactor *= ((lightTransmittance) * alpha + (1.0 - alpha));
        const float minStep = 20;
        float weight = (1.0-(clamp(lightOpacity, 0, 1)*0.90));
        fogFactor *= clamp(cameraPosition.y*4.0,0.0,1.0) *  (4.0/(length(ray * 0.1)+4.0)) *  weight/float(minStep);

        /* Add contribution to rays */
        rays += (1.0 - den) * fogFactor;  // Increment rays based on fogFactor and density
    }
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

/*LIGHTING BOLTS*/

float rand(float x)
{
    return fract(sin(x)*75154.32912);
}

float noise(float x)
{
    float i = floor(x);
    float a = rand(i), b = rand(i+1.);
    float f = x - i;
    return mix(a,b,f);
}

float perlin(float x)
{
    float r=0.,s=1.,w=1.;
    for (int i=0; i<6; i++) {
        s *= 2.0;
        w *= 0.5;
        r += w * noise(s*x);
    }
    return r;
}

float f(float y)
{
    float w = 0.4; // width of strike
    return w * (perlin(2. * y) - 0.5);
}

float plot(vec2 p, float d, bool thicker)
{
    if (thicker) d += 5. * abs(f(p.y + 0.001) - f(p.y));
    return smoothstep(d, 0., abs(f(p.y) - p.x));
}

vec3 lightingBolt(vec2 uv)
{
    float x = u_Time + 0.1;

    float m = 0.25; // max duration of strike
    float i = floor(x / m);
    float f = x / m - i;
    float k = 0.4; // frequency of strikes
    float n = perlin(i); // use Perlin noise for smoother transitions
    float t = ceil(n - k); // occurrence
    float d = max(0.0, n - k) / (1.0 - k); // duration
    float o = ceil(t - f - (1.0 - d)); // occurrence with duration
    float gt = 0.1; // glare duration
    float go = ceil(t - f - (1.0 - gt)); // glare occurrence

    float lightning = 0.0;
    float light = 0.0;
    float glare = 0.0;

    if (o == 1.0) {
        vec2 uv2 = uv;
        uv2.y += i * 2.0; // select type of lightning
        float p = (perlin(i + 10.0) - 0.5) * 2.0; // position of lightning
        uv2.x -= p;

        float strike = plot(uv2, 0.01, true); // thinner strike with noise
        float glow = plot(uv2, 0.04, false); // primary glow
        float glow2 = plot(uv2, 1.5, false); // secondary glow

        lightning = strike * 0.4 + glow * 0.15;
        
        // Add more detail to the lightning
        float h = perlin(i + 5.0); // height
        float noiseDetail = perlin(1.2 * uv.x + 4.0 * h) * 0.03;
        lightning *= smoothstep(h, h + 0.05, uv.y + noiseDetail);
        lightning += glow2 * 0.3;

        light = smoothstep(5.0, 0.0, abs(uv.x - p));
        glare = go * light;
    }

    return vec3(lightning + glare);
}

const float strikeFrequency = 0.7;
// The speed and duration of the bolts. The speed and frequency are linked
float speed = 2.5;

float fade(float t) { return t*t*t*(t*(6.*t-15.)+10.); }

float grad(float hash, float p){
    int i = int(1e4*hash);
    return (i & 1) == 0 ? p : -p;
}

float perlinNoise1D(float p){
    float pi = floor(p), pf = p - pi, w = fade(pf);
    return mix(grad(hash(pi), pf), grad(hash(pi + 1.0), pf - 1.0), w) * 2.0;
}

float fbm(float pos, int octaves){
    if(pos < 0.0){
        return 0.0;
    }
    float total = 0.0;
    float frequency = 0.2;
    float amplitude = 1.0;
    for(int i = 0; i < octaves; i++){
        if(i > 2){
            pos += 0.5*u_Time;
        }
        total += perlinNoise1D(pos * frequency) * amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return total;
}

float sdCappedCylinder( vec3 p, float h, float r ){
    vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(h,r);
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float getSDF(vec3 p) {
    float dist = FLT_MAX;

    // Shift everything to start at the cloud
    p.y -= sky_b_radius - 3490;

    // The counter of a bolt in a series
    float t = 0.0;
    // The offset of the series
    float shift = 0.0;

    // Number of noise levels for FBM
    int octaves = 4;
    // Scale of the y coordinate as noise input. Controls the smoothness of the bolt
    float scale = 0.5;
    // Offset to give simultaneous bolts different shapes
    float shapeOffset = 15.2;
    // Fraction of the total bolt length 0->1
    float progress;

    // The fraction of the lifetime of the bolt it takes for it to descend.
    // The bolt persists in full form for 1.0-descentDuration fraction of the total period.
    float descentDuration = 0.5;

    // Spatial range of the bolt
   // float range = g_radius * 0.3;
    float range = 200.0;
    float boltLength = (sky_b_radius - g_radius) * 0.1;
    // Bolt thickness
    //float radius = 0.01;
    float radius = 0.01;
    // xz: the shape of the bolt
    // y:  progress used as bolt length and positioning
    vec3 offset;
    vec2 location;

    float time;

    for(int i = 0; i < u_NumberOfBolts; i++){

        shapeOffset *= 2.0;
        shift = fract(shift + 0.25);
        time = u_Time * speed + shift;
        t = floor(time)+1.0;
        
        bolts[i] = vec2(FLT_MAX);

        // Bolts strike randomly
        if(hash(float(i)+t*0.026) > strikeFrequency){
            continue;
        }
        location = 2.0*vec2(hash(t+float(i)+0.43), hash(t+float(i)+0.3))-1.0;
        location *= range;
        progress = clamp(fract(time)/descentDuration, 0.0, 1.0);
        
        // Briefly increase the radius of the bolt the moment it makes contact
        if(progress > 0.95 && fract(time) - descentDuration < 0.1){
            radius = 0.1;
        }else{
            radius = 0.01;
       }
        radius *= 0.02;
        progress *= boltLength;
        offset = vec3(location.x+fbm(shapeOffset+t*0.2+(scale*p.y), octaves), 
                      progress, 
                      location.y+fbm(shapeOffset+t*0.12-(scale*p.y), octaves));

        // Store the xz location of the iteration bolt
        // Ensure the bolt is within a cloud
       // if (map(p + offset, 0) >= 0.0) {
       // if (getCloudCoverage(p + offset * 10, 0) > 0.1) 
        {
           
            // Proceed only if the current position is within the cloud
            bolts[i] = location.xy;

            // Calculate the minimum distance to the bolt using the cylinder SDF
            dist = min(dist, sdCappedCylinder(p + offset, radius, progress));
        }
    }

    return dist;
}

const int MAX_STEPS = 32;
const float EPSILON = 1e-4;

float distanceToScene(vec3 cameraPos, vec3 rayDir, float start, float end, out vec3 glow) {

    float depth = start;
    float dist;
    
    for (int i = 0; i < MAX_STEPS; i++) {

        vec3 p = cameraPos + depth * rayDir;
        if(p.y > 200000.0) //TO-DO: how to map lighting bolts to start at clouds, map returns if a current position is in cloud
        {
          // glow = vec3(0);
      //     return end;
        }

        // Warping the cylinder breaks the shape. Reduce step size to avoid this.
        dist = 1.0 * getSDF(p);
        // Accumulate the glow along the view ray.
        glow += getGlow(dist, 0.01, 0.8) * boltColour;

        if (dist < EPSILON){
            return depth;
        }

        depth += dist;

        if (cameraPos.y + depth >= end){ 
            return end; 
        }
    }

    return end;
}

float LinearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

const float A = 0.22;
const float B = 0.50;
const float C = 0.10;
const float D = 0.20;
const float E = 0.02;
const float F = 0.30;
const float W = 11.2;

const float TONEMAP_EXPOSURE			= 1.0;
const float TONEMAP_EXPOSURE_BIAS 		= 2.0;

vec3 Uncharted2Tonemap(vec3 x)
{
   return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 Tonemap(vec3 color)
{
    vec3 tonemapedColor = Uncharted2Tonemap(TONEMAP_EXPOSURE_BIAS * color);
    vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(W));
    color = tonemapedColor * whiteScale;
   
    return color;
}

void SetupInitialBoltsValues()
{
    for(int i = 0; i < u_NumberOfBolts; ++i)
    {
        bolts[i] = vec2(1e10);
    }
}

void main()
{    
    SetupInitialBoltsValues();
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
        if(opacitySample > 0.001 || u_UseSunrays > 0)
        {       
            // Calculate lighting for sun
            lightRayMarch(ray, vSunDirection, opacitySample, lightEnergy, transmittance, raystep, alpha, T, steps, cameraPosition, rays);                  
            opacity += opacitySample;        
        }

        ray += raystep;
    }
    vec3 glow = vec3(0);
    float dist = distanceToScene(cameraPosition, rayDirection, 0.1,sky_b_radius,glow);
    if(getCloudCoverage(start, 0) <= 0.1)
     glow *= 0.5; //HACK

    /* calculate final color */
    vec4 cloudOutput = vec4(vec3(lightEnergy), alpha);
    cloudOutput.xyz = U2Tone(cloudOutput.xyz)*cwhiteScale;
	cloudOutput.xyz = sqrt(cloudOutput.xyz);

    cloudOutput.rgb += glow; //HACK

    // mix with background (sky, sun and starts)
    vec3 background = vec3(0.0);
    if(cloudOutput.a < 0.99)
    { 
        background = preetham(rayDirection, cameraPosition);
        background *= mix(1.0, 0.5, u_thunderstormTimePassed);
        background = max(background, moon(rayDirection, background, u_Time));
        background = max(background, stars(rayDirection));

        if(u_thunderstorm > 0)
        {
            background += glow; //Thunder
        }
    }
    color = vec4(background*(1.0-cloudOutput.a)+cloudOutput.rgb*cloudOutput.a, 1.0);

    if(u_UseRain > 0 && cloudOutput.a > 0.1)
    {
        float rain = rain(uv);
        color += vec4(1.0) * rain;
    }

    if(u_UseSnow > 0 && cloudOutput.a > 0.1)
    {
        float snow = snow(gl_FragCoord.xy*u_downscale);

        float depth = texture(u_depthTexture, v_TexCoord).r;
        float worldDepth = LinearizeDepth(depth, 0.1, 1000.0);
        float occlusion = clamp(worldDepth / 1000.0, 0.0, 1.0);

        color += vec4(snow * occlusion);
    }

    if(u_UseRainbow > 0)
    {
       color += vec4(rainbow(gl_FragCoord.xy*u_downscale, getWorldPosition(gl_FragCoord.xy*u_downscale, gl_FragCoord.z*u_downscale), start), 0.0);
    }

    //raystep
    if(u_UseSunrays > 0)
    {
       color.rgb += (1.0-color.rgb) * clamp(rays * rays * vSunColor * (0.5 + rayDirection.y*0.5), 0.0, 1.0);
    // color.rgb = vec3(rays);
    }

    //color.rgb = Tonemap(color.rgb);
};