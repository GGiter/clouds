#version 460

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec2 v_SkyTexCoord;

uniform sampler3D u_Worley;
uniform sampler3D u_PerlinWorley;
uniform sampler2D u_Weather;
uniform float u_RotationX;
uniform float u_RotationY;
uniform float u_RotationZ;
uniform vec3 u_Translation;
uniform float u_Jitter;
uniform float u_Time;
uniform float u_CloudType;
uniform int u_UseMipMaps;

uniform float u_WeatherScale;
uniform float u_WorleyScale;
uniform float u_PerlinWorleyScale;


/* Configuration properties */
#define MAX_STEPS 256
#define VOLUME_LENGTH 6
#define SHADOW_STEPS 6
#define SHADOW_LENGTH 2

const float sky_start_radius = 2.0; //bottom of cloud layer
const float sky_end_radius = 10.0; //top of cloud layerr
const float earth_radius = 1.0; //top of cloud layerr

vec3 skyColor = vec3(0.0,0.0,0.0);
vec3 rayOrigin = vec3(0.0,0.0,0.0);

/* Helper functions used to rotate objects */
mat3 rotationX( in float angle ) {
	return mat3(	1.0,		0,			0,
			 		0, 	cos(angle),	-sin(angle),
					0, 	sin(angle),	 cos(angle));
}

mat3 rotationY( in float angle ) {
	return mat3(	cos(angle),		0,		sin(angle),
			 				0,		1.0,			 0,
					-sin(angle),	0,		cos(angle));
}

mat3 rotationZ( in float angle ) {
	return mat3(	cos(angle),		-sin(angle),	0,
			 		sin(angle),		cos(angle),		0,
							0,				0,		1);
}

/* Helper function used to remap values from one range to another */
float remap(const float originalValue, const float originalMin, const float originalMax, const float newMin, const float newMax) {
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

/* get how high is current point from 0 to 1 */
float GetHeightFractionForPoint(float inPosition)
{ 
	float height_fraction = (inPosition -  sky_start_radius) / (sky_end_radius - sky_start_radius); 
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

/* function, which tries to generate volumetic data using procedurally generated noise textures */
float map(vec3 p, float LOD) {

    /* rotate current position */
    p = ((p - vec3(0.5)) * rotationY(radians(u_RotationY)) * rotationX(radians(u_RotationX)) * rotationZ(radians(u_RotationZ)) + vec3(0.5));

    /* get weather map anc calculate cloud coverage */
	vec3 weather = texture(u_Weather, p.xz*u_WeatherScale).xyz;
    float cloudCoverage = smoothstep(0.3, 1.2, weather.x);

    /* make clouds move a little each frame */
    p.x += u_Time*0.01;

    // calculate position on the sky
	float heightFraction = GetHeightFractionForPoint(length(p));

    // sample low frequency worley-perlin noise (cloud shape noise  
	vec4 lowFrequencyNoise = textureLod(u_PerlinWorley, p*u_PerlinWorleyScale, LOD);

    // build low frequency worley-perlin noise fbm
	float lowFrequencyFBM = lowFrequencyNoise.g*0.625+lowFrequencyNoise.b*0.25+lowFrequencyNoise.a*0.125; 

    // calculate base cloud shape
	float baseCloud = remap(lowFrequencyNoise.r, -(1.0-lowFrequencyFBM), 1.0, 0.0, 1.0);

    // check if cloud should be in the sky based on cloud coverage and cloud type gradients
    baseCloud *= densityHeightGradient(heightFraction, u_CloudType);;	
	baseCloud = remap(baseCloud, 1.0-cloudCoverage, 1.0, 0.0, 1.0); 
	baseCloud *= cloudCoverage;
            
	// sample high-frequency worley noise (detail noise)
	vec3 highFrequencyNoises = textureLod(u_Worley, p * u_WorleyScale, LOD - 2.0).rgb;

	// build high frequency worley noise fbm
	float highFrequencyFBM = highFrequencyNoises.r * 0.625 + highFrequencyNoises.g * 0.25 + highFrequencyNoises.b * 0.125;

	// get the height fraction for use with blending noise types over height (NOT USED HERE, since no curl noise is being used in this demo!)

	// transition from wispy shapes to billowy shapes over height
    float highFrequencyHeightTransitionMultiplier = 10;

	float highFrequencyNoiseModifier = mix(highFrequencyFBM, 1.0 - highFrequencyFBM, clamp(heightFraction * highFrequencyHeightTransitionMultiplier, 0.0, 1.0));

    float highFrequencyNoiseErodeMuliplier = 0.190;

	// erode the base cloud shape with the distorted high-frequency noise (for fluffy or detail-rich clouds). Think of carving a detailed cloud out of a block of clay
	baseCloud = remap(baseCloud, highFrequencyNoiseModifier * highFrequencyNoiseErodeMuliplier, 1.0, 0.0, 1.0);
        
	return clamp(baseCloud, 0.0, 1.0);
}

/* lighting functions */
float beer(float d){
	return exp(-d);
}

float powder(float d){
	return (1. - exp(-2.*d));
}

float HG( float sundotrd, float g) {
	float gg = g * g;
	return (1. - gg) / pow( 1. + gg - 2. * g * sundotrd, 1.5);
}

/* raymarching function used to calculate lighting for each step from main raymarching function */
void lightRayMarch(in vec3 ray, in vec3 lightVector, in float opacitySample, inout vec3 lightEnergy, inout float transmittance)
{
    /* configure variables used to calculate lighting */
    float lightDotEye = dot(normalize(lightVector), normalize(ray));
    vec3 sunColor = vec3(1.0, 1.0, .95);
    vec3 ambientLight = (vec3(65., 70., 80.)*(1.5/255.));
    float density = 0.8;
    float shadowStepLength = SHADOW_LENGTH/float(SHADOW_STEPS);
    float densityMultiplier = -SHADOW_LENGTH*density;
    float light_density = 1.0;
    vec3 lightRay = ray;

    /* raymarch to light source and calculate light density */
    for(int s = 0; s < SHADOW_STEPS; ++s)
    {
        lightRay += lightVector;
        float lightSample = map(lightRay, u_UseMipMaps > 0 ? float(s) : 0);
        if(lightSample > 0.0 )
        {
            float Ti = exp(lightSample*densityMultiplier);
            light_density *= Ti;
        }
    }

    /* use calculated density to calculate lighting */
    float dTransmittance = exp(opacitySample*densityMultiplier);

    /* beer and powder */
    float beerPowderTerm = powder(opacitySample) * beer(opacitySample);

    /* calculate scattering using henyey-greenstein function */
    float scattering = mix(HG(lightDotEye, -0.08), HG(lightDotEye, 0.08), clamp(lightDotEye*0.5 + 0.5, 0.0, 1.0));
    scattering = max(scattering, 1.0); //clamp scattering

    /* calculate light color using beer law, powder and henyey-greenstein function */
    vec3 lightColor = (mix(mix(ambientLight*1.8, skyColor, 0.2), scattering*sunColor, beerPowderTerm*light_density)) * opacitySample;

    /* calculate color intensity */
    lightEnergy += transmittance * lightColor; 
    transmittance *= dTransmittance;
}

void main()
{
    /* configure camera position */
    vec2 position = v_TexCoord * 2 - vec2(0.5,0.5);
    vec3 rayOrigin =  vec3(position, 0);
  
    rayOrigin += u_Translation;
    rayOrigin.y += earth_radius;

    /* configure ray/camera direction */
    vec3 rayDirection =  normalize(vec3(0, 0.5, 1));

    float stepLength = VOLUME_LENGTH/float(MAX_STEPS);
    float shadowStepLength = SHADOW_LENGTH/float(SHADOW_STEPS);

    /* set light sources and their vectors from camera origin */
    vec3 lightPos = vec3(1, 2, 10) + vec3(u_Translation.x, u_Translation.y + earth_radius, 0.0);
    vec3 lightPos2 = vec3(-1, 2, 10) + vec3(u_Translation.x, u_Translation.y + earth_radius, 0.0);
    vec3 lightVector = normalize(rayOrigin - lightPos2);
    vec3 lightVector2 = normalize(rayOrigin - lightPos2);
    
    /* configure variables used in raymarching */
    vec3 lightEnergy = vec3(0, 0, 0);
    float transmittance = 1;
    float opacity = 0;
    vec3 ray = rayOrigin;

	ray += rayDirection * stepLength;

    for(int i = 0; i < MAX_STEPS; ++i)
    {
        /* small optimization no point in increasing opacity after it reaches 1.0 */
        if(opacity >= 1)
            break;

        /* sample opacity from texture data */
        float opacitySample = map(ray, .0);
        if(opacitySample > 0.001)
        {       
            // calculate lighting for each light source (in these case there are two sources */
            lightRayMarch(ray, lightVector, opacitySample, lightEnergy, transmittance);
            lightRayMarch(ray, lightVector2, opacitySample, lightEnergy, transmittance);
            opacity += opacitySample;        
        }

        ray += rayDirection * stepLength;
    }

    /* calculate final color */
    vec4 cloudOutput = vec4(vec3(lightEnergy), transmittance);
    color = vec4((cloudOutput.rgb + mix(vec3(0), skyColor, cloudOutput.a)), 1);
};